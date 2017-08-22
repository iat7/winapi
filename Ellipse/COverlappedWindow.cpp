#include "COverlappedWindow.h"
#include "resource.h"

COverlappedWindow::COverlappedWindow()
{
}


COverlappedWindow::~COverlappedWindow()
{
}

bool COverlappedWindow::RegisterClass( HINSTANCE hinstance )
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
	wcx.lpszClassName = "OverlappedWindow";
	wcx.hIconSm = NULL;

	return RegisterClassEx( &wcx );
}

bool COverlappedWindow::Create( HINSTANCE hinstance )
{
	hwnd = CreateWindow(
		"OverlappedWindow",
		"Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(HWND)NULL,
		(HMENU)NULL,
		hinstance,
		this
	);

	if ( !hwnd )
	{
		return FALSE;
	}

	return TRUE;
}

void COverlappedWindow::Show( int cmdShow )
{
	ShowWindow( hwnd, cmdShow );
	for (int i = 0; i < numChildren; i++ ) {
		window[i].Show( cmdShow );
	}

	UpdateWindow( hwnd );
}

void COverlappedWindow::OnCreate()
{
	CEllipseWindow::RegisterClass( GetModuleHandle( 0 ) );
	for ( int i = 0; i < numChildren; i++ )
	{
		window[i].Create( hwnd );
	}
}

void COverlappedWindow::OnDestroy()
{
	PostQuitMessage( 0 );
}

void COverlappedWindow::OnSize()
{
	RECT clientRect;
	GetClientRect( hwnd, &clientRect );
	
	int width = (clientRect.right - clientRect.left) / 2;
	int height = (clientRect.bottom - clientRect.top) / 2;
	
	SetWindowPos( window[0].GetHandle(), HWND_TOP, 
				  clientRect.left, clientRect.top, width, height, 0 );
	SetWindowPos( window[1].GetHandle(), HWND_TOP, 
				  (clientRect.left + clientRect.right) / 2, 
				  clientRect.top, width, height, 0 );
	SetWindowPos( window[2].GetHandle(), HWND_TOP, 
				  clientRect.left, 
				  (clientRect.top + clientRect.bottom) / 2,
				  width, height, 0 );
	SetWindowPos( window[3].GetHandle(), HWND_TOP, 
				  (clientRect.left + clientRect.right) / 2,
				  (clientRect.top + clientRect.bottom) / 2,
				  width, height, 0 );
}

HWND COverlappedWindow::GetHandle()
{
	return hwnd;
}

LRESULT __stdcall COverlappedWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_NCCREATE:
	{
		COverlappedWindow* window = (COverlappedWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr( handle, GWLP_USERDATA, (LONG)window );
		window->hwnd = handle;
		return DefWindowProc( handle, message, wParam, lParam );
	}
	case WM_CREATE:
	{
		COverlappedWindow* window = (COverlappedWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnCreate();
		return DefWindowProc( handle, message, wParam, lParam );
	}
	case WM_DESTROY:
	{
		COverlappedWindow* window = (COverlappedWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnDestroy();
		return 0L;
	}

	case WM_SIZE:
	{
		COverlappedWindow* window = (COverlappedWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		window->OnSize();
		return 0L;
	}

	case WM_ERASEBKGND:
	{
		return 0;
	}

	case WM_COMMAND: {
		COverlappedWindow* parentWindow = (COverlappedWindow*)GetWindowLongPtr( handle, GWLP_USERDATA );
		switch ( LOWORD( wParam ) )
		{
		case RIGHT_ARROW:
			for (int i = 0; i < numChildren; i++ ) {
				if ((i == 0 || i == 2) && parentWindow->window[i].IsInFocus()) {
					parentWindow->window[i + 1].OnLButtonDown();
				}
			}
			break;
		case UP_ARROW:
			for ( int i = 0; i < numChildren; i++ )
			{
				if ( parentWindow->window[2].IsInFocus() )
				{
					parentWindow->window[0].OnLButtonDown();
				}
				if ( parentWindow->window[3].IsInFocus() )
				{
					parentWindow->window[1].OnLButtonDown();
				}
			}
			break;
		case LEFT_ARROW:
			for ( int i = 0; i < numChildren; i++ )
			{
				if ( (i == 1 || i == 3) && parentWindow->window[i].IsInFocus() )
				{
					parentWindow->window[i - 1].OnLButtonDown();
				}
			}
			break;
		case DOWN_ARROW:
			for ( int i = 0; i < numChildren; i++ )
			{
				if ( parentWindow->window[0].IsInFocus() )
				{
					parentWindow->window[2].OnLButtonDown();
				}
				if ( parentWindow->window[1].IsInFocus() )
				{
					parentWindow->window[3].OnLButtonDown();
				}
			}
			break;

		default:
			break;
		}
		return 0L;
	}

	default:
		return DefWindowProc( handle, message, wParam, lParam );
	}

	return 0;
}