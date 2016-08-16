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
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция обработки ошибок                                   │ ErrorMessager │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐ FRAME ▐▐▐▐
*/
/*
┌────────────────────────────────────────────────────┬───────────────────────┐
│ Инициализировать приложение                        │ InitializeApplication │
│ (можно вынести в отдельный модуль)                 └───────────────────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL InitializeApplication ( ) {

	WNDCLASS wc;

	// Зарегистрировать Frame класс
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

	// Зарегестрировать все классы окон (STRUCT, RULE, DATA, CONCL)
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
┌───────────────────────────────────────────────────────┬────────────────────┐
│                                                       │ InitializeInstance │
│                                                       └────────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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

	// Загрузить все меню окон в память из ресурсов
	Frame.FrmMenu = LoadMenu ( Frame.hInstance, "framemenu");

	STRUCT::hMenu = LoadMenu ( Frame.hInstance, "structmenu");

	KNOW::hMenu = LoadMenu ( Frame.hInstance, "knowmenu");

	DATA::hMenu = LoadMenu ( Frame.hInstance, "datamenu");

	if ( ! Frame.FrmMenu || ! STRUCT::hMenu || ! KNOW::hMenu || ! DATA::hMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// Определить в них местоположение подменю Window
	Frame.WndMenu    = GetSubMenu ( Frame.FrmMenu, FRAME_WIND_POS );
	STRUCT::hWndMenu = GetSubMenu ( STRUCT::hMenu, STRUCT_WIND_POS );
	KNOW::hWndMenu   = GetSubMenu ( KNOW::hMenu, KNOW_WIND_POS );
	DATA::hWndMenu   = GetSubMenu ( DATA::hMenu, DATA_WIND_POS );

	if ( ! Frame.WndMenu || ! STRUCT::hWndMenu || ! KNOW::hWndMenu || ! DATA::hWndMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// Загрузить все необходимые акселераторы
	Frame.hFrmAccel = LoadAccelerators ( Frame.hInstance, "frameaccel" );
	STRUCT::hAccel = LoadAccelerators ( Frame.hInstance, "structaccel" );
	KNOW::hAccel = LoadAccelerators ( Frame.hInstance, "knowaccel" );
	DATA::hAccel = LoadAccelerators ( Frame.hInstance, "dataccel" );

	if ( ! Frame.hFrmAccel || ! STRUCT::hAccel || ! KNOW::hAccel || ! DATA::hAccel ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	Frame.hAccel = Frame.hFrmAccel;             // Начальный Frame акселератор

	// Создать Frame и Client окна
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

	// Создать MDIClient
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
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Выделить имя файла из пути                                   │ GetPathName │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPSTR GetPathName ( LPSTR Path ) {
	if ( ! Path ) return 0;
	if ( ! *Path ) return Path;

	LPSTR ptr = Path;
	for ( ptr += lstrlen ( Path ); ( ( ptr != Path - 1 ) && ( *ptr != '\\') && ( *ptr != ':') ); ptr-- );
    return ptr + 1;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ GetFileName │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Провести диалог и определить новый путь, который сохраняется  │ FileSaveAs │
│ в FullPath. Если диалог не состоялся (нажато Cancel), то      └────────────┤
│ возвращается FALSE, а FullPath содержит 0.                                 │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FileSaveAs ( ) {
	FARPROC lpSaveAsProc;

	lpSaveAsProc = MakeProcInstance ( (FARPROC) SaveAsProc, Frame.hInstance );
	DialogBox ( Frame.hInstance, "filesaveas", Frame.hwndFrame, lpSaveAsProc );
	FreeProcInstance ( lpSaveAsProc );

	if ( ! *FullPath )                // Диалог по переименованию не состоялся
		return ( FALSE );

	return ( TRUE );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ FileOpen │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void FileOpen ( void ) {
	int View;
	HANDLE hmem;

	View = GetFileName ();
	if ( ! View )          // Если диалог не состоялся или ошибка то закончить
		return;

	switch ( View ) {

		case IDD_STRUCT :       STRUCT::Open ( FullPath ); break;
		case IDD_KNOW   :       KNOW::Open ( FullPath );   break;
		case IDD_DATA   :       DATA::Open ( FullPath );   break;

		}
	return;
	}

/*
┌────────────────────────────────────────────────────────┬───────────────────┐
│ Вызывает диалог редактирования заголовка модуля и      │ ModuleDescription │
│ сохраняет либо нет его.                                └───────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID ModuleDescription ( VOID ) {
	MOD_DESCR ModDescr;
	LPSTRUCT pStruct;

	if ( ! Frame.hwndActive ) return;

	// По текущему окну узнать указатель на объект KBASE
	// Предполагаем, что это окно STRUCT. Поскольку для всех классов окон
	// поле KBase находится в одном месте, то для других типов окон
	// результат будет правильным
	pStruct = (LPSTRUCT) GetMem ( Frame.hwndActive );

	// Засунуть описание модуля в ModDescr
	pStruct->KBase->GetModuleHead ( (LPMOD_DESCR) & ModDescr );

	// Вызвать диалоговою процедуру и передать ей указатель на MOD_DESCR
	FARPROC lpModProc = MakeProcInstance ( (FARPROC) ModDescriptionProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "moddescr", Frame.hwndFrame, lpModProc, (LPARAM) & ModDescr );
	FreeProcInstance ( lpModProc );
	if ( ! i ) return;                                  // Диалог не состоялся

	// Записать новое описание модуля
	pStruct->KBase->SetModuleHead ( (LPMOD_DESCR) & ModDescr );

	return;
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│                                                                   │ isWild │
│                                                                   └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL isWild ( LPSTR lpfn ) {
	for ( ; ; )
		switch ( *lpfn++ ) {
			case '*':
			case '?':
				return TRUE;                               // Обнаружен шаблон
			case 0:
				return FALSE;                        // Достигнут конец строки
			default:
				continue;
			}
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ AddDefExt │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL AddDefExt ( LPSTR Name, LPSTR Ext ) {
	LPSTR ptr;

    for ( ptr = Name; *ptr && *ptr != '.'; ptr++ ) ;
	if ( *ptr != '.' ) { lstrcat ( Name, Ext ); return TRUE; }
	return FALSE;
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ Определяет существует ли указанный файл.                          │ isFile │
│                                                                   └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL isFile ( LPSTR lpPath ) {
	int i = _lopen ( lpPath, WRITE );
	if ( i == HFILE_ERROR ) return ( FALSE );
	_lclose ( i );
	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ CloseAll │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL CloseAll ( VOID ) {
/*
Сперва необходимо пройтись по всем открытым файлам и проверить сохранены ли они.
Если файл сохранен, то перейти к следующему. Если не сохранен, то предоставить
пользователю на выбор: (i) сохранить файл, тогда вызвать функцию сохранения
после чего в зависимости от ее результата либо перейти к следующему файлу,
либо прервать всю процедуру (если файл не именован, то будет вызван диалог SaveAs,
который может вернуть отказ, который прерывает всю процедуру закрытия окон, т.е.,
необходимо следить за тем, что возвращается после сохранения), (ii) не сохранять
файл, тогда просто перейти к следующему файлу и (iii) отказ, тогда прервать
всю процедуру закрытия окон. Во время процесса проверки ничего не меняется, кроме
возможного сохранения файлов. Если процедра прервана, то возвращается FALSE.

Если процедура проверки прошла до конца, то это означает, что можно закрыть
все окна. Для этого каждому из имеющихся окон необходимо послать сообщение
о безусловном закрытии (т.е., без проверки флага сохраненности).
*/
	LPKBASE pKBase, pKBaseNext;
	LPSTRUCT pStruct;  // Считаем, что это объект STRUCT. Неправильно, но на результат не влияет
	char szName[20];
	int Answ;

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		if ( pKBase->isKBaseSaved() ) continue;

		pStruct = (LPSTRUCT) GetMem ( pKBase->hWnd );

		lstrcpy ( szName, pKBase->GetNamePtr() );        // Получить имя файла
		if ( szName[0] == 0 ) lstrcpy ( szName, TITLE_UNTITLED );
		Answ = ErrorMessager  ( Frame.hwndFrame, MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_CLOSESAVE, szName );
		switch ( Answ ) {
			case IDYES :                                       // Сохранить БЗ
				if ( ! pStruct->Save () )
					return FALSE;       // Cancel in SaveAs or error in saving
				break;
			case IDNO :                                        // Не сохранять
				break;
			default   :
				           // Либо ошибка MessageBox(), либо отказ от закрытия
				return FALSE;
			}
		}

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		pKBaseNext = pKBase->GetFirstKBase();
		do {
			pKBase = pKBaseNext;
			pKBaseNext = pKBase->GetNextKBase();

/*			// Чтобы к окну не приходило сообщений, после удаления объекта
			DestroyWindow ( hwndAtt );
			DestroyWindow ( hwndVal );
*/
			DefMDIChildProc ( pKBase->hWnd, WM_CLOSE, 0, 0 );

			pKBase->CloseKBase();                      // Закрыть объект KBASE
			} while ( pKBaseNext );
		}

	return TRUE;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ SelectFile │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID SelectFile ( HWND hwnd ) {
	char szEdit[128];

	GetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit, 128 );   // Загрузить строку из EDIT

	if ( isWild ( (LPSTR) szEdit ) ) {                       // имеется шаблон
		// Выделить дир. и сделать ее листинг
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_DIRS, IDD_PATH, ATTR_DIRS );
		// После этого szEdit должно содержать только имя файла

		// Найти файлы в новой дир. подходящие под шаблон
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_FILES, IDD_PATH, ATTR_FILES );

		// Засунуть в EDIT только имя файла от всего пути
		SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit );
		}
	else {                // конкретная строка (нет шаблона); надо заканчивать
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
			// Сообщить, что это не файл ЭДИП
			ErrorMessager  ( hwnd, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_NOTEDIPFILE, szEdit );
			}
		SetActiveWindow ( hwnd );
		}
	}

