#ifndef HEADER_F41F889B66E76D20
#define HEADER_F41F889B66E76D20

class CFrameProc;

class CExtraFrame
{
public:
	CExtraFrame();
	virtual ~CExtraFrame();

	 CFrameProc* cfr{nullptr};

	int ReSizePane(HWND hWnd, std::vector<DOLOG>* vec);
	void Painter(HWND hWnd);
	void Notifier(HWND hWnd, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hprev{nullptr};
	void ReBope();


};

#endif // header guard

