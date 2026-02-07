#include "base.h"
#include "EView.h"
#include "resource.h"
#include "FrameProc.h"

#include <stdexcept>
#include <chrono>
#include <thread>
#include <immintrin.h>
#include <iomanip>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static WNDPROC lvproc = nullptr;
static HINSTANCE cev_hInst = nullptr;


CEView::CEView()
    : cfr(nullptr),
      m_font(nullptr),
      m_hwnd(nullptr),
      m_parent(nullptr),
      m_khwnd(nullptr),
      m_lv(nullptr),
      m_mela(0),
      m_ur(0),
      nNumLines(0)
{
}

CEView::~CEView()
{
#if defined(_WIN32)
    if (mf.hFile != nullptr) {
        Unmap_file(mf);
    }
#else
    if (mf.fd != -1) {
        Unmap_file(mf);
    }
#endif
}

void CEView::Initialize()
{
    cev_hInst = GetModuleHandle(nullptr);
    if (cev_hInst) {
        CreateDialogParamA(cev_hInst, MAKEINTRESOURCE(IDD_DIALOG2), m_parent, EaDialogProc, reinterpret_cast<LPARAM>(this));
    } else {
        MessageBox(nullptr, "ERROR: create CEView dlg.", SNOM, MB_OK);
    }
}

void CEView::Unmap_file(MappedFile& f)
{
#if defined(_WIN32)
    if (f.data) {
        UnmapViewOfFile(f.data);
    }
    if (f.hMap) {
        CloseHandle(f.hMap);
        f.hMap = nullptr;
    }
    if (f.hFile) {
        CloseHandle(f.hFile);
        f.hFile = nullptr;
    }
#else
    if (f.data && f.length > 0) {
        munmap((void*)f.data, f.length);
    }
    if (f.fd != -1) {
        close(f.fd);
        f.fd = -1;
    }
#endif
    f.data = nullptr;
    f.length = 0;
    f.hFile = 0;
}

MappedFile CEView::Map_file(const std::string& filename)
{
#if defined(_WIN32)
    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Can't open file");
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        throw std::runtime_error("GetFileSizeEx error");
    }

    HANDLE hMap = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!hMap) {
        CloseHandle(hFile);
        throw std::runtime_error("CreateFileMapping error");
    }

    void* addr = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!addr) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        throw std::runtime_error("MapViewOfFile error");
    }

    return {reinterpret_cast<const char*>(addr), static_cast<size_t>(size.QuadPart), hFile, hMap};
#else
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Can't open file");
    }

    struct stat sb {};
    if (fstat(fd, &sb) == -1) {
        close(fd);
        throw std::runtime_error("fstat error");
    }

    size_t length = sb.st_size;
    void* addr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        throw std::runtime_error("mmap error");
    }

    return {reinterpret_cast<const char*>(addr), length, fd};
#endif
}

void CEView::Parse_segment_simd(const char* data, std::size_t start, std::size_t end, std::vector<std::vector<std::string_view>>& out)
{
    std::vector<std::string_view> fila;
    fila.reserve(16);

    std::size_t field_start = start;

    const __m256i comma   = _mm256_set1_epi8(',');
    const __m256i newline = _mm256_set1_epi8('\n');

    std::size_t i = start;
    for (; i + 32 <= end; i += 32) {
        __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + i));

        __m256i eq_comma   = _mm256_cmpeq_epi8(chunk, comma);
        __m256i eq_newline = _mm256_cmpeq_epi8(chunk, newline);

        unsigned mask_comma   = _mm256_movemask_epi8(eq_comma);
        unsigned mask_newline = _mm256_movemask_epi8(eq_newline);

        unsigned mask = mask_comma | mask_newline;

        while (mask) {
            int pos = __builtin_ctz(mask);
            mask &= mask - 1;
            std::size_t delim = i + pos;

            if (data[delim] == ',') {
                fila.emplace_back(data + field_start, delim - field_start);
                field_start = delim + 1;
            } else { // '\n'
                size_t length = delim - field_start;
                if (length > 0 && data[delim - 1] == '\r') {
                    length -= 1;
                }
                fila.emplace_back(data + field_start, length);
                out.emplace_back(std::move(fila));
                fila.clear();
                fila.reserve(16);
                field_start = delim + 1;
            }
        }
    }

    for (; i < end; ++i) {
        if (data[i] == ',') {
            fila.emplace_back(data + field_start, i - field_start);
            field_start = i + 1;
        } else if (data[i] == '\n') {
            size_t length = i - field_start;
            if (length > 0 && data[i - 1] == '\r') {
                length -= 1;
            }
            fila.emplace_back(data + field_start, length);
            out.emplace_back(std::move(fila));
            fila.clear();
            fila.reserve(16);
            field_start = i + 1;
        }
    }

    if (field_start < end) {
        fila.emplace_back(data + field_start, end - field_start);
    }
    if (!fila.empty()) {
        out.emplace_back(std::move(fila));
    }
}

