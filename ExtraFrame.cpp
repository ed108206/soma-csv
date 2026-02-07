#include "base.h"
#include "resource.h"
#include "ExtraFrame.h"
#include "FrameProc.h"

CExtraFrame::CExtraFrame()
{
    m_hprev = NULL;
}

CExtraFrame::~CExtraFrame()
{

}

int CExtraFrame::ReSizePane(HWND hWnd, std::vector<DOLOG>* vec)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    // pane left
    int lef = rect.left + MARG;
    int top = rect.top + TOP_POS;
    int rig = rect.left + (cfr->left_width - WIDTH_ADJUST) - (MARG);
    int bot = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT + TOP_POS);
    MoveWindow(cfr->hTree, lef+1, top + TOP_TREE + 1, rig-2, bot - (TOP_TREE + 2), FALSE);

    int dx1 = rig;

    // ------------

    // pane right
    lef = rect.left + cfr->left_width + WIDTH_ADJUST;
    top = rect.top + TOP_POS;
    rig = rect.right - (cfr->left_width + WIDTH_ADJUST + MARG);
    bot = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT + TOP_POS);
    MoveWindow(cfr->hTab, lef, top, rig, bot, FALSE);

    int dx2 = lef;
    int differ = dx2- dx1;
    int topper = bot + differ;

    // ------------

    // pane bottom
    lef = rect.left + MARG;
    top = topper;
    rig = rect.right - (lef + MARG);
    bot = rect.bottom - (topper + BOTTOM_POS);
    MoveWindow(cfr->bottomhwnd, lef, top, rig, bot, FALSE);//TRUE);

    // ------------

    for(const auto& dol : *vec) {
        if(dol.stat == TabState::Attached) {
            PostMessage(dol.hwnd, WM_SIZE, 0, 0);
        }
    }

    InvalidateRect(hWnd, &rect, TRUE);
    return differ;
}

void CExtraFrame::Painter(HWND hWnd)
{
    RECT rect;
    int i = 0;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    GetClientRect(hWnd, &rect);

    // ------------

    // pane left
    RECT fc;
    GetWindowRect(cfr->hTree, &fc);

    int w = fc.right - fc.left;
    int h = fc.bottom - fc.top;

    POINT at;
    at.x = fc.left;
    at.y = fc.top;
    ScreenToClient(hWnd, &at);

    fc.left = at.x - 1;
    fc.top = at.y - 1;
    fc.right = fc.left + w + 2;
    fc.bottom = fc.top + h + 2;

    FrameRect(hdc, &fc, DBORD);

    // ------------

    // pane bottom
    HDC ahdc = BeginPaint(cfr->bottomhwnd, &ps);
    RECT gc;
    GetClientRect(cfr->bottomhwnd, &gc);
    FillRect(ahdc, &gc, WHITE);
    FrameRect(ahdc, &gc, DBORD);
    EndPaint(cfr->bottomhwnd, &ps);

    // ------------

    RECT rc;
    int lef;
    int top;
    int rig;
    int bot;

    int c = SendMessage(cfr->hTab, TCM_GETITEMCOUNT, 0, 0);
    if(!c) {
        HDC ehdc = BeginPaint(cfr->hTab, &ps);
        RECT zc;
        GetClientRect(cfr->hTab, &zc);
        FillRect(ehdc, &zc, WHITE);
        FrameRect(ehdc, &zc, DBORD);
        EndPaint(cfr->hTab, &ps);
    } else {
        // nothing yet ...
    }

    // ------------

    // small square splitter left-right
    lef = rect.left + cfr->left_width -1;
    top = (rect.bottom - (rect.top + cfr->pane_height))/2;
    top -= TOP_POS + BOTTOM_POS;
    top += 2;
    rig = lef + 2;
    bot = top + 2;

    SetRect(&rc, lef, top, rig, bot);

    for(i = 0; i < 4; i++) {
        FillRect(hdc, &rc, GRAYH);
        rc.top += 4;
        rc.bottom = rc.top + 2;
    }

    // ------------

    // small square splitter top-bottom
    lef = (rect.right - rect.left)/2;
    lef -= (MARG*2);
    top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);
    top += 2;
    rig = lef + 2;
    bot = top + 2;

    SetRect(&rc, lef, top, rig, bot);

    for(i = 0; i < 4; i++) {
        FillRect(hdc, &rc, GRAYH);
        rc.left += 4;
        rc.right = rc.left + 2;
    }

    // ------------

    EndPaint(hWnd, &ps);
}

void CExtraFrame::Notifier(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (((NMHDR*)lParam)->code) {

    case TVN_BEGINDRAG:
        cfr->m_fdrag = 1;
        cfr->ctt.TvnBeginDrag(cfr->hTree, lParam, hWnd);
        break;

    case TVN_SELCHANGED: {
            if(ID_TREEVIEW == (int) wParam) {
                int r = cfr->ctt.TvnSelChanged(cfr->hTree);

                switch(r) {
                case 0:
                    break;
                case 1:
                    break;
                case 2: // Make row 0 Header
                    cfr->fillHeaderListView();
                    break;
                case 6:
                    break;
                }

                TreeView_SelectItem(cfr->hTree, NULL);
            }
        }
        break;

    ///////////////////////////////////////////////////////

    case TCN_SELCHANGE: {
            int iPage = SendMessage(cfr->hTab, TCM_GETCURSEL, 0, 0);

            const DOLOG& dol = cfr->m_tabvec.at(iPage);
            ShowWindow(dol.hwnd, SW_SHOWNA);

            int c = static_cast<int>(cfr->m_tabvec.size());
            for(int i = 0; i < c; i++) {
                if(i == iPage) {
                    continue;
                }

                const DOLOG& other = cfr->m_tabvec.at(i);

                if(other.stat == TabState::Detached) {
                    ShowWindow(other.hwnd, SW_HIDE);
                } else {
                    if(!other.open) {
                        ShowWindow(other.hwnd, SW_HIDE);
                    }
                }
            }

            char bef[64] = {0};
            sprintf(bef, "%i", iPage);
            SetWindowText(hWnd, bef);
        }
        break;
    }
}

void CExtraFrame::ReBope()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
