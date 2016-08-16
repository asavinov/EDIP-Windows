// ‘®®¡é¥­¨ï, ¯®áë« ¥¬ë¥ ®ª­ ¬ ¯à¨ ¢ë¯®«­¥­¨¨ à §«¨ç­ëå äã­ªæ¨©
#define CHANGE_TITLE        1
#define CHANGE_CONTENT      2
#define CHANGE_COMPONENT    3
#define PUT_ATTRIBUTE       4
#define INS_ATTRIBUTE       5
#define DEL_ATTRIBUTE       6
#define PUT_VALUE           7
#define INS_VALUE           8
#define DEL_VALUE           9
#define INS_RULE            10                              // ¥ ¨á¯®«ì§ã¥âáï
#define DEL_RULE            11
#define INS_PROP            12
#define DEL_PROP            13
#define INVERT_PROP         14

#define STAT_GENERATION     1
#define STAT_ABSORPTION     2

typedef BOOL (FAR PASCAL *COMPILEOUT) ( int, int, int, int, int );

typedef struct {
		unsigned char nec, pos;
		}             COMP;                         // Š®¬¯®­¥­â  ¢ëáª §ë¢ ­¨¥
typedef COMP FAR     *LPCOMP;

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄ¿
³ ‘âàãªâãà  ®¯¨á ­¨ï ¬®¤ã«ï. ’® çâ® ¬®¦¥â ¬¥­ïâì ¯®«ì§®¢ â¥«ì.   ³ MOD_DESCR ³
³                                                                ÀÄÄÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
struct MOD_DESCR {
	char       Name[128];                                   //  §¢ ­¨¥ ¬®¤ã«ï
	char       Question[128];                             // ‚®¯à®á ¤«ï ¬®¤ã«ï
	char       Comment[1024];                            // Š®¬¬¥­â à¨© ¬®¤ã«ï
	char       IconFile[80];                             // ” ©« ¨ª®­ª¨ ¬®¤ã«ï
	char       HyperFile[80];                              // ” ©« £¨¯¥àâ¥ªáâ 
	DWORD      HyperIndex;                      // ˆ­¤¥ªá ¬®¤ã«ï ¢ £¨¯¥àâ¥ªáâ¥
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
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
³ „«ï ª ¦¤®£® ®âªàëâ®£® ä ©«  ‡ á®§¤ ¥âáï à®¢­® ®¤¨­ ®¡ê¥ªâ.       ³ KBFILE ³
³                                                                   ÀÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
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

	// ‚ ª ¦¤®¬ ®¡ê¥ªâ¥ KBFILE ¯®¤¤¥à¦¨¢ ¥âáï á¯¨á®ª ®¡ê¥ªâ®¢ KBASE
	LPKBASE    lpFirstKBase;                            // ¥à¢ë© í«¥¬­â KBASE
	LPKBASE    lpLastKBase;                          // ®á«¥¤­¨© í«¥¬­â KBASE

	MOD_DESCR  ModDescr;                                    // ¯¨á ­¨¥ ¬®¤ã«ï

	char       KBFilePath[PATHLENGTH];                              // ¯ãâì ‡
	char       KBFileName[13];                                       // ¨¬ï ‡
	int        isChanged;

	int        numAtt;                                 // Š®«¨ç¥áâ¢®  âà¨¡ãâ®¢
	int        numAllVal;                          // Š®«¨ç¥áâ¢® ¢á¥å §­ ç¥­¨©

	int        numRul;                           // Š®«¨ç¥áâ¢® ¨áå®¤­ëå ¯à ¢¨«
	int        numCon;                          // Š®«¨ç¥áâ¢® ª®­êî­ªâ®¢ ¢ „”
	int        numDis;                          // Š®«¨ç¥áâ¢® ¤¨§êî­ªâ®¢ ¢ Š”

	long       lenRul;                                    // „«¨­  ¢á¥å ¯à ¢¨«
	long       lenDNF;                          // „«¨­  „” (¢á¥å ª®­êî­ªâ®¢)
	long       lenCNF;                          // „«¨­  Š” (¢á¥å ¤¨§êî­ªâ®¢)

	int        isDNF;                                 // ‘ª®¬¯¨«¨à®¢ ­  «¨ „”
	int        isCNF;                                 // ‘ª®¬¯¨«¨à®¢ ­  «¨ Š”

