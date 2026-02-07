#include "base.h"
#include "SplitterE.h"
#include "FrameProc.h"
#include "resource.h"

extern HINSTANCE hInst;

CSplitterE::CSplitterE() : cfr(nullptr), m_dragging(false), m_lastPos{0,0}
{

}

CSplitterE::~CSplitterE() = default;

void CSplitterE::ButtonDown(HWND hWnd, LPARAM lParam)
{
    int xPos = LOWORD(lParam);
    int yPos = HIWORD(lParam);

    RECT rect{};
    GetClientRect(hWnd, &rect);

    cfr->xSizing = IsHorizontal(rect, xPos, yPos);
    cfr->ySizing = IsVertical(rect, xPos, yPos);

    if (!cfr->xSizing && !cfr->ySizing) return;

    SetCapture(hWnd);
    HDC hdc = GetDC(hWnd);

    HBRUSH hb = CreatePatternBrush(LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1)));
    SelectObject(hdc, hb);

    if (cfr->xSizing) {
        cfr->ProxyMoCursor(1);
        int lef = cfr->left_width - WIDTH_ADJUST;
        int top = rect.top + TOP_POS;
        int rig = cfr->left_width + WIDTH_ADJUST;
        int bot = rect.bottom - (BOTTOM_POS + SPLITTER_BAR_HEIGHT + cfr->pane_height);
        PatBlt(hdc, lef + 1, top, rig - (lef + 2), bot - top, PATINVERT);
    } else if (cfr->ySizing) {
        cfr->ProxyMoCursor(2);
        int lef = rect.left + MARG;
        int top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);
        int rig = rect.right - MARG;
        int bot = top + cfr->m_differ - BOTTOM_POS;
        PatBlt(hdc, lef, top + 1, rig - lef, bot - (top + 2), PATINVERT);
    }

    DeleteObject(hb);
    ReleaseDC(hWnd, hdc);

    m_dragging = true;
    m_lastPos = {xPos, yPos};
}

void CSplitterE::ButtonUp(HWND hWnd)
{
    if (!cfr->xSizing && !cfr->ySizing) return;

    ReleaseCapture();
    m_dragging = false;

    HDC hdc = GetDC(hWnd);
    RECT rect{};
    GetClientRect(hWnd, &rect);

    RECT focusrect{};
    if (cfr->xSizing) {
        int lef = cfr->left_width - (WIDTH_ADJUST * 2);
        int top = rect.top + TOP_POS;
        int rig = cfr->left_width + WIDTH_ADJUST;
        int bot = rect.bottom - (BOTTOM_POS + SPLITTER_BAR_HEIGHT + cfr->pane_height);
        cfr->xSizing = 0;
        SetRect(&focusrect, lef + 1, top, rig - 1, bot);
    } else if (cfr->ySizing) {
        int lef = rect.left + MARG;
        int top = rect.bottom - (BOTTOM_POS + SPLITTER_BAR_HEIGHT + cfr->pane_height);
        int rig = rect.right - MARG;
        int bot = rect.bottom - (BOTTOM_POS + cfr->pane_height);
        cfr->ySizing = 0;
        SetRect(&focusrect, lef, top + 1, rig, bot - 1);
    }

    InvertRect(hdc, &focusrect);
    ReleaseDC(hWnd, hdc);
    PostMessage(hWnd, WM_SIZE, 0, 0);
}

int CSplitterE::MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    int xPos = LOWORD(lParam);
    int yPos = HIWORD(lParam);

    RECT rect{};
    GetClientRect(hWnd, &rect);

    if (xPos < MINIMUM_SPACE || xPos > (rect.right - MINIMUM_SPACE) ||
        yPos < MINIMUM_SPACE || yPos > (rect.bottom - MINIMUM_SPACE)) {
        cfr->ProxyMoCursor(0);
        return 0;
    }

    if (wParam == MK_LBUTTON && m_dragging) {
        HDC hdc = GetDC(hWnd);
        HBRUSH hb = CreatePatternBrush(LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1)));
        SelectObject(hdc, hb);

        if (cfr->xSizing) {
            int lef = cfr->left_width - WIDTH_ADJUST;
            int top = rect.top + TOP_POS;
            int rig = cfr->left_width + WIDTH_ADJUST;
            int bot = rect.bottom - (BOTTOM_POS + SPLITTER_BAR_HEIGHT + cfr->pane_height);
            PatBlt(hdc, lef + 1, top, rig - (lef + 2), bot - top, PATINVERT);
            cfr->left_width = xPos;

            lef = cfr->left_width - SPLITTER_BAR_WIDTH;
            rig = cfr->left_width + SPLITTER_BAR_WIDTH;
            PatBlt(hdc, lef + 1, top, rig - (lef + 2), bot - top, PATINVERT);
        } else if (cfr->ySizing) {
            int lef = rect.left + MARG;
            int top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);
            int rig = rect.right - MARG;
            int bot = top + cfr->m_differ - BOTTOM_POS;
            PatBlt(hdc, lef, top + 1, rig - lef, bot - (top + 2), PATINVERT);
            cfr->pane_height = rect.bottom - (BOTTOM_POS + yPos);

            top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);
            bot = top + cfr->m_differ - BOTTOM_POS;
            PatBlt(hdc, lef, top + 1, rig - lef, bot - (top + 2), PATINVERT);
        }

        DeleteObject(hb);
        ReleaseDC(hWnd, hdc);
    } else {
        if (IsHorizontal(rect, xPos, yPos)) {
            cfr->ProxyMoCursor(1);
        } else if (IsVertical(rect, xPos, yPos)) {
            cfr->ProxyMoCursor(2);
        } else {
            cfr->ProxyMoCursor(0);
        }
    }

    m_lastPos = {xPos, yPos};
    return 1;
}

bool CSplitterE::IsVertical(const RECT& rect, int xPos, int yPos) const
{
    int top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);
    int bot = rect.bottom - (BOTTOM_POS + cfr->pane_height - SPLITTER_BAR_HEIGHT);

    return (yPos > top && yPos < bot && xPos > MARG && xPos < rect.right - MARG);
}

bool CSplitterE::IsHorizontal(const RECT& rect, int xPos, int yPos) const
{
    int lef = cfr->left_width - SPLITTER_BAR_WIDTH;
    int rig = cfr->left_width + SPLITTER_BAR_WIDTH;
    int top = rect.bottom - (BOTTOM_POS + cfr->pane_height + SPLITTER_BAR_HEIGHT);

    return (xPos > lef && xPos < rig && yPos < top && yPos > TOP_POS);
}
