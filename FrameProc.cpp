#include "base.h"
#include "FrameProc.h"
#include "resource.h"
#include <algorithm>

extern HINSTANCE hInst;

CFrameProc::CFrameProc()
    : m_fhwnd(nullptr),
      left_width(LEFT_WINDOW_WIDTH),
      pane_height(PANE_WINDOW_HEIGHT),
      m_differ(0),
      xSizing(0),
      ySizing(0),
      m_sol(0),
      hTree(nullptr),
      hTab(nullptr),
      bottomhwnd(nullptr),
      hIcon(nullptr),
      m_hi(nullptr),
      m_ixx(0),
      m_fdrag(0)
{
    csp.cfr = this;
    cef.cfr = this;
    ctt.cfr = this;
}

int CFrameProc::Callu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE: {
        m_fhwnd = hWnd;

        RECT rect{};
        GetClientRect(hWnd, &rect);

        // Pane left
        hTree = CreateWindowEx(0, WC_TREEVIEW, LEFT_WINDOW_CLASS,
                               WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS |
                               TVS_LINESATROOT | TVS_NOTOOLTIPS | TVS_NONEVENHEIGHT,
                               0, 0, 100, 100, hWnd,
                               reinterpret_cast<HMENU>(ID_TREEVIEW), hInst, nullptr);

        if (hTree) {
            NewFont(hTree);
            ctt.StartTreeView(hInst, hTree);
            ShowWindow(hTree, SW_SHOW);
            UpdateWindow(hTree);
        }

        // Pane right -> Tab control
        hTab = CreateWindowEx(0, WC_TABCONTROL, RIGHT_WINDOW_CLASS,
                              WS_CHILD | WS_VISIBLE | TCS_TABS | TCS_FLATBUTTONS,
                              0, 0, 100, 100, hWnd,
                              reinterpret_cast<HMENU>(ID_TABCTRL), hInst, nullptr);

        if (hTab) {
            NewFont(hTab);
            SendMessage(hTab, TCM_SETPADDING, 0, MAKELPARAM(20, 2));
            ShowWindow(hTab, SW_SHOW);
            UpdateWindow(hTab);
        }

        // Bottom window
        bottomhwnd = CreateWindowEx(SS_BLACKRECT, "STATIC", BOTTOM_WINDOW_CLASS,
                                    WS_CHILD | WS_VISIBLE,
                                    0, 0, 100, 100, hWnd,
                                    nullptr, hInst, nullptr);

        if (bottomhwnd) {
            ShowWindow(bottomhwnd, SW_SHOW);
            UpdateWindow(bottomhwnd);
        }

        // Menu + icon
        SetMenu(hWnd, LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)));

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        if (hIcon) {
            SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
        }
        break;
    }

    case WM_NOTIFY:
        cef.Notifier(hWnd, wParam, lParam);
        return 0;

    case WM_SIZE:
        m_differ = cef.ReSizePane(hWnd, &m_tabvec);
        break;

    case WM_SIZE_HTAB:
        InvalidateList();
        break;

    case WM_PAINT:
        cef.Painter(hWnd);
        break;

    case WM_LBUTTONDOWN:
        m_hi = GetCursor();
        csp.ButtonDown(hWnd, lParam);
        break;

    case WM_LBUTTONUP:
        csp.ButtonUp(hWnd);
        if (m_fdrag) {
            ctt.TvnLButtonUp(hTree);
            m_fdrag = 0;
        }
        break;

    case WM_MOUSEMOVE:
        if (m_fdrag) {
            ctt.TvnMouseMoveDrag(hTree, lParam);
        }
        if (!csp.MouseMove(hWnd, wParam, lParam)) {
            return 0;
        }
        break;

    case WM_DESTROY:
        ProxyMoCursor(4);
        KillObjects();
        DestroyWindow(hWnd);
        ProxyMoCursor(0);
        PostQuitMessage(0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_CLOSE_TAB:
            DeleteCurTabFromMenu();
            break;
        case ID_SALIR:
            PostMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        case ID_L1:
            CreateR(hWnd, 1, "CList");
            break;
        case ID_E1:
            CreateR(hWnd, 2, "CEView");
            break;
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CFrameProc::NewFont(HWND hw)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

	//HFONT g_hfont = CreateFontIndirect(&(ncm.lfMenuFont));
	HFONT g_hfont = CreateFontIndirect(&(ncm.lfMessageFont));

	SendMessage(hw, WM_SETFONT, (WPARAM) g_hfont, MAKELPARAM(1, 0));
}