	LPATT_IDX  lpAttIdx;                      // “ª § â¥«ì ­  ¨­¤¥ªá  âà¨¡ãâ®¢
	LPVAL_IDX  lpValIdx;                       // “ª § â¥«ì ­  ¨­¤¥ªá §­ ç¥­¨©
	LPHEAD_RUL lpRul;                                  // “ª § â¥«ì ­  ¯à ¢¨« 
	LPHEAD_RUL lpData;                                  // “ª § â¥«ì ­  ¤ ­­ë¥
	LPHEAD_RUL lpConc;                                  // “ª § â¥«ì ­  ¢ë¢®¤ë

	LPHEAD_RUL lpDNF;                                      // “ª § â¥«ì ­  „”
	LPHEAD_RUL lpCNF;                                      // “ª § â¥«ì ­  Š”

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ®¡ê¥ªâ®¬

	static LPKBFILE OpenKBFile ( LPSTR );                       // âªàëâì ä ©«
	int            CloseKBFile ( void );                       // ‡ ªàëâì ä ©«
	int            SaveKBFile ( void );                        // ‘®åà ­¨âì ‡
	int            RenameKBFile ( LPSTR );             // „ âì ­®¢®¥ ¨¬ï ä ©«ã
	void           ResetCNF ( void );                          // ‘¡à®á¨âì Š”
	void           ResetDNF ( void );                          // ‘¡à®á¨âì „”

	BOOL           isKBFileNamed ( void );                  // ˆ¬¥­®¢ ­  «¨ ‡
	BOOL           isKBFileSaved ( void );                  // ‘®åà ­¥­  «¨ ‡
	static LPKBFILE isAlreadyOpened ( LPSTR );             // âªàëâ  «¨ ã¦¥ ‡

// ”ã­ªæ¨¨ á®àâ¨à®¢ª¨

	void       SortAttributes ( void );              // âá®àâ¨à®¢ âì  âà¨¡ãâë
	void       SortAllValues ( void );           // âá®àâ¨à®¢ âì ¢á¥ §­ ç¥­¨ï
	void       SortValues ( int );   // âá®àâ¨à®¢ âì §­ ç¥­¨ï ®¤­®£®  âà¨¡ãâ 

	int        AttI2N ( int, int );              // â­®á¨â¥«ì­ë© ¢  ¡á®«îâ­ë©
	int        AttN2I ( int, int );              // €¡á®«îâ­ë© ¢ ®â­®á¨â¥«ì­ë©
	int        ValI2N ( int, int, int );         // â­®á¨â¥«ì­ë© ¢  ¡á®«îâ­ë©
	int        ValN2I ( int, int, int );         // €¡á®«îâ­ë© ¢ ®â­®á¨â¥«ì­ë©

	int        Prop2Att ( int, LPHEAD_RUL, int );
	int        Att2Prop ( int, LPHEAD_RUL, int );

// ƒàã¯¯®¢ë¥ (á¯¨á®ç­ë¥) äã­ªæ¨¨

/*	firstThat
	lastThat

	forEach
	forEachKBase */
	void       forEachWindow ( int, DWORD );

	static int KBFilesNumber ( VOID );
	int        KBaseNumber ( VOID );

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¨ ¤®áâã¯  ª  âà¨¡ãâ ¬

	LPATT_REC  GetAttribute ( int );                       // ®«ãç¨âì  âà¨¡ãâ
	int        PutAttribute ( int, LPATT_REC );            // ®«®¦¨âì  âà¨¡ãâ
	int        InsAttribute ( int, LPATT_REC );            // ‚áâ ¢¨âì  âà¨¡ãâ
	int        DelAttribute ( int );                        // “¤ «¨âì  âà¨¡ãâ

	int        AttNumber ();                           // Š®«¨ç¥áâ¢®  âà¨¡ãâ®¢

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¨ ¤®áâã¯  ª §­ ç¥­¨ï¬

	LPVAL_REC  GetValue ( int, int );                     // ®«ãç¨âì §­ ç¥­¨¥
	int        PutValue ( int, int, LPVAL_REC );          // ®«®¦¨âì §­ ç¥­¨¥
	int        InsValue ( int, int, LPVAL_REC );          // ‚áâ ¢¨âì §­ ç¥­¨¥
	int        DelValue ( int, int );                      // “¤ «¨âì §­ ç¥­¨¥

	int        ValNumber ( int );                       // Š®«¨ç¥áâ¢® §­ ç¥­¨©
	int        ValSum ( int );                // ‘ã¬¬ à­®¥ ª®«¨ç¥áâ¢® §­ ç¥­¨©
	int        ValSum ( int, int );
	int        ValSum ( int, LPHEAD_RUL, int );

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¨ ¤®áâã¯  ª ¯à ¢¨« ¬ (á¥¬ ­â¨ç¥áª¨¥)

