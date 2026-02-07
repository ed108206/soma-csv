#ifndef HEADER_D0F3FA5B5A2EA51E
#define HEADER_D0F3FA5B5A2EA51E

#include "BaseView.h"
class CFrameProc;

class CLView : public BaseView {
public:
    CLView();
    ~CLView() override;

    CFrameProc* cfr{nullptr};

    HFONT m_font{nullptr};
    HWND m_hwnd{nullptr};
    HWND m_parent{nullptr};
    int m_mela{0};

    void Initialize() override;
    HWND hwnd() const override { return m_hwnd; }
    void setParent(HWND parent) override { m_parent = parent; }
    void setFont(HFONT font) override { m_font = font; }

    LRESULT CallMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    HWND CreateListView(HINSTANCE hInstance, HWND hwndParent);
    void ResizeListView(HWND hwndParent);
    BOOL InitListView(HWND hwndListView);
    BOOL InsertListViewItems(HWND hwndListView);
    LRESULT ListViewNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void ShowText(HWND hwnd, const char* b);
};
#endif // header guard

