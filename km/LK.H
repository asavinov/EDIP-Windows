// ����饭��, ���뫠��� ����� �� �믮������ ࠧ����� �㭪権
#define CHANGE_TITLE        1
#define CHANGE_CONTENT      2
#define CHANGE_COMPONENT    3
#define PUT_ATTRIBUTE       4
#define INS_ATTRIBUTE       5
#define DEL_ATTRIBUTE       6
#define PUT_VALUE           7
#define INS_VALUE           8
#define DEL_VALUE           9
#define INS_RULE            10                              // �� �ᯮ������
#define DEL_RULE            11
#define INS_PROP            12
#define DEL_PROP            13
#define INVERT_PROP         14

#define STAT_GENERATION     1
#define STAT_ABSORPTION     2

typedef BOOL (FAR PASCAL *COMPILEOUT) ( int, int, int, int, int );

typedef struct {
		unsigned char nec, pos;
		}             COMP;                         // ��������� ��᪠�뢠���
typedef COMP FAR     *LPCOMP;

/*
����������������������������������������������������������������������������Ŀ
� ������� ���ᠭ�� �����. �� �� ����� ������ ���짮��⥫�.   � MOD_DESCR �
�                                                                �����������Ĵ
������������������������������������������������������������������������������
*/
struct MOD_DESCR {
	char       Name[128];                                   // �������� �����
	char       Question[128];                             // ����� ��� �����
	char       Comment[1024];                            // �������਩ �����
	char       IconFile[80];                             // ���� ������ �����
	char       HyperFile[80];                              // ���� �����⥪��
	DWORD      HyperIndex;                      // ������ ����� � �����⥪��
	};

class FAR KBFILE;
typedef KBFILE FAR   *LPKBFILE;

class FAR KBASE;
typedef KBASE FAR  *LPKBASE;

struct HEAD_REC;
typedef HEAD_REC FAR  *LPHEAD_REC;

struct MOD_DESCR;
typedef MOD_DESCR FAR  *LPMOD_DESCR;

struct ATT_REC;
typedef ATT_REC FAR  *LPATT_REC;

struct VAL_REC;
typedef VAL_REC FAR  *LPVAL_REC;

struct ATT_IDX;
typedef ATT_IDX FAR  *LPATT_IDX;

struct VAL_IDX;
typedef VAL_IDX FAR  *LPVAL_IDX;

struct HEAD_RUL;
typedef HEAD_RUL FAR  *LPHEAD_RUL;

struct HEAD_PROP;
typedef HEAD_PROP FAR  *LPHEAD_PROP;

extern unsigned char UNITY;

