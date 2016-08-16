#include <windows.h>
#include <alloc.h>
#include <stdlib.h>
#include "lk.h"
#include "km.h"


HCURSOR hcurWait;

FRAME Frame = { 0, 0, 0, "EDIP_FRAME" };

HANDLE  STRUCT::hAccel;
HMENU   STRUCT::hMenu;
HMENU   STRUCT::hWndMenu;
HCURSOR STRUCT::VerCur;
HCURSOR STRUCT::HorCur;
HBITMAP STRUCT::hbmpAtt;
HBITMAP STRUCT::hbmpVal;
char    STRUCT::Name[12] = "STRUCT";

HANDLE  KNOW::hAccel;
HMENU   KNOW::hMenu;
HMENU   KNOW::hWndMenu;
HCURSOR KNOW::VerCur;
HCURSOR KNOW::HorCur;
HBITMAP KNOW::hbmpAttIn;
HBITMAP KNOW::hbmpAttOut;
HBITMAP KNOW::hbmpVal;
char    KNOW::Name[12] = "KNOW";

HANDLE  DATA::hAccel;
HMENU   DATA::hMenu;
HMENU   DATA::hWndMenu;
HCURSOR DATA::VerCur;
HCURSOR DATA::HorCur;
HBITMAP DATA::hbmpAttIn;
HBITMAP DATA::hbmpAttOut;
HBITMAP DATA::hbmpVal;
char    DATA::Name[12] = "DATA";


char FAR DefExt[]    = ".edi";
char FAR DefSpec[]   = "*.edi";
char FAR FullPath[128];

/*
����������������������������������������������������������������������������Ŀ
� �㭪�� ��ࠡ�⪨ �訡��                                   � ErrorMessager �
�                                                            ���������������Ĵ
������������������������������������������������������������������������������
*/
int ErrorMessager ( HWND hwnd, WORD bFlags, WORD id,...) {

	char sz[160];
	char szFmt[128];

	LoadString ( Frame.hInstance, id, (LPSTR) szFmt, sizeof(szFmt) );
	wvsprintf ( (LPSTR) sz, (LPSTR) szFmt, (LPSTR) ( &id+1 ) );
	LoadString ( Frame.hInstance, IDS_APPNAME, (LPSTR) szFmt, sizeof(szFmt) );
	return MessageBox( hwnd, (LPSTR) sz, (LPSTR) szFmt, bFlags );
	}

