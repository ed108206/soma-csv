#ifndef HEADER_ADAFB5087A86027D
#define HEADER_ADAFB5087A86027D

#include <windows.h>
#include <commdlg.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include "BaseView.h"

class CFrameProc;

struct MappedFile {
    const char* data{nullptr};
    size_t length{0};
#if defined(_WIN32)
    HANDLE hFile {nullptr};
    HANDLE hMap{nullptr};
#else
    int fd {-1};
#endif
};

class CEView : public BaseView
{
public:
    CEView();
    ~CEView() override;

    CFrameProc* cfr{nullptr};

    HWND m_khwnd{nullptr};
    HWND m_hwnd{nullptr};

    void Initialize() override;
    HWND hwnd() const override
    {
        return m_hwnd;
    }
    void setParent(HWND parent) override
    {
        m_parent = parent;
    }
    void setFont(HFONT font) override
    {
        m_font = font;
    }

    LRESULT CallMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void ShowText(const char* b);
    void ShowText(HWND hwnd, const char* b);

    void PreMain(HWND hwnd);
    void FreeAll(HWND hwnd);
    void MakeRow0Header();
    CEView* GetLocalPtr(HWND hw);

private:
    HFONT m_font{nullptr};
    HWND m_parent{nullptr};

    HWND m_lv{nullptr};

    int m_mela{0};
    int m_ur{0};
    UINT nNumLines{0};

    std::unordered_map<int, std::string> cache;
    std::vector<std::vector<std::string_view>> filas;

    HWND CreateListView(HINSTANCE hInstance, HWND hwndParent);
    void ResizeListView(HWND hwndParent);
    BOOL InitListView(HWND hwndListView);
    LRESULT ListViewNotify(HWND hWnd, LPARAM lParam);

    // static callbacks
    static INT_PTR CALLBACK EaDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // file mapping
    MappedFile mf;
    MappedFile Map_file(const std::string& filename);
    void Unmap_file(MappedFile& f);

    // parsing
    void Parse_segment_simd(const char* data, std::size_t start, std::size_t end, std::vector<std::vector<std::string_view>>& out);
    void Parse_csv_parallel(const char* data, size_t len, int num_threads);

    void mostrar_fila(size_t idx);
    void DoFileOpenEx(HWND hwnd);
    float GetTicks();
    void ShowTime(const float& tk);
    void SetText(const char* b);
};
#endif // header guard

