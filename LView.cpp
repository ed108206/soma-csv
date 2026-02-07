#include "base.h"
#include "LView.h"
#include "FrameProc.h"
#include "resource.h"
#include <string>
#include <array>

HINSTANCE clv_hInst{nullptr};

INT_PTR CALLBACK KaDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

CLView::CLView()
{
    m_mela = 0;
}

CLView::~CLView() = default;

void CLView::Initialize()
{
    clv_hInst = GetModuleHandle(nullptr);
    if (clv_hInst) {
        CreateDialogParam(clv_hInst, MAKEINTRESOURCE(IDD_DIALOG2), m_parent, KaDialogProc, reinterpret_cast<LPARAM>(this));
    } else {
        MessageBox(nullptr, "ERROR create CLView dlg.", SNOM, MB_OK);
    }
}

INT_PTR CALLBACK KaDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CLView* view = nullptr;

    if (uMsg == WM_INITDIALOG) {
        view = reinterpret_cast<CLView*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(view));
    } else {
        view = reinterpret_cast<CLView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    return view ? view->CallMessage(hWnd, uMsg, wParam, lParam) : FALSE;
}

LRESULT CLView::CallMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CLView* pclv = reinterpret_cast<CLView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg) {
    case WM_INITDIALOG: {
        pclv = reinterpret_cast<CLView*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pclv);

        m_hwnd = hWnd;

        HWND hT = GetDlgItem(hWnd, IDC_STATIC2);
        ShowWindow(hT, SW_HIDE);

        HWND hwndListView = GetDlgItem(hWnd, IDC_LIST1);
        SendMessage(hwndListView, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), MAKELPARAM(TRUE, 0));

        HWND hL = GetDlgItem(hWnd, IDC_STATIC1);
        SendMessage(hL, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), MAKELPARAM(TRUE, 0));

        InitListView(hwndListView);

        DWORD style = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
        ListView_SetExtendedListViewStyle(hwndListView, style);

        ResizeListView(hWnd);

        HICON icon = LoadIcon(clv_hInst, MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));

        if (m_parent) {
            LONG_PTR dw = GetWindowLongPtr(hWnd, GWL_STYLE);
            dw &= ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_POPUP);
            dw |= WS_CHILD;
            SetWindowLongPtr(hWnd, GWL_STYLE, dw);
            SetParent(hWnd, m_parent);
        }

        ShowText(hWnd, "hello");
        break;
    }

    case WM_VSCROLL: {
        int y = LOWORD(wParam);
        int nPos = HIWORD(wParam);
        std::string bef = std::to_string(y) + " " + std::to_string(nPos);
        ShowText(hWnd, bef.c_str());
        break;
    }

    case WM_NOTIFY:
        return ListViewNotify(hWnd, wParam, lParam);

    case WM_SIZE: {
        if (!m_mela && m_parent) {
            RECT rc;
            GetClientRect(m_parent, &rc);
            constexpr int ru = 26;
            MoveWindow(hWnd, rc.left, rc.top + ru, rc.right - rc.left, rc.bottom - ru, TRUE);
        }
        ResizeListView(hWnd);
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HWND hL = GetDlgItem(hWnd, IDC_STATIC1);
        if (hL == reinterpret_cast<HWND>(lParam)) {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(WHITE);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, WHITE);

        auto drawFrame = [&](HWND child) {
            RECT rc;
            GetWindowRect(child, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            POINT at{ rc.left, rc.top };
            ScreenToClient(hWnd, &at);

            rc.left = at.x - 1;
            rc.top = at.y - 1;
            rc.right = rc.left + w + 2;
            rc.bottom = rc.top + h + 2;
            FrameRect(hdc, &rc, DBORD);
        };

        drawFrame(GetDlgItem(hWnd, IDC_LIST1));
        drawFrame(GetDlgItem(hWnd, IDC_STATIC1));

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        if (pclv->cfr) {
            pclv->cfr->DeleteCurTab(hWnd);
        }
        break;

    case WM_COMMAND:
    case WM_DESTROY:
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

void CLView::ShowText(HWND hwnd, const char* text)
{
    HWND hL = GetDlgItem(hwnd, IDC_STATIC1);
    SetWindowText(hL, text);
}

void CLView::ResizeListView(HWND hwndParent)
{
    RECT rc;
    GetClientRect(hwndParent, &rc);

    HWND hL = GetDlgItem(hwndParent, IDC_LIST1);
    constexpr int ex = 40;
    MoveWindow(hL, rc.left + 1, rc.top + ex, rc.right - (rc.left + 2), rc.bottom - (rc.top + ex + 1), TRUE);

    hL = GetDlgItem(hwndParent, IDC_STATIC1);
    MoveWindow(hL, rc.left + 1, rc.top + 1, rc.right - (rc.left + 2), (rc.top + ex - 4), TRUE);
}

BOOL CLView::InitListView(HWND hwndListView)
{
    ListView_DeleteAllItems(hwndListView);

    LV_COLUMN lvColumn{};
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = 120;

    for (int i = 0; i < 10; ++i) {
        std::string colName = "Col: " + std::to_string(i);
        lvColumn.pszText = colName.data();
        ListView_InsertColumn(hwndListView, i, &lvColumn);
    }

    InsertListViewItems(hwndListView);
    return TRUE;
}

BOOL CLView::InsertListViewItems(HWND hwndListView)
{
    ListView_DeleteAllItems(hwndListView);
    ListView_SetItemCount(hwndListView, 10);
    return TRUE;
}

LRESULT CLView::ListViewNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    auto* lpnmh = reinterpret_cast<LPNMHDR>(lParam);

    switch (lpnmh->code) {

    case LVN_GETDISPINFO: {
            auto* lpdi = reinterpret_cast<LV_DISPINFO*>(lParam);
            TCHAR szString[MAX_PATH];

            if (lpdi->item.iSubItem) {
                if (lpdi->item.mask & LVIF_TEXT) {
                    wsprintf(szString, TEXT("Column %d"), lpdi->item.iSubItem);
                    lstrcpy(lpdi->item.pszText, szString);
                }
            } else {
                if (lpdi->item.mask & LVIF_TEXT) {
                    wsprintf(szString, TEXT("Item %d"), lpdi->item.iItem + 1);
                    lstrcpy(lpdi->item.pszText, szString);
                }
                if (lpdi->item.mask & LVIF_IMAGE) {
                    lpdi->item.iImage = 0;
                }
            }
            return 0;
        }
    }
    return 0;
}
