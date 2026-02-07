#ifndef HEADER_206A2BDC9CF32A99
#define HEADER_206A2BDC9CF32A99

#include <windows.h>

class CFrameProc;

class CSplitterE {
public:
    CSplitterE();
    ~CSplitterE();

    CFrameProc* cfr{nullptr};

    void ButtonDown(HWND hWnd, LPARAM lParam);
    void ButtonUp(HWND hWnd);
    int  MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);

private:
    bool IsVertical(const RECT& rect, int xPos, int yPos) const;
    bool IsHorizontal(const RECT& rect, int xPos, int yPos) const;

    bool m_dragging{false};
    POINT m_lastPos{0,0};
};
#endif // header guard

