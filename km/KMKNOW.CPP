#include <windows.h>
#include <alloc.h>
#include <stdlib.h>
#include "lk.h"
#include "km.h"

extern FRAME FAR Frame;
extern char FAR FullPath[128];

/*
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐ KNOW ▐▐▐▐
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐
*/

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ Register │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KNOW::Register ( VOID ) {

	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = KnowWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof ( LPKNOW );   // Для хранения Указателя на объект
	wc.hInstance     = Frame.hInstance;
	wc.hIcon         = LoadIcon ( Frame.hInstance, "know" );
	wc.hCursor       = 0;
	wc.hbrBackground = COLOR_APPWORKSPACE + 1;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = KNOW::Name;

	if ( ! RegisterClass ( & wc ) ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│ Открыть окно знаний                                                 │ Open │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KNOW::Open ( LPSTR Path ) {
	LPKNOW tis;

	// Выделить память для объекта KNOW
	tis = (LPKNOW) farmalloc ( sizeof(KNOW) );
	if ( tis == NULL ) {
		ErrorMessager  ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return LKE_ALLOCMEM;
		}

	// Инициализировать его поля
	tis->KBase = KBASE::OpenKBase ( Path );
	if ( KBASE::Error != LKE_SUCCESS ) {
		ErrorMessager  ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		farfree ( tis );
		return IDS_CANTALLOC;                        // ????????????????????
		}

	// Дальнейшая инициализация полей
	tis->SplitState = IDM_VIEWNONE;
	tis->Part       = 0.6;
	tis->CurFocus   = IDM_ATT_LISTBOX;

	MDICREATESTRUCT mcs;
	char sz[80] = TITLE_KNOW;

	if ( Path ) lstrcat ( sz, GetPathName ( Path ) );
	else        lstrcat ( sz, TITLE_UNTITLED );

	mcs.szTitle = sz;

	mcs.szClass    = KNOW::Name;
	mcs.hOwner     = Frame.hInstance;
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
	mcs.style      = WS_VSCROLL;
	mcs.lParam     = (LPARAM) tis;

	// Создать основное окно структуры
	tis->hWnd = (HANDLE) SendMessage ( Frame.hwndClient, WM_MDICREATE, 0, (LPARAM) & mcs );
	if( ! tis->hWnd ) {
		tis->Close();
		ErrorMessager ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTCREATEWIN );
		return IDS_CANTCREATEWIN;
		}

	// Создать окно атрибутов (ListBox)
	tis->hwndAtt = CreateWindow ( "LISTBOX", NULL,
             LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
             tis->hWnd, IDM_ATT_LISTBOX, Frame.hInstance, NULL );

	// Создать окно значений (ListBox)
	tis->hwndVal = CreateWindow( "LISTBOX", NULL,
             LBS_HASSTRINGS | LBS_OWNERDRAWFIXED | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT,
             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
             tis->hWnd, IDM_VAL_LISTBOX, Frame.hInstance, NULL );

	// Произвести проверку корректности открытия ListBox'ов
	if ( ! tis->hwndAtt || ! tis->hwndVal ) {
		SendMessage ( tis->hWnd, WM_CLOSE, 0, 0 );
		ErrorMessager  ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTCREATEWIN );
		return IDS_CANTCREATEWIN;
		}

	// Инициализировать scrollbar
	SetScrollRange ( tis->hWnd, SB_VERT, 0, tis->KBase->RuleNumber(), FALSE );
	SetScrollPos ( tis->hWnd, SB_VERT, 0, FALSE );

	// Инициализация состояния
	tis->KBase->SetCurAtt ( -1 );
	tis->KBase->SetCurVal ( -1 );
	tis->KBase->SetAttSort ( SORT_ASINRULE );
	tis->KBase->SetValSort ( SORT_UNSORTED );
	tis->KBase->SetWindow ( tis->hWnd );
	tis->KBase->SetView ( VIEW_KNOWLEDGE );
	tis->KBase->SetCurRule ( 0 );

	tis->PrintOn();

	// Подстроить размеры ListBox'ов
	RECT rc;
	GetClientRect ( tis->hWnd, & rc );
	tis->Size ( SIZENORMAL, MAKELONG ( rc.right, rc.bottom ) );

	ShowWindow ( tis->hWnd, SW_SHOWNORMAL );
	ShowWindow ( tis->hwndAtt, SW_SHOWNORMAL );
	ShowWindow ( tis->hwndVal, SW_SHOWNORMAL );

	tis->SetFocus();

	return LKE_SUCCESS;
	}

