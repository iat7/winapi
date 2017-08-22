#include "CEllipseWindow.h"
#include <Windows.h>

#define numChildren 4

class COverlappedWindow
{
public:
	COverlappedWindow();
	~COverlappedWindow();
	static bool RegisterClass( HINSTANCE hinstance );
	bool Create( HINSTANCE hinstance );
	void Show( int cmdShow );
	HWND GetHandle();

protected:
	void OnCreate();
	void OnDestroy();
	void OnSize();

private:
	HWND hwnd;
	static LRESULT __stdcall windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
	CEllipseWindow window[numChildren];
};
