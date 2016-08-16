// #define EDIPDEMO

#define VK_X 0x58

#define UNITY 100


// Номер первого Child окна (далее последовательно без дыр)
#define IDM_WINDOWCHILD        4200

#define IDM_ATT_LISTBOX        4501           // Иденитификатор окна признаков
#define IDM_VAL_LISTBOX        4502            // Иденитификатор окна значений

// Используются в диалоге для заполнения соответствующих окон
#define ATTR_DIRS           0xc010
#define ATTR_FILES          0x0000
                                                 // Стили атрибутов и значений
#define AVS_HIDDEN      0x00000001L
#define AVS_GLOBAL      0x00000002L


#define ITEMHIGHT           14                 // !!! Потом проверить значения
#define BMPWIDTH            10                  // !!! (если они вообще нужны)
#define COMPLENGTH          30                // Место для значения компоненты

#define HELPFILE       "ediphelp.hlp"

#define TITLE_STRUCT   "STRUCTURE - "
#define TITLE_KNOW     "KNOWLEDGE - "
#define TITLE_DATA     "DATA - "
#define TITLE_CONC     "CONCLUSION - "
#define TITLE_UNTITLED "UNTITLED"

/*
┌────────────────────────────────────────────────────────────────────┬───────┐
│ Пункты меню                                                        │ MENU  │
│                                                                    └───────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define IDM_CHANGEWINDOW       1000

#define IDM_FILENEW	           1001                                    // File
#define IDM_FILEOPEN           1002
#define IDM_FILESAVE           1003
#define IDM_FILESAVEAS         1004
#define IDM_PROPERTIES         1005
#define IDM_FILEEXIT           1006
#define IDM_FILEABOUT          1007
#define IDM_FILECLOSE          1008
#define IDM_FILECLOSEALL       1009
#define IDM_CLOSENOQUERY       1009     // Безусловное закрытие независимо от сохраненности
#define IDM_FILEPREF           1010

#define IDM_VIEWNONE           1200                                    // View
#define IDM_VIEWHORIZONTAL     1201
#define IDM_VIEWVERTICAL       1202
#define IDM_VIEWDATA           1203
#define IDM_VIEWCONCLUSION     1204

#define IDM_PRICE              1205
#define IDM_IMPORTANCE         1206
#define IDM_VALUES             1207
#define IDM_UNSORTED           1208
#define IDM_CHARACTER          1209
#define IDM_REVERSE            1210
#define IDM_NAME               1211
#define IDM_ASINRULE           1212

#define IDM_ITEMADD            1301                                    // Item
#define IDM_ITEMDELETE         1302
#define IDM_ITEMEDIT           1303
#define IDM_ITEMINVERT         1305
#define IDM_ITEMHEADER         1304

#define IDM_RULEADD            1310
#define IDM_RULEDELETE         1311

#define IDM_COMPILE            1601                                 // Compile
#define IDM_INFERENCE          1602                               // Inference
#define IDM_HYPERTEXT          1603

#define IDM_WINDOWTILE         1401                                  // Window
#define IDM_WINDOWCASCADE      1402
#define IDM_WINDOWICONS        1404

#define IDM_HELPINDEX          1440                                    // Help

#define IDM_PREVRULE           1501
#define IDM_NEXTRULE           1502
#define IDM_FIRSTRULE          1503
#define IDM_LASTRULE           1504
#define IDM_INCREASE           1505
#define IDM_DECREASE           1506

#define IDM_CLIP               1600

/*
┌────────────────────────────────────────────────────────────────────┬───────┐
│ Пункты диалога                                                     │       │
│                                                                    └───────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define IDD_FILENAME    201
#define IDD_FILES       202
#define IDD_PATH        203
#define IDD_DIRS        204
#define IDD_STRUCT      205
#define IDD_KNOW        206
#define IDD_DATA        207
#define IDD_CONC        208

#define IDD_NAME        210
#define IDD_QUESTION    211
#define IDD_COMMENT     212
#define IDD_ICONFILE    213
#define IDD_HYPERFILE   214

#define IDD_PRICE         215
#define IDD_IMPORTANCE    216
#define IDD_HYPERINDEX    217
#define IDD_HIDDEN        218
#define IDD_GLOBAL        219
#define IDD_CHARACTER     220
#define IDD_SORTING       221
#define IDD_ASINRULE      222                                // Пункт Unsorted
#define IDD_UNSORTED      223                               // Пункт Structure
#define IDD_PROP          224
#define IDD_VALNUM        225
#define IDD_REVERSE       226
#define IDD_PREMCONCGROUP 227
#define IDD_PREMISE       228
#define IDD_CONCLUSION    229

#define IDD_CURINPUT    231
#define IDD_TOTINPUT    232
#define IDD_CUROUTPUT   233
#define IDD_TOTOUTPUT   234
#define IDD_STATUS	    236

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│                                                              │ LoadStrings │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/

#define IDS_CLOSESAVE           6
#define IDS_REPLACE             7
#define IDS_CANTREGISTER      	8
#define IDS_CANTCREATEWIN       9
#define IDS_CANTLOADRESOURCE    10
#define IDS_CANTALLOC           11
#define IDS_UNTITLED            12
#define IDS_DELETEVALUE         14
#define IDS_DELETEATTRIBUTE     15
#define IDS_APPNAME           	18
#define IDS_NOTUNIQUE           25
#define IDS_DELETEPROPOSITION   26
#define IDS_DELETERULE          27

#define IDS_CANTSAVE            50
#define IDS_NOTEDIPFILE         51
#define IDS_CANTOPENFILE        52

#define IDS_NOTCOMPILED         53
#define IDS_NOGOAL              54
#define IDS_NOHYPERFILE         55
#define IDS_NOINDEX             56

#define IDS_EDIPDEMO            100

/*
┌────────────────────────────────────────────────────────────────────┬───────┐
│                                                                    │ FRAME │
│                                                                    └───────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define FRAME_WIND_POS                0

struct FAR FRAME {

	HWND      hwndFrame;                         // Ключ Frame окна (главного)
	HWND      hwndClient;                                  // Ключ Client окна
	HANDLE    hInstance;
	char      Name[12];                               // Имя класаа Frame окна
	HWND      hwndActive;                       // Текущее активное Child окно
	HANDLE    hAccel;                            // Ключ текущего акселератора
                                                         // Ключ текущего меню

	HMENU     FrmMenu;
	HMENU     WndMenu;
	HANDLE    hFrmAccel;
	HWND      hwndStatus;
	int       BrdWidth;
	int       LBitemHight;
	};
typedef FRAME FAR   *LPFRAME;

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│                                                                   │ STRUCT │
│                                                                   └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define STRUCT_WIND_POS                3

class FAR STRUCT {
    private:
    public:
		// Присущие этому классу акселератор, меню, имя и т.п.
		static HANDLE  hAccel;
		static HMENU   hMenu;
		static HMENU   hWndMenu;
		static HCURSOR VerCur;
		static HCURSOR HorCur;
		static HBITMAP hbmpAtt;
		static HBITMAP hbmpVal;
		static char    Name[];

		LPKBASE       KBase;                         // Указатель на объект LK
		HWND          hWnd;                           // Основное окно (Child)
		HWND          hwndAtt;                     // Окно атрибутов (listbox)
		HWND          hwndVal;                      // Окно значений (listbox)
		int           SplitState;                         // Состояние ListBox
		float         Part;                // Доля окна атрибутов во всем окне
		int           CurFocus;            // Содержит имя активного ListBox'а

	static BOOL Register ( VOID );
	static int  Open ( LPSTR );                                // Открыть окно
	BOOL        Close ( VOID );                                // Закрыть окно
	BOOL        Save ( VOID );
	VOID        Size ( WPARAM, LPARAM );
	VOID        View ( int );
	VOID        PartMove ( UINT, LPARAM );
	VOID        DrawPart ( int, WORD );
	VOID        SetFocus ( VOID );

	VOID        FillAttListBox ( VOID );
	VOID        FillValListBox ( int );
	VOID        FillNoneView ( VOID );
	VOID        PrintOn ( VOID );

	int         MakeNoneIdx ( int, int );
	int         NoneIdx2Att ( int );
	int         NoneIdx2Val ( int );

	VOID        InsertAttribute ( VOID );
	VOID        InsertValue ( VOID );
	VOID        EditAttribute ( VOID );
	VOID        EditValue ( VOID );
	VOID        DeleteAttribute ( VOID );
	VOID        DeleteValue ( VOID );

	VOID        InitMenu ( VOID );

	VOID        DrawItemBmp ( LPDRAWITEMSTRUCT, int, BOOL );
	VOID        DrawItemText ( LPDRAWITEMSTRUCT, int );
	VOID        DrawItem ( LPDRAWITEMSTRUCT );

	VOID        ChangeWin ( VOID );
	BOOL        isAttSelected ( VOID );
	BOOL        isValSelected ( VOID );
	};
typedef STRUCT FAR   *LPSTRUCT;

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│                                                                     │ KNOW │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define KNOW_WIND_POS                4

class FAR KNOW {
    private:
    public:
		// Присущие этому классу акселератор, меню, имя и т.п.
		static HANDLE  hAccel;
		static HMENU   hMenu;
		static HMENU   hWndMenu;
		static HCURSOR VerCur;
		static HCURSOR HorCur;
		static HBITMAP hbmpAttIn;
		static HBITMAP hbmpAttOut;
		static HBITMAP hbmpVal;
		static char    Name[];

		LPKBASE       KBase;                         // Указатель на объект LK
		HWND          hWnd;                           // Основное окно (Child)
		HWND          hwndAtt;                     // Окно атрибутов (listbox)
		HWND          hwndVal;                      // Окно значений (listbox)
		int           SplitState;                         // Состояние ListBox
		float         Part;                // Доля окна атрибутов во всем окне
		int           CurFocus;            // Содержит имя активного ListBox'а

	static BOOL Register ( VOID );
	static int  Open ( LPSTR );                                // Открыть окно
	BOOL        Close ( VOID );                                // Закрыть окно
	BOOL        Save ( VOID );
	VOID        Size ( WPARAM, LPARAM );
	VOID        View ( int );
	VOID        PartMove ( UINT, LPARAM );
	VOID        DrawPart ( int, WORD );
	VOID        SetFocus ( VOID );

	VOID        FillAttListBox ( VOID );
	VOID        FillValListBox ( int );
	VOID        FillNoneView ( VOID );
	VOID        PrintOn ( VOID );

	int         CompSum ( int );
	int         MakeNoneIdx ( int, int );
	int         NoneIdx2Att ( int );
	int         NoneIdx2Val ( int );

	VOID        InsertProposition ( VOID );
	VOID        InsertValue ( VOID );
	VOID        InvertProposition ( VOID );
	VOID        EditValue ( VOID );
	VOID        DeleteProposition ( VOID );
	VOID        DeleteValue ( VOID );
	VOID        InsertRule ( VOID );
	VOID        DeleteRule ( VOID );
	VOID        IncreaseComp ( VOID );
	VOID        DecreaseComp ( VOID );
	VOID        ChangeComponent ( int, int );
	VOID        Compile ( VOID );

	VOID        InitMenu ( VOID );

	VOID        DrawItemBmp ( LPDRAWITEMSTRUCT, int, BOOL );
    VOID        DrawItemText ( LPDRAWITEMSTRUCT, int, int );
	VOID        DrawItem ( LPDRAWITEMSTRUCT );

    VOID        ChangeWin ( VOID );
	BOOL        isAttSelected ( VOID );
	BOOL        isValSelected ( VOID );
	VOID        FillPropositions ( HWND );
	};
typedef KNOW FAR   *LPKNOW;

/*
┌─────────────────────────────────────────────────────────────────────┬──────┐
│                                                                     │ DATA │
│                                                                     └──────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
#define DATA_WIND_POS                4

class FAR DATA {
    private:
    public:
		// Присущие этому классу акселератор, меню, имя и т.п.
		static HANDLE  hAccel;
		static HMENU   hMenu;
		static HMENU   hWndMenu;
		static HCURSOR VerCur;
		static HCURSOR HorCur;
		static HBITMAP hbmpAttIn;
		static HBITMAP hbmpAttOut;
		static HBITMAP hbmpVal;
		static char    Name[];

		LPKBASE       KBase;                         // Указатель на объект LK
		HWND          hWnd;                           // Основное окно (Child)
		HWND          hwndAtt;                     // Окно атрибутов (listbox)
		HWND          hwndVal;                      // Окно значений (listbox)
		int           SplitState;                         // Состояние ListBox
		float         Part;                // Доля окна атрибутов во всем окне
		int           CurFocus;            // Содержит имя активного ListBox'а

	static BOOL Register ( VOID );
	static int  Open ( LPSTR );                                // Открыть окно
	BOOL        Close ( VOID );                                // Закрыть окно
	BOOL        Save ( VOID );
	VOID        Size ( WPARAM, LPARAM );
	VOID        View ( int );
	VOID        PartMove ( UINT, LPARAM );
	VOID        DrawPart ( int, WORD );
	VOID        SetFocus ( VOID );

	VOID        FillAttListBox ( VOID );
	VOID        FillValListBox ( int );
	VOID        FillNoneView ( VOID );
	VOID        PrintOn ( VOID );

	int         CompSum ( int );
	int         MakeNoneIdx ( int, int );
	int         NoneIdx2Att ( int );
	int         NoneIdx2Val ( int );

	VOID        InsertProposition ( VOID );
	VOID        InsertValue ( VOID );
	VOID        InvertProposition ( VOID );
	VOID        EditValue ( VOID );
	VOID        DeleteProposition ( VOID );
	VOID        DeleteValue ( VOID );
	VOID        IncreaseComp ( VOID );
	VOID        DecreaseComp ( VOID );
	VOID        ChangeComponent ( int, int );

	VOID        InitMenu ( VOID );

	VOID        DrawItemBmp ( LPDRAWITEMSTRUCT, int, BOOL );
    VOID        DrawItemText ( LPDRAWITEMSTRUCT, int, int );
	VOID        DrawItem ( LPDRAWITEMSTRUCT );

    VOID        ChangeWin ( VOID );
	BOOL        isAttSelected ( VOID );
	BOOL        isValSelected ( VOID );
	VOID        FillPropositions ( HWND );
	};
typedef DATA FAR   *LPDATA;


extern int  ErrorMessager ( HWND, WORD, WORD,...);
extern LPSTR GetPathName ( LPSTR );
BOOL InitializeInstance ( LPSTR , WORD );
BOOL InitializeApplication ( VOID );
int PASCAL WinMain ( HANDLE, HANDLE, LPSTR, int );
VOID CommandHandler ( HWND, WPARAM, LPARAM );
VOID FileOpen ( VOID );
VOID ModuleDescription ( VOID );
BOOL FileSaveAs ( VOID );
BOOL CloseAll ( VOID );

extern BOOL FAR PASCAL _export ModDescriptionProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export AboutDlgProc ( HWND, UINT, WPARAM, LPARAM );
extern long FAR PASCAL _export FrameWndProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export FileOpenProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export SaveAsProc ( HWND, UINT, WPARAM, LPARAM );

extern long FAR PASCAL _export StructWndProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export AttributeProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export ValueProc ( HWND, UINT, WPARAM, LPARAM);

extern long FAR PASCAL _export KnowWndProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export PropositionProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export ComponentProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export StatusDlgProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export CompileOutProc ( int, int, int, int, int );

extern long FAR PASCAL _export DataWndProc ( HWND, UINT, WPARAM, LPARAM );
extern BOOL FAR PASCAL _export DataProposProc ( HWND, UINT, WPARAM, LPARAM );

inline LPVOID GetMem ( HWND hWnd ) { return (LPVOID) GetWindowLong ( hWnd, 0 ); }
inline VOID  SetMem ( HWND hWnd, LPVOID Ptr ) { SetWindowLong ( hWnd, 0, (DWORD) Ptr ); }



