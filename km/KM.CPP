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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴컴커
� 붵�ゆ⑨ �□젩�洙� �鼇‘�                                   � ErrorMessager �
�                                                            읕컴컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙沙� FRAME 沙沙
*/
/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴컴컴컴컴컴커
� 댂ⓩ쯄エ㎤昔쥯筍 �黍ギ┘���                        � InitializeApplication �
� (М┃� �萸α殊 � �搜�レ�硫 Мㅳレ)                 읕컴컴컴컴컴컴컴컴컴컴컴�
�                                                                            �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL InitializeApplication ( ) {

	WNDCLASS wc;

	// 뇿誓（飡黍昔쥯筍 Frame か졹�
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

	// 뇿誓％飡黍昔쥯筍 ㏇� か졹頌 �ぎ� (STRUCT, RULE, DATA, CONCL)
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컴컴컴커
�                                                       � InitializeInstance �
�                                                       읕컴컴컴컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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

	// 뇿｀習ⓥ� ㏇� Д�� �ぎ� � 캙э筍 �� 誓率褻��
	Frame.FrmMenu = LoadMenu ( Frame.hInstance, "framemenu");

	STRUCT::hMenu = LoadMenu ( Frame.hInstance, "structmenu");

	KNOW::hMenu = LoadMenu ( Frame.hInstance, "knowmenu");

	DATA::hMenu = LoadMenu ( Frame.hInstance, "datamenu");

	if ( ! Frame.FrmMenu || ! STRUCT::hMenu || ! KNOW::hMenu || ! DATA::hMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// 렞誓ㄵエ筍 � �ⓨ Д飡���ギ┘��� ��ㄼ��� Window
	Frame.WndMenu    = GetSubMenu ( Frame.FrmMenu, FRAME_WIND_POS );
	STRUCT::hWndMenu = GetSubMenu ( STRUCT::hMenu, STRUCT_WIND_POS );
	KNOW::hWndMenu   = GetSubMenu ( KNOW::hMenu, KNOW_WIND_POS );
	DATA::hWndMenu   = GetSubMenu ( DATA::hMenu, DATA_WIND_POS );

	if ( ! Frame.WndMenu || ! STRUCT::hWndMenu || ! KNOW::hWndMenu || ! DATA::hWndMenu ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	// 뇿｀習ⓥ� ㏇� ���▼�ㄸщ� 젶醒ゥ�졻�贍
	Frame.hFrmAccel = LoadAccelerators ( Frame.hInstance, "frameaccel" );
	STRUCT::hAccel = LoadAccelerators ( Frame.hInstance, "structaccel" );
	KNOW::hAccel = LoadAccelerators ( Frame.hInstance, "knowaccel" );
	DATA::hAccel = LoadAccelerators ( Frame.hInstance, "dataccel" );

	if ( ! Frame.hFrmAccel || ! STRUCT::hAccel || ! KNOW::hAccel || ! DATA::hAccel ) {
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTLOADRESOURCE );
		return FALSE;
		}

	Frame.hAccel = Frame.hFrmAccel;             // 뜝�젷彛硫 Frame 젶醒ゥ�졻��

	// 뫌ℓ졻� Frame � Client �き�
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

	// 뫌ℓ졻� MDIClient
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
� 귣ㄵエ筍 º� �젵쳽 �� �呻�                                   � GetPathName �
�                                                              읕컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
LPSTR GetPathName ( LPSTR Path ) {
	if ( ! Path ) return 0;
	if ( ! *Path ) return Path;

	LPSTR ptr = Path;
	for ( ptr += lstrlen ( Path ); ( ( ptr != Path - 1 ) && ( *ptr != '\\') && ( *ptr != ':') ); ptr-- );
    return ptr + 1;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
�                                                              � GetFileName �
�                                                              읕컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
� 뤲�´飡� ㄸ젷�� � ��誓ㄵエ筍 ���硫 �呻�, ぎ獸贍� 貰魚젺畑恂�  � FileSaveAs �
� � FullPath. 끷エ ㄸ젷�� �� 貰飡�纏碎 (췅쬊獸 Cancel), 獸      읕컴컴컴컴컴캑
� ¡㎖�좈젰恂� FALSE, � FullPath 貰ㄵ逝ⓥ 0.                                 �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL FileSaveAs ( ) {
	FARPROC lpSaveAsProc;

	lpSaveAsProc = MakeProcInstance ( (FARPROC) SaveAsProc, Frame.hInstance );
	DialogBox ( Frame.hInstance, "filesaveas", Frame.hwndFrame, lpSaveAsProc );
	FreeProcInstance ( lpSaveAsProc );

	if ( ! *FullPath )                // 꽙젷�� �� ��誓º���쥯�⑧ �� 貰飡�纏碎
		return ( FALSE );

	return ( TRUE );
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴커
�                                                                 � FileOpen �
�                                                                 읕컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void FileOpen ( void ) {
	int View;
	HANDLE hmem;

	View = GetFileName ();
	if ( ! View )          // 끷エ ㄸ젷�� �� 貰飡�纏碎 Œ� �鼇―� 獸 쭬ぎ�葉筍
		return;

	switch ( View ) {

		case IDD_STRUCT :       STRUCT::Open ( FullPath ); break;
		case IDD_KNOW   :       KNOW::Open ( FullPath );   break;
		case IDD_DATA   :       DATA::Open ( FullPath );   break;

		}
	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴컴컴컴커
� 귣㏓쥯β ㄸ젷�� 誓쩆もⓣ�쥯�⑨ 쭬．ギ˚� Мㅳワ �      � ModuleDescription �
� 貰魚젺畑� エ‘ �β ⅲ�.                                읕컴컴컴컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
VOID ModuleDescription ( VOID ) {
	MOD_DESCR ModDescr;
	LPSTRUCT pStruct;

	if ( ! Frame.hwndActive ) return;

	// 룼 收ゃ耀с �き� 習췅筍 丞젳졻�レ 췅 �↔ⅹ� KBASE
	// 뤲ⅳ��쳽짛��, 譽� 將� �き� STRUCT. 룼稅�レゃ ㄻ� ㏇ε か졹貰� �ぎ�
	// ��ゥ KBase 췅若ㄸ恂� � �ㄽ�� Д飡�, 獸 ㄻ� ㅰ膝ⓨ 殊��� �ぎ�
	// 誓㎯レ�졻 ▲ㄵ� ��젪Œ彛臾
	pStruct = (LPSTRUCT) GetMem ( Frame.hwndActive );

	// 뇿率�呻� ��ⓤ젺�� Мㅳワ � ModDescr
	pStruct->KBase->GetModuleHead ( (LPMOD_DESCR) & ModDescr );

	// 귣㎖졻� ㄸ젷�．¡� �昔璵ㅳ說 � ��誓쩆筍 ⅸ 丞젳졻�レ 췅 MOD_DESCR
	FARPROC lpModProc = MakeProcInstance ( (FARPROC) ModDescriptionProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "moddescr", Frame.hwndFrame, lpModProc, (LPARAM) & ModDescr );
	FreeProcInstance ( lpModProc );
	if ( ! i ) return;                                  // 꽙젷�� �� 貰飡�纏碎

	// 뇿�ⓤ졻� ��¡� ��ⓤ젺�� Мㅳワ
	pStruct->KBase->SetModuleHead ( (LPMOD_DESCR) & ModDescr );

	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴커
�                                                                   � isWild �
�                                                                   읕컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL isWild ( LPSTR lpfn ) {
	for ( ; ; )
		switch ( *lpfn++ ) {
			case '*':
			case '?':
				return TRUE;                               // 렊췅說┘� �젩ギ�
			case 0:
				return FALSE;                        // 꽡飡ª�呻 ぎ�ζ 飡昔え
			default:
				continue;
			}
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴커
�                                                                � AddDefExt �
�                                                                읕컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL AddDefExt ( LPSTR Name, LPSTR Ext ) {
	LPSTR ptr;

    for ( ptr = Name; *ptr && *ptr != '.'; ptr++ ) ;
	if ( *ptr != '.' ) { lstrcat ( Name, Ext ); return TRUE; }
	return FALSE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴커
� 렞誓ㄵワβ 率耀飡㏂β エ 丞젳젺�硫 �젵�.                          � isFile �
�                                                                   읕컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL isFile ( LPSTR lpPath ) {
	int i = _lopen ( lpPath, WRITE );
	if ( i == HFILE_ERROR ) return ( FALSE );
	_lclose ( i );
	return TRUE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴커
�                                                                 � CloseAll �
�                                                                 읕컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL CloseAll ( VOID ) {
/*
뫍�舒� ���▼�ㄸМ �昔⒱ⓤ� �� ㏇�� �洙贍瞬� �젵쳽� � �昔´黍筍 貰魚젺��� エ ���.
끷エ �젵� 貰魚젺��, 獸 ��誓⒱� � 笹ⅳ莘耀с. 끷エ �� 貰魚젺��, 獸 �誓ㄾ飡젪ⓥ�
��レ㎜쥯收ヮ 췅 �濡��: (i) 貰魚젺ⓥ� �젵�, 獸＄� �留쥯筍 押�ゆ⑧ 貰魚젺��⑨
��笹� 曄． � 쭬˘歲М飡� �� ⅴ 誓㎯レ�졻� エ‘ ��誓⒱� � 笹ⅳ莘耀с �젵ャ,
エ‘ �誓舒졻� ㏇� �昔璵ㅳ說 (αエ �젵� �� º���쥯�, 獸 ▲ㄵ� �留쥯� ㄸ젷�� SaveAs,
ぎ獸贍� М┘� ´惜呻� �洙젳, ぎ獸贍� �誓贍쥯β ㏇� �昔璵ㅳ說 쭬む淫⑨ �ぎ�, �.�.,
���▼�ㄸМ 笹ⅳⓥ� 쭬 收�, 譽� ¡㎖�좈젰恂� ��笹� 貰魚젺��⑨), (ii) �� 貰魚젺汀�
�젵�, 獸＄� �昔飡� ��誓⒱� � 笹ⅳ莘耀с �젵ャ � (iii) �洙젳, 獸＄� �誓舒졻�
㏇� �昔璵ㅳ說 쭬む淫⑨ �ぎ�. 궙 №�э �昔璵遜� �昔´夕� �①ⅲ� �� Д�畑恂�, む�Д
¡㎚�┃�． 貰魚젺��⑨ �젵ギ�. 끷エ �昔璵ㅰ� �誓舒젺�, 獸 ¡㎖�좈젰恂� FALSE.

끷エ �昔璵ㅳ�� �昔´夕� �昔獄� ㄾ ぎ���, 獸 將� �㎛좂젰�, 譽� М┃� 쭬む淫�
㏇� �き�. 꽞� 將�． 첓┐�с �� ºξ蟯齬� �ぎ� ���▼�ㄸМ ��笹졻� 貰�↓����
� ‥㎯笹�˛�� 쭬む淫Ŀ (�.�., ‥� �昔´夕� 氏젫� 貰魚젺����飡�).
*/
	LPKBASE pKBase, pKBaseNext;
	LPSTRUCT pStruct;  // 묊ⓥ젰�, 譽� 將� �↔ⅹ� STRUCT. 뜢��젪Œ彛�, �� 췅 誓㎯レ�졻 �� ˙⑨β
	char szName[20];
	int Answ;

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		if ( pKBase->isKBaseSaved() ) continue;

		pStruct = (LPSTRUCT) GetMem ( pKBase->hWnd );

		lstrcpy ( szName, pKBase->GetNamePtr() );        // 룼ャ葉筍 º� �젵쳽
		if ( szName[0] == 0 ) lstrcpy ( szName, TITLE_UNTITLED );
		Answ = ErrorMessager  ( Frame.hwndFrame, MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_CLOSESAVE, szName );
		switch ( Answ ) {
			case IDYES :                                       // 뫌魚젺ⓥ� 걞
				if ( ! pStruct->Save () )
					return FALSE;       // Cancel in SaveAs or error in saving
				break;
			case IDNO :                                        // 뜢 貰魚젺汀�
				break;
			default   :
				           // 떒‘ �鼇―� MessageBox(), エ‘ �洙젳 �� 쭬む淫⑨
				return FALSE;
			}
		}

	InitFilesCounter();
	while ( (pKBase = GetNextFileObject()) != NULL ) {
		pKBaseNext = pKBase->GetFirstKBase();
		do {
			pKBase = pKBaseNext;
			pKBaseNext = pKBase->GetNextKBase();

/*			// 쀢�〓 � �き� �� �黍若ㄸギ 貰�↓��Ł, ��笹� 蝨젷��⑨ �↔ⅹ��
			DestroyWindow ( hwndAtt );
			DestroyWindow ( hwndVal );
*/
			DefMDIChildProc ( pKBase->hWnd, WM_CLOSE, 0, 0 );

			pKBase->CloseKBase();                      // 뇿む淫� �↔ⅹ� KBASE
			} while ( pKBaseNext );
		}

	return TRUE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
�                                                               � SelectFile �
�                                                               읕컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
VOID SelectFile ( HWND hwnd ) {
	char szEdit[128];

	GetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit, 128 );   // 뇿｀習ⓥ� 飡昔ゃ �� EDIT

	if ( isWild ( (LPSTR) szEdit ) ) {                       // ºⅴ恂� �젩ギ�
		// 귣ㄵエ筍 ㄸ�. � 誠�쳽筍 ⅴ エ飡Þ�
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_DIRS, IDD_PATH, ATTR_DIRS );
		// 룼笹� 將�． szEdit ㄾウ�� 貰ㄵ逝졻� 獸レぎ º� �젵쳽

		// 뜝⒱� �젵ル � ��¡� ㄸ�. ��ㅵ�ㅿ蟯� ��� �젩ギ�
		DlgDirList ( hwnd, (LPSTR) szEdit, IDD_FILES, IDD_PATH, ATTR_FILES );

		// 뇿率�呻� � EDIT 獸レぎ º� �젵쳽 �� ㏇ⅲ� �呻�
		SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) szEdit );
		}
	else {                // ぎ�むβ췅� 飡昔첓 (�β �젩ギ췅); 췅ㄾ 쭬첓�葉쥯筍
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
			// 뫌�↓ⓥ�, 譽� 將� �� �젵� 쓣닆
			ErrorMessager  ( hwnd, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_NOTEDIPFILE, szEdit );
			}
		SetActiveWindow ( hwnd );
		}
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컴컴컴커
�                                                       � ModDescriptionProc �
�                                                       읕컴컴컴컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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

			// 댂ⓩ쯄エ㎤昔쥯筍 Þㄵめ （��設ⅹ飡� (�誓�□젳�쥯筍 � 飡昔ゃ)
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

					// 룼ャ葉筍 Þㄵめ （��設ⅹ飡� (�誓�□젳�쥯筍 飡昔ゃ � 葉笹�)
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴커
�                                                             � AboutDlgProc �
�                                                             읕컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴커
�                                                             � FrameWndProc �
�                                                             읕컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
			// 끷エ М┃�, 獸 쭬む淫� ㏇� �き� ����涉ㄺ�
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컴커
�                                                           � CommandHandler �
�                                                           읕컴컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
VOID CommandHandler ( HWND hwnd, WPARAM wParam, LPARAM lParam  ) {

	switch( wParam ) {
		case IDM_FILENEW : {
			STRUCT::Open ( 0 );         // 뫌ℓ젺�� 걞 췅葉췅β碎 貰 飡說も侁�
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴커
�                                                             � FileOpenProc �
�                                                             읕컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
extern BOOL FAR PASCAL _export FileOpenProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

    switch ( msg ) {

		case WM_INITDIALOG : {
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_LIMITTEXT, 64, 0L );
			// 볚�젺�˘筍 �졹鼇誓��� *.db � EDIT � 쭬��キⓥ� �젵ル � ㄸ�-Ŀ
			SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) DefSpec );
			SendDlgItemMessage ( hwnd, IDD_STRUCT, BM_SETCHECK, TRUE, 0L );
			SelectFile ( hwnd );
			SendDlgItemMessage ( hwnd, IDD_FILENAME, EM_SETSEL, NULL, MAKELONG(0,0x7fff) );
			SetFocus( GetDlgItem ( hwnd, IDD_FILENAME ) );
			return( FALSE );   // 룼첓㏓쥯β, 譽� 兒ゃ� 信�젺�˙�� 췅 ぎ�循�ゥ
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					SelectFile ( hwnd );
					break;
					}
				case IDCANCEL : {
					// 꽑筍 ���汀� �泣�, 譽� ㄸ젷�� �� 貰飡�纏碎
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
					// 룼㎖�エ筍 OK αエ EDIT ºⅴ� 收め�
					EnableWindow ( GetDlgItem ( hwnd, IDOK ),
						GetWindowTextLength ( (HWND) LOWORD(lParam) ) );
					break;
					}
				case IDD_FILES : {
					switch ( HIWORD(lParam) ) {
						case LBN_SELCHANGE : {
							char buf[100];
							// 뮢 쭬｀習ⓥ� �濡�젺��� º� �젵쳽 � ▲芯� �
							// 信�젺�˘筍 收め� � EDIT
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_FILES );
							SetDlgItemText ( hwnd, IDD_FILENAME, (LPSTR) buf );
							break;
							}
						case LBN_DBLCLK : {
							// � EDIT 췅若ㄸ恂� º� �젵쳽, ��將�с ⅲ� 췅ㄾ
							// ��飡졷졻藺� �洙贍筍
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

							// 룼ャ葉筍 ���硫 ㄸ稅 Œ� ㄸ�.
							DlgDirSelect ( hwnd, (LPSTR) buf, IDD_DIRS );

							// 뤲ⓤ收－呻� � ��с º� �젵쳽 �� EDIT
							pEdit = (LPSTR) buf + lstrlen ( (LPSTR) buf );
							GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );

							if ( ! isWild ( pEdit ) ) {
								// 뇿｀習ⓥ� �졹鼇誓��� �� 僧�ョ젺⑧
								SetDlgItemText ( hwnd, IDD_FILENAME, DefSpec );
								// � 쭬收� ���昔‘쥯筍 �黍飡ⅲ�呻� 說�쥯
								GetDlgItemText ( hwnd, IDD_FILENAME, pEdit, 64 );
								}
							else {                      // 끷筍 �-獸 º� �젵쳽
								// 뜝� Þ收誓率β � ��� 獸レぎ º� �젵쳽 (罌�飡
								// 飡昔え), ��將�с ㏇� �飡젷彛�� �昔�信첓β碎
								for ( pDst = pSrc = pEdit; *pSrc; pSrc++ ) {
									if ( *pSrc == '\\' || *pSrc == ':' )
										pDst = pEdit;         // 궙遜�젺�˘筍
									else
										*pDst++ = *pSrc;
									}
								*pDst = 0;
								}
							// 뇿率�呻� � EDIT ��㏂� 飡昔ゃ
							SetDlgItemText ( hwnd, IDD_FILENAME, buf );
							break;
							}
						case LBN_DBLCLK : {
							// � EDIT 췅若ㄸ恂� ㄸ�. � �젩ギ���, ��將�с 췅ㄾ
							// ��誓⒱� 栒쩆 � 쭬��キⓥ� �젵ル �� �젩ギ��
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
�                                                               � SaveAsProc �
�                                                               읕컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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
					// 꽑筍 ���汀� �泣�, 譽� ㄸ젷�� �� 貰飡�纏碎
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
					// 룼㎖�エ筍 OK αエ EDIT ºⅴ� 收め�
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴커
� 꺂젪췅� 押�ゆ⑨                                                  � WinMain �
�                                                                  읕컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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


