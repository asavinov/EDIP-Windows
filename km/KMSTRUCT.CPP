#include <windows.h>
#include <alloc.h>
#include <stdlib.h>
#include "lk.h"
#include "km.h"

extern FRAME FAR Frame;
extern char FAR FullPath[128];

/*
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐ STRUCT ▐▐▐
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐
*/
/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ Register │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL STRUCT::Register ( VOID ) {

	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = StructWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(LPSTRUCT);   // Для хранения Указателя на объект
	wc.hInstance     = Frame.hInstance;
	wc.hIcon         = LoadIcon ( Frame.hInstance, "struct" );
	wc.hCursor       = 0;
	wc.hbrBackground = COLOR_APPWORKSPACE + 1;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = STRUCT::Name;

	if ( ! RegisterClass ( & wc ) ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTREGISTER );
		return FALSE;
		}

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│ Открыть окно структуры                                              │ Open │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int STRUCT::Open ( LPSTR Path ) {
	LPSTRUCT tis;

	// Выделить память для объекта STRUCT
	tis = (LPSTRUCT) farmalloc ( sizeof(STRUCT) );
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
	char sz[80] = TITLE_STRUCT;

	if ( Path ) lstrcat ( sz, GetPathName ( Path ) );
	else        lstrcat ( sz, TITLE_UNTITLED );

	mcs.szTitle = sz;

	mcs.szClass    = STRUCT::Name;
	mcs.hOwner     = Frame.hInstance;
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
	mcs.style      = 0;
	mcs.lParam     = (LPARAM) tis;

	// Создать основное окно структуры
	tis->hWnd = (HANDLE) SendMessage ( Frame.hwndClient, WM_MDICREATE, 0, (LPARAM) & mcs );
	if ( ! tis->hWnd ) {
		tis->Close();
		ErrorMessager  ( Frame.hwndFrame, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTCREATEWIN );
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

	// Инициализация состояния
	tis->KBase->SetCurAtt ( -1 );
	tis->KBase->SetCurVal ( -1 );
	tis->KBase->SetAttSort ( SORT_UNSORTED );
	tis->KBase->SetValSort ( SORT_UNSORTED );
	tis->KBase->SetWindow ( tis->hWnd );
	tis->KBase->SetView ( VIEW_STRUCT );

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
│ Закрыть окно структуры. Перед закрытием необходимо проверить       │ Close │
│ можно ли это делать. Если закрыто, то возвращается TRUE.           └───────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL STRUCT::Close ( VOID ) {

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
BOOL STRUCT::Save ( VOID ) {

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
			ErrorMessager ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTSAVE );
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
VOID STRUCT::Size ( WPARAM wParam, LPARAM lParam ) {

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
VOID STRUCT::View ( int NewView ) {
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
VOID STRUCT::PartMove ( UINT msg, LPARAM lParam ) {
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
VOID STRUCT::DrawPart ( int AttSize, WORD vh ) {
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
VOID STRUCT::SetFocus ( VOID ) {
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
VOID STRUCT::FillAttListBox ( VOID ) {
	int i, w, maxWidth = 0;

//	ShowWindow ( hwndAtt, SW_HIDE );
	SendMessage ( hwndAtt, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndAtt, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	HDC hDC = GetDC ( hwndAtt );              // Для определения ширины строки
	for ( i = 0; i < KBase->AttNumber(); i++ ) {          // Цикл по атрибутам
		ATT_REC  AttRec;

		KBase->GetAttribute ( i, & AttRec );
		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & AttRec.Name, lstrlen ( (LPSTR) & AttRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести атрибут в ListBox
		SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM) & AttRec.Name );
		}
	ReleaseDC ( hwndAtt, hDC );

	SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM)"" );
	SendMessage ( hwndAtt, LB_SETCURSEL, (WPARAM) KBase->GetCurAtt(), 0 );
	SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, maxWidth+6+3*BMPWIDTH, 0 );
	SendMessage ( hwndAtt, WM_SETREDRAW, TRUE, 0L );
	InvalidateRect ( hwndAtt, NULL, FALSE );
//	ShowWindow ( hwndAtt, SW_SHOW );
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Заполняет окно значений строками. Нельзя использовать в   │ FillValListBox │
│ однооконном режиме. Текущее значение должно быть          └────────────────┤
│ правильно установлено.                                                     │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::FillValListBox ( int Att ) {
	VAL_REC  ValRec;
	int j, w, maxWidth = 0;

	if ( Att == -1 || Att == KBase->AttNumber() ) {
		SendMessage ( hwndVal, LB_SETHORIZONTALEXTENT, 0, 0 );
		SendMessage ( hwndVal, LB_RESETCONTENT, 0, 0 );
		return;
		}

//	ShowWindow ( hwndVal, SW_HIDE );
	SendMessage ( hwndVal, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndVal, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	HDC hDC = GetDC ( hwndVal );              // Для определения ширины строки
	for ( j = 0; j < KBase->ValNumber ( Att ); j++ ) {    // Цикл по значениям
		// Найти значение с индексом i в зависимости от текущей сортировки
		KBase->GetValue ( Att, j, & ValRec );

		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & ValRec.Name, lstrlen ( (LPSTR) & ValRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести значение в ListBox, а индекс записать (он должен быть равен исходному)
		SendMessage ( hwndVal, LB_ADDSTRING, 0, (LPARAM) & ValRec.Name );

		}
	ReleaseDC ( hwndVal, hDC );

	SendMessage ( hwndVal, LB_ADDSTRING, 0, (LPARAM)"" );   // Добавит пустую строку

	SendMessage ( hwndVal, LB_SETCURSEL, KBase->GetCurVal(), 0 );

	SendMessage ( hwndVal, LB_SETHORIZONTALEXTENT, maxWidth+6+3*BMPWIDTH, 0 );
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
VOID STRUCT::FillNoneView ( VOID ) {
	int i, j, w, maxWidth = 0;
	ATT_REC  AttRec;
	VAL_REC  ValRec;

//	ShowWindow ( hwndAtt, SW_HIDE );
	SendMessage ( hwndAtt, WM_SETREDRAW, FALSE, 0L );
	SendMessage ( hwndAtt, LB_RESETCONTENT, 0, 0 );        // Очистить ListBox

	HDC hDC = GetDC ( hwndAtt );              // Для определения ширины строки
	for ( i = 0; i < KBase->AttNumber(); i++ ) {          // Цикл по атрибутам

		KBase->GetAttribute ( i, & AttRec );
		// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
		w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & AttRec.Name, lstrlen ( (LPSTR) & AttRec.Name) ) );
		if ( maxWidth < w ) maxWidth = w;

		// Занести атрибут в ListBox
		SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM) & AttRec.Name );

		for ( j = 0; j < KBase->ValNumber ( i ); j++ ) {  // Цикл по значениям

			KBase->GetValue ( i, j, & ValRec );
			// Вычислить ширину его названия и сравнить с текущей (если больше, то сделать текущей)
			w = LOWORD ( GetTextExtent ( hDC, (LPSTR) & ValRec.Name, lstrlen ( (LPSTR) & ValRec.Name) ) );
			if ( maxWidth < w ) maxWidth = w;

			// Занести значение в ListBox
			SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM) & ValRec.Name );
			}
		SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM)"" );

		}
	ReleaseDC ( hwndAtt, hDC );

	SendMessage ( hwndAtt, LB_ADDSTRING, 0, (LPARAM)"" );
	SendMessage ( hwndAtt, LB_SETCURSEL, (WPARAM) MakeNoneIdx ( KBase->GetCurAtt(), KBase->GetCurVal() ), 0 );
	SendMessage ( hwndAtt, LB_SETHORIZONTALEXTENT, maxWidth+6+5*BMPWIDTH, 0 );
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
VOID STRUCT::PrintOn ( VOID ) {
	if ( SplitState == IDM_VIEWNONE ) FillNoneView();
	else {
		FillAttListBox();
		FillValListBox ( KBase->GetCurAtt() );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить положение в дереве по атрибуту и значению         │ MakeNoneIdx │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int STRUCT::MakeNoneIdx ( int Att, int Val ) {
	if ( Att == -1 ) return -1;
	// Двойка│ потому, что у каждого атрибута по одному вспомогательному значению
	//       │             Один вспомогательный атрибут │
	//       V                                          V
	return ( 2 * Att + KBase->ValPosSum ( Att ) + Val + 1 );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить номер атрибута по положению в дереве              │ NoneIdx2Att │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int STRUCT::NoneIdx2Att ( int Idx ) {
	if ( Idx == -1 ) return -1;

	int i, n = KBase->AttNumber();
	for ( i = 1; i <= n; i++ ) if ( MakeNoneIdx ( i, -1 ) > Idx ) break;

	return i-1;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Определить номер значения по положению в дереве              │ NoneIdx2Val │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int STRUCT::NoneIdx2Val ( int Idx ) {
	if ( Idx == -1 ) return -1;

	return ( Idx - 1 - MakeNoneIdx ( NoneIdx2Att ( Idx ), -1 ) );
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│                                                          │ InsertAttribute │
│                                                          └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::InsertAttribute ( VOID ) {
	ATT_REC AttRec = { "", "", "", 0 ,0 ,0 ,0 };

	if ( KBase->GetCurAtt() == -1 ) return;

	// Вызвать диалоговою процедуру и передать ей указатель на ATT_REC
	FARPROC lpAttProc = MakeProcInstance ( (FARPROC) AttributeProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "attribute", Frame.hwndFrame, lpAttProc, (LPARAM) & AttRec );
	FreeProcInstance ( lpAttProc );
	if ( ! i ) return;                                  // Диалог не состоялся

	// Произвести добавление аттрибута.
	int Ret = KBase->InsAttribute ( & AttRec );

	if ( Ret == LKE_NOTUNIQUE ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_NOTUNIQUE );
		return;
		}
	if ( Ret == LKE_ALLOCMEM ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	PrintOn();

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ InsertValue │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::InsertValue ( VOID ) {
	VAL_REC ValRec = { "", "", "", 0 ,0 ,0 ,0 };


	if ( ( KBase->GetCurVal() == -1 ) || ( KBase->GetCurAtt() == -1 ) ||
         ( KBase->GetCurAtt() == KBase->AttNumber() ) )
		return;

	// Вызвать диалоговою процедуру и передать ей указатель на VAL_REC
	FARPROC lpValProc = MakeProcInstance ( (FARPROC) ValueProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "value", Frame.hwndFrame, lpValProc, (LPARAM) & ValRec );
	FreeProcInstance ( lpValProc );
	if ( ! i ) return;                                  // Диалог не состоялся

	// Произвести добавление аттрибута.
	int Ret = KBase->InsValue ( & ValRec );

	if ( Ret == LKE_NOTUNIQUE ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_NOTUNIQUE );
		return;
		}
	if ( Ret == LKE_ALLOCMEM ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	PrintOn();

	return;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ EditAttribute │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::EditAttribute ( VOID ) {
	ATT_REC AttRec;

	if ( ( KBase->GetCurAtt() == -1 ) || ( KBase->GetCurAtt() == KBase->AttNumber() ) )
		return;

	// Засунуть текущий атрибут в AttRec
	KBase->GetAttribute ( KBase->GetCurAtt(), & AttRec );

	// Вызвать диалоговою процедуру и передать ей указатель на ATT_REC
	FARPROC lpAttProc = MakeProcInstance ( (FARPROC) AttributeProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "attribute", Frame.hwndFrame, lpAttProc, (LPARAM) & AttRec );
	FreeProcInstance ( lpAttProc );
	if ( ! i ) return;                                  // Диалог не состоялся

	// Произвести добавление значения.
	int Ret = KBase->PutAttribute ( & AttRec );

	if ( Ret == LKE_NOTUNIQUE ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_NOTUNIQUE );
		return;
		}
	if ( Ret == LKE_ALLOCMEM ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	// Заполнить ListBox'ы
	PrintOn();

	return;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ EditValue │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::EditValue ( VOID ) {
	VAL_REC ValRec;

	if ( ( KBase->GetCurVal() == -1 ) || ( KBase->GetCurAtt() == -1 ) ||
         ( KBase->GetCurAtt() == KBase->AttNumber() ) ||
         ( KBase->GetCurVal() == KBase->ValNumber ( KBase->GetCurAtt() ) ) )
		return;

	// Засунуть текущее значение в ValRec
	KBase->GetValue ( KBase->GetCurAtt(), KBase->GetCurVal(), & ValRec );

	// Вызвать диалоговою процедуру и передать ей указатель на VAL_REC
	FARPROC lpValProc = MakeProcInstance ( (FARPROC) ValueProc, Frame.hInstance );
	int i = DialogBoxParam ( Frame.hInstance, "value", Frame.hwndFrame, lpValProc, (LPARAM) & ValRec );
	FreeProcInstance ( lpValProc );
	if ( ! i ) return;                                  // Диалог не состоялся

	// Произвести добавление значения.
	int Ret = KBase->PutValue ( & ValRec );

	if ( Ret == LKE_NOTUNIQUE ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_NOTUNIQUE );
		return;
		}
	if ( Ret == LKE_ALLOCMEM ) {
		ErrorMessager  ( 0, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL, IDS_CANTALLOC );
		return;
		}

	// Заполнить ListBox'ы
	PrintOn();

	return;
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│                                                          │ DeleteAttribute │
│                                                          └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::DeleteAttribute ( VOID ) {

	if ( ( KBase->GetCurAtt() == -1) || ( KBase->GetCurAtt() == KBase->AttNumber() ) )
		return;

	if ( IDYES != ErrorMessager  ( hWnd, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_DELETEATTRIBUTE ) )
		return;

	// Удалить атрибут
	KBase->DelAttribute();

	// Отразить текущее состояние на экране
	PrintOn();

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ DeleteValue │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::DeleteValue ( VOID ) {

	if ( ( KBase->GetCurVal() == -1) ||
         ( KBase->GetCurVal() == KBase->ValNumber ( KBase->GetCurAtt() ) ) ||
         ( KBase->GetCurAtt() == -1 ) ||
         ( KBase->GetCurAtt() == KBase->AttNumber() ) )
		return;

	if ( IDYES != ErrorMessager ( hWnd, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION, IDS_DELETEVALUE ) )
		return;

	// Удалить значение из файла
	KBase->DelValue();

	// Отразить текущее состояние LISTBOX'ов на экране
	PrintOn();

	return;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ InitMenu │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::InitMenu ( VOID ) {

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

		// Запртеить пункты меню не относящиеся к этому окну
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES, MF_ENABLED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER, MF_GRAYED );

		// Отметить текущую сортировку
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_UNCHECKED );

		switch ( KBase->GetAttSort() ) {
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
			case - SORT_IMPORTANCE:
			case SORT_IMPORTANCE  : {
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
	else {

		// Для окна значений
		// Запртеить пункты меню не относящиеся к этому окну
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES, MF_DISABLED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES, MF_GRAYED );
		EnableMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER, MF_ENABLED );

		// Отметить текущую сортировку
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_UNSORTED,   MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_NAME,       MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_PRICE,      MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_IMPORTANCE, MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_UNCHECKED );
		CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_CHARACTER,  MF_UNCHECKED );

		switch ( KBase->GetValSort() ) {
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
/*			case - SORT_VALNUM   :
			case SORT_VALNUM     : {
				CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_VALUES,     MF_CHECKED );
				break;
				}
*/			case - SORT_PRICE    :
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
		if ( KBase->GetValSort() < 0 ) CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_CHECKED );
		else CheckMenuItem ( GetSubMenu ( hMenu, 1 ), IDM_REVERSE, MF_UNCHECKED );

		}

	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ DrawItemBmp │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::DrawItemBmp ( LPDRAWITEMSTRUCT lpDIS, int WhatToDraw, BOOL Last ) {

	HPEN SavePen = SelectObject ( lpDIS->hDC, CreatePen ( PS_SOLID, 1, RGB(0,0,255) ) );
	HDC hdcCompat = CreateCompatibleDC ( lpDIS->hDC );
	if ( WhatToDraw == IDM_ATT_LISTBOX ) SelectObject ( hdcCompat, hbmpAtt );
	else                                 SelectObject ( hdcCompat, hbmpVal );

	if ( SplitState != IDM_VIEWNONE )                      // Двуоконный режим
		if ( Last) {
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
        	}
        else {
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
			MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
			LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
			BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5, 10, 10, hdcCompat, 0, 0, SRCCOPY );
        	}
	else                                                  // Однооконный режим
		if ( WhatToDraw == IDM_ATT_LISTBOX )                 // Рисуем атрибут
			if ( Last ) {
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
   		 		}
			else {
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 3 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5, 10, 10, hdcCompat, 0, 0, SRCCOPY );
       		 	}
		else                                                // Рисуем значение
			if ( Last ) {
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top);
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + BMPWIDTH, lpDIS->rcItem.top + Frame.LBitemHight );
				MoveTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 2 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				LineTo ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 );
				BitBlt ( lpDIS->hDC, lpDIS->rcItem.left + 4 * BMPWIDTH, lpDIS->rcItem.top + (int) Frame.LBitemHight/2 - 5 , 10, 10, hdcCompat, 0, 0, SRCCOPY );
   		 		}
			else {
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
VOID STRUCT::DrawItemText ( LPDRAWITEMSTRUCT lpDIS, int Pos ) {
	char buf[128];
	RECT rc = lpDIS->rcItem;
	HBRUSH hBrush;

	if ( lpDIS->itemState & ODS_SELECTED ) {
		hBrush = CreateSolidBrush ( GetSysColor ( COLOR_HIGHLIGHT ) );
		SetBkColor ( lpDIS->hDC, GetSysColor ( COLOR_HIGHLIGHT ) );
		SetTextColor ( lpDIS->hDC, GetSysColor ( COLOR_HIGHLIGHTTEXT ) );
		}
	else {
		hBrush = CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
		SetBkColor ( lpDIS->hDC, GetSysColor ( COLOR_WINDOW ) );
		SetTextColor ( lpDIS->hDC, GetSysColor ( COLOR_WINDOWTEXT ) );
		}

	// Подготовить область для рисования, т.е., закрасить ее нужным цветом в зависимости от выделения
	rc.left += Pos;
	FillRect ( lpDIS->hDC, & rc, hBrush );

	TextOut (
		lpDIS->hDC,
		rc.left + 3,                          // Отступ слева на всякий случай
		rc.top + 1,                                           // Отступ справа
		buf,
		(int) SendMessage ( lpDIS->hwndItem, LB_GETTEXT, lpDIS->itemID, (LPARAM) buf )
		);

	DeleteObject ( hBrush );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│                                                                 │ DrawItem │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID STRUCT::DrawItem ( LPDRAWITEMSTRUCT lpDIS ) {

	if ( SplitState != IDM_VIEWNONE ) {
		RECT rc = lpDIS->rcItem;

		rc.left += 3 * BMPWIDTH;

		switch ( lpDIS->itemAction ) {

		case ODA_SELECT :
			// Выполняем полную программу. Это немного неэффективно (битмэпы
			// рисовать в действительности не надо, а только текст), но зато просто.

			// Пристегните ремни и дышите глубже; проваливаемся в следующий кейс...

		case ODA_DRAWENTIRE : {                  // Нарисовать полностью пункт

			if ( lpDIS->CtlID == IDM_ATT_LISTBOX ) {// Рисуем в окне атрибутов

				if ( lpDIS->itemID == KBase->AttNumber() )   // Пустой атрибут
					DrawItemBmp ( lpDIS, lpDIS->CtlID, TRUE );
				else                                     // Нормальный атрибут
					DrawItemBmp ( lpDIS, lpDIS->CtlID, FALSE );

				DrawItemText ( lpDIS, 3 * BMPWIDTH );
				}
	        else {                                   // Рисуем в окне значений

				if ( lpDIS->itemID == KBase->ValNumber ( KBase->GetCurAtt() ) )
					DrawItemBmp ( lpDIS, lpDIS->CtlID, TRUE );      // Пустое значение
				else
					DrawItemBmp ( lpDIS,  lpDIS->CtlID, FALSE );// Нормальное значение

				DrawItemText ( lpDIS, 3 * BMPWIDTH );
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
		else                                        rc.left += 5 * BMPWIDTH;

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

				DrawItemText ( lpDIS, 4 * BMPWIDTH );
				}
			else {                                          // Рисуем значение

				if ( NoneIdx2Val ( lpDIS->itemID ) == KBase->ValNumber ( NoneIdx2Att ( lpDIS->itemID ) ) )
					DrawItemBmp ( lpDIS, IDM_VAL_LISTBOX, TRUE ); // Пустое значение
				else
					DrawItemBmp ( lpDIS, IDM_VAL_LISTBOX, FALSE ); // Нормальное значение

				DrawItemText ( lpDIS, 5 * BMPWIDTH );
				}
			break;
			}

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
VOID STRUCT::ChangeWin ( VOID ) {
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
BOOL STRUCT::isAttSelected ( VOID ) {
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
BOOL STRUCT::isValSelected ( VOID ) {
	if ( ( ( SplitState != IDM_VIEWNONE ) && ( CurFocus == IDM_VAL_LISTBOX ) ) ||
           ( SplitState == IDM_VIEWNONE ) && ( KBase->GetCurVal() != -1 ) )
		return TRUE;
	else
		return FALSE;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ ValueProc │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export ValueProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPVAL_REC ValRec;

    switch ( msg ) {
		case WM_INITDIALOG : {
			char buf[16];

			ValRec = (LPVAL_REC) lParam;

			SendDlgItemMessage ( hwnd, IDD_NAME, WM_SETTEXT, 0, (LPARAM) ValRec->Name );
			SendDlgItemMessage ( hwnd, IDD_NAME, EM_LIMITTEXT, sizeof(ValRec->Name) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_SETTEXT, 0, (LPARAM) ValRec->Question );
			SendDlgItemMessage ( hwnd, IDD_QUESTION, EM_LIMITTEXT, sizeof(ValRec->Question) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_SETTEXT, 0, (LPARAM) ValRec->Comment );
			SendDlgItemMessage ( hwnd, IDD_COMMENT, EM_LIMITTEXT, sizeof(ValRec->Comment) - 1, 0L );

			// Инициализировать цену значения (преобразовать в строку)
			itoa ( (int) ValRec->Price, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_PRICE, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_PRICE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать важность значения (преобразовать в строку)
			itoa( (int) ValRec->Importance, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать характерность значения (преобразовать в строку)
			itoa( (int) ValRec->Character, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_CHARACTER, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_CHARACTER, EM_LIMITTEXT, 3, 0L );

			// Инициализировать индекс гипертекста значения (преобразовать в строку)
			ltoa ( (long) ValRec->HyperIndex, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, EM_LIMITTEXT, 5, 0L );

			LocalUnlock ( LOWORD(lParam) );
			break;
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					char buf[16];

					SendDlgItemMessage ( hwnd, IDD_NAME, WM_GETTEXT, sizeof(ValRec->Name), (LPARAM) ValRec->Name );
					SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_GETTEXT, sizeof(ValRec->Question), (LPARAM) ValRec->Question );
					SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_GETTEXT, sizeof(ValRec->Comment), (LPARAM) ValRec->Comment );

					// Получить цену значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_PRICE, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					ValRec->Price = (char) atoi ( buf );

					// Получить важность значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					ValRec->Importance = (char) atoi ( buf );

					// Получить характерность значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_CHARACTER, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					ValRec->Character = (char) atoi ( buf );

					// Получить индекс гипертекста значения (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					ValRec->HyperIndex = (DWORD) atol ( buf );

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
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ AttributeProc │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL FAR PASCAL _export AttributeProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static LPATT_REC AttRec;

    switch ( msg ) {
		case WM_INITDIALOG : {
			char buf[16];

			AttRec = (LPATT_REC) lParam;

			SendDlgItemMessage ( hwnd, IDD_NAME, WM_SETTEXT, 0, (LPARAM) AttRec->Name );
			SendDlgItemMessage ( hwnd, IDD_NAME, EM_LIMITTEXT, sizeof(AttRec->Name) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_SETTEXT, 0, (LPARAM) AttRec->Question );
			SendDlgItemMessage ( hwnd, IDD_QUESTION, EM_LIMITTEXT, sizeof(AttRec->Question) - 1, 0L );

			SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_SETTEXT, 0, (LPARAM) AttRec->Comment );
			SendDlgItemMessage ( hwnd, IDD_COMMENT, EM_LIMITTEXT, sizeof(AttRec->Comment) - 1, 0L );

			// Инициализировать цену признака (преобразовать в строку)
			itoa ( (int) AttRec->Price, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_PRICE, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage (  hwnd, IDD_PRICE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать важность признака (преобразовать в строку)
			itoa ( (int) AttRec->Importance, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, EM_LIMITTEXT, 3, 0L );

			// Инициализировать индекс гипертекста (преобразовать в строку)
			ltoa ( (long) AttRec->HyperIndex, buf, 10 );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_SETTEXT, 0, (LPARAM) buf );
			SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, EM_LIMITTEXT, 5, 0L );

			SendDlgItemMessage ( hwnd, IDD_HIDDEN, BM_SETCHECK, (AttRec->Style & AVS_HIDDEN) ? 1 : 0, 0L );
			SendDlgItemMessage ( hwnd, IDD_GLOBAL, BM_SETCHECK, (AttRec->Style & AVS_GLOBAL) ? 1 : 0, 0L );

			break;
			}
		case WM_COMMAND : {
			switch( wParam ) {
				case IDOK : {
					char buf[16];

					SendDlgItemMessage ( hwnd, IDD_NAME, WM_GETTEXT, sizeof(AttRec->Name), (LPARAM) AttRec->Name );
					SendDlgItemMessage ( hwnd, IDD_QUESTION, WM_GETTEXT, sizeof(AttRec->Question), (LPARAM) AttRec->Question );
					SendDlgItemMessage ( hwnd, IDD_COMMENT, WM_GETTEXT, sizeof(AttRec->Comment), (LPARAM) AttRec->Comment );

					// Получить цену признака (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_PRICE, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					AttRec->Price = (char) atoi ( buf );

					// Получить важность признака (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_IMPORTANCE, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					AttRec->Importance = (char) atoi ( buf );

					// Получить индекс гипертекста (преобразовать строку в число)
					SendDlgItemMessage ( hwnd, IDD_HYPERINDEX, WM_GETTEXT, sizeof(buf), (LPARAM) buf );
					AttRec->HyperIndex = (DWORD) atol ( buf );

					if ( SendDlgItemMessage ( hwnd, IDD_HIDDEN, BM_GETCHECK, 0, 0L ) )
						AttRec->Style |= AVS_HIDDEN;
					if ( SendDlgItemMessage ( hwnd, IDD_GLOBAL, BM_SETCHECK, 0, 0L ) )
						AttRec->Style |= AVS_GLOBAL;

					EndDialog ( hwnd, 1 );
					break;
					}
				case IDCANCEL : {
					EndDialog ( hwnd, 0 );
					break;
					}
				case IDD_HIDDEN : {
					SendDlgItemMessage ( hwnd, IDD_HIDDEN, BM_SETCHECK, ! SendDlgItemMessage(hwnd,IDD_HIDDEN,BM_GETCHECK,0,0L), 0L );
					break;
					}
				case IDD_GLOBAL : {
					SendDlgItemMessage ( hwnd, IDD_GLOBAL, BM_SETCHECK, ! SendDlgItemMessage(hwnd,IDD_GLOBAL,BM_GETCHECK,0,0L), 0L );
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
│                                                            │ StructWndProc │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
long FAR PASCAL _export StructWndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	LPSTRUCT tis = (LPSTRUCT) GetMem ( hwnd );

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
			if ( wParam == TRUE ) {      // Окно структуры становится активным
				// Загрузить соответствующее меню, акселератор и курсор
				SendMessage ( Frame.hwndClient, WM_MDISETMENU, FALSE, MAKELONG ( STRUCT::hMenu, STRUCT::hWndMenu ) );
				Frame.hAccel = STRUCT::hAccel;
				if ( tis->SplitState == IDM_VIEWHORIZONTAL )
					SetClassWord ( hwnd, GCW_HCURSOR, STRUCT::HorCur );
				else
					SetClassWord ( hwnd, GCW_HCURSOR, STRUCT::VerCur );
				Frame.hwndActive = hwnd;
//				tis->SetFocus ( );
				}
			if ( wParam == FALSE ) {       // Окно структуры теряет активность
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
		case WM_COMMAND : {
			switch( wParam ) {
				case CHANGE_TITLE    : {
					char buf[80] = TITLE_STRUCT;
					lstrcat ( buf, tis->KBase->GetNamePtr() );
					SetWindowText ( hwnd, buf );
					break;
					}
				// Если хоть что-либо изменилось в структуре, то заново отображаем
				// все содержимое окна (независимо от типа изменения). Это м.б.
				// неэффективно, зато надежно. При более тщательном рассмотрении
				// необходимо для каждого сообщения проверить, действительно ли
				// данное изменение отражается в окне. В основном это касается
				// проверки номеров измененных атрибута или значения (например,
				// видны ли они на экране в зависимости от вида). Проверку есть
				// смысл делать только для значений в двуоконном режиме, когда
				// чаще всего текущий атрибут не совпадает с тем, у которого меняется
				// значение, а значит, изменяемое значение не показывается на экране.
				// Другими словами, необходимо отслеживать изменения значений
				// (три сообщения). В случае однооконного режима (NONEVIEW)
				// происходит безусловная перерисовка (чтобы не мучиться с определением
				// видимости значения и его влияния на другие). В противном случае
				// (двуоконный режим) перерисовываем окно значений только если
				// текущий атрибут такой же как в сообщении. Вообще, вопрос сложнее.
				// Например, изменение количества значений может повлиять на
				// сортировку атрибутов.
				case CHANGE_CONTENT  :
				case PUT_ATTRIBUTE   :
				case INS_ATTRIBUTE   :
				case DEL_ATTRIBUTE   :
				case PUT_VALUE       :
				case INS_VALUE       :
				case DEL_VALUE       : {
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
				case IDM_UNSORTED       : {
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
					if ( tis->isAttSelected() ) {
						if ( tis->KBase->GetAttSort() > 0 ) tis->KBase->SetAttSort ( SORT_VALNUM );
						else                                tis->KBase->SetAttSort ( - SORT_VALNUM );
						tis->PrintOn();
						}
					break;
					}
				case IDM_CHARACTER      : {
					if ( tis->isValSelected() ) {
						if ( tis->KBase->GetValSort() > 0 ) tis->KBase->SetValSort ( SORT_CHARACTER );
						else                                tis->KBase->SetValSort ( - SORT_CHARACTER );
						tis->PrintOn();
						}
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
					if ( tis->isAttSelected() ) tis->InsertAttribute();
					else                        tis->InsertValue();
					tis->SetFocus();
					break;
					}
				case IDM_ITEMEDIT       : {
					if ( tis->isAttSelected() ) tis->EditAttribute();
					else                        tis->EditValue();
					tis->SetFocus ( );
					break;
					}
				case IDM_ITEMDELETE     : {
					if ( tis->isAttSelected() ) tis->DeleteAttribute();
					else                        tis->DeleteValue();
					tis->SetFocus();
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
								SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMEDIT, 0L );
							else SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMADD, 0L );
						else
							if ( tis->KBase->GetCurVal() == -1 )
								if ( tis->KBase->GetCurAtt() != tis->KBase->AttNumber() )
									SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMEDIT, 0L );
								else SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMADD, 0L );
							else if ( tis->KBase->GetCurVal() != tis->KBase->ValNumber ( tis->KBase->GetCurAtt() ) )
									SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMEDIT, 0L );
								else SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMADD, 0L );
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
						else if ( idx != tis->KBase->GetCurAtt() ) {
							tis->KBase->SetCurAtt ( idx );
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
							else SendMessage ( Frame.hwndFrame, WM_COMMAND, IDM_ITEMADD, 0L );
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SETFOCUS ) {
						if ( tis->SplitState != IDM_VIEWNONE )
							tis->CurFocus = IDM_VAL_LISTBOX;
						break;
						}
					if ( HIWORD ( lParam ) == LBN_SELCHANGE ) {
						int idx = SendMessage ( LOWORD ( lParam ), LB_GETCURSEL, 0, 0L);
						if ( tis->SplitState != IDM_VIEWNONE ) {        // !!! И не может быть равно
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
			if ( tis->Close() ) {                // Если действительно закрыли
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