void CFrameProc::ReParent(HWND hw, const std::string& name, int op)
{
    LockWindowUpdate(hTab);

    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });

    if (it != m_tabvec.end()) {
        if (op) {
            // detach window
            it->stat = TabState::Detached;

            RECT rc{};
            GetClientRect(it->hwnd, &rc);
            rc.bottom += GetSystemMetrics(SM_CYCAPTION) + 12;

            SetParent(it->hwnd, nullptr);

            auto dw = GetWindowLongPtr(it->hwnd, GWL_STYLE);
            dw &= ~WS_CHILD;
            dw |= WS_OVERLAPPEDWINDOW;
            dw &= ~WS_MINIMIZEBOX;
            SetWindowLongPtr(it->hwnd, GWL_STYLE, dw);

            int cx = GetSystemMetrics(SM_CXSCREEN);
            int cy = GetSystemMetrics(SM_CYSCREEN);

            int x1 = (cx - rc.right) / 2;
            int y1 = (cy - rc.bottom) / 2;

            m_sol++;
            MoveWindow(it->hwnd, x1, y1, rc.right, rc.bottom, FALSE);

            SendMessage(hTab, TCM_SETCURSEL, static_cast<WPARAM>(std::distance(m_tabvec.begin(), it)), 0);

            NMHDR m_nmhdr{ m_fhwnd, ID_TABCTRL, TCN_SELCHANGE };
            SendMessage(hTab, WM_NOTIFY, ID_TABCTRL, reinterpret_cast<LPARAM>(&m_nmhdr));
            SetActiveWindow(it->hwnd);
        } else {
            // attach to parent
            it->stat = TabState::Attached;

            ShowWindow(it->hwnd, SW_HIDE);

            auto dw = GetWindowLongPtr(it->hwnd, GWL_STYLE);
            dw &= ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_POPUP);
            dw |= WS_CHILD;
            SetWindowLongPtr(it->hwnd, GWL_STYLE, dw);
            SetParent(it->hwnd, hTab);
            ShowWindow(it->hwnd, SW_SHOWNA);
        }
    }

    LockWindowUpdate(nullptr);
}

void CFrameProc::MiniFreePtrVec(HWND hw)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });

    if (it != m_tabvec.end()) {
        int index = std::distance(m_tabvec.begin(), it);
        m_tabvec.erase(it);
        SendMessage(hTab, TCM_DELETEITEM, static_cast<WPARAM>(index), 0);
    }

    if (m_tabvec.empty()) {
        PostMessage(m_fhwnd, WM_SIZE, 0, 0);
    } else {
        SendMessage(hTab, TCM_SETCURFOCUS, static_cast<WPARAM>(m_tabvec.size() - 1), 0);
    }
}

void CFrameProc::CreateR(HWND hWnd, int u, const std::string& name)
{
    for (auto& dol : m_tabvec) {
        if (dol.stat == TabState::Detached && !dol.open) {
            if (IsWindowVisible(dol.hwnd)) {
                ShowWindow(dol.hwnd, SW_HIDE);
            }
        }
    }

    m_ixx = u;
    DOLOG dol;

    if (m_ixx == 1) {
        auto clv = std::make_unique<CLView>();
        clv->cfr = this;
        clv->setParent(hTab);
        clv->setFont(reinterpret_cast<HFONT>(SendMessage(hTab, WM_GETFONT, 0, 0)));
        clv->Initialize();

        dol.hwnd = clv->hwnd();
        dol.ixx = m_ixx;
        dol.ptr = std::move(clv);
        dol.nam = name;
        dol.stat = TabState::Attached;
        dol.open = false;
        m_tabvec.push_back(std::move(dol));
    }
    else if (m_ixx == 2) {
        auto cev = std::make_unique<CEView>();
        cev->cfr = this;
        cev->setParent(hTab);
        cev->setFont(reinterpret_cast<HFONT>(SendMessage(hTab, WM_GETFONT, 0, 0)));
        cev->m_khwnd = hWnd;
        cev->Initialize();

        dol.hwnd = cev->hwnd();
        dol.ixx = m_ixx;
        dol.ptr = std::move(cev);
        dol.nam = name;
        dol.stat = TabState::Attached;
        dol.open = false;
        m_tabvec.push_back(std::move(dol));
    }

    LockWindowUpdate(hTab);

    TCITEM tie{};
    tie.mask = TCIF_TEXT | TCIF_PARAM;
    tie.pszText = const_cast<char*>(name.c_str());
    tie.lParam = reinterpret_cast<LPARAM>(m_tabvec.back().hwnd);

    int count = static_cast<int>(SendMessage(hTab, TCM_GETITEMCOUNT, 0, 0));
    SendMessage(hTab, TCM_INSERTITEM, static_cast<WPARAM>(count), reinterpret_cast<LPARAM>(&tie));
    SendMessage(hTab, TCM_SETCURSEL, static_cast<WPARAM>(count), 0);

    SetWindowText(hWnd, std::to_string(count).c_str());

    PostMessage(m_tabvec.back().hwnd, WM_SIZE, 0, 0);
    ShowWindow(m_tabvec.back().hwnd, SW_SHOWNA);

    LockWindowUpdate(nullptr);
}