/*
����������������������������������������������������������������������������Ŀ
� ��� ������� ����⮣� 䠩�� �� ᮧ������ ஢�� ���� ��ꥪ�.       � KBFILE �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/
#define BUF_SIZE             32000
#define PATHLENGTH           80

class FAR KBFILE {
public:
	static int      Error;
	static LPKBFILE lpFirstKBFile;
	static LPKBFILE lpLastKBFile;

	LPKBFILE   lpNextKBFile;
	LPKBFILE   lpPrevKBFile;

	// � ������ ��ꥪ� KBFILE �����ন������ ᯨ᮪ ��ꥪ⮢ KBASE
	LPKBASE    lpFirstKBase;                            // ���� ������ KBASE
	LPKBASE    lpLastKBase;                          // ��᫥���� ������ KBASE

	MOD_DESCR  ModDescr;                                    // ���ᠭ�� �����

	char       KBFilePath[PATHLENGTH];                              // ���� ��
	char       KBFileName[13];                                       // ��� ��
	int        isChanged;

	int        numAtt;                                 // ������⢮ ��ਡ�⮢
	int        numAllVal;                          // ������⢮ ��� ���祭��

	int        numRul;                           // ������⢮ ��室��� �ࠢ��
	int        numCon;                          // ������⢮ ����⮢ � ���
	int        numDis;                          // ������⢮ ����⮢ � ���

	long       lenRul;                                    // ����� ��� �ࠢ��
	long       lenDNF;                          // ����� ��� (��� ����⮢)
	long       lenCNF;                          // ����� ��� (��� ����⮢)

	int        isDNF;                                 // ��������஢��� �� ���
	int        isCNF;                                 // ��������஢��� �� ���

	LPATT_IDX  lpAttIdx;                      // �����⥫� �� ������ ��ਡ�⮢
	LPVAL_IDX  lpValIdx;                       // �����⥫� �� ������ ���祭��
	LPHEAD_RUL lpRul;                                  // �����⥫� �� �ࠢ���
	LPHEAD_RUL lpData;                                  // �����⥫� �� �����
	LPHEAD_RUL lpConc;                                  // �����⥫� �� �뢮��

	LPHEAD_RUL lpDNF;                                      // �����⥫� �� ���
	LPHEAD_RUL lpCNF;                                      // �����⥫� �� ���

// �㭪樨 �����㫨஢���� ��ꥪ⮬

	static LPKBFILE OpenKBFile ( LPSTR );                       // ������ 䠩�
	int            CloseKBFile ( void );                       // ������� 䠩�
	int            SaveKBFile ( void );                        // ���࠭��� ��
	int            RenameKBFile ( LPSTR );             // ���� ����� ��� 䠩��
	void           ResetCNF ( void );                          // ������ ���
	void           ResetDNF ( void );                          // ������ ���

	BOOL           isKBFileNamed ( void );                  // ��������� �� ��
	BOOL           isKBFileSaved ( void );                  // ���࠭��� �� ��
	static LPKBFILE isAlreadyOpened ( LPSTR );             // ����� �� 㦥 ��

// �㭪樨 ���஢��

	void       SortAttributes ( void );              // �����஢��� ��ਡ���
	void       SortAllValues ( void );           // �����஢��� �� ���祭��
	void       SortValues ( int );   // �����஢��� ���祭�� ������ ��ਡ��

	int        AttI2N ( int, int );              // �⭮�⥫�� � ��᮫���
	int        AttN2I ( int, int );              // ��᮫��� � �⭮�⥫��
	int        ValI2N ( int, int, int );         // �⭮�⥫�� � ��᮫���
	int        ValN2I ( int, int, int );         // ��᮫��� � �⭮�⥫��

	int        Prop2Att ( int, LPHEAD_RUL, int );
	int        Att2Prop ( int, LPHEAD_RUL, int );

// ��㯯��� (ᯨ���) �㭪樨

/*	firstThat
	lastThat

	forEach
	forEachKBase */
	void       forEachWindow ( int, DWORD );

	static int KBFilesNumber ( VOID );
	int        KBaseNumber ( VOID );

// �㭪樨 �����㫨஢���� � ����㯠 � ��ਡ�⠬

	LPATT_REC  GetAttribute ( int );                       // ������� ��ਡ��
	int        PutAttribute ( int, LPATT_REC );            // �������� ��ਡ��
	int        InsAttribute ( int, LPATT_REC );            // ��⠢��� ��ਡ��
	int        DelAttribute ( int );                        // ������� ��ਡ��

	int        AttNumber ();                           // ������⢮ ��ਡ�⮢

// �㭪樨 �����㫨஢���� � ����㯠 � ���祭��

	LPVAL_REC  GetValue ( int, int );                     // ������� ���祭��
	int        PutValue ( int, int, LPVAL_REC );          // �������� ���祭��
	int        InsValue ( int, int, LPVAL_REC );          // ��⠢��� ���祭��
	int        DelValue ( int, int );                      // ������� ���祭��

	int        ValNumber ( int );                       // ������⢮ ���祭��
	int        ValSum ( int );                // �㬬�୮� ������⢮ ���祭��
	int        ValSum ( int, int );
	int        ValSum ( int, LPHEAD_RUL, int );

