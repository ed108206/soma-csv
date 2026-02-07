#include "base.h"
#include "ExtraTree.h"
#include "FrameProc.h"
#include "resource.h"


CExtraTree::CExtraTree()
    : cfr(nullptr),
      m_mela(0),
      m_drag(false),
      hImageList1(nullptr),
      m_hDragItem(nullptr),
      m_hTarget(nullptr)
{
    m_tvI = {};
    m_tvHit = {};
}

CExtraTree::~CExtraTree() = default;

void CExtraTree::StartTreeView(HINSTANCE hInst, HWND hT)
{
    InitTreeView(hInst, hT);
}

void CExtraTree::TvnBeginDrag(HWND hT, LPARAM lParam, HWND hw)
{
    if (!m_drag) {
        auto* lItem = reinterpret_cast<NM_TREEVIEW*>(lParam);
        BeginDrag(hT, lItem, hw);
        m_tvI = lItem->itemNew;
        m_hDragItem = m_tvI.hItem;
    }
}

void CExtraTree::BeginDrag(HWND hT, NM_TREEVIEW* lItem, HWND hw)
{
    HIMAGELIST hIml = TreeView_CreateDragImage(hT, lItem->itemNew.hItem);
    ImageList_BeginDrag(hIml, 0, lItem->ptDrag.x, lItem->ptDrag.y - TOP_TREE);
    SetCapture(hw);
    m_drag = true;
}

void CExtraTree::TvnMouseMoveDrag(HWND hT, LPARAM lParam)
{
    if (m_drag) {
        MouseMoveDrag(hT, lParam);
    }
}

void CExtraTree::MouseMoveDrag(HWND hT, LPARAM lParam)
{
    ImageList_DragMove(LOWORD(lParam), HIWORD(lParam));

    m_tvHit.pt.x = LOWORD(lParam);
    m_tvHit.pt.y = HIWORD(lParam) - TOP_TREE;

    m_hTarget = TreeView_HitTest(hT, &m_tvHit);
    if (m_hTarget) {
        TreeView_SelectDropTarget(hT, m_hTarget);
    }
}

void CExtraTree::TvnLButtonUp(HWND hT)
{
    if (m_drag) {
        LButtonUpDrag(hT);
    }
}

void CExtraTree::LButtonUpDrag(HWND hT)
{
    DropItem(m_hDragItem, hT);
    ImageList_EndDrag();

    ReleaseCapture();

    m_drag = false;
    m_hDragItem = nullptr;
    m_hTarget = nullptr;
}

void CExtraTree::DropItem(HTREEITEM hDragItem, HWND hT)
{
    HTREEITEM hTarget = TreeView_GetDropHilight(hT);
    HTREEITEM hParent = TreeView_GetParent(hT, hTarget);

    char buffer[32]{};
    TVITEM tvTarget{};
    tvTarget.hItem = hDragItem;
    tvTarget.cchTextMax = sizeof(buffer);
    tvTarget.mask = TVIF_TEXT | TVIF_PARAM;
    tvTarget.pszText = buffer;

    TreeView_GetItem(hT, &tvTarget);

    int index = static_cast<int>(tvTarget.lParam);

    AddItem(hParent, buffer, hTarget, hT, index, 0);
    TreeView_DeleteItem(hT, hDragItem);
    TreeView_SelectDropTarget(hT, nullptr);
}

int CExtraTree::TvnSelChanged(HWND hT)
{
    int ret = -1;
    HTREEITEM ht = TreeView_GetSelection(hT);

    char buffer[256]{};
    TVITEM tvi{};
    tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
    tvi.hItem = ht;
    tvi.pszText = buffer;
    tvi.cchTextMax = sizeof(buffer);

    if (SendMessage(hT, TVM_GETITEM, TVGN_CARET, reinterpret_cast<LPARAM>(&tvi))) {
        ret = static_cast<int>(tvi.lParam);
    }
    return ret;
}

void CExtraTree::ResizeTreeView(HWND hT)
{
    RECT rc{};
    GetClientRect(hT, &rc);
    MoveWindow(hT,
               rc.left + MARG + 1,
               rc.top + TOP_TREE,
               rc.right - (rc.left + 2),
               rc.bottom - (rc.top + TOP_TREE + 1),
               TRUE);
}

