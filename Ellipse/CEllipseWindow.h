#include <Windows.h>

class CEllipseWindow
{
public:
	CEllipseWindow();
	~CEllipseWindow();
	static bool RegisterClass( HINSTANCE hinstance );
	bool Create( HWND parentWindow );
	void Show( int cmdShow );
	HWND GetHandle();
	bool IsInFocus();
	void OnLButtonDown();

protected:
	void OnDestroy();
	void OnCreate();
	void OnPaint();
	void OnTimer();

private:
	HWND hwnd;
	static LRESULT __stdcall windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );

	int stepHorizontal;
	int stepVertical;
	int directionH;
	int directionV;
	int left, right, top, bottom;

	static const int timerID = 1;

	void ShowEllipse( HDC hdc, int clientLeft, int clientRight, int clientTop, int clientBottom );
};