// �㭪樨 �����㫨஢���� � ����㯠 � �ࠢ���� (ᥬ����᪨�)

	int        RuleNumber ();                        // ������⢮ �ࠢ�� � ��
	LPHEAD_RUL GetRuleHead ( int );           // ������� 㪠��⥫� �� �ࠢ���
	int        InsRule ( LPHEAD_RUL );              // ��⠢��� ���⮥ �ࠢ���
	int        DelRule ( LPHEAD_RUL );                      // ������� �ࠢ���

	void       InsAttInKBase ( int );   // ��������஢��� ���������� ��ਡ��
	void       InsAttInDNF ( int );
	void       InsAttInCNF ( int );
	LPHEAD_RUL InsAttInRule ( int, LPHEAD_RUL );

	void       DelAttInKBase ( int );     // ��������஢��� 㤠����� ��ਡ��
	LPHEAD_RUL DelAttInRule ( int, LPHEAD_RUL );

	void       InsValInKBase ( int, int );       // ��������஢��� ���������� ���祭��
	void       InsValInDNF ( int, int );
	void       InsValInCNF ( int, int );
	LPHEAD_RUL InsValInRule ( int, int, LPHEAD_RUL );
	LPHEAD_RUL InsValInData ( int, int );
	LPHEAD_RUL InsValInConc ( int, int );

	void       DelValInKBase ( int, int );     // ��������஢��� 㤠����� ���祭��
	LPHEAD_RUL DelValInRule ( int, int, LPHEAD_RUL );
	LPHEAD_RUL DelValInData ( int Att, int Val );
	LPHEAD_RUL DelValInConc ( int Att, int Val );

	int        PropNumber ( LPHEAD_RUL ); // ������⢮ ��᪠�뢠��� � �ࠢ���
	int        PropNumber ( int );
	LPHEAD_PROP GetPropHead ( LPHEAD_RUL, int );  // �����⥫� �� ��᪠�뢠���
	int        AbsProp2Att ( int, LPHEAD_RUL );
	int        AbsAtt2Prop ( int, LPHEAD_RUL );

	int        InsPropInData ( int, int );            // ��⠢��� ��᪠�뢠���
	int        InsPropInConc ( int, int );
	int        InsPropInRule ( LPHEAD_RUL, int, int );

	int        DelPropInData ( int );                  // ������� ��᪠�뢠���
	int        DelPropInConc ( int );
	int        DelPropInRule ( LPHEAD_RUL, int );
	int        DelPropInKBase ( int );

	int        InvertProp ( LPHEAD_RUL, int );   // ������஢��� ��᪠�뢠���
	int        InvertComps ( LPHEAD_RUL, int );      // ������஢��� �� ����

	LPCOMP     GetComp ( LPHEAD_PROP, int );            // ������� ����������
	LPCOMP     GetComp ( LPHEAD_RUL, int, int );

	// �����᪨� �㭪樨

	BOOL       CompileToCNF ( COMPILEOUT );
	int        DisIntoDis ( LPHEAD_RUL );
	VOID       ReduceDis ( LPHEAD_RUL );
	VOID       ReduceDisTo ( LPHEAD_RUL );
	VOID       ReduceConMax ( LPHEAD_RUL );
	VOID       ReduceConMaxTo ( LPHEAD_RUL );
	VOID       ReduceDisMaxTo ( LPHEAD_RUL );
	VOID       ExtendCon ( LPHEAD_RUL );
	VOID       ExtendDis ( LPHEAD_RUL );
	BOOL       AbsorbLast ( LPHEAD_RUL, int, long );
	BOOL       AbsorbDis ( LPHEAD_RUL, LPHEAD_RUL );
	BOOL       AbsorbConMax ( LPHEAD_RUL, LPHEAD_RUL );
	BOOL       ImposeConMax ( LPHEAD_RUL, LPHEAD_RUL );
	BOOL       ImposeConOnDis ( LPHEAD_RUL, LPHEAD_RUL );
	BOOL       ImposeProjectDisOnCon ( LPHEAD_RUL, LPHEAD_RUL );
	int        ProjectDis ( LPHEAD_RUL, LPCOMP );
	};

/*
����������������������������������������������������������������������������Ŀ
� �᭮���� ����� ��� ����㯠 � 䠩��� � �㭪�� LK EDIP.            � KBASE �
� �� ������ ����⨨ 䠩�� �� ᮧ������ ���� ��ꥪ� �⮣� �����.   �������Ĵ
� ����ন� ����� � �뢮��.                                                  �
������������������������������������������������������������������������������
*/
#define SORT_UNSORTED        1
#define SORT_ALPHA           2
#define SORT_VALNUM          3
#define SORT_PRICE           4
#define SORT_IMPORTANCE      5
#define SORT_CHARACTER       6
#define SORT_ASINRULE        7