/*
������������������������������������������������������������������� FRAME ����
*/
/*
����������������������������������������������������������������������������Ŀ
� ���樠����஢��� �ਫ������                        � InitializeApplication �
� (����� �뭥�� � �⤥��� �����)                 �����������������������Ĵ
�                                                                            �
������������������������������������������������������������������������������
*/
BOOL InitializeApplication ( ) {

	WNDCLASS wc;

	// ��ॣ����஢��� Frame �����
	wc.style         = 0;
	wc.lpfnWndProc   = FrameWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = Frame.hInstance;
	wc.hIcon         = LoadIcon ( Frame.hInstance, "frameicon" );
	wc.hCursor       = LoadCursor ( NULL, IDC_ARROW );
	wc.hbrBackground = COLOR_APPWORKSPACE + 1;
	wc.lpszMenuName  = 0;
    wc.lpszClassName = Frame.Name;

	if ( ! RegisterClass ( &wc ) ) {
		ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}

	// ��ॣ����஢��� �� ������ ���� (STRUCT, RULE, DATA, CONCL)
	if ( ! STRUCT::Register () ) {
		ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}


	if ( ! KNOW::Register () ) {
		ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}

	if ( ! DATA::Register () ) {
		ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}

	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                       � InitializeInstance �
�                                                       ��������������������Ĵ
������������������������������������������������������������������������������
*/
BOOL InitializeInstance ( LPSTR lpCmdLine, WORD nCmdShow ) {

	Frame.BrdWidth  = GetSystemMetrics ( SM_CYFRAME );
	{
	TEXTMETRIC tm;
	HDC hDC = GetDC ( 0 );
	GetTextMetrics ( hDC, & tm );
	ReleaseDC ( 0, hDC );
	Frame.LBitemHight = tm.tmHeight + tm.tmExternalLeading + 2;   // ITEMHIGHT
	}

	hcurWait        = LoadCursor ( Frame.hInstance, IDC_WAIT );

	STRUCT::VerCur  = LoadCursor ( Frame.hInstance, "vercur" );
	STRUCT::HorCur  = LoadCursor ( Frame.hInstance, "horcur" );
	STRUCT::hbmpAtt = LoadBitmap ( Frame.hInstance, "strattbmp" );
	STRUCT::hbmpVal = LoadBitmap ( Frame.hInstance, "strvalbmp" );

	KNOW::VerCur     = STRUCT::VerCur;
	KNOW::HorCur     = STRUCT::HorCur;
	KNOW::hbmpAttIn  = LoadBitmap ( Frame.hInstance, "knowattin" );
	KNOW::hbmpAttOut = LoadBitmap ( Frame.hInstance, "knowattout" );
	KNOW::hbmpVal    = LoadBitmap ( Frame.hInstance, "knowvalbmp" );

	DATA::VerCur     = STRUCT::VerCur;
	DATA::HorCur     = STRUCT::HorCur;
	DATA::hbmpAttIn  = LoadBitmap ( Frame.hInstance, "datattin" );
	DATA::hbmpAttOut = LoadBitmap ( Frame.hInstance, "datattout" );
	DATA::hbmpVal    = LoadBitmap ( Frame.hInstance, "datavalbmp" );

	// ����㧨�� �� ���� ���� � ������ �� ����ᮢ
	Frame.FrmMenu = LoadMenu ( Frame.hInstance, "framemenu");

	STRUCT::hMenu = LoadMenu ( Frame.hInstance, "structmenu");

	KNOW::hMenu = LoadMenu ( Frame.hInstance, "knowmenu");

	DATA::hMenu = LoadMenu ( Frame.hInstance, "datamenu");

	if ( ! Frame.FrmMenu || ! STRUCT::hMenu || ! KNOW::hMenu || ! DATA::hMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// ��।����� � ��� ���⮯�������� ������� Window
	Frame.WndMenu    = GetSubMenu ( Frame.FrmMenu, FRAME_WIND_POS );
	STRUCT::hWndMenu = GetSubMenu ( STRUCT::hMenu, STRUCT_WIND_POS );
	KNOW::hWndMenu   = GetSubMenu ( KNOW::hMenu, KNOW_WIND_POS );
	DATA::hWndMenu   = GetSubMenu ( DATA::hMenu, DATA_WIND_POS );

	if ( ! Frame.WndMenu || ! STRUCT::hWndMenu || ! KNOW::hWndMenu || ! DATA::hWndMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// ����㧨�� �� ����室��� ��ᥫ�����
	Frame.hFrmAccel = LoadAccelerators ( Frame.hInstance, "frameaccel" );
	STRUCT::hAccel = LoadAccelerators ( Frame.hInstance, "structaccel" );
	KNOW::hAccel = LoadAccelerators ( Frame.hInstance, "knowaccel" );
	DATA::hAccel = LoadAccelerators ( Frame.hInstance, "dataccel" );

	if ( ! Frame.hFrmAccel || ! STRUCT::hAccel || ! KNOW::hAccel || ! DATA::hAccel ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	Frame.hAccel = Frame.hFrmAccel;             // ��砫�� Frame ��ᥫ����

	// ������� Frame � Client ����
	char sz[80];
    LoadString ( Frame.hInstance, IDS_APPNAME, sz, sizeof(sz) );

	Frame.hwndFrame = CreateWindow ( Frame.Name,
             sz,
             WS_OVERLAPPEDWINDOW |  WS_CLIPCHILDREN,
             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
             NULL,
             Frame.FrmMenu,
             Frame.hInstance,
             NULL );

	if ( ! Frame.hwndFrame ) {
		ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTCREATEWIN );
    	return FALSE;
		}

	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = Frame.WndMenu;
	ccs.idFirstChild = IDM_WINDOWCHILD;

	// ������� MDIClient
	Frame.hwndClient = CreateWindow ( "MDICLIENT",
					  NULL,
					  WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
					  0, 0, 0, 0,
					  Frame.hwndFrame,
					  0xCAC,
					  Frame.hInstance,
					  (LPSTR) &ccs );

	if ( ! Frame.hwndClient ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTCREATEWIN );
		return FALSE;
		}

	ShowWindow ( Frame.hwndFrame, nCmdShow );
	ShowWindow ( Frame.hwndClient, SW_SHOW );

	Frame.hwndActive = 0;

    return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
� �뤥���� ��� 䠩�� �� ���                                   � GetPathName �
�                                                              �������������Ĵ
������������������������������������������������������������������������������
*/
LPSTR GetPathName ( LPSTR Path ) {
	if ( ! Path ) return 0;
	if ( ! *Path ) return Path;

	LPSTR ptr = Path;
	for ( ptr += lstrlen ( Path ); ( ( ptr != Path - 1 ) && ( *ptr != '\\') && ( *ptr != ':') ); ptr-- );
    return ptr + 1;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                              � GetFileName �
�                                                              �������������Ĵ
������������������������������������������������������������������������������
*/
int GetFileName ( VOID ) {
	FARPROC lpOpenProc;
	int View;

	lpOpenProc = MakeProcInstance ( (FARPROC) FileOpenProc, Frame.hInstance );
	View = DialogBox ( Frame.hInstance, "FILEOPEN", Frame.hwndFrame, lpOpenProc );
	FreeProcInstance ( lpOpenProc );
	if ( View == -1 ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return 0;
		}

	return ( View );
	}

/*
����������������������������������������������������������������������������Ŀ
� �஢��� ������ � ��।����� ���� ����, ����� ��࠭����  � FileSaveAs �
� � FullPath. �᫨ ������ �� ������ (����� Cancel), �      ������������Ĵ
� �����頥��� FALSE, � FullPath ᮤ�ন� 0.                                 �
������������������������������������������������������������������������������
*/
BOOL FileSaveAs ( ) {
	FARPROC lpSaveAsProc;

	lpSaveAsProc = MakeProcInstance ( (FARPROC) SaveAsProc, Frame.hInstance );
	DialogBox ( Frame.hInstance, "filesaveas", Frame.hwndFrame, lpSaveAsProc );
	FreeProcInstance ( lpSaveAsProc );

	if ( ! *FullPath )                // ������ �� ��२��������� �� ������
		return ( FALSE );

	return ( TRUE );
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                                 � FileOpen �
�                                                                 ����������Ĵ
������������������������������������������������������������������������������
*/
void FileOpen ( void ) {
	int View;
	HANDLE hmem;

	View = GetFileName ();
	if ( ! View )          // �᫨ ������ �� ������ ��� �訡�� � ��������
		return;

	switch ( View ) {

		case IDD_STRUCT :       STRUCT::Open ( FullPath ); break;
		case IDD_KNOW   :       KNOW::Open ( FullPath );   break;
		case IDD_DATA   :       DATA::Open ( FullPath );   break;

		}
	return;
	}

/*
����������������������������������������������������������������������������Ŀ
� ��뢠�� ������ ।���஢���� ��������� ����� �      � ModuleDescription �
� ��࠭�� ���� ��� ���.                                �������������������Ĵ
������������������������������������������������������������������������������
*/
VOID ModuleDescription ( VOID ) {
	MOD_DESCR ModDescr;
	LPSTRUCT pStruct;

	if ( ! Frame.hwndActive ) return;

	// �� ⥪�饬� ���� 㧭��� 㪠��⥫� �� ��ꥪ� KBASE
	// �।��������, �� �� ���� STRUCT. ��᪮��� ��� ��� ����ᮢ ����
	// ���� KBase ��室���� � ����� ����, � ��� ��㣨� ⨯�� ����
	// १���� �㤥� �ࠢ����
	pStruct = (LPSTRUCT) GetMem ( Frame.hwndActive );

	// ������� ���ᠭ�� ����� � ModDescr
	pStruct->KBase->GetModuleHead ( (LPMOD_DESCR) & ModDescr );

	// �맢��� ���������� ��楤��� � ��।��� �� 㪠��⥫� �� MOD_DESCR
	FARPROC lpModProc = MakeProcInstance ( (FARPROC) ModDescriptionProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "moddescr", Frame.hwndFrame, lpModProc, (LPARAM) & ModDescr );
	FreeProcInstance ( lpModProc );
	if ( ! i ) return;                                  // ������ �� ������

	// ������� ����� ���ᠭ�� �����
	pStruct->KBase->SetModuleHead ( (LPMOD_DESCR) & ModDescr );

	return;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                                   � isWild �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/
BOOL isWild ( LPSTR lpfn ) {
	for ( ; ; )
		switch ( *lpfn++ ) {
			case '*':
			case '?':
				return TRUE;                               // �����㦥� 蠡���
			case 0:
				return FALSE;                        // ���⨣��� ����� ��ப�
			default:
				continue;
			}
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                                � AddDefExt �
�                                                                �����������Ĵ
������������������������������������������������������������������������������
*/
BOOL AddDefExt ( LPSTR Name, LPSTR Ext ) {
	LPSTR ptr;

    for ( ptr = Name; *ptr && *ptr != '.'; ptr++ ) ;
	if ( *ptr != '.' ) { lstrcat ( Name, Ext ); return TRUE; }
	return FALSE;
	}

/*
����������������������������������������������������������������������������Ŀ
� ��।���� ������� �� 㪠����� 䠩�.                          � isFile �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/
BOOL isFile ( LPSTR lpPath ) {
	int i = _lopen ( lpPath, WRITE );
	if ( i == HFILE_ERROR ) return ( FALSE );
	_lclose ( i );
	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                                 � CloseAll �
�                                                                 ����������Ĵ
������������������������������������������������������������������������������
*/
BOOL CloseAll ( VOID ) {
/*
���ࢠ ����室��� �ன��� �� �ᥬ ������ 䠩��� � �஢���� ��࠭��� �� ���.
�᫨ 䠩� ��࠭��, � ��३� � ᫥���饬�. �᫨ �� ��࠭��, � �।��⠢���
���짮��⥫� �� �롮�: (i) ��࠭��� 䠩�, ⮣�� �맢��� �㭪�� ��࠭����
��᫥ 祣� � ����ᨬ��� �� �� १���� ���� ��३� � ᫥���饬� 䠩��,
���� ��ࢠ�� ��� ��楤��� (�᫨ 䠩� �� ��������, � �㤥� �맢�� ������ SaveAs,
����� ����� ������ �⪠�, ����� ���뢠�� ��� ��楤��� ������� ����, �.�.,
����室��� ᫥���� �� ⥬, �� �����頥��� ��᫥ ��࠭����), (ii) �� ��࠭���
䠩�, ⮣�� ���� ��३� � ᫥���饬� 䠩�� � (iii) �⪠�, ⮣�� ��ࢠ��
��� ��楤��� ������� ����. �� �६� ����� �஢�ન ��祣� �� �������, �஬�
���������� ��࠭���� 䠩���. �᫨ ��楤� ��ࢠ��, � �����頥��� FALSE.

�᫨ ��楤�� �஢�ન ��諠 �� ����, � �� ����砥�, �� ����� �������
�� ����. ��� �⮣� ������� �� �������� ���� ����室��� ��᫠�� ᮮ�饭��
� ����᫮���� �����⨨ (�.�., ��� �஢�ન 䫠�� ��࠭������).
*/
	LPKBASE pKBase, pKBaseNext;
	LPSTRUCT pStruct;  // ��⠥�, �� �� ��ꥪ� STRUCT. ���ࠢ��쭮, �� �� १���� �� �����
	char szName[20];
	int Answ;

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		if ( pKBase->isKBaseSaved() ) continue;

		pStruct = (LPSTRUCT) GetMem ( pKBase->hWnd );

		lstrcpy ( szName, pKBase->GetNamePtr() );        // ������� ��� 䠩��
		if ( szName[0] == 0 ) lstrcpy ( szName, TITLE_UNTITLED );
		Answ = ErrorMessager  ( Frame.hwndFrame, MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_CLOSESAVE, szName );
		switch ( Answ ) {
			case IDYES :                                       // ���࠭��� ��
				if ( ! pStruct->Save () )
					return FALSE;       // Cancel in SaveAs or error in saving
				break;
			case IDNO :                                        // �� ��࠭���
				break;
			default   :
				           // ���� �訡�� MessageBox(), ���� �⪠� �� �������
				return FALSE;
			}
		}

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		pKBaseNext = pKBase->GetFirstKBase();
		do {
			pKBase = pKBaseNext;
			pKBaseNext = pKBase->GetNextKBase();

/*			// �⮡� � ���� �� ��室��� ᮮ�饭��, ��᫥ 㤠����� ��ꥪ�
			DestroyWindow ( hwndAtt );
			DestroyWindow ( hwndVal );
*/
			DefMDIChildProc ( pKBase->hWnd, WM_CLOSE, 0, 0 );

			pKBase->CloseKBase();                      // ������� ��ꥪ� KBASE
			} while ( pKBaseNext );
		}

	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                               � SelectFile �
�                                                               ������������Ĵ
������������������������������������������������������������������������������
*/
VOID SelectFile ( HWND hwnd ) {
	char szEdit[128];

	GetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit, 128 );   // ����㧨�� ��ப� �� EDIT

	if ( isWild ( (LPSTR) szEdit ) ) {                       // ������� 蠡���
		// �뤥���� ���. � ᤥ���� �� ���⨭�
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_DIRS, IDD_PATH, ATTR_DIRS );
		// ��᫥ �⮣� szEdit ������ ᮤ�ঠ�� ⮫쪮 ��� 䠩��

		// ���� 䠩�� � ����� ���. ���室�騥 ��� 蠡���
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_FILES, IDD_PATH, ATTR_FILES );

		// ������� � EDIT ⮫쪮 ��� 䠩�� �� �ᥣ� ���
		SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit );
		}
	else {                // �����⭠� ��ப� (��� 蠡����); ���� �����稢���
		int len = GetDlgItemText ( hwnd, IDD_PATH, FullPath, 128 );
		if ( FullPath[0] && FullPath[len-1] != '\\' ) lstrcat ( FullPath, "\\" );
		AddDefExt ( szEdit, DefExt );
		lstrcat ( FullPath, szEdit );
		AnsiUpper ( FullPath );
		AnsiUpper ( szEdit );

		if ( ! isFile ( FullPath ) ) {
			ErrorMessager  ( hwnd, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_CANTOPENFILE, szEdit );
			}
		else if ( isFileValid ( FullPath ) == LKE_SUCCESS ) {
			if ( SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_GETCHECK, 0, 0L ) )
				EndDialog ( hwnd, IDD_STRUCT );
			if ( SendDlgItemMessage ( hwnd, IDD_KNOW, BM_GETCHECK, 0, 0L ) )
				EndDialog ( hwnd, IDD_KNOW );
			if ( SendDlgItemMessage ( hwnd, IDD_DATA, BM_GETCHECK, 0, 0L ) )
				EndDialog ( hwnd, IDD_DATA );
			if ( SendDlgItemMessage ( hwnd, IDD_CONC, BM_GETCHECK, 0, 0L ) )
				EndDialog ( hwnd, IDD_CONC );
            }
		else {
			// �������, �� �� �� 䠩� ����
			ErrorMessager  ( hwnd, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_NOTEDIPFILE, szEdit );
			}
		SetActiveWindow ( hwnd );
		}
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                       � ModDescriptionProc �
�                                                       ��������������������Ĵ
������������������������������������������������������������������������������
*/
BOOL FAR PASCAL _export ModDescriptionProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPMOD_DESCR ModDescr;

    switch ( msg ) {
		case WM_INITDIALOG : {
			char buf[16];

			ModDescr = (LPMOD_DESCR) lParam;

			SendDlgItemMessage ( hwnd, IDD_NAME, WM_SETTEXT, 0, (LPARAM) ModDescr->Name );
			SendDlgItemMessage ( hwnd, IDD_NAME, EM_LIMITTEXT, sizeof(ModDescr->Name) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_SETTEXT, 0, (LPARAM) ModDescr->Question );
			SendDlgItemMessage ( hwnd, IDD_QUESTION, EM_LIMITTEXT, sizeof(ModDescr->Question) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_SETTEXT, 0, (LPARAM) ModDescr->Comment );
			SendDlgItemMessage ( hwnd, IDD_COMMENT, EM_LIMITTEXT, sizeof(ModDescr->Comment) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_ICONFILE, WM_SETTEXT, 0, (LPARAM) ModDescr->IconFile );
			SendDlgItemMessage ( hwnd, IDD_ICONFILE, EM_LIMITTEXT, sizeof(ModDescr->IconFile) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_HYPERFILE, WM_SETTEXT, 0, (LPARAM) ModDescr->HyperFile );
			SendDlgItemMessage ( hwnd, IDD_HYPERFILE, EM_LIMITTEXT, sizeof(ModDescr->HyperFile) - 1, 0L );

			// ���樠����஢��� ������ �����⥪�� (�८�ࠧ����� � ��ப�)
			ltoa ( (long) ModDescr->HyperIndex, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, EM_LIMITTEXT, 5, 0L );

			break;
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					char buf[16];

					SendDlgItemMessage ( hwnd, IDD_NAME, WM_GETTEXT, sizeof(ModDescr->Name), (LPARAM) ModDescr->Name );
					SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_GETTEXT, sizeof(ModDescr->Question), (LPARAM) ModDescr->Question );
					SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_GETTEXT, sizeof(ModDescr->Comment), (LPARAM) ModDescr->Comment );
					SendDlgItemMessage ( hwnd, IDD_ICONFILE, WM_GETTEXT, sizeof(ModDescr->IconFile), (LPARAM) ModDescr->IconFile );
					SendDlgItemMessage ( hwnd, IDD_HYPERFILE, WM_GETTEXT, sizeof(ModDescr->HyperFile), (LPARAM) ModDescr->HyperFile );

					// ������� ������ �����⥪�� (�८�ࠧ����� ��ப� � �᫮)
					SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					ModDescr->HyperIndex = (DWORD) atol ( buf );

					EndDialog ( hwnd, 1 );
					break;
					}
				case IDCANCEL : {
					EndDialog ( hwnd, 0 );
					break;
					}
				default:
					return FALSE;
				}
		    break;
			}
		default :
			return FALSE ;
		}
	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                             � AboutDlgProc �
�                                                             ��������������Ĵ
������������������������������������������������������������������������������
*/
BOOL FAR PASCAL _export AboutDlgProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch ( msg ) {
		case WM_INITDIALOG :
        	return TRUE;
		case WM_COMMAND    :
			switch ( wParam ) {
                case IDOK  :
                	EndDialog ( hwnd, 0 );
                    return TRUE;
				}
            break;
        }
    return FALSE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                             � FrameWndProc �
�                                                             ��������������Ĵ
������������������������������������������������������������������������������
*/
long FAR PASCAL _export FrameWndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

    switch ( msg ) {
        case WM_CREATE  : {
			break;
			}
		case WM_INITMENU  : {
			if ( Frame.hwndActive != 0 )
				SendMessage ( Frame.hwndActive, WM_INITMENU, wParam, lParam );
			break;
			}
		case WM_COMMAND : {
			CommandHandler ( hwnd, wParam, lParam );
			break;
			}
		case WM_CLOSE   : {
			// �᫨ �����, � ������� �� ���� �����浪�
			if ( CloseAll() )
				DestroyWindow ( hwnd );
			break;
			}
		case WM_QUERYENDSESSION :
			if ( CloseAll() ) return TRUE;
			else              return FALSE;
		case WM_DESTROY : {
			PostQuitMessage (0);
			break;
			}
		default :
			return DefFrameProc ( hwnd, Frame.hwndClient, msg, wParam, lParam );
		}
	return 0;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                           � CommandHandler �
�                                                           ����������������Ĵ
������������������������������������������������������������������������������
*/
VOID CommandHandler ( HWND hwnd, WPARAM wParam, LPARAM lParam  ) {

	switch( wParam ) {
		case IDM_FILENEW : {
			STRUCT::Open ( 0 );         // �������� �� ��稭����� � ��������
			break;
			}
		case IDM_FILEOPEN : {
			FileOpen();
			break;
			}
		case IDM_FILEABOUT : {
			FARPROC lpAboutDlgProc = MakeProcInstance ( (FARPROC) AboutDlgProc, Frame.hInstance );
			DialogBox ( Frame.hInstance, "ABOUTEDIP", Frame.hwndFrame, lpAboutDlgProc );
			FreeProcInstance ( lpAboutDlgProc );
			break;
			}
		case IDM_FILEEXIT : {
			SendMessage ( hwnd, WM_CLOSE, 0, 0L );
			break;
			}
		case IDM_FILECLOSEALL : {
			CloseAll();
			break;
			}
		case IDM_WINDOWTILE   : {
			SendMessage ( Frame.hwndClient, WM_MDITILE, 0, 0 );
			break;
			}
		case IDM_WINDOWCASCADE: {
			SendMessage ( Frame.hwndClient, WM_MDICASCADE, 0, 0 );
			break;
			}
		case IDM_WINDOWICONS  : {
			SendMessage ( Frame.hwndClient, WM_MDIICONARRANGE, 0, 0 );
			break;
			}
		case IDM_HELPINDEX  :   {
			WinHelp ( hwnd, HELPFILE, HELP_CONTENTS, 0 );
			break;
			}
		case CHANGE_TITLE       :
		case CHANGE_CONTENT     :
		case CHANGE_COMPONENT   :
		case IDM_CHANGEWINDOW   :
		case IDM_FILESAVE       :
		case IDM_FILESAVEAS     :
		case IDM_FILECLOSE      :
		case IDM_VIEWHORIZONTAL :
		case IDM_VIEWVERTICAL   :
		case IDM_VIEWDATA       :
		case IDM_VIEWCONCLUSION :
		case IDM_ASINRULE       :
		case IDM_UNSORTED       :
		case IDM_PRICE          :
		case IDM_NAME           :
		case IDM_IMPORTANCE     :
		case IDM_VALUES         :
		case IDM_CHARACTER      :
		case IDM_REVERSE        : {
			SendMessage ( Frame.hwndActive, WM_COMMAND, wParam, lParam );
			break;
			}
		case IDM_ITEMADD        :
		case IDM_ITEMDELETE     :
		case IDM_ITEMEDIT       :
		case IDM_ITEMINVERT     :
		case IDM_RULEADD        :
		case IDM_RULEDELETE     :
		case IDM_HYPERTEXT      :
		case IDM_COMPILE        :
		case IDM_INFERENCE      :
		case IDM_PREVRULE       :
		case IDM_NEXTRULE       :
		case IDM_FIRSTRULE      :
		case IDM_LASTRULE       :
		case IDM_INCREASE       :
		case IDM_DECREASE       : {
			if ( ! IsIconic ( Frame.hwndActive ) )
				SendMessage ( Frame.hwndActive, WM_COMMAND, wParam, lParam );
			break;
			}
		case IDM_ITEMHEADER     : {
			if ( ! IsIconic ( Frame.hwndActive ) ) ModuleDescription();
			break;
			}
		default:
			DefFrameProc ( hwnd, Frame.hwndClient, WM_COMMAND, wParam, lParam );
		}
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                             � FileOpenProc �
�                                                             ��������������Ĵ
������������������������������������������������������������������������������
*/
extern BOOL FAR PASCAL _export FileOpenProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

    switch ( msg ) {

		case WM_INITDIALOG : {
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_LIMITTEXT, 64, 0L );
			// ��⠭����� ���७�� *.db � EDIT � ��������� 䠩�� � ���-��
			SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) DefSpec );
			SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, TRUE, 0L );
			SelectFile ( hwnd );
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_SETSEL, NULL, MAKELONG(0,0x7fff) );
			SetFocus( GetDlgItem ( hwnd, IDD_FILENAME ) );
			return( FALSE );   // �����뢠��, �� 䮪�� ��⠭����� �� ����஫�
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					SelectFile ( hwnd );
					break;
					}
				case IDCANCEL : {
					// ���� ������ ���, �� ������ �� ������
					*FullPath = 0;
					EndDialog ( hwnd, 0 );
					break;
					}
				case IDD_STRUCT : {
					SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, TRUE, 0L );
					SendDlgItemMessage ( hwnd, IDD_KNOW, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_DATA, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_CONC, BM_SETCHECK, FALSE, 0L );
					break;
					}
				case IDD_KNOW : {
					SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_KNOW, BM_SETCHECK, TRUE, 0L );
					SendDlgItemMessage ( hwnd, IDD_DATA, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_CONC, BM_SETCHECK, FALSE, 0L );
					break;
					}
				case IDD_DATA : {
					SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_KNOW, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_DATA, BM_SETCHECK, TRUE, 0L );
					SendDlgItemMessage ( hwnd, IDD_CONC, BM_SETCHECK, FALSE, 0L );
					break;
					}
				case IDD_CONC : {
					SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_KNOW, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_DATA, BM_SETCHECK, FALSE, 0L );
					SendDlgItemMessage ( hwnd, IDD_CONC, BM_SETCHECK, TRUE, 0L );
					break;
					}
				case IDD_FILENAME : {
					// ��������� OK �᫨ EDIT ����� ⥪��
					EnableWindow ( GetDlgItem ( hwnd, IDOK ),
						GetWindowTextLength ( (HWND) LOWORD(lParam) ) );
					break;
					}
				case IDD_FILES : {
					switch ( HIWORD(lParam) ) {
						case LBN_SELCHANGE : {
							char buf[100];
							// �� ����㧨�� ��࠭��� ��� 䠩�� � ���� �
							// ��⠭����� ⥪�� � EDIT
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_FILES );
							SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) buf );
							break;
							}
						case LBN_DBLCLK : {
							// � EDIT ��室���� ��� 䠩��, ���⮬� ��� ����
							// ��������� ������
							SelectFile( hwnd );
							break;
							}
						}
					break;
					}
				case IDD_DIRS : {
					switch ( HIWORD(lParam) ) {
						case LBN_SELCHANGE : {
							LPSTR pEdit, pDst, pSrc;
							char buf[100];

							// ������� ���� ��� ��� ���.
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_DIRS );

							// ���⥣���� � ���� ��� 䠩�� �� EDIT
							pEdit = (LPSTR) buf + lstrlen ( (LPSTR) buf );
							GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );

							if ( ! isWild ( pEdit ) ) {
								// ����㧨�� ���७�� �� 㬮�砭��
								SetDlgItemText ( hwnd, IDD_FILENAME, DefSpec );
								// � ��⥬ ���஡����� ���⥣���� ᭮��
								GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );
								}
							else {                      // ���� �-� ��� 䠩��
								// ��� ������� � ��� ⮫쪮 ��� 䠩�� (墮��
								// ��ப�), ���⮬� �� ��⠫쭮� �ய�᪠����
								for ( pDst = pSrc = pEdit; *pSrc; pSrc++ ) {
									if ( *pSrc == '\\' || *pSrc == ':' )
										pDst = pEdit;         // ����⠭�����
									else
										*pDst++ = *pSrc;
									}
								*pDst = 0;
								}
							// ������� � EDIT ����� ��ப�
							SetDlgItemText ( hwnd, IDD_FILENAME, buf );
							break;
							}
						case LBN_DBLCLK : {
							// � EDIT ��室���� ���. � 蠡�����, ���⮬� ����
							// ��३� �㤠 � ��������� 䠩�� �� 蠡����
							SelectFile ( hwnd );
							}
						}
					break;
                    }
				default:
					return FALSE;
				}
		    break;
			}
		default :
			return FALSE;
		}
	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
