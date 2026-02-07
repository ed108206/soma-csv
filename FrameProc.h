#ifndef HEADER_6933F19E27FFE302
#define HEADER_6933F19E27FFE302

#include <vector>
#include <memory>
#include <string>
#include <windows.h>

#include "LView.h"
#include "EView.h"
#include "SplitterE.h"
#include "ExtraTree.h"
#include "ExtraFrame.h"


class CFrameProc {
public:
    CFrameProc();
    ~CFrameProc() = default;

    friend class CExtraFrame;

    int xSizing{0};
    int ySizing{0};
    int left_width{0};
    int pane_height{0};
    int m_differ{0};
    HWND m_fhwnd{nullptr};

    int Callu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void ProxyMoCursor(int u);
    void ReParent(HWND hw, const std::string& name, int op);

    void MiniFreePtrVec(HWND hw);
    //void* GetCurrentPtr(HWND hw);
    CEView* GetCurrentPtr(HWND hw);
    TabState GetCurrentStat(HWND hw);
    void SetOpen(HWND hw, bool u);

    void TextCaption(const std::string& b);

    void DeleteCurTab(HWND hw);
    void DeleteCurTabFromMenu();
    HWND GetCurrentHwnd(CEView* cvv);

    void fillHeaderListView();

private:
    CSplitterE csp;
    CExtraTree ctt;
    CExtraFrame cef;

    std::vector<DOLOG> m_tabvec;

    HWND hTree{nullptr};
    HWND hTab{nullptr};
    HWND bottomhwnd{nullptr};

    RECT m_trec{};
    HICON hIcon{nullptr};
    HICON m_hi{nullptr};

    int m_ixx{0};
    int m_fdrag{0};
    int m_sol{0};

    void InvalidateList();
    void NewFont(HWND hw);
    void CreateR(HWND hWnd, int u, const std::string& name);
    void MoCursor(int u);
    void KillObjects();
};
#endif // header guard