	int        RuleNumber ();                        // Š®«¨ç¥áâ¢® ¯à ¢¨« ¢ ‡
	LPHEAD_RUL GetRuleHead ( int );           // ®«ãç¨âì ãª § â¥«ì ­  ¯à ¢¨«®
	int        InsRule ( LPHEAD_RUL );              // ‚áâ ¢¨âì ¯ãáâ®¥ ¯à ¢¨«®
	int        DelRule ( LPHEAD_RUL );                      // “¤ «¨âì ¯à ¢¨«®

	void       InsAttInKBase ( int );   // ‘ª®¬¯¥­á¨à®¢ âì ¤®¡ ¢«¥­¨¥  âà¨¡ãâ 
	void       InsAttInDNF ( int );
	void       InsAttInCNF ( int );
	LPHEAD_RUL InsAttInRule ( int, LPHEAD_RUL );

	void       DelAttInKBase ( int );     // ‘ª®¬¯¥­á¨à®¢ âì ã¤ «¥­¨¥  âà¨¡ãâ 
	LPHEAD_RUL DelAttInRule ( int, LPHEAD_RUL );

	void       InsValInKBase ( int, int );       // ‘ª®¬¯¥­á¨à®¢ âì ¤®¡ ¢«¥­¨¥ §­ ç¥­¨ï
	void       InsValInDNF ( int, int );
	void       InsValInCNF ( int, int );
	LPHEAD_RUL InsValInRule ( int, int, LPHEAD_RUL );
	LPHEAD_RUL InsValInData ( int, int );
	LPHEAD_RUL InsValInConc ( int, int );

	void       DelValInKBase ( int, int );     // ‘ª®¬¯¥­á¨à®¢ âì ã¤ «¥­¨¥ §­ ç¥­¨ï
	LPHEAD_RUL DelValInRule ( int, int, LPHEAD_RUL );
	LPHEAD_RUL DelValInData ( int Att, int Val );
	LPHEAD_RUL DelValInConc ( int Att, int Val );

	int        PropNumber ( LPHEAD_RUL ); // Š®«¨ç¥áâ¢® ¢ëáª §ë¢ ­¨© ¢ ¯à ¢¨«¥
	int        PropNumber ( int );
	LPHEAD_PROP GetPropHead ( LPHEAD_RUL, int );  // “ª § â¥«ì ­  ¢ëáª §ë¢ ­¨¥
	int        AbsProp2Att ( int, LPHEAD_RUL );
	int        AbsAtt2Prop ( int, LPHEAD_RUL );

	int        InsPropInData ( int, int );            // ‚áâ ¢¨âì ¢ëáª §ë¢ ­¨¥
	int        InsPropInConc ( int, int );
	int        InsPropInRule ( LPHEAD_RUL, int, int );

	int        DelPropInData ( int );                  // “¤ «¨âì ¢ëáª §ë¢ ­¨¥
	int        DelPropInConc ( int );
	int        DelPropInRule ( LPHEAD_RUL, int );
	int        DelPropInKBase ( int );

	int        InvertProp ( LPHEAD_RUL, int );   // ˆ­¢¥àâ¨à®¢ âì ¢ëáª §ë¢ ­¨¥
	int        InvertComps ( LPHEAD_RUL, int );      // ˆ­¢¥àâ¨à®¢ âì ¢á¥ ª®¬¯

	LPCOMP     GetComp ( LPHEAD_PROP, int );            // ®«ãç¨âì ª®¬¯®­¥­âã
	LPCOMP     GetComp ( LPHEAD_RUL, int, int );

	// ‹®£¨ç¥áª¨¥ äã­ªæ¨¨

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
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ¿
³ á­®¢­®© ª« áá ¤«ï ¤®áâã¯  ª ä ©« ¬ ¨ äã­ªæ¨ï¬ LK EDIP.            ³ KBASE ³
³ à¨ ª ¦¤®¬ ®âªàëâ¨¨ ä ©«  ‡ á®§¤ ¥âáï ®¤¨­ ®¡ê¥ªâ íâ®£® ª« áá .   ÀÄÄÄÄÄÄÄ´
³ ‘®¤¥à¦¨â ¤ ­­ë¥ ¨ ¢ë¢®¤ë.                                                  ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
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

	LPKBASE    lpNextKBase;                    // “ª § â¥«ì ­  á«¥¤ãîé¨© KBASE
	LPKBASE    lpPrevKBase;                   // “ª § â¥«ì ­  ¯à¥¤ë¤ãé¨© KBASE

