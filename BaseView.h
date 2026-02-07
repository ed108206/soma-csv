#ifndef HEADER_3E99A4DAD39F52C1
#define HEADER_3E99A4DAD39F52C1

#include <windows.h>

class BaseView {
public:
    virtual ~BaseView() = default;

    virtual void Initialize() = 0;
    virtual HWND hwnd() const = 0;
    virtual void setParent(HWND parent) = 0;
    virtual void setFont(HFONT font) = 0;
};
#endif // header guard