/*
┌────────────────────────────────────────────────────────────────────┬───────┐
│ Закрыть окно знаний. Перед закрытием необходимо проверить          │ Close │
│ можно ли это делать. Если закрыто, то возвращается TRUE.           └───────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KNOW::Close ( VOID ) {

	if ( 1 == KBase->KBaseNumber() ) {             // Это последнее окно файла
		if ( ! KBase->isKBaseSaved() ) {     // и если содержимое не сохранено
			// то запросить пользователя о сохранении/несохранении/отмене
			char szName[20];
			int Answ;
			lstrcpy ( szName, KBase->GetNamePtr() );     // Получить имя файла
			if ( szName[0] == 0 ) lstrcpy ( szName, TITLE_UNTITLED );
			Answ = ErrorMessager  ( Frame.hwndFrame, MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_CLOSESAVE, szName );
			switch ( Answ ) {
				case IDYES :                         // Сохранить БЗ и закрыть
					if ( ! Save () ) return FALSE;
					break;
				case IDNO :                          // Закрыть без сохранения
					break;
				default   :
					// Либо ошибка MessageBox(), либо отказ от закрытия
					return FALSE;
				}
			}
		}

	// Чтобы к окну не приходило сообщений, после удаления объекта
	DestroyWindow ( hwndAtt );
	DestroyWindow ( hwndVal );

	// Закрыть объект KBASE
	KBase->CloseKBase();

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│ Сохранить содержимое БЗ. Если неименована БЗ, то вызвать диалог     │ Save │
│ SaveAs. Если БЗ сохранена, то вернуть TRUE.                         └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KNOW::Save ( VOID ) {

	if ( KBase->isKBaseNamed() ) {                          // Сразу сохранить
#ifdef EDIPDEMO
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND, IDS_EDIPDEMO );
#else
		if ( LKE_SUCCESS != KBase->SaveKBase() ) {
			ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTSAVE );
			return FALSE;
			}
#endif
		}
	else if ( FileSaveAs() ) {               // Сохранить после переименования
		KBase->RenameKBase ( FullPath );
#ifdef EDIPDEMO
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND, IDS_EDIPDEMO );
#else
		if ( LKE_SUCCESS != KBase->SaveKBase() ) {
			ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTSAVE );
			return FALSE;
			}
#endif
		}
	else return FALSE;                             // Отказ (Cancel) в диалоге

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│                                                                     │ Size │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::Size ( WPARAM wParam, LPARAM lParam ) {

	if ( wParam != SIZENORMAL && wParam != SIZEFULLSCREEN ) return;

	ShowWindow ( hwndAtt, SW_HIDE );
	ShowWindow ( hwndVal, SW_HIDE );

	if ( SplitState == IDM_VIEWHORIZONTAL ) {
		int AttHight = (int) ( HIWORD(lParam) * Part     ) + 1;
		int ValHight = (int) ( HIWORD(lParam) * (1-Part) ) - Frame.BrdWidth + 2;

		MoveWindow ( hwndAtt, -1, -1,                            LOWORD(lParam) + 2, AttHight, 1 );
		MoveWindow ( hwndVal, -1, AttHight + Frame.BrdWidth - 1, LOWORD(lParam) + 2, ValHight, 1 );
		ShowWindow ( hwndAtt, SW_SHOWNOACTIVATE );
		ShowWindow ( hwndVal, SW_SHOWNOACTIVATE );
		}

	else if ( SplitState == IDM_VIEWVERTICAL ) {
		int AttWidth = (int) ( LOWORD(lParam) * Part     ) + 1;
		int ValWidth = (int) ( LOWORD(lParam) * (1-Part) ) - Frame.BrdWidth + 2;

		MoveWindow ( hwndAtt, -1,                            -1, AttWidth, HIWORD(lParam) + 2, 1 );
		MoveWindow ( hwndVal, AttWidth + Frame.BrdWidth - 1, -1, ValWidth, HIWORD(lParam) + 2, 1 );
		ShowWindow ( hwndAtt, SW_SHOWNOACTIVATE );
		ShowWindow ( hwndVal, SW_SHOWNOACTIVATE );
		}
	else {
		MoveWindow ( hwndAtt, -1, -1, LOWORD(lParam) + 2, HIWORD(lParam) + 2, 1 );
		ShowWindow ( hwndAtt, SW_SHOWNOACTIVATE );
		}
	SetFocus ( );
	}

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│                                                                     │ View │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::View ( int NewView ) {
	RECT rc;
	GetClientRect ( hWnd, & rc );
	int Prev = SplitState;

	// Установить новый вид
	if ( ( NewView == IDM_VIEWHORIZONTAL && Prev == IDM_VIEWHORIZONTAL ) ||
         ( NewView == IDM_VIEWVERTICAL   && Prev == IDM_VIEWVERTICAL ) )
		SplitState = IDM_VIEWNONE;
	else
		SplitState = NewView;

	if ( SplitState == IDM_VIEWNONE ) {
		// Заполнить окна новым содержимым
		// в однооконном режиме с сохранением текущих параметров
		CurFocus = IDM_ATT_LISTBOX;
		FillNoneView();
		}

	if ( Prev == IDM_VIEWNONE ) {
		// Заполнить окна новым содержимым
		// в двуоконным режиме с сохранением текущих параметров
        FillAttListBox();
		FillValListBox ( KBase->GetCurAtt() );
		}

	if ( SplitState == IDM_VIEWHORIZONTAL )
		SetClassWord ( hWnd, GCW_HCURSOR, HorCur );
	else
		SetClassWord ( hWnd, GCW_HCURSOR, VerCur );

	if ( Prev == IDM_VIEWNONE ) {
		ShowWindow ( hwndVal, SW_SHOWNOACTIVATE );   // Показать окно значений
		}
	if ( SplitState == IDM_VIEWNONE ) {
		ShowWindow ( hwndVal, SW_HIDE );                   // Скрыть одно окно
		}

	Size ( SIZENORMAL, MAKELONG ( rc.right, rc.bottom ) );

//	PrintOn ( );
//	CurFocus = IDM_ATT_LISTBOX;
//	SetFocus ( );

	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ PartMove │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::PartMove ( UINT msg, LPARAM lParam ) {
/*
Определить текущий размер (высота или ширина) окна атрибутов и
запомнить его вместе начальными координатами мыши. При движении
мыши определять смещение относительно начальной координаты и
соответственно изменять размер окна атрибутов. Поскольку диапазон
изменения окна атрибутов всегда известен (между 0 и размером папы
без ширины бордюра), то необходимо держать его в этом диапазоне.
Перерисовывание полосы происходит исходя из текущего размера окна
атрибутов. При отпускании мыши, поле Part находится как отношение
размеров окна атрибутов и папы.

Таким образом, основная переменная это текущий размер окна
атрибутов, она отражает положение полосы. Ее начальное состояние
определяется при первом нажатии мыши (независимо от места нажатия
внутри полосы, что хорошо) исходя из текущего Part и размеров
папы (см. функцию Size). Изменение этой переменной происходит
путем добавления к ней текущего смещения мыши относительно
начального положения. Отсюда следует, что в момент нажатия мыши
необходимо запомнить координату.
*/

	static BOOL isMove;
	static int AttSizeStart;
	static int AttSize;
	static int StartPos;
	static RECT rc;

	if ( msg == WM_LBUTTONDOWN ) {
		isMove = TRUE;
		SetCapture ( hWnd );
		GetClientRect ( hWnd, & rc );
		if ( SplitState == IDM_VIEWHORIZONTAL ) {
			AttSizeStart = AttSize = (int) ( rc.bottom * Part ) + 1; // Высота
			StartPos = HIWORD ( lParam );
			DrawPart ( AttSize, IDM_VIEWHORIZONTAL );
			}
		else if ( SplitState == IDM_VIEWVERTICAL ) {
			AttSizeStart = AttSize = (int) ( rc.right * Part ) + 1;  // Ширина
			StartPos = LOWORD ( lParam );
			DrawPart ( AttSize, IDM_VIEWVERTICAL );
			}
		}
	else if ( msg == WM_MOUSEMOVE && isMove) {
		// Перерисовать полосу если изменение в нужном направлении
		if ( SplitState == IDM_VIEWHORIZONTAL ) {
			DrawPart ( AttSize, IDM_VIEWHORIZONTAL );
			AttSize = AttSizeStart + HIWORD(lParam) - StartPos;
			if ( AttSize < 1 ) AttSize = 1;
			if ( AttSize > rc.bottom - Frame.BrdWidth + 1 ) AttSize = rc.bottom - Frame.BrdWidth + 1;
			DrawPart ( AttSize, IDM_VIEWHORIZONTAL );
			}
		else if ( SplitState == IDM_VIEWVERTICAL ) {
			DrawPart ( AttSize, IDM_VIEWVERTICAL );
			AttSize = AttSizeStart + LOWORD(lParam) - StartPos;
			if ( AttSize < 1 ) AttSize = 1;
			if ( AttSize > rc.right - Frame.BrdWidth + 1 ) AttSize = rc.right - Frame.BrdWidth + 1;
			DrawPart ( AttSize, IDM_VIEWVERTICAL );
			}
		}
	else if ( msg == WM_LBUTTONUP && isMove ) {
		// Вычислить новое значение Part и послать сообщение WM_SIZE
		if ( SplitState == IDM_VIEWHORIZONTAL ) {
			Part = (float) (AttSize - 1) / (float) ( rc.bottom );
			DrawPart ( AttSize, IDM_VIEWHORIZONTAL );
			}
		else if ( SplitState == IDM_VIEWVERTICAL ) {
			Part = (float) (AttSize - 1) / (float) ( rc.right );
			DrawPart ( AttSize, IDM_VIEWVERTICAL );
			}
		isMove = FALSE;
		ReleaseCapture ( );
		Size ( SIZENORMAL, MAKELONG ( rc.right, rc.bottom ) );

		}
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ DrawPart │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DrawPart ( int AttSize, WORD vh ) {
	RECT rc;
	GetClientRect ( hWnd, & rc );

	if ( vh == IDM_VIEWHORIZONTAL ) {
		rc.top = AttSize-1;    rc.bottom = AttSize + Frame.BrdWidth - 1;
		}
	if ( vh == IDM_VIEWVERTICAL ) {
		rc.left = AttSize-1;   rc.right = AttSize + Frame.BrdWidth - 1;
		}

	HDC hdc = GetDC ( hWnd );

	InvertRect ( hdc, & rc );

	ReleaseDC ( hWnd, hdc );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Устанавливает фокус в зависимости от текущего состояния.        │ SetFocus │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::SetFocus ( VOID ) {
	if ( ( SplitState == IDM_VIEWNONE ) || ( CurFocus == IDM_ATT_LISTBOX ) )
		::SetFocus ( hwndAtt );
	else ::SetFocus ( hwndVal );
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Прежде всего эта функция очищает окно признаков. Затем    │ FillAttListBox │
│ она выбирает в определенном порядке (зависит от текущей   └────────────────┤
│ сортировки, которая должна быть уже проведена) признаки из файла и         │
│ заполняет ими окно (ListBox). Последняя строка в ListBox'е пустая и        │
│ предназначена для добавления признаков в конец списка. В конце устана-     │
│ вливается текущий номер атрибута и переводится курсор. По мере заполнения  │
│ окна строками, подсчитывается их максимальная длина (в пикселах), после    │
│ чего устанавливается ширина ListBox'а.                                     │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::FillAttListBox ( VOID ) {
	int i, w, maxWidth = 0;

//	ShowWindow ( hwndAtt, SW_HIDE );
	SendMessage ( hwndAtt, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndAtt, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	int Rule = KBase->GetCurRule();
	if (  Rule < 0 || Rule >= KBase->RuleNumber() ) {
		SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, 0, 0 );
		SendMessage ( hwndAtt, WM_SETREDRAW, TRUE, 0L );
		InvalidateRect ( hwndAtt, NULL, FALSE );
		return;
		}

	int n = KBase->PropNumber();
	HDC hDC = GetDC ( hwndAtt );              // Для определения ширины строки
	for ( i = 0; i < n; i++ ) {                       // Цикл по высказываниям
		ATT_REC  AttRec;

		KBase->GetAttribute ( KBase->Prop2Att ( i ), & AttRec );
		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & AttRec.Name, lstrlen ( (LPSTR) & AttRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести атрибут в ListBox
		SendMessage ( hwndAtt, LB_ADDSTRING, 0, (DWORD) & AttRec.Name );
		}
	ReleaseDC ( hwndAtt, hDC );

	SendMessage ( hwndAtt, LB_ADDSTRING, 0, (DWORD)"" );
	SendMessage ( hwndAtt, LB_SETCURSEL, KBase->GetCurAttPos(), 0 );
	SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, maxWidth+6+3*BMPWIDTH, 0 );
	SendMessage ( hwndAtt, WM_SETREDRAW, TRUE, 0L );
	InvalidateRect ( hwndAtt, NULL, FALSE );
//	ShowWindow ( hwndAtt, SW_SHOW );
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Заполняет окно значений строками. Нельзя использовать в   │ FillValListBox │
│ однооконном режиме. Текущее значение должно быть          └────────────────┤
│ правильно установлено. Вспомогательная строка не вводится.                 │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::FillValListBox ( int Att ) {
	VAL_REC  ValRec;
	int j, w, maxWidth = 0;

//	ShowWindow ( hwndVal, SW_HIDE );
	SendMessage ( hwndVal, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndVal, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	int Rule = KBase->GetCurRule();
	if ( Att < 0 || Att >= KBase->AttNumber() || Rule < 0 || Rule >= KBase->RuleNumber() ) {
		SendMessage ( hwndVal, LB_SETHORIZONTALEXTENT, 0, 0 );
		SendMessage ( hwndVal, WM_SETREDRAW, TRUE, 0L );
		InvalidateRect ( hwndVal, NULL, FALSE );
		return;
		}

	HDC hDC = GetDC ( hwndVal );              // Для определения ширины строки
	for ( j = 0; j < KBase->ValNumber ( Att ); j++ ) {    // Цикл по значениям
		// Найти значение с индексом i в зависимости от текущей сортировки
        KBase->GetValue ( Att, j, & ValRec );

		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & ValRec.Name, lstrlen ( (LPSTR) & ValRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести значение в ListBox, а индекс записать (он должен быть равен исходному)
		SendMessage ( hwndVal, LB_ADDSTRING, 0, (DWORD) & ValRec.Name );

		}
	ReleaseDC ( hwndVal, hDC );

	SendMessage ( hwndVal, LB_SETCURSEL, KBase->GetCurVal(), 0 );

	SendMessage ( hwndVal, LB_SETHORIZONTALEXTENT, maxWidth + 6 + 3*BMPWIDTH + COMPLENGTH, 0 );
	SendMessage ( hwndVal, WM_SETREDRAW, TRUE, 0L );
	InvalidateRect ( hwndVal, NULL, FALSE );
//	ShowWindow ( hwndVal, SW_SHOW );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Заполняет окно атрибутами и значениями.                     │ FillNoneView │
│ Текущие атрибут и значение должны быть правильно установлены└──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::FillNoneView ( VOID ) {
	int i, j, w, n, maxWidth = 0;
	ATT_REC  AttRec;
	VAL_REC  ValRec;

//	ShowWindow ( hwndAtt, SW_HIDE );
	SendMessage ( hwndAtt, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndAtt, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	int Rule = KBase->GetCurRule();
	if (  Rule < 0 || Rule >= KBase->RuleNumber() ) {
		SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, 0, 0 );
		SendMessage ( hwndAtt, WM_SETREDRAW, TRUE, 0L );
		InvalidateRect ( hwndAtt, NULL, FALSE );
		return;
		}

	n = KBase->PropNumber();
	HDC hDC = GetDC ( hwndAtt );              // Для определения ширины строки
	for ( i = 0; i < n; i++ ) {                       // Цикл по высказываниям

		int p = KBase->Prop2Att ( i );
		KBase->GetAttribute ( p, & AttRec );
		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & AttRec.Name, lstrlen ( (LPSTR) & AttRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести атрибут в ListBox
		SendMessage ( hwndAtt, LB_ADDSTRING, 0, (DWORD) & AttRec.Name );

		for ( j = 0; j < KBase->ValNumber ( p ); j++ ) {  // Цикл по значениям

			KBase->GetValue ( p, j, & ValRec );
			// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
			w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & ValRec.Name, lstrlen ( (LPSTR) & ValRec.Name) ) );
			if ( maxWidth < w ) maxWidth = w;

			// Занести значение в ListBox
			SendMessage ( hwndAtt, LB_ADDSTRING, 0, (DWORD) & ValRec.Name );
			}

		}
	ReleaseDC ( hwndAtt, hDC );

	SendMessage ( hwndAtt, LB_ADDSTRING, 0, (DWORD)"" );
	SendMessage ( hwndAtt, LB_SETCURSEL, MakeNoneIdx ( KBase->GetCurAtt(), KBase->GetCurVal() ), 0 );
	SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, maxWidth+6+5*BMPWIDTH+COMPLENGTH, 0 );
	SendMessage ( hwndAtt, WM_SETREDRAW, TRUE, 0L );
	InvalidateRect ( hwndAtt, NULL, FALSE );
//	ShowWindow ( hwndAtt, SW_SHOW );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Выводит содержимое объекта в окно                                │ PrintOn │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::PrintOn ( VOID ) {
	if ( SplitState == IDM_VIEWNONE ) FillNoneView ( );
	else {
		FillAttListBox ( );
		FillValListBox ( KBase->GetCurAtt() );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить положение в дереве по атрибуту и значению         │ MakeNoneIdx │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KNOW::MakeNoneIdx ( int Att, int Val ) {
	if ( KBase->GetCurRule() >= KBase->RuleNumber() ) return -1;
	if ( Att == -1 ) return -1;
	return ( KBase->Att2Prop ( Att ) + KBase->ValPosSum ( Att ) + Val + 1 );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить номер атрибута по положению в дереве              │ NoneIdx2Att │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KNOW::NoneIdx2Att ( int Idx ) {
	if ( KBase->GetCurRule() == KBase->RuleNumber() ) return -1;
	if ( Idx == -1 ) return -1;

	int i, n = KBase->PropNumber();
	// Цикл по высказываниям
	// Вычисляем индекс последовательно для всех высказываний и останавливаемся
	// когда перевалим за заданный индекс Idx
	for ( i = 1; i <= n; i++ ) if ( MakeNoneIdx ( KBase->Prop2Att ( i ), -1 ) > Idx ) break;

	return KBase->Prop2Att ( i - 1 );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить номер значения по положению в дереве              │ NoneIdx2Val │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KNOW::NoneIdx2Val ( int Idx ) {
	if ( KBase->GetCurRule() == KBase->RuleNumber() ) return -1;
	if ( Idx == -1 ) return -1;

	return ( Idx - 1 - MakeNoneIdx ( NoneIdx2Att ( Idx ), -1 ) );
	}

/*
┌────────────────────────────────────────────────────────┬───────────────────┐
│ Вставить новое высказывание из имеющихся в правило.    │ InsertProposition │
│                                                        └───────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::InsertProposition ( VOID ) {

	if ( KBase->GetCurRule() >= KBase->RuleNumber() ) return;
	if ( KBase->GetCurAtt() < 0 ) return;

	// Вызвать диалоговою процедуру и передать ей указатель на объект
	FARPROC lpPropProc = MakeProcInstance ( (FARPROC) PropositionProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "proposit", Frame.hwndFrame, lpPropProc, (DWORD) this );
	FreeProcInstance ( lpPropProc );
	if ( i == -1 ) return;                              // Диалог не состоялся

	// Произвести добавление аттрибута.
	int Ret = KBase->InsProp ( i );

	if ( Ret == LKE_ALLOCMEM ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	// Установить параметры высказывания
	if ( FullPath[0] == IDD_PREMISE ) KBase->InvertProp ( i );

	PrintOn();
	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Увеличить компоненту, соответствующую значению.              │ InsertValue │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::InsertValue ( VOID ) {
	if ( ( KBase->GetCurVal() < 0 ) || ( KBase->GetCurAtt() < 0 ) ||
         ( KBase->GetCurAtt() >= KBase->AttNumber() ) )
		return;

	if ( SplitState != IDM_VIEWNONE && CurFocus == hwndAtt ) return;

	COMP Comp;
	RECT rc;
	HWND hwnd;
	int Att = KBase->GetCurAtt();
	int Val = KBase->GetCurVal();

	if ( Val == -1 ) return;

	KBase->GetComp ( Att, Val, & Comp );
	if ( Comp.pos != UNITY )  Comp.pos = UNITY;
	else                      return;
	KBase->SetComp ( Att, Val, & Comp );

	if ( SplitState == IDM_VIEWNONE )  hwnd = hwndAtt;
	else                               hwnd = hwndVal;
	SendMessage ( hwnd, LB_GETITEMRECT, SendMessage ( hwnd, LB_GETCURSEL, 0, 0L ), (DWORD) & rc );
	if ( SplitState != IDM_VIEWNONE )
		rc.left = 3 * BMPWIDTH + 3;
	else
		rc.left = 5 * BMPWIDTH + 3;
	rc.right = rc.left + COMPLENGTH;
	InvalidateRect ( hwnd, & rc, TRUE );

	return;
	}

/*
┌────────────────────────────────────────────────────────┬───────────────────┐
│ Инвертировать высказывание.                            │ InvertProposition │
│                                                        └───────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::InvertProposition ( VOID ) {

	if ( ( KBase->GetCurAtt() < 0 ) || ( KBase->GetCurAtt() >= KBase->AttNumber() ) ) return;

	// Изменить конъюнкт/дизъюнкт и все компоненты
	KBase->InvertProp();

	// Заполнить ListBox'ы
	PrintOn();

	return;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Начать редактирование (явный ввод) значения компоненты.        │ EditValue │
│ Используется процедура ComponentProc.                          └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::EditValue ( VOID ) {
	// Создать окно EDIT без границ на месте компоненты
	// Передать управление этому окну и позволить ввести значение компоненты
	// После нажатия Enter вернуть назад, уничтожить окно
	// Полученное значение приравнять компоненте
	// ??????????????????????????????????????????????????????????
	return;
	}

/*
┌────────────────────────────────────────────────────────┬───────────────────┐
│ Удалить высказывание об атрибуте из правила.           │ DeleteProposition │
│                                                        └───────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DeleteProposition ( VOID ) {

	if ( ( KBase->GetCurAtt() < 0 ) || ( KBase->GetCurAtt() >= KBase->AttNumber() ) )
		return;

	if ( IDYES != ErrorMessager  ( hWnd, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_DELETEPROPOSITION ) )
		return;

	// Удалить высказывание
	KBase->DelProp();

	// Отразить текущее состояние на экране
	PrintOn();

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Уменьшить значение компоненты, соответствующей значению.     │ DeleteValue │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DeleteValue ( VOID ) {

	if ( ( KBase->GetCurVal() < 0 ) || ( KBase->GetCurAtt() < 0 ) ||
         ( KBase->GetCurAtt() >= KBase->AttNumber() ) )
		return;

	if ( SplitState != IDM_VIEWNONE && CurFocus == hwndAtt ) return;

	COMP Comp;
	RECT rc;
	HWND hwnd;
	int Att = KBase->GetCurAtt();
	int Val = KBase->GetCurVal();

	if ( Val == -1 ) return;

	KBase->GetComp ( Att, Val, & Comp );
	if ( Comp.pos != 0 )      Comp.pos = 0;
	else                      return;
	KBase->SetComp ( Att, Val, & Comp );

	if ( SplitState == IDM_VIEWNONE )  hwnd = hwndAtt;
	else                               hwnd = hwndVal;
	SendMessage ( hwnd, LB_GETITEMRECT, SendMessage ( hwnd, LB_GETCURSEL, 0, 0L ), (DWORD) & rc );
	if ( SplitState != IDM_VIEWNONE )
		rc.left = 3 * BMPWIDTH + 3;
	else
		rc.left = 5 * BMPWIDTH + 3;
	rc.right = rc.left + COMPLENGTH;
	InvalidateRect ( hwnd, & rc, TRUE );

	return;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ InsertRule │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::InsertRule ( VOID ) {
	int Ret = KBase->InsRule();
	if ( Ret != LKE_SUCCESS ) {                  //???????
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	// Переинициализировать scrollbar
	SetScrollRange ( hWnd, SB_VERT, 0, KBase->RuleNumber(), FALSE );
	SetScrollPos ( hWnd, SB_VERT, KBase->GetCurRule(), TRUE );

	PrintOn();
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ DeleteRule │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DeleteRule ( VOID ) {
	// Нельзя удалить то, чего нет !
	if ( KBase->GetCurRule() >= KBase->RuleNumber() ) return;

	if ( IDYES != ErrorMessager  ( hWnd, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_DELETERULE ) )
		return;

	int Ret = KBase->DelRule();
	if ( Ret != LKE_SUCCESS ) {                  //???????
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	// Переинициализировать scrollbar
	SetScrollRange ( hWnd, SB_VERT, 0, KBase->RuleNumber(), FALSE );
	SetScrollPos ( hWnd, SB_VERT, KBase->GetCurRule(), TRUE );

	PrintOn();
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Увеличить текущую компоненту на 1. Если она равна UNITY, то │ IncreaseComp │
│ не увеличивать.                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::IncreaseComp ( VOID ) {
	if ( SplitState != IDM_VIEWNONE && CurFocus == hwndAtt ) return;

	COMP Comp;
	RECT rc;
	HWND hwnd;
	int Att = KBase->GetCurAtt();
	int Val = KBase->GetCurVal();

	if ( Val == -1 ) return;

	KBase->GetComp ( Att, Val, & Comp );
	if ( Comp.pos != UNITY )  Comp.pos++;
	else                      return;
	KBase->SetComp ( Att, Val, & Comp );

	if ( SplitState == IDM_VIEWNONE )  hwnd = hwndAtt;
	else                               hwnd = hwndVal;
	SendMessage ( hwnd, LB_GETITEMRECT, SendMessage ( hwnd, LB_GETCURSEL, 0, 0L ), (DWORD) & rc );
	if ( SplitState != IDM_VIEWNONE )
		rc.left = 3 * BMPWIDTH;
	else
		rc.left = 5 * BMPWIDTH;
	rc.right = rc.left + COMPLENGTH;
	InvalidateRect ( hwnd, & rc, TRUE );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Уменьшить текущую компоненту на 1. Если она равна 0, то     │ DecreaseComp │
│ не уменьшать.                                               └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DecreaseComp ( VOID ) {
	if ( SplitState != IDM_VIEWNONE && CurFocus == hwndAtt ) return;

	COMP Comp;
	RECT rc;
	HWND hwnd;
	int Att = KBase->GetCurAtt();
	int Val = KBase->GetCurVal();

	if ( Val == -1 ) return;

	KBase->GetComp ( Att, Val, & Comp );
	if ( Comp.pos != 0 )      Comp.pos--;
	else                      return;
	KBase->SetComp ( Att, Val, & Comp );

	if ( SplitState == IDM_VIEWNONE )  hwnd = hwndAtt;
	else                               hwnd = hwndVal;
	SendMessage ( hwnd, LB_GETITEMRECT, SendMessage ( hwnd, LB_GETCURSEL, 0, 0L ), (DWORD) & rc );
	if ( SplitState != IDM_VIEWNONE )
		rc.left = 3 * BMPWIDTH;
	else
		rc.left = 5 * BMPWIDTH;
	rc.right = rc.left + COMPLENGTH;
	InvalidateRect ( hwnd, & rc, TRUE );
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Выполняется при изменении компоненты. Находит положение  │ ChangeComponent │
│ заданной компоненты и инвалидирует его.                  └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::ChangeComponent ( int Att, int Val ) {
/*
Положение зависит от вида. В двуоконном режиме если текущий атрибут не равен
заданному, то ничего не делать, т.к., компоненты просто не видно на экране.
Иначе необходимо найти позицию заданного значения (преобразовав относительный
номер в абсолютный) и если она видна на экране, то инвалидировать ее.

В случае однооконного режима необходимо по заданным атрибуту и значению найти
позицию компоненты в списке (с помощью функции MakeNoneIdx) и инвалидировать ее.

Вообще, инвалидировать можно не весь прямоугольник строки, а только ту часть,
которая содержит компоненту.
*/
	HWND hwnd;
	int  idx;
	RECT rc;

	if ( SplitState == IDM_VIEWNONE ) {
		hwnd = hwndAtt;
		idx = MakeNoneIdx ( Att, Val );
		}
	else {
		hwnd = hwndVal;
		idx = Val;
		}

	SendMessage ( hwnd, LB_GETITEMRECT, idx, (LPARAM) & rc );

	if ( SplitState == IDM_VIEWNONE )
		rc.left = 5 * BMPWIDTH;
	else
		rc.left = 3 * BMPWIDTH;

	rc.right = rc.left + COMPLENGTH;

	InvalidateRect ( hwnd, & rc, TRUE );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│                                                                  │ Compile │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::Compile ( VOID ) {
	BOOL isCompiled;

	FullPath[0] = 0;

	EnableWindow ( Frame.hwndFrame, FALSE );

	// Создать диалог и передать указатель на FullPath[0];
	// Если в диалоге нажато Cancel, то устанавливается FullPath[0]
	FARPROC lpStatProc = MakeProcInstance ( (FARPROC) StatusDlgProc, Frame.hInstance );
	Frame.hwndStatus = CreateDialogParam ( Frame.hInstance, "STATCOMP", Frame.hwndFrame, lpStatProc, (DWORD) this );
	// Теперь на экране появилось окно статистики (диалог) с кнопкой Cancel

	FARPROC lpProcOut = MakeProcInstance ( (FARPROC) CompileOutProc, Frame.hInstance );

	// Запустить компиляцию с параметром-функцией выдачи статистики и проверки на прерывание
	// Если lpProcOut возвращает TRUE, то компиляция прерывается
	isCompiled = KBase->lpHost->CompileToCNF ( (COMPILEOUT) lpProcOut );
	// Функция статистики вызывается периодически из функции компиляции,
	// рисует цифры и смотрит не установлено ли FullPath[0]
	// Если да, то возвращается TRUE (т.е., компиляция прервана)
	// Именно функция статистики дает возможность выполниться другим программам
	// включая отслеживание действий (например, нажатие Cancel) в самом диалоге статистике.

	FreeProcInstance ( lpProcOut );

	// Компиляция закончена или прервана, но мы остаемся в диалоге и ждем
	// нажатия OK, т.е. организуем свой цикл обработки сообщений, который необходим
	// поскольку окно статистики -- это обычное окно, а из него надо сделать диалог.

	if ( isCompiled ) {                          // Компиляция прошла до конца

		// Заменить кнопку Cancel на OK
		ShowWindow ( GetDlgItem ( Frame.hwndStatus, IDCANCEL ), SW_HIDE );
		ShowWindow ( GetDlgItem ( Frame.hwndStatus, IDOK ), SW_SHOW );
		::SetFocus ( Frame.hwndStatus );

		// Организовать бесконечный цикл обработки сообщений. Прервать его
		// только если в диалоге нажато OK, о чем мы узнаем по FullPath[0]
		MSG  msg;

		while ( TRUE ) {
			// Если есть сообщение, то обработать его путем предварительной фильтрации
			if ( PeekMessage ( & msg, NULL, 0, 0, PM_REMOVE ) ) {
				if ( msg.message == WM_QUIT ) break;    // Этого не может быть

				if ( Frame.hwndStatus == 0 || ! IsDialogMessage ( Frame.hwndStatus, & msg ) )
					if ( ! TranslateMDISysAccel ( Frame.hwndClient, & msg ) && ! TranslateAccelerator ( Frame.hwndFrame, Frame.hAccel, & msg ) ) {
						TranslateMessage ( & msg );
						DispatchMessage ( & msg );
						}
				}

			else {
				// Свободное для выполнения левой работы время
				// но мы ничего не делаем, а просто возобновляем цикл обработки сообщений
				// Прерывается цикл только если в диалоге нажато OK
				if ( FullPath[0] ) break;
				}
			}

		}

	EnableWindow ( Frame.hwndFrame, TRUE );

	// Уничтожить диалог
	DestroyWindow ( Frame.hwndStatus );
	FreeProcInstance ( lpStatProc );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ InitMenu │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::InitMenu ( VOID ) {

	// Отметить состояние LISTBOX'ов
	if ( SplitState == IDM_VIEWVERTICAL ) {
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWVERTICAL, MF_CHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWHORIZONTAL, MF_UNCHECKED );
		}
	else if ( SplitState == IDM_VIEWHORIZONTAL ) {
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWVERTICAL, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWHORIZONTAL, MF_CHECKED );
		}
	else if ( SplitState == IDM_VIEWNONE ) {
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWVERTICAL, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VIEWHORIZONTAL, MF_UNCHECKED );
		}

	if ( isAttSelected() ) {

		// Запретить пункты меню не относящиеся к этому окну
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_ASINRULE, MF_ENABLED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES, MF_ENABLED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER, MF_GRAYED );

		// Отметить текущую сортировку
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_ASINRULE,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_UNCHECKED );

		switch ( KBase->GetAttSort() ) {
			case - SORT_ASINRULE :
			case SORT_ASINRULE   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_ASINRULE,   MF_CHECKED );
				break;
				}
			case - SORT_UNSORTED :
			case SORT_UNSORTED   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_CHECKED );
				break;
				}
			case - SORT_ALPHA    :
			case SORT_ALPHA      : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_CHECKED );
				break;
				}
			case - SORT_VALNUM   :
			case SORT_VALNUM     : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_CHECKED );
				break;
				}
			case - SORT_PRICE    :
			case SORT_PRICE      : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_CHECKED );
				break;
				}
			case - SORT_IMPORTANCE :
			case SORT_IMPORTANCE   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_CHECKED );
				break;
				}