#define VIEW_STRUCT         1
#define VIEW_KNOWLEDGE      2
#define VIEW_DATA           3
#define VIEW_CONCLUSION     4

class FAR KBASE {
public:
	static int Error;

	LPKBASE    lpNextKBase;                    // �����⥫� �� ᫥���騩 KBASE
	LPKBASE    lpPrevKBase;                   // �����⥫� �� �।��騩 KBASE

	LPKBFILE   lpHost;                           // �����⥫� �� ��ꥪ� KBFILE

	int        curAtt;                               // ����騩 ����� ��ਡ��
	int        curVal;                               // ����騩 ����� ���祭��
	int        curRul;                      // ����騩 ����� ��室���� �ࠢ���

	LPHEAD_RUL lpRul;                          // �����⥫� �� ⥪�饥 �ࠢ���

	int        curAttSort;                     // ������ ���஢�� ��ਡ�⮢
	int        curValSort;                      // ������ ���஢�� ���祭��

	int        View;                              // �� ��ᬠ�ਢ���� � ����
	HWND       hWnd;                                         // ���� ��ᬮ��

// �㭪樨 �����㫨஢���� ��ꥪ⮬

	static LPKBASE OpenKBase ( LPSTR );                         // ���������
	int        CloseKBase ( VOID );                              // ��������
	int        SaveKBase ( VOID );                             // ���࠭��� ��
	int        RenameKBase ( LPSTR );                  // ���� ����� ��� 䠩��
	int        MoveKBase ( LPKBFILE );           // ��।��� ���� ������ 䠩��

	BOOL       isKBaseNamed ( VOID );                       // ��������� �� ��
	BOOL       isKBaseSaved ( VOID );                       // ���࠭��� �� ��

	HWND       SetWindow ( HWND );                     // ��⠭����� ���� ����
	HWND       GetWindow ( VOID );                       // ������� ���� ����

	int        SetView ( int );                              // ��⠭����� ���
	int        GetView ( VOID );                               // ������� ���

	LPSTR      GetNamePtr ( VOID ) { return lpHost->KBFileName; };// ������� 㪠��⥫� �� ��� 䠩��
	LPSTR      GetPathPtr ( VOID ) { return lpHost->KBFilePath; };// ������� 㪠��⥫� �� ����� ���� 䠩��

	int        GetModuleHead ( LPMOD_DESCR );      // ������� ���ᠭ�� �����
	int        SetModuleHead ( LPMOD_DESCR );     // ���࠭��� ���ᠭ�� �����

// ��㯯��� (ᯨ���) �㭪樨

/*	firstThat
	lastThat

	forEach
	forEachKBase */

	int        KBFilesNumber ( VOID );
	int        KBaseNumber ( VOID );

	LPKBASE    GetFirstKBase ( VOID ) { return lpHost->lpFirstKBase; };
	LPKBASE    GetNextKBase ( VOID ) { return lpNextKBase; }

// �㭪樨 �����㫨஢���� � ����㯠 � ��ਡ�⠬

	int        GetAttribute ( int, LPATT_REC );            // ������� ��ਡ��
	int        GetAttribute ( LPATT_REC );
	int        PutAttribute ( LPATT_REC );                 // �������� ��ਡ��
	int        InsAttribute ( LPATT_REC );                 // ��⠢��� ��ਡ��
	int        DelAttribute ( VOID );                       // ������� ��ਡ��

	int        AttNumber ( VOID ) { return lpHost->numAtt; };    // ������⢮ ��ਡ�⮢

	int        GetCurAtt ( VOID );                 // ������� ⥪�騩 ��ਡ��
	int        SetCurAtt ( int );                // ��⠭����� ⥪�騩 ��ਡ��

	int        GetCurAttPos ( VOID );    // ������� ������ ��ਡ�� � ᯨ᪥
	int        SetCurAttPos ( int );   // ��⠭����� ������ ��ਡ�� � ᯨ᪥
	int        NextAttPos ( VOID ) { return SetCurAttPos ( GetCurAttPos() + 1 ); };
	int        PrevAttPos ( VOID ) { return SetCurAttPos ( GetCurAttPos() - 1 ); };

