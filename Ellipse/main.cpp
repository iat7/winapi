#include "COverlappedWindow.h"
#include <Windows.h>
#include "resource.h"

int WINAPI WinMain( HINSTANCE hinstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow )
{
	MSG message;

	COverlappedWindow window;
	if ( !window.RegisterClass( hinstance ) ) {
		return FALSE;
	}

	if ( !window.Create( hinstance ) ) {
		return FALSE;
	}
	window.Show( nCmdShow );

	HACCEL haccel = LoadAccelerators( hinstance, MAKEINTRESOURCE( ARROWS ) );

	BOOL isMessageGot;
	while ( ( isMessageGot = GetMessage( &message, (HWND) NULL, 0, 0 ) ) != 0 && isMessageGot != -1 )
	{
		if ( !TranslateAccelerator(
			window.GetHandle(),
			haccel,
			&message ) )
		{
			TranslateMessage( &message );
			DispatchMessage( &message );
		}
	}
	return message.wParam;

	UNREFERENCED_PARAMETER( lpCmdLine );
}