void CFrameProc::ProxyMoCursor(int u)
{
    MoCursor(u);
}

void CFrameProc::MoCursor(int u)
{
    switch (u) {
    case 1:
        SetCursor(LoadCursor(nullptr, IDC_SIZEWE));
        break;
    case 2:
        SetCursor(LoadCursor(nullptr, IDC_SIZENS));
        break;
    case 3:
        SetCursor(LoadCursor(nullptr, MAKEINTRESOURCE(32649))); // small hand
        break;
    case 4:
        SetCursor(LoadCursor(nullptr, IDC_WAIT));
        break;
    default:
        if (m_hi != nullptr) {
            SetCursor(m_hi);
        }
        break;
    }
}

void CFrameProc::KillObjects()
{
    for (auto& dol : m_tabvec) {
        if (dol.hwnd) {
            DestroyWindow(dol.hwnd);
            SendMessage(dol.hwnd, WM_CLOSE, 0, 0);
        }
    }
    m_tabvec.clear();

    if (m_hi) {
        DeleteObject(m_hi);
        m_hi = nullptr;
    }
}

HWND CFrameProc::GetCurrentHwnd(CEView* cvv)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [cvv](const DOLOG& d){ return d.ptr.get() == cvv; });
    return (it != m_tabvec.end()) ? it->hwnd : nullptr;
}

CEView* CFrameProc::GetCurrentPtr(HWND hw)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });
    //return (it != m_tabvec.end()) ? static_cast<CEView*>(it->ptr.get()) : nullptr;
    return (it != m_tabvec.end()) ?  dynamic_cast<CEView*>(it->ptr.get()) : nullptr;
}

TabState CFrameProc::GetCurrentStat(HWND hw)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });
    return (it != m_tabvec.end()) ? it->stat : TabState::Attached;
}

void CFrameProc::SetOpen(HWND hw, bool u)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });
    if (it != m_tabvec.end()) {
        it->open = u;
    }
}

void CFrameProc::DeleteCurTab(HWND hw)
{
    auto it = std::find_if(m_tabvec.begin(), m_tabvec.end(), [hw](const DOLOG& d){ return d.hwnd == hw; });

	unsigned int k = 0;

    if (it != m_tabvec.end()) {
        int index = std::distance(m_tabvec.begin(), it);

		k = index;

        it->ptr.reset();

        m_tabvec.erase(it);

        SendMessage(hTab, TCM_DELETEITEM, static_cast<WPARAM>(index), (LPARAM)0);

        if (m_tabvec.empty()) {
            PostMessage(m_fhwnd, WM_SIZE, 0, 0);
            SetWindowText(m_fhwnd, "");
        } else {
            int newIndex = std::clamp(index, 0, static_cast<int>(m_tabvec.size() - 1));
            SendMessage(hTab, TCM_SETCURFOCUS, static_cast<WPARAM>(newIndex), 0);
        }
    }
}

void CFrameProc::DeleteCurTabFromMenu()
{
    int idx = static_cast<int>(SendMessage(hTab, TCM_GETCURSEL, 0, 0));
    if (idx < 0 || idx >= static_cast<int>(m_tabvec.size())) return;

    switch(m_tabvec[idx].ixx) {
    case 1: {
            auto* pp = dynamic_cast<CLView*>(m_tabvec[idx].ptr.get());
            SendMessage(pp->m_hwnd, WM_CLOSE, 0,0);
        }
        break;

    case 2: {
            auto* pp = dynamic_cast<CEView*>(m_tabvec[idx].ptr.get());
            SendMessage(pp->m_hwnd, WM_CLOSE, 0,0);
        }
        break;
    }
}

void CFrameProc::InvalidateList()
{
    int idx = static_cast<int>(SendMessage(hTab, TCM_GETCURSEL, 0, 0));
    if (idx < 0 || idx >= static_cast<int>(m_tabvec.size())) return;

    if (m_tabvec[idx].ixx == 2) {
        auto* pp = static_cast<CEView*>(m_tabvec[idx].ptr.get());
        HWND hL = GetDlgItem(pp->m_hwnd, IDC_LIST1);
        // ...
    }
}

void CFrameProc::fillHeaderListView()
{
    int idx = static_cast<int>(SendMessage(hTab, TCM_GETCURSEL, 0, 0));
    if (idx < 0 || idx >= static_cast<int>(m_tabvec.size())) return;

    if (m_tabvec[idx].ixx == 2) {
        auto* pp = static_cast<CEView*>(m_tabvec[idx].ptr.get());
        pp->MakeRow0Header();
    }
}

void CFrameProc::TextCaption(const std::string& b)
{
    SetWindowText(m_fhwnd, b.c_str());
}

LRESULT CALLBACK LeftProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK RightProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK BottomProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