	LPKBFILE   lpHost;                           // “ª § â¥«ì ­  ®¡ê¥ªâ KBFILE

	int        curAtt;                               // ’¥ªãé¨© ­®¬¥à  âà¨¡ãâ 
	int        curVal;                               // ’¥ªãé¨© ­®¬¥à §­ ç¥­¨ï
	int        curRul;                      // ’¥ªãé¨© ­®¬¥à ¨áå®¤­®£® ¯à ¢¨« 

	LPHEAD_RUL lpRul;                          // “ª § â¥«ì ­  â¥ªãé¥¥ ¯à ¢¨«®

	int        curAttSort;                     // ’¥ªãé ï á®àâ¨à®¢ª   âà¨¡ãâ®¢
	int        curValSort;                      // ’¥ªãé ï á®àâ¨à®¢ª  §­ ç¥­¨©

	int        View;                              // —â® ¯à®á¬ âà¨¢ âáï ¢ ®ª­¥
	HWND       hWnd;                                         // ª­® ¯à®á¬®âà 

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ®¡ê¥ªâ®¬

	static LPKBASE OpenKBase ( LPSTR );                         // Š®­áâàãªâ®à
	int        CloseKBase ( VOID );                              // „¥áâàãªâ®à
	int        SaveKBase ( VOID );                             // ‘®åà ­¨âì ‡
	int        RenameKBase ( LPSTR );                  // „ âì ­®¢®¥ ¨¬ï ä ©«ã
	int        MoveKBase ( LPKBFILE );           // ¥à¥¤ âì ®ª­® ­®¢®¬ã ä ©«ã

	BOOL       isKBaseNamed ( VOID );                       // ˆ¬¥­®¢ ­  «¨ ‡
	BOOL       isKBaseSaved ( VOID );                       // ‘®åà ­¥­  «¨ ‡

	HWND       SetWindow ( HWND );                     // “áâ ­®¢¨âì ª«îç ®ª­ 
	HWND       GetWindow ( VOID );                       // ®«ãç¨âì ª«îç ®ª­ 

	int        SetView ( int );                              // “áâ ­®¢¨âì ¢¨¤
	int        GetView ( VOID );                               // ®«ãç¨âì ¢¨¤

	LPSTR      GetNamePtr ( VOID ) { return lpHost->KBFileName; };// ®«ãç¨âì ãª § â¥«ì ­  ¨¬ï ä ©« 
	LPSTR      GetPathPtr ( VOID ) { return lpHost->KBFilePath; };// ®«ãç¨âì ãª § â¥«ì ­  ¯®«­ë© ¯ãâì ä ©« 

	int        GetModuleHead ( LPMOD_DESCR );      // ®«ãç¨âì ®¯¨á ­¨¥ ¬®¤ã«ï
	int        SetModuleHead ( LPMOD_DESCR );     // ‘®åà ­¨âì ®¯¨á ­¨¥ ¬®¤ã«ï

// ƒàã¯¯®¢ë¥ (á¯¨á®ç­ë¥) äã­ªæ¨¨

/*	firstThat
	lastThat

	forEach
	forEachKBase */

	int        KBFilesNumber ( VOID );
	int        KBaseNumber ( VOID );

	LPKBASE    GetFirstKBase ( VOID ) { return lpHost->lpFirstKBase; };
	LPKBASE    GetNextKBase ( VOID ) { return lpNextKBase; }

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¨ ¤®áâã¯  ª  âà¨¡ãâ ¬

	int        GetAttribute ( int, LPATT_REC );            // ®«ãç¨âì  âà¨¡ãâ
	int        GetAttribute ( LPATT_REC );
	int        PutAttribute ( LPATT_REC );                 // ®«®¦¨âì  âà¨¡ãâ
	int        InsAttribute ( LPATT_REC );                 // ‚áâ ¢¨âì  âà¨¡ãâ
	int        DelAttribute ( VOID );                       // “¤ «¨âì  âà¨¡ãâ

	int        AttNumber ( VOID ) { return lpHost->numAtt; };    // Š®«¨ç¥áâ¢®  âà¨¡ãâ®¢

	int        GetCurAtt ( VOID );                 // ®«ãç¨âì â¥ªãé¨©  âà¨¡ãâ
	int        SetCurAtt ( int );                // “áâ ­®¢¨âì â¥ªãé¨©  âà¨¡ãâ