void CEView::Parse_csv_parallel(const char* data, size_t len, int num_threads)
{
    std::vector<size_t> cuts = {0};
    for (int k = 1; k < num_threads; ++k) {
        size_t pos = (len * k) / num_threads;
        while (pos < len && data[pos] != '\n') {
            ++pos;
        }
        cuts.push_back(pos + 1);
    }
    cuts.push_back(len);

    std::vector<std::vector<std::vector<std::string_view>>> results(num_threads);
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            Parse_segment_simd(data, cuts[t], cuts[t + 1], results[t]);
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    for (int t = 0; t < num_threads; ++t) {
        filas.insert(filas.end(),
                     std::make_move_iterator(results[t].begin()),
                     std::make_move_iterator(results[t].end()));
    }
}

void CEView::FreeAll(HWND hwnd)
{
    HWND hL = GetDlgItem(hwnd, IDC_LIST1);
    if (hL) {
        SendMessage(hL, LVM_SETITEMCOUNT, (WPARAM)0, (LPARAM)LVSICF_NOINVALIDATEALL);
    }
    filas.clear();

    if (lvproc != nullptr && hL) {
        SetWindowLongPtr(hL, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(lvproc));
    }

    if (mf.hFile != nullptr) {
        Unmap_file(mf);
    }
}

CEView* CEView::GetLocalPtr(HWND hw)
{
    return cfr->GetCurrentPtr(hw);
}

void CEView::ShowText(const char* b)
{
    HWND hL = GetDlgItem(m_hwnd, IDC_STATIC1);
    SetWindowText(hL, b);
}

void CEView::ShowText(HWND hwnd, const char* b)
{
    HWND hL = GetDlgItem(hwnd, IDC_STATIC1);
    SetWindowText(hL, b);
}

void CEView::ResizeListView(HWND hwndParent)
{
    RECT rc{};
    GetClientRect(hwndParent, &rc);

    HWND hL = GetDlgItem(hwndParent, IDC_LIST1);
    const int ex = 40;
    MoveWindow(hL, rc.left + 1, rc.top + ex, rc.right - (rc.left + 2), rc.bottom - (rc.top + ex + 1), TRUE);

    hL = GetDlgItem(hwndParent, IDC_STATIC2);
    MoveWindow(hL, rc.left + 1, rc.top + 1, rc.left + 49, (rc.top + ex - 6), TRUE);

    hL = GetDlgItem(hwndParent, IDC_STATIC1);
    MoveWindow(hL, rc.left + 55, rc.top + 1, rc.right - (rc.left + 56), (rc.top + ex - 6), TRUE);
}

void CEView::MakeRow0Header()
{
    if(m_ur) {
        MessageBox(0, "It appears that row 0 is now in the header.", SNOM, MB_OK);
        return;
    }
    unsigned int rcol = filas.at(0).size();

    LV_COLUMN   lvColumn;
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = 120;

    HWND lv = GetDlgItem(m_hwnd, IDC_LIST1);
    for(unsigned int i = 0; i < rcol; i++) {
        ListView_GetColumn(lv, i, &lvColumn);
        lvColumn.pszText = (char*)std::string(filas.at(0).at(i)).c_str();
        ListView_SetColumn(lv, i, &lvColumn);
    }

    filas.erase(filas.begin());

    nNumLines = filas.size();

    if(nNumLines) {
        SendMessage(m_lv, LVM_SETITEMCOUNT, nNumLines, 0);
    }

    m_ur = 1;
}