	int        GetAttSort ( VOID );   // ������� ⥪���� ���஢�� ��ਡ�⮢
	int        SetAttSort ( int );            // ��⠭����� ⥪���� ���஢��

	int        Prop2Att ( int );                            // ������ � �����
	int        Att2Prop ( int );                            // ����� � ������
	int        Name2Att ( LPSTR );                   // �� ����� ���� ��ਡ��
	BOOL       isAttEnabled ( int );             // �室�� �� ��ਡ�� � ᯨ᮪

// �㭪樨 �����㫨஢���� � ����㯠 � ���祭��

	int        GetValue ( int, int, LPVAL_REC );          // ������� ���祭��
	int        GetValue ( LPVAL_REC );
	int        PutValue ( LPVAL_REC );                    // �������� ���祭��
	int        InsValue ( LPVAL_REC );                    // ��⠢��� ���祭��
	int        DelValue ( VOID );                          // ������� ���祭��

	int        ValNumber ( int );                       // ������⢮ ���祭��
	int        ValNumber ( VOID );

	int        ValPosSum ( int );             // �㬬�୮� ������⢮ ���祭��

	int        GetCurVal ( VOID );                // ������� ⥪�饥 ���祭��
	int        SetCurVal ( int );               // ��⠭����� ⥪�饥 ���祭��
	int        NextVal ( VOID ) { return SetCurVal ( GetCurVal() + 1 ); };
	int        PrevVal ( VOID ) { return SetCurVal ( GetCurVal() - 1 ); };

	int        GetValSort ( VOID );    // ������� ⥪���� ���஢�� ���祭��
	int        SetValSort ( int );            // ��⠭����� ⥪���� ���஢��

	// �㭪樨 �����㫨஢���� �ࠢ�����
	int        InsRule ( VOID );      // ��⠢��� ���⮥ �ࠢ��� ��। ⥪�騬
	int        DelRule ( VOID );                    // ������� ⥪�饥 �ࠢ���

	int        RuleNumber ( VOID ) { return lpHost->numRul; };  // ������⢮ �ࠢ�� � ��

	int        GetCurRule ( VOID ) { return curRul; };      // ������� ����� ⥪�饣� �ࠢ���
	int        SetCurRule ( int );        // ��⠭����� ����� ⥪�饣� �ࠢ���
	int        NextRule ( VOID ) { return SetCurRule ( GetCurRule() + 1 ); };// ��३� � ᫥���饬� �ࠢ���
	int        PrevRule ( VOID ) { return SetCurRule ( GetCurRule() - 1 ); };// ��३� � �।��饬� �ࠢ���

	BOOL       isRuleDisjunct ( VOID );           // ����饥 �ࠢ��� �����?
	BOOL       SetRuleDisjunct ( BOOL );         // ��⠭����� ����⨢�����

	// �㭪樨 �����㫨஢���� ��᪠�뢠��ﬨ
	int        InsProp ( int );           // ��⠢��� �ਢ���쭮� ��᪠�뢠���
	int        DelProp ( VOID );               // ������� ⥪�饥 ��᪠�뢠���

	int        InvertProp ( int );               // ������஢��� ��᪠�뢠���
	int        InvertProp ( VOID );      // ������஢��� ⥪�饥 ��᪠�뢠���
	int        InvertComps ( VOID );       // ������஢��� �� ���� ⥪�饣� ��᪠�뢠���

	int        PropNumber ( VOID );                 // ������⢮ ��᪠�뢠���

	BOOL       isPropConjunct ( int );               // ��᪠�뢠��� �����?
	BOOL       SetPropConjunct ( int, BOOL );    // ��⠭����� ����⨢�����

	// �㭪樨 �����㫨஢���� ��������⠬�

	int        GetComp ( int, int, LPCOMP );            // ������� ����������
	int        SetComp ( int, int, LPCOMP );          // ��⠭����� ����������


	// �����᪨� �㭪樨
	int        InferenceCNF ( VOID );
	int        InferencePassCNF ( LPHEAD_RUL, LPHEAD_RUL, LPHEAD_RUL );
	VOID       InferenceDisCNF ( LPHEAD_RUL, LPHEAD_RUL, LPHEAD_RUL );

	VOID       TransferConIntoWindow ( LPHEAD_RUL );

	};