	int        GetCurAttPos ( VOID );    // ®«ãç¨âì ¯®§¨æ¨î  âà¨¡ãâ  ¢ á¯¨áª¥
	int        SetCurAttPos ( int );   // “áâ ­®¢¨âì ¯®§¨æ¨î  âà¨¡ãâ  ¢ á¯¨áª¥
	int        NextAttPos ( VOID ) { return SetCurAttPos ( GetCurAttPos() + 1 ); };
	int        PrevAttPos ( VOID ) { return SetCurAttPos ( GetCurAttPos() - 1 ); };

	int        GetAttSort ( VOID );   // ®«ãç¨âì â¥ªãéãî á®àâ¨à®¢ªã  âà¨¡ãâ®¢
	int        SetAttSort ( int );            // “áâ ­®¢¨âì â¥ªãéãî á®àâ¨à®¢ªã

	int        Prop2Att ( int );                            // ®§¨æ¨ï ¢ ­®¬¥à
	int        Att2Prop ( int );                            // ®¬¥à ¢ ¯®§¨æ¨î
	int        Name2Att ( LPSTR );                   // ® ¨¬¥­¨ ­ ©â¨  âà¨¡ãâ
	BOOL       isAttEnabled ( int );             // ‚å®¤¨â «¨  âà¨¡ãâ ¢ á¯¨á®ª

// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¨ ¤®áâã¯  ª §­ ç¥­¨ï¬

	int        GetValue ( int, int, LPVAL_REC );          // ®«ãç¨âì §­ ç¥­¨¥
	int        GetValue ( LPVAL_REC );
	int        PutValue ( LPVAL_REC );                    // ®«®¦¨âì §­ ç¥­¨¥
	int        InsValue ( LPVAL_REC );                    // ‚áâ ¢¨âì §­ ç¥­¨¥
	int        DelValue ( VOID );                          // “¤ «¨âì §­ ç¥­¨¥

	int        ValNumber ( int );                       // Š®«¨ç¥áâ¢® §­ ç¥­¨©
	int        ValNumber ( VOID );

	int        ValPosSum ( int );             // ‘ã¬¬ à­®¥ ª®«¨ç¥áâ¢® §­ ç¥­¨©

	int        GetCurVal ( VOID );                // ®«ãç¨âì â¥ªãé¥¥ §­ ç¥­¨¥
	int        SetCurVal ( int );               // “áâ ­®¢¨âì â¥ªãé¥¥ §­ ç¥­¨¥
	int        NextVal ( VOID ) { return SetCurVal ( GetCurVal() + 1 ); };
	int        PrevVal ( VOID ) { return SetCurVal ( GetCurVal() - 1 ); };

	int        GetValSort ( VOID );    // ®«ãç¨âì â¥ªãéãî á®àâ¨à®¢ªã §­ ç¥­¨©
	int        SetValSort ( int );            // “áâ ­®¢¨âì â¥ªãéãî á®àâ¨à®¢ªã

	// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¯à ¢¨« ¬¨
	int        InsRule ( VOID );      // ‚áâ ¢¨âì ¯ãáâ®¥ ¯à ¢¨«® ¯¥à¥¤ â¥ªãé¨¬
	int        DelRule ( VOID );                    // “¤ «¨âì â¥ªãé¥¥ ¯à ¢¨«®

	int        RuleNumber ( VOID ) { return lpHost->numRul; };  // Š®«¨ç¥áâ¢® ¯à ¢¨« ¢ ‡

	int        GetCurRule ( VOID ) { return curRul; };      // ®«ãç¨âì ­®¬¥à â¥ªãé¥£® ¯à ¢¨« 
	int        SetCurRule ( int );        // “áâ ­®¢¨âì ­®¬¥à â¥ªãé¥£® ¯à ¢¨« 
	int        NextRule ( VOID ) { return SetCurRule ( GetCurRule() + 1 ); };// ¥à¥©â¨ ª á«¥¤ãîé¥¬ã ¯à ¢¨«ã
	int        PrevRule ( VOID ) { return SetCurRule ( GetCurRule() - 1 ); };// ¥à¥©â¨ ª ¯à¥¤ë¤ãé¥¬ã ¯à ¢¨«ã

	BOOL       isRuleDisjunct ( VOID );           // ’¥ªãé¥¥ ¯à ¢¨«® ¤¨§êî­ªâ?
	BOOL       SetRuleDisjunct ( BOOL );         // “áâ ­®¢¨âì ¤¨§êî­ªâ¨¢­®áâì

	// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ¢ëáª §ë¢ ­¨ï¬¨
	int        InsProp ( int );           // ‚áâ ¢¨âì âà¨¢¨ «ì­®¥ ¢ëáª §ë¢ ­¨¥
	int        DelProp ( VOID );               // “¤ «¨âì â¥ªãé¥¥ ¢ëáª §ë¢ ­¨¥

	int        InvertProp ( int );               // ˆ­¢¥àâ¨à®¢ âì ¢ëáª §ë¢ ­¨¥
	int        InvertProp ( VOID );      // ˆ­¢¥àâ¨à®¢ âì â¥ªãé¥¥ ¢ëáª §ë¢ ­¨¥
	int        InvertComps ( VOID );       // ˆ­¢¥àâ¨à®¢ âì ¢á¥ ª®¬¯ â¥ªãé¥£® ¢ëáª §ë¢ ­¨¥

	int        PropNumber ( VOID );                 // Š®«¨ç¥áâ¢® ¢ëáª §ë¢ ­¨©

	BOOL       isPropConjunct ( int );               // ‚ëáª §ë¢ ­¨¥ ª®­êî­ªâ?
	BOOL       SetPropConjunct ( int, BOOL );    // “áâ ­®¢¨âì ª®­êî­ªâ¨¢­®áâì

	// ”ã­ªæ¨¨ ¬ ­¨¯ã«¨à®¢ ­¨ï ª®¬¯®­¥­â ¬¨

	int        GetComp ( int, int, LPCOMP );            // ®«ãç¨âì ª®¬¯®­¥­âã
	int        SetComp ( int, int, LPCOMP );          // “áâ ­®¢¨âì ª®¬¯®­¥­âã


	// ‹®£¨ç¥áª¨¥ äã­ªæ¨¨
	int        InferenceCNF ( VOID );
	int        InferencePassCNF ( LPHEAD_RUL, LPHEAD_RUL, LPHEAD_RUL );
	VOID       InferenceDisCNF ( LPHEAD_RUL, LPHEAD_RUL, LPHEAD_RUL );

	VOID       TransferConIntoWindow ( LPHEAD_RUL );

	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄ¿
³ ‡ ¯¨áì-§ £®«®¢®ª ä ©«  ¡ §ë §­ ­¨©                              ³ HEAD_REC ³
³                                                                 ÀÄÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
struct HEAD_REC {
	char       CopyWrite[80];
	WORD       Version;

	int        numAtt;                                 // Š®«¨ç¥áâ¢®  âà¨¡ãâ®¢
	int        numAllVal;                          // Š®«¨ç¥áâ¢® ¢á¥å §­ ç¥­¨©

	int        numRul;                           // Š®«¨ç¥áâ¢® ¨áå®¤­ëå ¯à ¢¨«
	int        numCon;                          // Š®«¨ç¥áâ¢® ª®­êî­ªâ®¢ ¢ „”
	int        numDis;                          // Š®«¨ç¥áâ¢® ¤¨§êî­ªâ®¢ ¢ Š”

	long       lenRul;                                  // „«¨­  ¡ãä¥à  ¯à ¢¨«
	long       lenDNF;                             // ¡é ï ¤«¨­  „” ¢ ¡ ©â å
	long       lenCNF;                             // ¡é ï ¤«¨­  Š” ¢ ¡ ©â å

	long       ofsAtt;                         // ‘¬¥é¥­¨¥ ¤®  âà¨¡ãâ®¢ (¡ ©â)
	long       ofsVal;                          // ‘¬¥é¥­¨¥ ¤® §­ ç¥­¨© (¡ ©â)
	long       ofsRul;                   // ‘¬¥é¥­¨¥ ¤® ¨áå®¤­ëå ¯à ¢¨« (¡ ©â)
	long       ofsDNF;                             // ‘¬¥é¥­¨¥ ¤® „” ¢ ¡ ©â å
	long       ofsCNF;                             // ‘¬¥é¥­¨¥ ¤® Š” ¢ ¡ ©â å

	int        isDNF;                                 // ‘ª®¬¯¨«¨à®¢ ­  «¨ „”
	int        isCNF;                                 // ‘ª®¬¯¨«¨à®¢ ­  «¨ Š”

	float      Entropy;                                // ­âà®¯¨ï ¡ §ë §­ ­¨©
	float      Correlation;              // Š®àà¥«ïæ¨ï ¯à¨§­ ª®¢ ¢ ¡ §¥ §­ ­¨©