int CExtraTree::InitTreeView(HINSTANCE hInst, HWND hT)
{
    hImageList1 = ImageList_Create(0, 0, ILC_COLOR32 | ILC_MASK, 1, 0);
    ImageList_AddIcon(hImageList1, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2)));
    TreeView_SetImageList(hT, hImageList1, TVSIL_NORMAL);

    HTREEITEM hStocks = AddItem(nullptr, "Stocks", TVI_ROOT, hT, -1, 1);
    AddItem(hStocks, "• Function Index 0", TVI_LAST, hT, 0, 0);
    AddItem(hStocks, "• Function Index 1", TVI_LAST, hT, 1, 0);
    AddItem(hStocks, "• Make Row 0 Header", TVI_LAST, hT, 2, 1);
    AddItem(hStocks, "• Function Index 3", TVI_LAST, hT, 3, 0);
    SendMessage(hT, TVM_EXPAND, TVE_EXPAND, reinterpret_cast<LPARAM>(hStocks));

    HTREEITEM hSales = AddItem(nullptr, "Sales", TVI_ROOT, hT, -1, 1);
    AddItem(hSales, "• Function Index 4", TVI_LAST, hT, 4, 0);
    AddItem(hSales, "• Function Index 5", TVI_LAST, hT, 5, 0);
    AddItem(hSales, "• Function Index 6", TVI_LAST, hT, 6, 0);
    AddItem(hSales, "• Function Index 7", TVI_LAST, hT, 7, 0);
    AddItem(hSales, "• Function Index 8", TVI_LAST, hT, 8, 0);
    SendMessage(hT, TVM_EXPAND, TVE_EXPAND, reinterpret_cast<LPARAM>(hSales));

    HTREEITEM hData = AddItem(nullptr, "Data", TVI_ROOT, hT, -1, 1);
    AddItem(hData, "• Function Index 9", TVI_LAST, hT, 9, 0);
    AddItem(hData, "• Function Index 10", TVI_LAST, hT, 11, 0);
    AddItem(hData, "• Function Index 11", TVI_LAST, hT, 10, 0);
    AddItem(hData, "• Function Index 12", TVI_LAST, hT, 12, 0);
    AddItem(hData, "• Function Index 13", TVI_LAST, hT, 13, 0);
    SendMessage(hT, TVM_EXPAND, TVE_EXPAND, reinterpret_cast<LPARAM>(hData));

    HTREEITEM hReports = AddItem(nullptr, "Reports", TVI_ROOT, hT, -1, 1);
    AddItem(hReports, "• Function Index 14", TVI_LAST, hT, 14, 0);
    AddItem(hReports, "• Function Index 15", TVI_LAST, hT, 15, 0);
    AddItem(hReports, "• Function Index 16", TVI_LAST, hT, 16, 0);
    AddItem(hReports, "• Function Index 17", TVI_LAST, hT, 17, 0);
    AddItem(hReports, "• Function Index 18", TVI_LAST, hT, 18, 0);
    SendMessage(hT, TVM_EXPAND, TVE_EXPAND, reinterpret_cast<LPARAM>(hReports));

    SendMessage(hT, TVM_ENSUREVISIBLE, 0, reinterpret_cast<LPARAM>(hStocks));
    return 1;
}

HTREEITEM CExtraTree::AddItem(HTREEITEM hParent, const char* szText, HTREEITEM hInsAfter, HWND hT, int data, int bold)
{
    TVITEM tvi{};
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    if (bold) {
        tvi.mask |= TVIF_STATE;
        tvi.state = TVIS_BOLD;
        tvi.stateMask = TVIS_BOLD;
    }

    tvi.pszText = const_cast<char*>(szText);
    tvi.cchTextMax = lstrlen(szText);
    tvi.lParam = data;

    TV_INSERTSTRUCT tvins{};
    tvins.item = tvi;
    tvins.hInsertAfter = hInsAfter;
    tvins.hParent = hParent;

    return reinterpret_cast<HTREEITEM>(
        SendMessage(hT, TVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&tvins))
    );
}