/*
����������������������������������������������������������������������������Ŀ
� ������-��������� 䠩�� ���� ������                              � HEAD_REC �
�                                                                 ����������Ĵ
������������������������������������������������������������������������������
*/
struct HEAD_REC {
	char       CopyWrite[80];
	WORD       Version;

	int        numAtt;                                 // ������⢮ ��ਡ�⮢
	int        numAllVal;                          // ������⢮ ��� ���祭��

	int        numRul;                           // ������⢮ ��室��� �ࠢ��
	int        numCon;                          // ������⢮ ����⮢ � ���
	int        numDis;                          // ������⢮ ����⮢ � ���

	long       lenRul;                                  // ����� ���� �ࠢ��
	long       lenDNF;                             // ���� ����� ��� � �����
	long       lenCNF;                             // ���� ����� ��� � �����

	long       ofsAtt;                         // ���饭�� �� ��ਡ�⮢ (����)
	long       ofsVal;                          // ���饭�� �� ���祭�� (����)
	long       ofsRul;                   // ���饭�� �� ��室��� �ࠢ�� (����)
	long       ofsDNF;                             // ���饭�� �� ��� � �����
	long       ofsCNF;                             // ���饭�� �� ��� � �����

	int        isDNF;                                 // ��������஢��� �� ���
	int        isCNF;                                 // ��������஢��� �� ���

	float      Entropy;                                // ���ய�� ���� ������
	float      Correlation;              // ���५��� �ਧ����� � ���� ������

	MOD_DESCR  ModDescr;                                    // ���ᠭ�� �����
	};

/*
����������������������������������������������������������������������������Ŀ
� ������ �� ��ਡ�� 䠩�� ���� ������                             � ATT_REC �
�                                                                  ���������Ĵ
������������������������������������������������������������������������������
*/
#define ATT_NAME_LEN         128
#define ATT_QUE_LEN          128
#define ATT_CMNT_LEN         256

struct ATT_REC {
	char         Name[ATT_NAME_LEN];                // ��� (��������) ��ਡ��
	char         Question[ATT_QUE_LEN];        // ����� �⭮�⥫쭮 ��ਡ��
	char         Comment[ATT_CMNT_LEN];                // �������਩ ��ਡ��

	DWORD        Style;                                      // �⨫� �ਧ����

	char         Price;                             // ���� ����७�� ��ਡ��
	char         Importance;                              // �������� ��ਡ��

	int          numVal;                                // ������⢮ ���祭��
	DWORD        HyperIndex;                           // ������ � �����⥪��
	};

/*
����������������������������������������������������������������������������Ŀ
� ������ � ���祭�� 䠩�� ���� ������                              � VAL_REC �
�                                                                  ���������Ĵ
������������������������������������������������������������������������������
*/
#define VAL_NAME_LEN         80
#define VAL_QUE_LEN          80
#define VAL_CMNT_LEN         128

struct VAL_REC {
	char         Name[VAL_NAME_LEN];                // ��� (��������) ���祭��
	char         Question[VAL_QUE_LEN];        // ����� �⭮�⥫쭮 ���祭��
	char         Comment[VAL_CMNT_LEN];                // �������਩ ���祭��

	DWORD        Style;                                      // �⨫� ���祭��

	char         Price;                             // ���� ����७�� ���祭��
	char         Importance;                              // �������� ���祭��
	char         Character;                          // ��ࠪ�୮��� ���祭��

	DWORD        HyperIndex;                           // ������ � �����⥪��
	};

/*
����������������������������������������������������������������������������Ŀ
� ������� ������ ��ਡ�⮢.                                       � ATT_IDX �
� ������ ��ਡ�⮢ - �� ������᪨� ���ᨢ ������⮢ �����       ���������Ĵ
� ATT_IDX, ������⢮ ������ ࠢ�� ��� ��ਡ�⮢. ���ᠭ�� ��ਡ��       �
� ����砥� � ᥡ� ��� ���� ����� ATT_REC (���ᠭ�� ��ਡ�� �� 䠩��).      �
������������������������������������������������������������������������������
*/
struct ATT_IDX {
	ATT_REC    AttRec;

	int        idxName;
	int        idxPrice;
	int        idxImportance;
	int        idxNumVal;
	};