�                                                               � SaveAsProc �
�                                                               ������������Ĵ
������������������������������������������������������������������������������
*/
BOOL FAR PASCAL _export SaveAsProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

    switch ( msg ) {

		case WM_INITDIALOG : {
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_LIMITTEXT, 64, 0L );
			DlgDirList ( hwnd, "*.*", IDD_DIRS, IDD_PATH, ATTR_DIRS );
			EnableWindow ( GetDlgItem( hwnd, IDOK ), FALSE );
			break;
			}
		case WM_COMMAND : {
			switch ( wParam ) {
				case IDCANCEL : {
					// ���� ������ ���, �� ������ �� ������
					FullPath[0] = 0;
					EndDialog ( hwnd, 0 );
					break;
					}
				case IDOK : {
					char szEdit[128];
					int i;

					GetDlgItemText ( hwnd, IDD_PATH, FullPath, 128 );
					if ( FullPath[0] ) lstrcat ( FullPath, "\\" );
					GetDlgItemText ( hwnd, IDD_FILENAME, szEdit, 128 );
					AddDefExt ( szEdit, DefExt );
					lstrcat ( FullPath, szEdit );
					AnsiUpper ( FullPath );
					AnsiUpper ( szEdit );

					if ( ! isFile ( FullPath ) ) {
						EndDialog ( hwnd, 1 );
						break;
						}

					i = ErrorMessager ( hwnd, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_REPLACE, szEdit );

					switch ( i ) {
						case IDYES : EndDialog ( hwnd, 1 );
						case IDNO :  break;
						}

					SetActiveWindow ( hwnd );
					break;
					}
				case IDD_FILENAME : {
					// ��������� OK �᫨ EDIT ����� ⥪��
					EnableWindow ( GetDlgItem ( hwnd, IDOK ), GetWindowTextLength ( (HWND) LOWORD ( lParam ) ) );
					break;
					}
				case IDD_DIRS : {
					if ( HIWORD (lParam) == LBN_DBLCLK ) {
						char szT[64];

						DlgDirSelect ( hwnd, szT, IDD_DIRS );
						lstrcat ( szT, "*.*" );
						DlgDirList ( hwnd, szT, IDD_DIRS, IDD_PATH, ATTR_DIRS );
						break;
						}
					return ( FALSE );
					}
				default:
					return FALSE;
				}
		    break;
			}
		default :
			return FALSE;
		}
	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
� ������� �㭪��                                                  � WinMain �
�                                                                  ���������Ĵ
������������������������������������������������������������������������������
*/
int PASCAL WinMain ( HANDLE hInstance, HANDLE hPrevInstance, LPSTR  lpszCmdLine, int    nCmdShow ) {
	MSG msg;

    Frame.hInstance = hInstance;

	if ( ! hPrevInstance )
		if ( ! InitializeApplication () )                   exit( FALSE );

	if ( ! InitializeInstance ( lpszCmdLine, nCmdShow ) )   exit( FALSE );


	while ( GetMessage ( &msg, NULL, 0, 0 ) ) {

		if ( ! TranslateMDISysAccel ( Frame.hwndClient, &msg ) &&
             ! TranslateAccelerator ( Frame.hwndFrame, Frame.hAccel, &msg ) ) {
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
			}
		}
	return msg.wParam;

	}

