#ifndef HEADER_EFA8546A798D8EC8
#define HEADER_EFA8546A798D8EC8

#include <windows.h>
#include <commctrl.h>

class CFrameProc;

class CExtraTree {
public:
    CExtraTree();
    ~CExtraTree();

    CFrameProc* cfr{nullptr};

    int m_mela{0};
    void StartTreeView(HINSTANCE hInst, HWND hT);
    void TvnBeginDrag(HWND hT, LPARAM lParam, HWND hw);
    int  TvnSelChanged(HWND hT);
    void TvnMouseMoveDrag(HWND hT, LPARAM lParam);
    void TvnLButtonUp(HWND hT);
    void ResizeTreeView(HWND hwndParent);

private:
    bool m_drag{false};

    HIMAGELIST hImageList1{nullptr};
    TVITEM m_tvI{};
    TVHITTESTINFO m_tvHit{};
    HTREEITEM m_hDragItem{nullptr};
    HTREEITEM m_hTarget{nullptr};


    int InitTreeView(HINSTANCE hInst, HWND hT);
    HTREEITEM AddItem(HTREEITEM hParent, const char* szText, HTREEITEM hInsAfter, HWND hT, int data, int bold);

    void BeginDrag(HWND hwndTree, NM_TREEVIEW* lItem, HWND hw);
    void MouseMoveDrag(HWND hT, LPARAM lParam);
    void LButtonUpDrag(HWND hT);
    void DropItem(HTREEITEM hDragItem, HWND hT);
};
#endif // header guard

