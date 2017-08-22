#include <windows.h>
#include "CEllipseWindow.h"
#include <iostream>

CEllipseWindow::CEllipseWindow()
{
	hwnd = NULL;
}

CEllipseWindow::~CEllipseWindow()
{
}

bool CEllipseWindow::RegisterClass( HINSTANCE hinstance )
{
	WNDCLASSEX wcx;

	wcx.cbSize = sizeof( wcx );
	wcx.style = WS_OVERLAPPED;
	wcx.lpfnWndProc = windowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hinstance;
	wcx.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wcx.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName = "MainMenu";
	wcx.lpszClassName = "MainWClass";
	wcx.hIconSm = NULL;

	return RegisterClassEx( &wcx );
}

bool CEllipseWindow::Create( HWND parentWindow )
{
	hwnd = CreateWindow(
		"MainWClass",
		"Child Window",
		WS_CHILD,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		parentWindow,
		(HMENU)NULL,
		GetModuleHandle( 0 ),
		this
	);

	if ( !hwnd )
	{
		return FALSE;
	}

	return TRUE;
}

void CEllipseWindow::Show( int cmdShow )
{
	ShowWindow( hwnd, cmdShow );
	UpdateWindow( hwnd );
}

void CEllipseWindow::OnCreate()
{
	stepHorizontal = 0;
	stepVertical = 0;
	directionH = 1;
	directionV = 1;
}

void CEllipseWindow::ShowEllipse( HDC hdc, 
								  int clientLeft, int clientRight, 
								  int clientTop, int clientBottom )
{
	COLORREF colorRect = RGB( 0, 255, 0 );
	COLORREF colorEllipse = RGB( 50, 132, 20 );
	COLORREF colorEllipseInactive = RGB( 255, 105, 255 );

	HGDIOBJ oldBrush;
	HBRUSH brushEllipse;

	int width;
	int height;

	/* Rectangle */
	Rectangle( hdc, clientLeft, clientTop,
			   clientRight, clientBottom );

	HBRUSH brushRect = CreateSolidBrush( colorRect );
	SelectObject( hdc, brushRect );
	Rectangle( hdc, clientLeft, clientTop,
			   clientRight, clientBottom );

	/* Ellipse */
	if ( GetFocus() == hwnd )
	{
		brushEllipse = CreateSolidBrush( colorEllipse );
	}
	else
	{
		brushEllipse = CreateSolidBrush( colorEllipseInactive );
		KillTimer(hwnd, 1);
	}

	oldBrush = SelectObject( hdc, brushEllipse );

	width = (clientRight - clientLeft) / 2;
	height = (clientBottom - clientTop) / 2;

	if ( width > stepHorizontal )
	{
		left = clientLeft + directionH * stepHorizontal;
		right = left + width;
	}
	else
	{
		stepHorizontal = -width;
		directionH = -1 * directionH;
	}
	if ( height > stepVertical )
	{
		top = clientTop + directionV * stepVertical;
		bottom = top + height;
	}
	else
	{
		stepVertical = -height;
		directionV = -1 * directionV;
	}

	Ellipse( hdc, left, top, right, bottom );

	DeleteObject( brushRect );
	DeleteObject( brushEllipse );
	DeleteObject( oldBrush );
}

void CEllipseWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rcClient;

	hdc = BeginPaint( hwnd, &ps );
	GetClientRect( hwnd, &rcClient );

	HDC bufferDC = CreateCompatibleDC( hdc );
	HBITMAP buffer = CreateCompatibleBitmap( hdc, 
											 rcClient.right - rcClient.left,
											 rcClient.bottom - rcClient.top );
	HGDIOBJ oldBuffer = SelectObject( bufferDC, buffer );

	ShowEllipse( bufferDC, rcClient.left, rcClient.right, rcClient.top, rcClient.bottom );

	BitBlt( hdc, rcClient.left, rcClient.top, 
			rcClient.right - rcClient.left,
			rcClient.bottom - rcClient.top, 
			bufferDC, 0, 0, SRCCOPY );

	SelectObject( bufferDC, oldBuffer );

	rcClient.left = rcClient.right = 0;
	rcClient.top = rcClient.bottom = 0;

	DeleteDC( bufferDC );
	DeleteObject( buffer );
	DeleteObject( oldBuffer );

	EndPaint( hwnd, &ps );
}

void CEllipseWindow::OnTimer()
{
	if ( GetFocus() == hwnd )
	{
		stepHorizontal++;
		stepVertical++;
	}
	InvalidateRect( hwnd, NULL, FALSE );
}

void CEllipseWindow::OnLButtonDown()
{
	SetFocus( hwnd );
}

bool CEllipseWindow::IsInFocus()
{
	return GetFocus() == hwnd;
}

LRESULT __stdcall CEllipseWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam )
{
	UINT timer = SetTimer( handle, timerID, 20, (TIMERPROC)NULL );

	switch ( message )
	{
	case WM_NCCREATE:
	{
		CEllipseWindow* window = (CEllipseWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr( handle, GWLP_USERDATA, (LONG)window );
		window->hwnd = handle;
		return DefWindowProc( handle, message, wParam, lParam );
	}
	case WM_CREATE:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnCreate();
		return DefWindowProc( handle, message, wParam, lParam );
	}
	case WM_DESTROY:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnDestroy();
		return 0L;
	}

	case WM_PAINT:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnPaint();
		return 0L;
	}

	case WM_SIZE:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		InvalidateRect( window->hwnd, NULL, FALSE );
	}

	case WM_TIMER:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnTimer();
		return 0;
	}

	case WM_ERASEBKGND:
	{
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		CEllipseWindow* window = (CEllipseWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnLButtonDown();
		return 0;
	}

	default:
		return DefWindowProc( handle, message, wParam, lParam );
	}

	return 0;
}

void CEllipseWindow::OnDestroy()
{
	PostQuitMessage( 0 );
}

HWND CEllipseWindow::GetHandle()
{
	return hwnd;
}

// + передаем this последним параметром в CreateWindow
// + on посылает NC_CREATE
// + обрабатываем NC_CREATE
// + SetWindowProc с lparam = this
// + GetWindowProc в OnCreate(), OnPaint()
// + вынести логику отрисовки в OnPaint()
// + OnCreate(), OnPaint() -- не статические
// + из статической функции можно получить экземпл€р класса через this
// + USER_DATA
// змейку надо чтобы она раздел€лась на две, когда пытаешьс€ ее в противоположную сторону направить