INT_PTR CALLBACK CEView::EaDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEView* pcev = reinterpret_cast<CEView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
            pcev = reinterpret_cast<CEView*>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pcev);

            pcev->m_hwnd = hWnd;

            HWND hwndListView = GetDlgItem(hWnd, IDC_LIST1);
            SendMessage(hwndListView, WM_SETFONT, (WPARAM)pcev->m_font, MAKELPARAM(1, 0));

            ListView_SetBkColor(hwndListView, GetSysColor(COLOR_WINDOW));
            ListView_SetTextBkColor(hwndListView, GetSysColor(COLOR_WINDOW));
            ListView_SetTextColor(hwndListView, RGB(0,0,0));

            HWND hL = GetDlgItem(hWnd, IDC_STATIC1);
            SendMessage(hL, WM_SETFONT, (WPARAM)pcev->m_font, MAKELPARAM(1, 0));

            hL = GetDlgItem(hWnd, IDC_STATIC2);
            SendMessage(hL, WM_SETFONT, (WPARAM)pcev->m_font, MAKELPARAM(1, 0));

            pcev->m_lv = hwndListView;
            pcev->InitListView(hwndListView);

            DWORD style = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER;
            ListView_SetExtendedListViewStyle(hwndListView, style);

            pcev->ResizeListView(hWnd);

            lvproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwndListView, GWLP_WNDPROC, (LONG_PTR)ListProc));

            HICON ICON = LoadIcon(cev_hInst, MAKEINTRESOURCE(IDI_ICON1));
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)ICON);

            if (pcev->m_parent) {
                LONG_PTR dw = GetWindowLongPtr(hWnd, GWL_STYLE);
                dw &= ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_POPUP);
                dw |= WS_CHILD;
                SetWindowLongPtr(hWnd, GWL_STYLE, dw);
                SetParent(hWnd, pcev->m_parent);
            }

            pcev->PreMain(hWnd);
            ShowWindow(hWnd, SW_SHOW);
            break;
        }

    case WM_NOTIFY:
        if (pcev) {
            pcev->ListViewNotify(pcev->m_hwnd, lParam);
        }
        return 0;

    case WM_SIZE:
        if (pcev && !pcev->m_mela && pcev->m_parent) {
            RECT rc{};
            GetClientRect(pcev->m_parent, &rc);
            int ru = 26;
            MoveWindow(hWnd, rc.left, rc.top + ru,
                       rc.right - rc.left, rc.bottom - ru, TRUE);
        }
        if (pcev) {
            pcev->ResizeListView(hWnd);
        }
        break;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_CTLCOLORDLG:
        return (INT_PTR)WBRUSH;
        break;

        /*
    case WM_CTLCOLORSTATIC: {
            HWND target = reinterpret_cast<HWND>(lParam);
            HDC hdc     = reinterpret_cast<HDC>(wParam);
            const HWND controls[] = {
                GetDlgItem(hWnd, IDC_STATIC1),
                GetDlgItem(hWnd, IDC_STATIC2)
            };

            for (HWND ctrl : controls) {
                if (ctrl == target) {
                    SetTextColor(hdc, RGB(0, 0, 0));
                    SetBkMode(hdc, TRANSPARENT);
                    return reinterpret_cast<INT_PTR>(WHITE);
                }
            }
        }
        break;
        */

    case WM_CTLCOLORSTATIC: {
            HDC hdc     = reinterpret_cast<HDC>(wParam);
            HWND target = reinterpret_cast<HWND>(lParam);

            int ctrlId = GetDlgCtrlID(target);
            SetBkMode(hdc, TRANSPARENT);

            switch (ctrlId) {
                case IDC_STATIC1:
                    SetTextColor(hdc, RGB(0, 0, 0));
                    return reinterpret_cast<INT_PTR>(WHITE);

                case IDC_STATIC2:
                    SetTextColor(hdc, RGB(0, 0, 255));
                    return reinterpret_cast<INT_PTR>(WBRUSH);

                default:
                    SetTextColor(hdc, RGB(0, 0, 0));
                    return reinterpret_cast<INT_PTR>(WHITE);
            }
        }
        break;

    case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, WHITE);

            const HWND controls[] = {
                GetDlgItem(hWnd, IDC_LIST1),
                GetDlgItem(hWnd, IDC_STATIC1),
                GetDlgItem(hWnd, IDC_STATIC2)
            };

            for (HWND ctrl : controls) {
                if (!ctrl) {
                    continue;
                }

                RECT cr;
                GetWindowRect(ctrl, &cr);

                int w = cr.right - cr.left;
                int h = cr.bottom - cr.top;

                POINT at{ cr.left, cr.top };
                ScreenToClient(hWnd, &at);

                cr.left   = at.x - 1;
                cr.top    = at.y - 1;
                cr.right  = cr.left + w + 2;
                cr.bottom = cr.top + h + 2;
                FrameRect(hdc, &cr, DBORD);
            }
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_COMMAND: {
            switch(LOWORD(wParam)) {
            case IDC_STATIC1:
                PostMessage(hWnd, WM_CLOSE, 0,0);
                break;

            case IDC_STATIC2: {
                    HWND hL = GetDlgItem(hWnd, IDC_STATIC2);
                    if(!pcev->m_mela) {
                        pcev->m_mela = 1;
                        SetWindowText(hL, "Attach");
                        pcev->cfr->ReParent(hWnd, "Attach", 1);
                    } else {
                        pcev->m_mela = 0;
                        SetWindowText(hL, "Detach");
                        pcev->cfr->ReParent(hWnd, "Detach", 0);
                    }
                }
                break;

            default:
                return false;
            }
        }
        break;

    case WM_CLOSE:
        {
            if (pcev) {
                pcev->cfr->ProxyMoCursor(4);
                pcev->FreeAll(hWnd);
                DestroyWindow(hWnd);
                if (pcev->cfr) {
                    pcev->cfr->DeleteCurTab(hWnd);
                }
                pcev->cfr->ProxyMoCursor(0);
            }
        }
        break;

    default:
        return false;
    }

    return 0;
}