/*
����������������������������������������������������������������������������Ŀ
� ������� ������ ���祭��.                                        � VAL_IDX �
� ������ ���祭�� - �� �������᪨� ���ᨢ ������⮢ �����       ���������Ĵ
� VAL_IDX, ������⢮ ������ ࠢ�� �㬬� ��� ���票�. ���砫� ����         �
� �����浪� ���祭�� �㫥���� ��ਡ��, ��⥬ ��ࢮ�� � �.�. (��� �          �
� ᥪ樮��஢����� �����). ���ᠭ�� ���祭�� ����砥� � ᥡ� ��� ����     �
� ����� VAL_REC (���ᠭ�� ���祭�� ��� 䠩��).                               �
������������������������������������������������������������������������������
*/
struct VAL_IDX {
	VAL_REC    ValRec;

	int        idxName;
	int        idxPrice;
	int        idxImportance;
	int        idxCharacter;
	};

/*
����������������������������������������������������������������������������Ŀ
� ��������� �ࠢ���                                               � HEAD_RUL �
�                                                                 ����������Ĵ
������������������������������������������������������������������������������
*/
#define RUL_DISJUNCT         0x0001                      // �᫨ � �離�� OR

struct HEAD_RUL {
	int        numProp;                 // ������⢮ ��᪠�뢠��� (��ਡ�⮢)
	int        lenRul;             // ����� �ࠢ��� � ����� ������ ���������
	COMP       Const;        // ����⠭� �ࠢ��� (�������� �� �������樨)
	WORD       Style;                      // �⨫� (�����/����� � �.�.)
	int        numParent1;                                  // ���� த�⥫�
	int        numParent2;                                  // ��ன த�⥫�
	COMP       RuleProp1;                            // ��᪠�뢠��� � �ࠢ���
	COMP       RuleProp2;
	};

/*
����������������������������������������������������������������������������Ŀ
� ��������� ��᪠�뢠���                                         � HEAD_PROP �
�                                                                �����������Ĵ
������������������������������������������������������������������������������
*/
#define PROP_CONJUNCT        0x0001                     // �᫨ � �離�� AND

struct HEAD_PROP {
	int        curAtt;     // � ����� ��ਡ�� ������ ��᪠�뢠��� (���.�����)
	WORD       Style;                      // �⨫� (�����/����� � �.�.)
	};

/*
����������������������������������������������������������������������������Ŀ
� ���� �訡��                                                       � ERRORS �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/
#define LKE_SUCCESS          0
#define LKE_GENERAL          1
#define LKE_ALLOCMEM         2
#define LKE_CANTOPEN         3
#define LKE_CANTREAD         4
#define LKE_CANTWRITE        5
#define LKE_NOTUNIQUE        6
#define LKE_NOCURRENTPROP    7              // ����饣� ��ਡ�� ��� � �ࠢ���
#define LKE_NOTCOMPILED      8                           // �� �⪮�����஢���
#define LKE_NOGOAL           9
#define LKE_NOTEDIPFILE      10

/*
����������������������������������������������������������������������������Ŀ
�                                                                   �        �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/

#define LK_COPYWRITE         "Logical Kernel EDIP Version 1.0, January 1993, (c) Alexander Savinov"
#define LK_VERSION           0x0100                              // ����� 1.0

/*
����������������������������������������������������������������������������Ŀ
� ���⨯� �ᯮ��㥬�� �㭪権                                    �        �
�                                                                   ��������Ĵ
������������������������������������������������������������������������������
*/
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

BOOL     InsDelMem ( LPVOID, long, LPVOID, int ); // �������� ��� ࠧ������� ������
long     MemorySize ( LPVOID );
int      ReadFile ( int, long, long, LPSTR );
int      WriteFile ( int, long, long, LPSTR );
int      isFileValid ( LPSTR );
int      isFileValid ( int );
LPSTR    GetNameFromPath ( LPSTR );                  // ���� � ��� ��� 䠩��
VOID     InitFilesCounter ( VOID );
LPKBASE  GetNextFileObject ( VOID );
VOID     AddBakExt ( LPSTR );

LPVOID LK_malloc ( unsigned long );
LPVOID LK_realloc ( LPVOID, unsigned long );
HGLOBAL LK_free ( LPVOID );