/*
┌───────────────────────────────────────────────────────┬────────────────────┐
│                                                       │ ModDescriptionProc │
│                                                       └────────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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

			// Инициализировать индекс гипертекста (преобразовать в строку)
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

					// Получить индекс гипертекста (преобразовать строку в число)
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
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ AboutDlgProc │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ FrameWndProc │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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
			// Если можно, то закрыть все окна попорядку
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
┌───────────────────────────────────────────────────────────┬────────────────┐
│                                                           │ CommandHandler │
│                                                           └────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID CommandHandler ( HWND hwnd, WPARAM wParam, LPARAM lParam  ) {

	switch( wParam ) {
		case IDM_FILENEW : {
			STRUCT::Open ( 0 );         // Создание БЗ начинается со структуры
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
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ FileOpenProc │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
extern BOOL FAR PASCAL _export FileOpenProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

    switch ( msg ) {

		case WM_INITDIALOG : {
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_LIMITTEXT, 64, 0L );
			// Установить расширение *.db в EDIT и заполнить файлы и дир-ии
			SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) DefSpec );
			SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, TRUE, 0L );
			SelectFile ( hwnd );
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_SETSEL, NULL, MAKELONG(0,0x7fff) );
			SetFocus( GetDlgItem ( hwnd, IDD_FILENAME ) );
			return( FALSE );   // Показывает, что фокус установлен на контроле
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					SelectFile ( hwnd );
					break;
					}
				case IDCANCEL : {
					// Дать понять выше, что диалог не состоялся
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
					// Позволить OK если EDIT имеет текст
					EnableWindow ( GetDlgItem ( hwnd, IDOK ),
						GetWindowTextLength ( (HWND) LOWORD(lParam) ) );
					break;
					}
				case IDD_FILES : {
					switch ( HIWORD(lParam) ) {
						case LBN_SELCHANGE : {
							char buf[100];
							// То загрузить выбранное имя файла в буфер и
							// установить текст в EDIT
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_FILES );
							SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) buf );
							break;
							}
						case LBN_DBLCLK : {
							// В EDIT находится имя файла, поэтому его надо
							// постараться открыть
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

							// Получить новый диск или дир.
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_DIRS );

							// Пристегнуть к нему имя файла из EDIT
							pEdit = (LPSTR) buf + lstrlen ( (LPSTR) buf );
							GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );

							if ( ! isWild ( pEdit ) ) {
								// Загрузить расширение по умолчанию
								SetDlgItemText ( hwnd, IDD_FILENAME, DefSpec );
								// А затем попробовать пристегнуть снова
								GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );
								}
							else {                      // Есть к-то имя файла
								// Нас интересует в нем только имя файла (хвост
								// строки), поэтому все остальное пропускается
								for ( pDst = pSrc = pEdit; *pSrc; pSrc++ ) {
									if ( *pSrc == '\\' || *pSrc == ':' )
										pDst = pEdit;         // Восстановить
									else
										*pDst++ = *pSrc;
									}
								*pDst = 0;
								}
							// Засунуть в EDIT новую строку
							SetDlgItemText ( hwnd, IDD_FILENAME, buf );
							break;
							}
						case LBN_DBLCLK : {
							// В EDIT находится дир. с шаблоном, поэтому надо
							// перейти туда и заполнить файлы по шаблону
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
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ SaveAsProc │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
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
					// Дать понять выше, что диалог не состоялся
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
					// Позволить OK если EDIT имеет текст
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
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Главная функция                                                  │ WinMain │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
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