BOOL CEView::InitListView(HWND hwndListView)
{
    ListView_DeleteAllItems(hwndListView);
    return TRUE;
}

void CEView::PreMain(HWND hwnd)
{
    DoFileOpenEx(hwnd);
}

void CEView::DoFileOpenEx(HWND hwnd)
{
    OPENFILENAME ofn{};
    char szFileName[MAX_PATH] = "";
    char fname[MAX_PATH] = "";

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
     ofn.lpstrFilter = "CSV(*.csv)\0*.csv\0";
    ofn.lpstrFile = szFileName;
    ofn.lpstrFileTitle = fname;
    ofn.nMaxFile = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "csv";

    if (GetOpenFileName(&ofn)) {
        cfr->ProxyMoCursor(4);

        auto t0 = std::chrono::steady_clock::now();
        mf = Map_file(szFileName);
        auto t1 = std::chrono::steady_clock::now();

        Parse_csv_parallel(mf.data, mf.length, 8);
        auto t2 = std::chrono::steady_clock::now();

        std::chrono::duration<double> t_read  = t1 - t0;
        std::chrono::duration<double> t_parse = t2 - t1;
        std::chrono::duration<double> t_total = t2 - t0;

        cfr->ProxyMoCursor(0);

        char gef[256] {};
        sprintf(gef, "%s | parser: %.6f sec | rows: %zu", fname, t_parse.count(), filas.size());
        ShowText(gef);

        unsigned int rcol = filas.at(0).size();
        LV_COLUMN lvColumn{};
        lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvColumn.fmt = LVCFMT_LEFT;
        lvColumn.cx = 120;

        HWND hwndListView = GetDlgItem(hwnd, IDC_LIST1);
        for (unsigned int i = 0; i < rcol; i++) {
            char bef[64];
            sprintf(bef, "Col: %u", i);
            lvColumn.pszText = bef;
            ListView_InsertColumn(hwndListView, i, &lvColumn);
        }

        nNumLines = static_cast<UINT>(filas.size());
        if (nNumLines) {
            SendMessage(m_lv, LVM_SETITEMCOUNT, nNumLines, 0);
        }
    }
}

LRESULT CEView::ListViewNotify(HWND hWnd, LPARAM lParam)
{
    LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);

    switch (lpnmh->code) {
    case LVN_GETDISPINFO: {
            LV_DISPINFO* pDispInfo = reinterpret_cast<LV_DISPINFO*>(lParam);

            if (pDispInfo->item.mask & LVIF_TEXT) {
                const auto& row = filas[pDispInfo->item.iItem];
                const auto& col = row[pDispInfo->item.iSubItem];
                lstrcpyn(pDispInfo->item.pszText,
                         std::string(col).c_str(),
                         pDispInfo->item.cchTextMax);
            }
            return 0;
        }
    default:
        break;
    }
    return 0;
}

LRESULT CALLBACK CEView::ListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_ERASEBKGND) {
        return false;
    }

    /*
    if (msg == WM_HSCROLL) {
        switch (LOWORD(wParam)) {
        case SB_ENDSCROLL: {
                // CEView* pcev = reinterpret_cast<CEView*>(GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA));
                // if (pcev && pcev->cfr) {
                //    PostMessage(pcev->cfr->m_fhwnd, WM_SIZE_HTAB, 0, 0);
                //}
                return TRUE;
            }
        }
    }
    */
    return CallWindowProc(lvproc, hwnd, msg, wParam, lParam);
}

float CEView::GetTicks()
{
    static double freq = -1.0;
    LARGE_INTEGER lint;

    if (freq < 0.0) {
        if (!QueryPerformanceFrequency(&lint)) {
            return -1.0f;
        }
        freq = static_cast<double>(lint.QuadPart);
    }

    if (!QueryPerformanceCounter(&lint)) {
        return -1.0f;
    }
    return static_cast<float>(lint.QuadPart / freq);
}

void CEView::ShowTime(const float& tk)
{
    char buf[256] = {'\0'};
    sprintf(buf, "time: %.3f ms.", tk * 1000.0f);
    printf("%s\n", buf);
}