	MOD_DESCR  ModDescr;                                    // ¯¨á ­¨¥ ¬®¤ã«ï
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄ¿
³ ‡ ¯¨áì ®¡  âà¨¡ãâ¥ ä ©«  ¡ §ë §­ ­¨©                             ³ ATT_REC ³
³                                                                  ÀÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#define ATT_NAME_LEN         128
#define ATT_QUE_LEN          128
#define ATT_CMNT_LEN         256

struct ATT_REC {
	char         Name[ATT_NAME_LEN];                // ˆ¬ï (­ §¢ ­¨¥)  âà¨¡ãâ 
	char         Question[ATT_QUE_LEN];        // ‚®¯à®á ®â­®á¨â¥«ì­®  âà¨¡ãâ 
	char         Comment[ATT_CMNT_LEN];                // Š®¬¬¥­â à¨©  âà¨¡ãâ 

	DWORD        Style;                                      // ‘â¨«ì ¯à¨§­ ª 

	char         Price;                             // –¥­  ¨§¬¥à¥­¨ï  âà¨¡ãâ 
	char         Importance;                              // ‚ ¦­®áâì  âà¨¡ãâ 

	int          numVal;                                // Š®«¨ç¥áâ¢® §­ ç¥­¨©
	DWORD        HyperIndex;                           // ˆ­¤¥ªá ¢ £¨¯¥àâ¥ªáâ¥
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄ¿
³ ‡ ¯¨áì ® §­ ç¥­¨¨ ä ©«  ¡ §ë §­ ­¨©                              ³ VAL_REC ³
³                                                                  ÀÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#define VAL_NAME_LEN         80
#define VAL_QUE_LEN          80
#define VAL_CMNT_LEN         128

struct VAL_REC {
	char         Name[VAL_NAME_LEN];                // ˆ¬ï (­ §¢ ­¨¥) §­ ç¥­¨ï
	char         Question[VAL_QUE_LEN];        // ‚®¯à®á ®â­®á¨â¥«ì­® §­ ç¥­¨ï
	char         Comment[VAL_CMNT_LEN];                // Š®¬¬¥­â à¨© §­ ç¥­¨ï

	DWORD        Style;                                      // ‘â¨«ì §­ ç¥­¨ï

	char         Price;                             // –¥­  ¨§¬¥à¥­¨ï §­ ç¥­¨ï
	char         Importance;                              // ‚ ¦­®áâì §­ ç¥­¨ï
	char         Character;                          // • à ªâ¥à­®áâì §­ ç¥­¨ï

	DWORD        HyperIndex;                           // ˆ­¤¥ªá ¢ £¨¯¥àâ¥ªáâ¥
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄ¿
³ «¥¬¥­â ¨­¤¥ªá   âà¨¡ãâ®¢.                                       ³ ATT_IDX ³
³ ˆ­¤¥ªá  âà¨¡ãâ®¢ - íâ® ¤¨ ¬¨ç¥áª¨© ¬ áá¨¢ í«¥¬¥­â®¢ ª« áá        ÀÄÄÄÄÄÄÄÄÄ´
³ ATT_IDX, ª®«¨ç¥áâ¢® ª®â®àëå à ¢­® ç¨á«ã  âà¨¡ãâ®¢. ¯¨á ­¨¥  âà¨¡ãâ        ³
³ ¢ª«îç ¥â ¢ á¥¡ï ª ª ç áâì ª« áá ATT_REC (®¯¨á ­¨¥  âà¨¡ãâ  ¨§ ä ©« ).      ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
struct ATT_IDX {
	ATT_REC    AttRec;

	int        idxName;
	int        idxPrice;
	int        idxImportance;
	int        idxNumVal;
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄ¿
³ «¥¬¥­â ¨­¤¥ªá  §­ ç¥­¨©.                                        ³ VAL_IDX ³
³ ˆ­¤¥ªá §­ ç¥­¨© - íâ® ¤¨­ ¬¨ç¥áª¨© ¬ áá¨¢ í«¥¬¥­â®¢ ª« áá        ÀÄÄÄÄÄÄÄÄÄ´
³ VAL_IDX, ª®«¨ç¥áâ¢® ª®â®àëå à ¢­® áã¬¬¥ ¢á¥å §­ ç¥¨©. ‚­ ç «¥ ¨¤ãâ         ³
³ ¯®¯®àï¤ªã §­ ç¥­¨ï ­ã«¥¢®£®  âà¨¡ãâ , § â¥¬ ¯¥à¢®£® ¨ â.¤. (ª ª ¢          ³
³ á¥ªæ¨®­¨à®¢ ­­®© ¬ âà¨æ¥). ¯¨á ­¨¥ §­ ç¥­¨ï ¢ª«îç ¥â ¢ á¥¡ï ª ª ç áâì     ³
³ ª« áá VAL_REC (®¯¨á ­¨¥ §­ ç¥­¨ï ¤«ï ä ©« ).                               ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
struct VAL_IDX {
	VAL_REC    ValRec;

