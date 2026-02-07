#ifndef HEADER_7FC984C68C92646F
#define HEADER_7FC984C68C92646F

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <commctrl.h>
#include <memory>
#include "BaseView.h"

#define WM_SIZE_HTAB                   (WM_USER + 100)

#define WBRUSH		GetSysColorBrush(COLOR_BTNFACE)
#define DBORD		GetSysColorBrush(COLOR_SCROLLBAR)//(COLOR_BTNSHADOW)
#define WHITE		GetSysColorBrush(COLOR_WINDOW)
#define BLACK		GetSysColorBrush(COLOR_WINDOWTEXT)
#define GRAYH		GetSysColorBrush(COLOR_3DDKSHADOW)

#define MAIN_WINDOW_CLASS		"Main Window"
#define LEFT_WINDOW_CLASS		"Left Window"
#define RIGHT_WINDOW_CLASS		"Right Window"
#define BOTTOM_WINDOW_CLASS		"Bottom Window"

// left pane
#define LEFT_WINDOW_WIDTH		270
#define PANE_WINDOW_HEIGHT		100

#define WIDTH_ADJUST			3
#define	SPLITTER_BAR_WIDTH		3

#define HEIGHT_ADJUST			3
#define	SPLITTER_BAR_HEIGHT		3

#define MARG					6
#define	TOP_POS					6
#define	BOTTOM_POS				6

#define TOP_TREE				26
#define	MINIMUM_SPACE			15


enum class TabState { Attached, Detached };
struct DOLOG {
    HWND hwnd{nullptr};
    int ixx{0};
    std::unique_ptr<BaseView> ptr;
    std::string nam;
    TabState stat{TabState::Attached};
    bool open{false};
};

#define SNOM	"Soma"


#endif // header guard