/*			case - SORT_CHARACTER:
			case SORT_CHARACTER  : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_CHECKED );
				break;
				}
*/			}

	   	 // Отметить присутствие обратной сортировки
		if ( KBase->GetAttSort() < 0 )
			CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_CHECKED );
		else
			CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_UNCHECKED );

		}
	else {                                                // Для окна значений

		// Запртеить пункты меню не относящиеся к этому окну
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_ASINRULE, MF_GRAYED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES, MF_GRAYED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER, MF_ENABLED );

		// Отметить текущую сортировку
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_ASINRULE,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_UNCHECKED );

		switch ( KBase->GetValSort() ) {
/*			case - IDM_ASINRULE :
			case IDM_ASINRULE   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_CHECKED );
				break;
				}
*/			case - SORT_UNSORTED :
			case SORT_UNSORTED   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_CHECKED );
				break;
				}
			case - SORT_ALPHA    :
			case SORT_ALPHA      : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_CHECKED );
				break;
				}
			case - SORT_VALNUM   :
			case SORT_VALNUM     : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_CHECKED );
				break;
				}
			case - SORT_PRICE    :
			case SORT_PRICE      : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_CHECKED );
				break;
				}
			case - SORT_IMPORTANCE :
			case SORT_IMPORTANCE   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_CHECKED );
				break;
				}
			case - SORT_CHARACTER :
			case SORT_CHARACTER   : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_CHECKED );
				break;
				}
			}

	   	 // Отметить присутствие обратной сортировки
		if ( KBase->GetValSort() < 0 )
			CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_CHECKED );
		else
			CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_UNCHECKED );

		}

	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ DrawItemBmp │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DrawItemBmp ( LPDRAWITEMSTRUCT lpDIS, int WhatToDraw, BOOL Last ) {

	HPEN SavePen = SelectObject ( lpDIS->hDC, CreatePen ( PS_SOLID, 1, RGB(0,0,255) ) );
	HDC hdcCompat = CreateCompatibleDC ( lpDIS->hDC );

	// Подготавливаем битмапы для рисования
	// Для последнего (вспомогательного) битмап не загружается т.к. не используется
	// В принципе, можно было бы и загрузить (без использования), но функция
	// isPropConjunct для вспомогательного атрибута дает ошибку.
	// (Если атрибут последний, то загружается битмап значений и не используется.)
	if ( WhatToDraw == IDM_ATT_LISTBOX && ! Last )

		if ( SplitState == IDM_VIEWNONE )
			if ( KBase->isPropConjunct ( NoneIdx2Att ( lpDIS->itemID ) ) )
				SelectObject ( hdcCompat, hbmpAttIn );       // конъюнктивное высказывание
			else
				SelectObject ( hdcCompat, hbmpAttOut );      // дизъюнктивное высказывание
		else
			if ( KBase->isPropConjunct ( KBase->Prop2Att ( lpDIS->itemID ) ) )
				SelectObject ( hdcCompat, hbmpAttIn );
			else
				SelectObject ( hdcCompat, hbmpAttOut );

	else
		SelectObject ( hdcCompat, hbmpVal );

	if ( SplitState != IDM_VIEWNONE )                      // Двуоконный режим

		if ( Last ) {                                            // Рисуем └─
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );

			if ( WhatToDraw == IDM_VAL_LISTBOX )             // Добавляем └─■
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
        	}
		else if ( lpDIS->itemID == 0 ) {                        // Рисуем ┌─■
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.bottom );
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
			}
        else {                                                  // Рисуем ├─■
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5, 10, 10, hdcCompat, 0, 0, SRCCOPY );
        	}

	else                                                  // Однооконный режим
		if ( WhatToDraw == IDM_ATT_LISTBOX )                 // Рисуем атрибут
			if ( Last ) {                                     // Рисуем └───■
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				// Картинка для последнего (вспомогаетльного) атрибута (пока нет)
				// BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
   		 		}
			else {                                            // Рисуем ├─┬─■
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5, 10, 10, hdcCompat, 0, 0, SRCCOPY );
       		 	}
		else                                                // Рисуем значение
			if ( Last ) {                                     // Рисуем │ └─■
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
   		 		}
			else {                                            // Рисуем │ ├─■
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5, 10, 10, hdcCompat, 0, 0, SRCCOPY );
       		 	}

	DeleteObject ( SelectObject ( lpDIS->hDC, SavePen ) );
	DeleteDC ( hdcCompat );

	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ DrawItemText │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DrawItemText ( LPDRAWITEMSTRUCT lpDIS, int WhatToDraw, int Pos ) {
	char buf[128];
	HBRUSH hCompBrush, hTextBrush;
	RECT rcComp = lpDIS->rcItem;  rcComp.left += Pos; rcComp.right = rcComp.left + COMPLENGTH;
	RECT rcText = lpDIS->rcItem;  rcText.left += Pos + COMPLENGTH;
	COLORREF hTextClr, hTextBkg, hCompBkg, hCompClr = RGB(255,0,0);

	hCompBrush = CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
	hCompBkg = GetSysColor ( COLOR_WINDOW );

	if ( lpDIS->itemState & ODS_SELECTED ) {
		hTextBrush = CreateSolidBrush ( GetSysColor ( COLOR_HIGHLIGHT ) );
		hTextClr = GetSysColor ( COLOR_HIGHLIGHTTEXT );
		hTextBkg = GetSysColor ( COLOR_HIGHLIGHT );
		}
	else {
		hTextBrush = hCompBrush;
		hTextClr = GetSysColor ( COLOR_WINDOWTEXT );
		hTextBkg = hCompBkg;
		}

	// Все подготовили, начинаем рисовать
	if ( WhatToDraw == IDM_VAL_LISTBOX ) {

		// Получить значение компоненты и преобразовать его в текст
		COMP Comp;
		int Att, Val;
		if ( SplitState != IDM_VIEWNONE ) {
			Att = KBase->GetCurAtt();
			Val = lpDIS->itemID;
			}
		else {
			Att = NoneIdx2Att ( lpDIS->itemID );
			Val = NoneIdx2Val ( lpDIS->itemID );
			}
		// Вспомогательная компонента не показывается, поэтому ошибки не будет
		KBase->GetComp ( Att, Val, & Comp );            // Получить компоненту
		itoa ( (int) Comp.pos, buf, 10 );

		// Подготовить область для рисования, т.е., закрасить ее нужным цветом в зависимости от выделения
		FillRect ( lpDIS->hDC, & rcComp, hCompBrush );
		FillRect ( lpDIS->hDC, & rcText, hTextBrush );

		// Установить цвета для рисования компоненты
		SetTextColor ( lpDIS->hDC, hCompClr );
		SetBkColor ( lpDIS->hDC, hCompBkg );

		TextOut ( lpDIS->hDC, rcComp.left + 3, rcComp.top, buf, lstrlen ( buf ) );

		// Установить цвета для рисования текста
		SetTextColor ( lpDIS->hDC, hTextClr );
		SetBkColor ( lpDIS->hDC, hTextBkg );

		TextOut ( lpDIS->hDC, rcText.left + 3, rcText.top, buf,
			(int) SendMessage ( lpDIS->hwndItem, LB_GETTEXT, lpDIS->itemID, (LPARAM) buf )
			);
		}
	else {
		// Подготовить область для рисования, т.е., закрасить ее нужным цветом в зависимости от выделения
		rcText = lpDIS->rcItem;
		rcText.left += Pos;
		FillRect ( lpDIS->hDC, & rcText, hTextBrush );

		// Установить цвета для рисования текста
		SetTextColor ( lpDIS->hDC, hTextClr );
		SetBkColor ( lpDIS->hDC, hTextBkg );

		TextOut ( lpDIS->hDC, rcText.left + 3, rcText.top, buf,
			(int) SendMessage ( lpDIS->hwndItem, LB_GETTEXT, lpDIS->itemID, (DWORD) buf ) );

		}

	DeleteObject ( hCompBrush );
	if ( lpDIS->itemState & ODS_SELECTED )
		DeleteObject ( hTextBrush );

	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ DrawItem │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::DrawItem ( LPDRAWITEMSTRUCT lpDIS ) {

	if ( SplitState != IDM_VIEWNONE ) {
		RECT rc = lpDIS->rcItem;

		if ( lpDIS->CtlID == IDM_ATT_LISTBOX )  rc.left += 3 * BMPWIDTH;
		else                                    rc.left += 3 * BMPWIDTH + COMPLENGTH;

		switch ( lpDIS->itemAction ) {

		case ODA_SELECT :
			// Выполняем полную программу. Это немного неэффективно (битмэпы
			// рисовать в действительности не надо, а только текст), но зато просто.

			// Пристегните ремни и дышите глубже; проваливаемся в следующий кейс...

		case ODA_DRAWENTIRE : {                  // Нарисовать полностью пункт

			if ( lpDIS->CtlID == IDM_ATT_LISTBOX ) {// Рисуем в окне атрибутов

				if ( lpDIS->itemID == KBase->PropNumber() )  // Пустой атрибут
					DrawItemBmp ( lpDIS, lpDIS->CtlID, TRUE );
				else                                     // Нормальный атрибут
					DrawItemBmp ( lpDIS, lpDIS->CtlID, FALSE );

				DrawItemText ( lpDIS, IDM_ATT_LISTBOX, 3 * BMPWIDTH );
				}
	        else {                                   // Рисуем в окне значений

				if ( lpDIS->itemID == KBase->ValNumber ( KBase->GetCurAtt() ) - 1 )
					DrawItemBmp ( lpDIS, lpDIS->CtlID, TRUE );   // Последнее значение
				else
					DrawItemBmp ( lpDIS,  lpDIS->CtlID, FALSE );// Нормальное значение

				DrawItemText ( lpDIS, IDM_VAL_LISTBOX, 3 * BMPWIDTH );
				}
			break;
			}                                           // case ODA_DRAWENTIRE

		case ODA_FOCUS : {
			// Поставить фокус
			DrawFocusRect ( lpDIS->hDC, & rc );
			break;
			}
		}                                      // switch ( lpDIS->itemAction )

        }                                 // if ( SplitState != IDM_VIEWNONE )

	else {                                              // Рисуем при NONEVIEW
		RECT rc = lpDIS->rcItem;

		if ( NoneIdx2Val ( lpDIS->itemID ) == -1 )  rc.left += 4 * BMPWIDTH;
		else                                        rc.left += 5 * BMPWIDTH + COMPLENGTH;

		switch ( lpDIS->itemAction ) {

		case ODA_SELECT :
			// Выполняем полную программу. Это немного неэффективно (битмэпы
			// рисовать в действительности не надо, а только текст), но зато просто.

			// Пристегните ремни и дышите глубже; проваливаемся в следующий кейс...

		case ODA_DRAWENTIRE : {                  // Нарисовать полностью пункт

			if ( NoneIdx2Val ( lpDIS->itemID ) == -1 ) {     // Рисуем атрибут

				if ( NoneIdx2Att ( lpDIS->itemID ) == KBase->AttNumber() ) // Пустой атрибут
					DrawItemBmp ( lpDIS, IDM_ATT_LISTBOX, TRUE );
				else                                     // Нормальный атрибут
					DrawItemBmp ( lpDIS, IDM_ATT_LISTBOX, FALSE );

				DrawItemText ( lpDIS, IDM_ATT_LISTBOX, 4 * BMPWIDTH );
				}

			else {                                          // Рисуем значение
				if ( NoneIdx2Val ( lpDIS->itemID ) == KBase->ValNumber ( NoneIdx2Att ( lpDIS->itemID ) ) - 1 )
					DrawItemBmp ( lpDIS, IDM_VAL_LISTBOX, TRUE );// Последнее значение
				else
					DrawItemBmp ( lpDIS, IDM_VAL_LISTBOX, FALSE ); // Нормальное значение

				DrawItemText ( lpDIS, IDM_VAL_LISTBOX, 5 * BMPWIDTH );
				}
			break;
			}                                           // case ODA_DRAWENTIRE

		case ODA_FOCUS : {
			// Поставить фокус
			DrawFocusRect ( lpDIS->hDC, & rc );
			break;
			}
		}                                      // switch ( lpDIS->itemAction )
		}

	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ ChangeWin │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::ChangeWin ( VOID ) {
	if ( SplitState != IDM_VIEWNONE ) {
        if ( CurFocus == IDM_ATT_LISTBOX ) CurFocus = IDM_VAL_LISTBOX;
        else CurFocus = IDM_ATT_LISTBOX;
    	SetFocus ( );
    	}
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ isAttSelected │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KNOW::isAttSelected ( VOID ) {
	if ( ( ( SplitState != IDM_VIEWNONE ) && ( CurFocus == IDM_ATT_LISTBOX ) ) ||
           ( SplitState == IDM_VIEWNONE ) && ( KBase->GetCurVal() == -1 ) )
		return TRUE;
	else
		return FALSE;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ isValSelected │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KNOW::isValSelected ( VOID ) {
	if ( ( ( SplitState != IDM_VIEWNONE ) && ( CurFocus == IDM_VAL_LISTBOX ) ) ||
           ( SplitState == IDM_VIEWNONE ) && ( KBase->GetCurVal() != -1 ) )
		return TRUE;
	else
		return FALSE;
	}

/*
┌─────────────────────────────────────────────────────────┬──────────────────┐
│ Заполняет ListBox в диалоге hwnd именами атрибутов,     │ FillPropositions │
│ которые не встречаются в текущем правиле.               └──────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KNOW::FillPropositions ( HWND hwnd ) {

	int i, s, w = 0, maxWidth = 0;
	if ( SendDlgItemMessage ( hwnd, IDD_UNSORTED,   BM_GETCHECK, 0, 0L ) ) s = SORT_UNSORTED;
	if ( SendDlgItemMessage ( hwnd, IDD_NAME,       BM_GETCHECK, 0, 0L ) ) s = SORT_ALPHA;
	if ( SendDlgItemMessage ( hwnd, IDD_VALNUM,     BM_GETCHECK, 0, 0L ) ) s = SORT_VALNUM;
	if ( SendDlgItemMessage ( hwnd, IDD_PRICE,      BM_GETCHECK, 0, 0L ) ) s = SORT_PRICE;
	if ( SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, BM_GETCHECK, 0, 0L ) ) s = SORT_IMPORTANCE;

	if ( SendDlgItemMessage ( hwnd, IDD_REVERSE, BM_GETCHECK, 0, 0L ) )
		KBase->SetAttSort ( - s );
	else
		KBase->SetAttSort ( s );

	// Заполнить ListBox именами атрибутов
	SendDlgItemMessage ( hwnd, IDD_PROP, WM_SETREDRAW, FALSE, 0L );
	SendDlgItemMessage ( hwnd, IDD_PROP, LB_RESETCONTENT, 0, 0L );
	HDC hDC = GetDC ( GetDlgItem ( hwnd, IDD_PROP ) );

	for ( i = 0; i < KBase->AttNumber(); i++ ) {

		if ( ! KBase->isAttEnabled ( i ) ) {
			ATT_REC  AttRec;
			KBase->GetAttribute ( i, & AttRec );

			// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
			w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & AttRec.Name, lstrlen ( (LPSTR) & AttRec.Name) ) );
			if ( maxWidth < w ) maxWidth = w;

			// Занести атрибут в ListBox
			SendDlgItemMessage ( hwnd, IDD_PROP, LB_ADDSTRING, 0, (DWORD) & AttRec.Name );
			}
		}
	ReleaseDC ( GetDlgItem ( hwnd, IDD_PROP ), hDC );
	SendDlgItemMessage ( hwnd, IDD_PROP, LB_SETHORIZONTALEXTENT, maxWidth+5, 0 );

	SendDlgItemMessage ( hwnd, IDD_PROP, LB_SETCURSEL, 0, 0 );
	SendDlgItemMessage ( hwnd, IDD_PROP, WM_SETREDRAW, TRUE, 0L );
	InvalidateRect ( GetDlgItem ( hwnd, IDD_PROP ), NULL, FALSE );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Процедура ввода (заполнения) значения компоненты.          │ ComponentProc │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export ComponentProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPVAL_REC ValRec;

    switch ( msg ) {
		case WM_INITDIALOG : {
			char buf[16];

			ValRec = (LPVAL_REC) lParam;

			SendDlgItemMessage ( hwnd, IDD_NAME, WM_SETTEXT, 0, (DWORD) ValRec->Name );
			SendDlgItemMessage ( hwnd, IDD_NAME, EM_LIMITTEXT, sizeof(ValRec->Name) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_SETTEXT, 0, (DWORD) ValRec->Question );
			SendDlgItemMessage ( hwnd, IDD_QUESTION, EM_LIMITTEXT, sizeof(ValRec->Question) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_SETTEXT, 0, (DWORD) ValRec->Comment );
			SendDlgItemMessage ( hwnd, IDD_COMMENT, EM_LIMITTEXT, sizeof(ValRec->Comment) - 1, 0L );

			// Инициализировать цену значения (преобразовать в строку)
			itoa ( (int) ValRec->Price, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_PRICE, WM_SETTEXT, 0, (DWORD) buf );
			SendDlgItemMessage ( hwnd, IDD_PRICE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать важность значения (преобразовать в строку)
			itoa ( (int) ValRec->Importance, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_SETTEXT, 0, (DWORD) buf );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать характерность значения (преобразовать в строку)
			itoa ( (int) ValRec->Character, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_CHARACTER, WM_SETTEXT, 0, (DWORD) buf );
			SendDlgItemMessage ( hwnd, IDD_CHARACTER, EM_LIMITTEXT, 3, 0L );

			LocalUnlock ( LOWORD(lParam) );
			break;
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					char buf[16];

					SendDlgItemMessage ( hwnd, IDD_NAME, WM_GETTEXT, sizeof(ValRec->Name), (DWORD) ValRec->Name );
					SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_GETTEXT, sizeof(ValRec->Question), (DWORD) ValRec->Question );
					SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_GETTEXT, sizeof(ValRec->Comment), (DWORD) ValRec->Comment );

					// Получить цену значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_PRICE, WM_GETTEXT, sizeof(buf), (DWORD) buf );
					ValRec->Price = (char) atoi ( buf );

					// Получить важность значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_GETTEXT, sizeof(buf), (DWORD) buf );
					ValRec->Importance = (char) atoi ( buf );

					// Получить характерность значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_CHARACTER, WM_GETTEXT, sizeof(buf), (DWORD) buf );
					ValRec->Character = (char) atoi ( buf );

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
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Функция выбора атрибута из списка для вставки высказыва- │ PropositionProc │
│ ния в правило. Получает указатель на объект окна правил  └─────────────────┤
│ и возвращает номер выбранного пользователем атрибута. В списке атрибутов   │
│ указываются только те, которых еще нет в текущем правиле.                  │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export PropositionProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPKNOW pKnow;
	static int PrevSort;

    switch ( msg ) {
		case WM_INITDIALOG : {

			pKnow = (LPKNOW) lParam;
			PrevSort = pKnow->KBase->GetAttSort();

			// Показать текущую сортировку и положительность
			if ( PrevSort < 0 )
				SendDlgItemMessage ( hwnd, IDD_REVERSE, BM_SETCHECK, 1, 0L );
			else
				SendDlgItemMessage ( hwnd, IDD_REVERSE, BM_SETCHECK, 0, 0L );

			SendDlgItemMessage ( hwnd, IDD_CONCLUSION, BM_SETCHECK, 1, 0L );

			int i;
			switch ( PrevSort ) {
				case - SORT_ASINRULE   :
				case SORT_ASINRULE     : i = IDD_UNSORTED;   break;
				case - SORT_UNSORTED   :
				case SORT_UNSORTED     : i = IDD_UNSORTED;   break;
				case - SORT_ALPHA      :
				case SORT_ALPHA        : i = IDD_NAME;       break;
				case - SORT_VALNUM     :
				case SORT_VALNUM       : i = IDD_VALNUM;     break;
				case - SORT_PRICE      :
				case SORT_PRICE        : i = IDD_PRICE;      break;
				case - SORT_IMPORTANCE :
				case SORT_IMPORTANCE   : i = IDD_IMPORTANCE; break;
				}
			SendDlgItemMessage ( hwnd, i, BM_SETCHECK, 1, 0L );

			pKnow->FillPropositions ( hwnd );
			::SetFocus ( GetDlgItem ( hwnd, IDD_PROP ) );
			break;
			}
		case WM_COMMAND : {
			switch ( wParam ) {
				case IDOK : {
					char Name[ATT_NAME_LEN];

					if ( -1 == SendDlgItemMessage ( hwnd, IDD_PROP, LB_GETCURSEL, 0, 0L ) ) {
						EndDialog ( hwnd, -1 );
						break;
						}

					// Взять строку имени атрибута
					SendDlgItemMessage (
						hwnd, IDD_PROP, LB_GETTEXT,
                        SendDlgItemMessage ( hwnd, IDD_PROP, LB_GETCURSEL, 0, 0L ),
						(DWORD) Name
						);
					// Установить исходную сортировку
					pKnow->KBase->SetAttSort ( PrevSort );

					if ( SendDlgItemMessage ( hwnd, IDD_PREMISE, BM_GETCHECK, 0, 0L ) )
						FullPath[0] = IDD_PREMISE;
					else
						FullPath[0] = IDD_CONCLUSION;

					// Определить по имени номер атрибута
					EndDialog ( hwnd, pKnow->KBase->Name2Att ( Name ) );
					break;
					}
				case IDCANCEL : {
					pKnow->KBase->SetAttSort ( PrevSort );
					EndDialog ( hwnd, -1 );
					break;
					}
				case IDD_UNSORTED : {
					pKnow->FillPropositions ( hwnd );
					break;
					}
				case IDD_NAME : {
					pKnow->FillPropositions ( hwnd );
					break;
					}
				case IDD_PRICE : {
					pKnow->FillPropositions ( hwnd );
					break;
					}
				case IDD_IMPORTANCE : {
					pKnow->FillPropositions ( hwnd );
					break;
					}
				case IDD_VALNUM : {
					pKnow->FillPropositions ( hwnd );
					break;
					}
				case IDD_REVERSE : {
					pKnow->FillPropositions ( hwnd );
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
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Управление диалогом состояния компиляции                   │ StatusDlgProc │
│                                                            └───────────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export StatusDlgProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPKNOW pKnow;

    switch ( msg ) {
		case WM_INITDIALOG : {
			char str[16];

			pKnow = (LPKNOW) lParam;

			itoa ( pKnow->KBase->RuleNumber(), str, 10 );
			SetDlgItemText ( hwnd, IDD_TOTINPUT, (LPSTR) str );

			ShowWindow ( GetDlgItem ( hwnd, IDOK ), SW_HIDE );
			ShowWindow ( GetDlgItem ( hwnd, IDCANCEL ), SW_SHOW );

			break;
			}
		case WM_COMMAND : {
			if ( wParam == IDOK ) {
					FullPath[0] = 1;
					break;
					}
			if ( wParam == IDCANCEL ) {
					FullPath[0] = 1;
					break;
					}
			return FALSE;
			}

		default :
			return FALSE ;
		}
	return TRUE;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ KnowWndProc │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
long FAR PASCAL _export KnowWndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	LPKNOW tis = (LPKNOW) GetMem ( hwnd );

    switch ( msg ) {

		case WM_GETMINMAXINFO  : {
			((LPPOINT) lParam)[3].x = 64;
			((LPPOINT) lParam)[3].y = 64;
			break;
			}
		case WM_CREATE         : {
			SetMem ( hwnd, (LPVOID) ( (LPMDICREATESTRUCT) ( (LPCREATESTRUCT) lParam )->lpCreateParams )->lParam );
			break;
			}
		case WM_MEASUREITEM    : {
			// Одинаковая высота для атрибутов и значений
			((LPMEASUREITEMSTRUCT) lParam) -> itemHeight = Frame.LBitemHight;
			break;
			}
		case WM_DRAWITEM       : {
			tis->DrawItem ( (LPDRAWITEMSTRUCT) lParam );
			break;
			}
		case WM_MDIACTIVATE    : {
			if ( wParam == TRUE ) {
				// Загрузить соответствующее меню, акселератор и курсор
				SendMessage ( Frame.hwndClient, WM_MDISETMENU, FALSE, MAKELONG ( KNOW::hMenu, KNOW::hWndMenu ) );
				Frame.hAccel = KNOW::hAccel;
				if ( tis->SplitState == IDM_VIEWHORIZONTAL )
					SetClassWord ( hwnd, GCW_HCURSOR, KNOW::HorCur );
				else
					SetClassWord ( hwnd, GCW_HCURSOR, KNOW::VerCur );
				Frame.hwndActive = hwnd;
//				tis->SetFocus ( );
			}
			if ( wParam == FALSE ) {
				// Загрузить фреймовые меню и акселератор (при отсутствии окон)
				SendMessage ( Frame.hwndClient, WM_MDISETMENU, FALSE, MAKELONG ( Frame.FrmMenu, Frame.WndMenu ) );
				Frame.hAccel = Frame.hFrmAccel;
				Frame.hwndActive = 0;
				}
			DrawMenuBar ( Frame.hwndFrame );
			break;
			}
		case WM_SETFOCUS : {
			tis->SetFocus();
			break;
			}
		case WM_INITMENU : {
			tis->InitMenu();
			break;
			}
		case WM_SIZE     : {
			tis->Size ( wParam, lParam );
			break;
			}
		case WM_LBUTTONDOWN : {
			tis->PartMove ( WM_LBUTTONDOWN, lParam );
			break;
			}
		case WM_MOUSEMOVE : {
			tis->PartMove ( WM_MOUSEMOVE, lParam );
			break;
			}
		case WM_LBUTTONUP : {
			tis->PartMove ( WM_LBUTTONUP, lParam );
			break;
			}
		case WM_VSCROLL :   {
			switch ( wParam ) {
				case SB_BOTTOM   : SendMessage ( hwnd, WM_COMMAND, IDM_FIRSTRULE, 0 ); break;
				case SB_TOP      : SendMessage ( hwnd, WM_COMMAND, IDM_LASTRULE, 0 ); break;
				case SB_PAGEDOWN :
				case SB_LINEDOWN : SendMessage ( hwnd, WM_COMMAND, IDM_NEXTRULE, 0 ); break;
				case SB_PAGEUP   :
				case SB_LINEUP   : SendMessage ( hwnd, WM_COMMAND, IDM_PREVRULE, 0 ); break;
				case SB_THUMBPOSITION :
					if ( tis->KBase->GetCurRule() != LOWORD(lParam) ) {
						tis->KBase->SetCurRule ( LOWORD(lParam) );
						tis->KBase->SetCurAttPos ( -1 ); tis->KBase->SetCurVal ( -1 );
						SetScrollPos ( tis->hWnd, SB_VERT, tis->KBase->GetCurRule(), TRUE );
						tis->PrintOn();
						}
					break;
				default          : goto CallDCP;
				}
			break;
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case CHANGE_TITLE    : {
					char buf[80] = TITLE_KNOW;
					lstrcat ( buf, tis->KBase->GetNamePtr() );
					SetWindowText ( hwnd, buf );
					break;
					}
				case CHANGE_COMPONENT: {
					// Изменилась компонента атрибута LOWORD(lParam) и значения HIWORD(lParam)
					tis->ChangeComponent ( LOWORD(lParam), HIWORD(lParam) );
					break;
					}
				case CHANGE_CONTENT  :
				case PUT_ATTRIBUTE   :         // Могло измениться название и сортировка
//				case INS_ATTRIBUTE   :                             // Не нужно
				case DEL_ATTRIBUTE   :         // Может исчезнуть высказывание
				case PUT_VALUE       :         // Может измениться название и сортировка
				case INS_VALUE       :       // Может появиться новое значение
				case DEL_VALUE       :             // Может исчезнуть значение
//				case INS_RULE        :                      // Не используется
				case DEL_RULE        :
				case INS_PROP        :
				case DEL_PROP        :
				case INVERT_PROP     : {
					tis->PrintOn();
					break;
					}
				case IDM_CHANGEWINDOW: {
					tis->ChangeWin();
					break;
					}
				case IDM_FILESAVE       : {
					tis->Save();
					break;
					}
				case IDM_FILESAVEAS     : {
					if ( FileSaveAs() ) {
						tis->KBase->RenameKBase ( FullPath );
						if ( LKE_SUCCESS != tis->KBase->SaveKBase() ) ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTSAVE );
						}
					break;
					}
				case IDM_VIEWHORIZONTAL : {
					tis->View ( IDM_VIEWHORIZONTAL );
					break;
					}
				case IDM_VIEWVERTICAL   : {
					tis->View ( IDM_VIEWVERTICAL );
					break;
					}
				case IDM_ASINRULE       : {         // От пункта меню Unsorted
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_ASINRULE );
						else                                tis->KBase->SetAttSort ( - SORT_ASINRULE );
					else ;                // Для значений такой сортировки нет
					tis->PrintOn();
					break;
					}
				case IDM_UNSORTED       : {        // От пункта меню Structure
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_UNSORTED );
						else                                tis->KBase->SetAttSort ( - SORT_UNSORTED );
					else
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_UNSORTED );
						else                                tis->KBase->SetValSort ( - SORT_UNSORTED );
					tis->PrintOn();
					break;
					}
				case IDM_NAME           : {
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_ALPHA );
						else                                tis->KBase->SetAttSort ( - SORT_ALPHA );
					else
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_ALPHA );
						else                                tis->KBase->SetValSort ( - SORT_ALPHA );
					tis->PrintOn();
					break;
					}
				case IDM_PRICE          : {
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_PRICE );
						else                                tis->KBase->SetAttSort ( - SORT_PRICE );
					else
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_PRICE );
						else                                tis->KBase->SetValSort ( - SORT_PRICE );
					tis->PrintOn();
					break;
					}
				case IDM_IMPORTANCE     : {
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_IMPORTANCE );
						else                                tis->KBase->SetAttSort ( - SORT_IMPORTANCE );
					else
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_IMPORTANCE );
						else                                tis->KBase->SetValSort ( - SORT_IMPORTANCE );
					tis->PrintOn();
					break;
					}
				case IDM_VALUES         : {
					if ( tis->isAttSelected() )
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_VALNUM );
						else                                tis->KBase->SetAttSort ( - SORT_VALNUM );
					else ;                // Для значений такой сортировки нет
					tis->PrintOn();
					break;
					}
				case IDM_CHARACTER      : {
					if ( tis->isValSelected() )
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_CHARACTER );
						else                                tis->KBase->SetValSort ( - SORT_CHARACTER );
					else ;               // Для атрибутов такой сортировки нет
					tis->PrintOn();
					break;
					}
				case IDM_REVERSE        : {
					if ( tis->isAttSelected() )
						tis->KBase->SetAttSort ( - tis->KBase->GetAttSort() );
					else
						tis->KBase->SetValSort ( - tis->KBase->GetValSort() );
					tis->PrintOn();
					break;
					}
				case IDM_ITEMADD        : {
					if ( tis->isAttSelected() ) tis->InsertProposition();
					else                        tis->InsertValue();
					tis->SetFocus();
					break;
					}
				case IDM_ITEMINVERT       : {
					if ( tis->isAttSelected() ) tis->InvertProposition();
					else                        tis->EditValue();
					tis->SetFocus();
					break;
					}
				case IDM_ITEMDELETE     : {
					if ( tis->isAttSelected() ) tis->DeleteProposition();
					else                        tis->DeleteValue();
					tis->SetFocus();
					break;
					}
				case IDM_RULEADD        : {
					tis->InsertRule(); tis->SetFocus();
					break;
					}
				case IDM_RULEDELETE : {
					tis->DeleteRule(); tis->SetFocus();
					break;
					}
				case IDM_PREVRULE   : {
					if ( tis->KBase->GetCurRule() != 0 ) {
						tis->KBase->PrevRule();
						tis->KBase->SetCurAttPos ( -1 ); tis->KBase->SetCurVal ( -1 );
						SetScrollPos ( tis->hWnd, SB_VERT, tis->KBase->GetCurRule(), TRUE );
						tis->PrintOn();
						}
					break;
					}
				case IDM_NEXTRULE   : {
					if ( tis->KBase->GetCurRule() != tis->KBase->RuleNumber() ) {
						tis->KBase->NextRule();
						tis->KBase->SetCurAttPos ( -1 ); tis->KBase->SetCurVal ( -1 );
						SetScrollPos ( tis->hWnd, SB_VERT, tis->KBase->GetCurRule(), TRUE );
						tis->PrintOn();
						}
					break;
					}
				case IDM_FIRSTRULE  : {
					if ( tis->KBase->GetCurRule() != 0 ) {
						tis->KBase->SetCurRule ( 0 );
						tis->KBase->SetCurAttPos ( -1 ); tis->KBase->SetCurVal ( -1 );
						SetScrollPos ( tis->hWnd, SB_VERT, tis->KBase->GetCurRule(), TRUE );
						tis->PrintOn();
						}
					break;
					}
				case IDM_LASTRULE  : {
					if ( tis->KBase->GetCurRule() != tis->KBase->RuleNumber() ) {
						tis->KBase->SetCurRule ( tis->KBase->RuleNumber() );
						tis->KBase->SetCurAttPos ( -1 ); tis->KBase->SetCurVal ( -1 );
						SetScrollPos ( tis->hWnd, SB_VERT, tis->KBase->GetCurRule(), TRUE );
						tis->PrintOn();
						}
					break;
					}
				case IDM_INCREASE  : {
					tis->IncreaseComp();
					break;
					}
				case IDM_DECREASE  : {
					tis->DecreaseComp();
					break;
					}
				case IDM_COMPILE  : {
					tis->Compile();
					break;
					}
				case IDM_HYPERTEXT  : {
					MOD_DESCR md;
					tis->KBase->GetModuleHead ( & md );
					if ( md.HyperFile[0] == 0 ) {
						ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_NOHYPERFILE );
						break;
						}

					ATT_REC ar;
					VAL_REC vr;
					DWORD idx;
					int Att = tis->KBase->GetCurAtt();
					int Val = tis->KBase->GetCurVal();
					if ( tis->isAttSelected() ) {
						if (  Att != -1 && Att != tis->KBase->AttNumber() ) {
							tis->KBase->GetAttribute ( Att, & ar );
							idx = ar.HyperIndex;
							}
						else break;
						}
					else if ( tis->isValSelected() ) {
						if ( Val != -1 && Val != tis->KBase->ValNumber ( Att ) ) {
							tis->KBase->GetValue ( Att, Val, & vr );
							idx = vr.HyperIndex;
							}
						else break;
						}
					else break;
					if ( idx == 0 ) {
						ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_APPLMODAL, IDS_NOINDEX );
						break;
						}
					WinHelp ( hwnd, md.HyperFile, HELP_CONTEXT, idx );
					break;
					}
				case IDM_ATT_LISTBOX    : {
					if ( HIWORD ( lParam ) == LBN_DBLCLK ) {
						if ( tis->SplitState != IDM_VIEWNONE )
							if ( tis->KBase->GetCurAtt() != tis->KBase->AttNumber() )
								SendMessage ( hwnd, WM_COMMAND, IDM_ITEMINVERT, 0L );
							else SendMessage ( hwnd, WM_COMMAND, IDM_ITEMADD, 0L );
						else
							if ( tis->KBase->GetCurVal() == -1 )
								if ( tis->KBase->GetCurAtt() != tis->KBase->AttNumber () )
									SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMINVERT, 0L );
								else SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMADD, 0L );
							else if ( tis->KBase->GetCurVal() != tis->KBase->ValNumber ( tis->KBase->GetCurAtt() ) )
									SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMEDIT, 0L );
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SETFOCUS ) {
						if ( tis->SplitState != IDM_VIEWNONE )
							tis->CurFocus = IDM_ATT_LISTBOX;
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SELCHANGE ) {
						int idx = SendMessage ( LOWORD(lParam), LB_GETCURSEL, 0, 0L);
						if ( tis->SplitState == IDM_VIEWNONE ) {
							tis->KBase->SetCurAtt ( tis->NoneIdx2Att ( idx ) );
							tis->KBase->SetCurVal ( tis->NoneIdx2Val ( idx ) );
							}
						else if ( idx != tis->KBase->GetCurAttPos() ) {
							tis->KBase->SetCurAttPos ( idx );
							tis->KBase->SetCurVal ( -1 );
							tis->FillValListBox ( tis->KBase->GetCurAtt() );
							}
						break;
						}
					break;
					}
				case IDM_VAL_LISTBOX    : {
					if ( HIWORD ( lParam ) == LBN_DBLCLK ) {
                        if ( tis->SplitState != IDM_VIEWNONE )
							if ( tis->KBase->GetCurVal() != tis->KBase->ValNumber ( tis->KBase->GetCurAtt() ) )
								SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMEDIT, 0L );
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SETFOCUS ) {
						if ( tis->SplitState != IDM_VIEWNONE )
							tis->CurFocus = IDM_VAL_LISTBOX;
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SELCHANGE ) {
						int idx = SendMessage ( LOWORD ( lParam ), LB_GETCURSEL, 0, 0L);
						if ( tis->SplitState != IDM_VIEWNONE ) {    // !!! И не может быть равно
							tis->KBase->SetCurVal ( idx );
							}
						break;
						}
					break;
					}

				default : goto CallDCP;
				}
			break;
			}

		case WM_CLOSE   : {
			if ( tis->Close ( ) ) {              // Если действительно закрыли
				goto CallDCP;                                  // Закрыть окно
				}
			break;
			}

		default :
CallDCP:
			return DefMDIChildProc ( hwnd, msg, wParam, lParam );
		}
	return FALSE;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ CompileOut │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export CompileOutProc ( int Status, int curInput, int curOutput, int totInput, int totOutput ) {
/*
╔═══╤══════════════════════════════╗
║ ─ │        Compiling             ║
╟───┴──────────────────────────────╢
║  Status: <Status>                ║
║                                  ║
║          Current   Total         ║
║        ┌───────────────────────┐ ║
║   Input│<curInput>  <totInput> │ ║
║  Output│<curOutput> <totOutput>│ ║
║        └───────────────────────┘ ║
║                     <?????????>  ║
║                                  ║
╚══════════════════════════════════╝
*/
	static int  prevStatus;
	static int  prevCurInput;
	static int  prevCurOutput;
	static int  prevTotInput;
	static int  prevTotOutput;

	MSG  msg;
	char str[16];

	// Выдать параметры в диалог
	if ( Status != prevStatus ) {
		if ( Status == STAT_GENERATION )
			SetDlgItemText ( Frame.hwndStatus, IDD_STATUS, (LPSTR) "GENERATION" );
		else if ( Status == STAT_ABSORPTION )
			SetDlgItemText ( Frame.hwndStatus, IDD_STATUS, (LPSTR) "ABSORPTION" );
		prevStatus = Status;
		}

	if ( prevCurInput != curInput ) {
		itoa ( curInput, str, 10 );
		SetDlgItemText ( Frame.hwndStatus, IDD_CURINPUT, (LPSTR) str );
		prevCurInput = curInput;
		}

	if ( prevCurOutput != curOutput ) {
		itoa ( curOutput, str, 10 );
		SetDlgItemText ( Frame.hwndStatus, IDD_CUROUTPUT , (LPSTR) str );
		prevCurOutput = curOutput;
		}

	if ( prevTotInput != totInput ) {
		itoa ( totInput, str, 10 );
		SetDlgItemText ( Frame.hwndStatus, IDD_TOTINPUT, (LPSTR) str );
		prevTotInput = totInput;
		}

	if ( prevTotOutput != totOutput ) {
		itoa ( totOutput, str, 10 );
		SetDlgItemText ( Frame.hwndStatus, IDD_TOTOUTPUT, (LPSTR) str );
		prevTotOutput = totOutput;
		}


	// Обрабатываем сообщения пока они есть и возвращаем управление для
	// продолжения компиляции как только появилось "окно", т.е., свободное
	// от обработки сообщений время. Обработка сообщений заключается в том,
	// что сперва необходимо узнать для кого оно. Если это сообщение для
	// диалога статистики компиляции, то его необходимо обработать по всем
	// правилам. В противном случае (если это сообщение не для диалога)
	// необходимо отругать пользователя звуковым сигналом за то, что он не
	// туда тыкает мышкой (т.е., организовать обычное поведение диалогов).
	while ( TRUE ) {
		// Если есть сообщение, то обработать его путем предварительной фильтрации
		if ( PeekMessage ( & msg, NULL, 0, 0, PM_REMOVE ) ) {
			if ( msg.message == WM_QUIT ) break;        // Этого не может быть

			if ( Frame.hwndStatus == 0 || ! IsDialogMessage ( Frame.hwndStatus, & msg ) )
				if ( ! TranslateMDISysAccel ( Frame.hwndClient, & msg ) && ! TranslateAccelerator ( Frame.hwndFrame, Frame.hAccel, & msg ) ) {
					TranslateMessage ( & msg );
					DispatchMessage ( & msg );
					}

			}
		else {
			// Свободное для выполнения левой работы время
			// Вернуть управление процедуре компиляции для продолжения работы
			if ( FullPath[0] ) return TRUE;
			else               return FALSE;
			}
		}

	return TRUE;             // Чтобы успокоить компилятор (недостижимый код)
	}