	int        idxName;
	int        idxPrice;
	int        idxImportance;
	int        idxCharacter;
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄ¿
³ ‡ ª®«®¢®ª ¯à ¢¨«                                                ³ HEAD_RUL ³
³                                                                 ÀÄÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#define RUL_DISJUNCT         0x0001                      // …á«¨ á® á¢ï§ª®© OR

struct HEAD_RUL {
	int        numProp;                 // Š®«¨ç¥áâ¢® ¢ëáª §ë¢ ­¨© ( âà¨¡ãâ®¢)
	int        lenRul;             // „«¨­  ¯à ¢¨«  ¢ ¡ ©â å ¢ª«îç ï § £®«®¢®ª
	COMP       Const;        // Š®­áâ ­â  ¯à ¢¨«  (¢ëç¨á«ï¥âáï ¯à¨ ª®¬¯¨«ïæ¨¨)
	WORD       Style;                      // ‘â¨«ì (ª®­êî­ªâ/¤¨§êî­ªâ ¨ â.¯.)
	int        numParent1;                                  // ¥à¢ë© à®¤¨â¥«ì
	int        numParent2;                                  // ‚â®à®© à®¤¨â¥«ì
	COMP       RuleProp1;                            // ‚ëáª §ë¢ ­¨¥ ® ¯à ¢¨«¥
	COMP       RuleProp2;
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄ¿
³ ‡ £®«®¢®ª ¢ëáª §ë¢ ­¨ï                                         ³ HEAD_PROP ³
³                                                                ÀÄÄÄÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#define PROP_CONJUNCT        0x0001                     // …á«¨ á® á¢ï§ª®© AND

struct HEAD_PROP {
	int        curAtt;     //  ª ª®¬  âà¨¡ãâ¥ ¤ ­­®¥ ¢ëáª §ë¢ ­¨¥ ( ¡á.­®¬¥à)
	WORD       Style;                      // ‘â¨«ì (ª®­êî­ªâ/¤¨§êî­ªâ ¨ â.¯.)
	};

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
³ Š®¤ë ®è¨¡®ª                                                       ³ ERRORS ³
³                                                                   ÀÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#define LKE_SUCCESS          0
#define LKE_GENERAL          1
#define LKE_ALLOCMEM         2
#define LKE_CANTOPEN         3
#define LKE_CANTREAD         4
#define LKE_CANTWRITE        5
#define LKE_NOTUNIQUE        6
#define LKE_NOCURRENTPROP    7              // ’¥ªãé¥£®  âà¨¡ãâ  ­¥â ¢ ¯à ¢¨«¥
#define LKE_NOTCOMPILED      8                           // ¥ ®âª®¬¯¨«¨à®¢ ­®
#define LKE_NOGOAL           9
#define LKE_NOTEDIPFILE      10

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
³                                                                   ³        ³
³                                                                   ÀÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

#define LK_COPYWRITE         "Logical Kernel EDIP Version 1.0, January 1993, (c) Alexander Savinov"
#define LK_VERSION           0x0100                              // ‚¥àá¨ï 1.0

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄ¿
³ à®â®â¨¯ë ¨á¯®«ì§ã¥¬ëå äã­ªæ¨©                                    ³        ³
³                                                                   ÀÄÄÄÄÄÄÄÄ´
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

BOOL     InsDelMem ( LPVOID, long, LPVOID, int ); // ‘¤¢¨­ãâì ¨«¨ à §¤¢¨­ãâì ¯ ¬ïâì
long     MemorySize ( LPVOID );
int      ReadFile ( int, long, long, LPSTR );
int      WriteFile ( int, long, long, LPSTR );
int      isFileValid ( LPSTR );
int      isFileValid ( int );
LPSTR    GetNameFromPath ( LPSTR );                  //  ©â¨ ¢ ¯ãâ¨ ¨¬ï ä ©« 
VOID     InitFilesCounter ( VOID );
LPKBASE  GetNextFileObject ( VOID );
VOID     AddBakExt ( LPSTR );

LPVOID LK_malloc ( unsigned long );
LPVOID LK_realloc ( LPVOID, unsigned long );
HGLOBAL LK_free ( LPVOID );


