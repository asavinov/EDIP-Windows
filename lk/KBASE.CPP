#include <windows.h>
#include <alloc.h>                                                   // size_t
#include <mem.h>
#include <dos.h>                                                     // FP_SEG
#include "lk.h"

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ Глобальные переменные                                             │        │
│                                                                   └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/

// Определение статических членов класса KBASE
int  KBASE::Error = 0;

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования объектом ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Конструктор класса KBASE.                                      │ OpenKBase │
│ Если БЗ еще не открыта, то создается KBFILE.                   └───────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPKBASE KBASE::OpenKBase ( LPSTR lpPath ) {

	// Выделить место для объекта
	LPKBASE tis = (LPKBASE) LK_malloc ( sizeof(KBASE) );
	if ( tis == NULL ) { Error = LKE_ALLOCMEM; return NULL; }

	// Найти или сделать ему родителя
	tis->lpHost = KBFILE::OpenKBFile ( lpPath );
	if ( tis->lpHost == 0 ) {
		LK_free ( tis ); Error = KBFILE::Error; return NULL;
		}

	// Внести в конец списка
	tis->lpPrevKBase = tis->lpHost->lpLastKBase;    // Передо мной экс-крайний
	tis->lpNextKBase = NULL;                 // Я крайний (за мной никого нет)
	tis->lpHost->lpLastKBase = tis;
	if ( tis->lpPrevKBase )                           // За экс-крайним стою я
		tis->lpPrevKBase->lpNextKBase = tis;
	else
		tis->lpHost->lpFirstKBase     = tis;

	// Инициализировать компоненеты объекта KBASE
	tis->curAtt = -1;
	tis->curVal = -1;
	tis->curRul = 0;
	tis->lpRul = tis->lpHost->lpRul;           // Текущее самое первое правило

	tis->curAttSort = SORT_UNSORTED;
	tis->curValSort = SORT_UNSORTED;

	tis->View = VIEW_STRUCT;
	tis->hWnd = 0;

	Error = LKE_SUCCESS;

	return tis;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Деструктор класса KBASE.                                      │ CloseKBase │
│                                                               └────────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::CloseKBase ( VOID ) {

	// Вычеркнуть из списка
	if ( lpPrevKBase ) lpPrevKBase->lpNextKBase = lpNextKBase;
	else               lpHost->lpFirstKBase     = lpNextKBase;
	if ( lpNextKBase ) lpNextKBase->lpPrevKBase = lpPrevKBase;
	else               lpHost->lpLastKBase      = lpPrevKBase;

	// Вместе с последним закрываем и хозяина
	if ( lpHost->lpFirstKBase == NULL && lpHost->lpLastKBase == NULL ) {
		lpHost->CloseKBFile();
		}

	LK_free ( this );
	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Сохраняет БЗ в файл на диске. Если БЗ не имеет имени файла     │ SaveKBase │
│ (новая), то возвращается ошибка.                               └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SaveKBase ( VOID ) {
	return ( Error = lpHost->SaveKBFile ( ) );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Дать файлу БЗ новое имя. Параметром обязательно является     │ RenameKBase │
│ полный путь файла базы знаний. После переименования поле     └─────────────┤
│ сохраненности не меняется. Если файл с таким именем уже открыт, то         │
│ уже существующие его окна присоеденяются к данному, а их содержимое        │
│ теряется (содержимое объекта в памяти, а не в файле на диске). Вообще      │
│ полезно проверять, существует ли уже файл с таким именем заранее.          │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::RenameKBase ( LPSTR lpNewPath ) {
	return ( Error = lpHost -> RenameKBFile ( lpNewPath ) );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Перемещение данного объекта новому хозяину. При перемещении    │ MoveKBase │
│ предполагается, что содержимое объекта полностью меняется,     └───────────┤
│ поэтому все поля сбрасываются. Окну посылается сообщение                   │
│ о смене содержимого.                                                       │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::MoveKBase ( LPKBFILE lpNewHost ) {

	// Для начала необходимо избавиться от старого хозяина
	// Вычеркнуть себя из его списка
	if ( lpPrevKBase ) lpPrevKBase->lpNextKBase = lpNextKBase;
	else               lpHost->lpFirstKBase     = lpNextKBase;
	if ( lpNextKBase ) lpNextKBase->lpPrevKBase = lpPrevKBase;
	else               lpHost->lpLastKBase      = lpPrevKBase;
	// Теперь я свободен и нигде не числюсь

	// Если я последний у хозяина, то убить его (хозяина без ничего небывает)
	if ( lpHost->lpFirstKBase == NULL && lpHost->lpLastKBase == NULL ) {
		lpHost->CloseKBFile ( );
		}

	lpHost = lpNewHost; // Ура! Я обрел нового хозяина! Но он об этом не знает

	// Теперь необходимо, чтобы новый хозяин обрел меня
	// Внести объект в конец списка нового хозяина
	lpPrevKBase = lpHost->lpLastKBase;              // Передо мной экс-крайний
	lpNextKBase = NULL;                       // Я крайний, за мной никого нет
	lpHost->lpLastKBase = (LPKBASE) this;
	if ( lpPrevKBase )                                // За экс-крайним стою я
		lpPrevKBase->lpNextKBase = (LPKBASE) this;
	else
		lpHost->lpFirstKBase     = (LPKBASE) this;

	// При новом хозяине необходимо самому измениться
	// В простейшем случае просто сбрасываемся в нач сост
	curAtt = -1;
	curVal = -1;
	curRul = 0;
	lpRul = lpHost->lpRul;

	if ( hWnd ) SendMessage ( hWnd, WM_COMMAND, CHANGE_CONTENT, 0L );

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Именована ли БЗ?                                            │ isKBaseNamed │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::isKBaseNamed ( VOID ) {
	if ( lpHost->KBFilePath[0] == 0 )
		return FALSE;

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Сохранена ли БЗ?                                            │ isKBaseSaved │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::isKBaseSaved ( VOID ) {
	return ( ! lpHost->isChanged );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Установить ключ окна просмотра                                 │ SetWindow │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
HWND KBASE::SetWindow ( HWND hwnd ) {
	HWND Prev = hWnd;
	hWnd = hwnd;
	return Prev;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Получить ключ окна просмотра                                   │ GetWindow │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
HWND KBASE::GetWindow ( VOID ) {
	return hWnd;
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Установить текущий вид                                           │ SetView │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetView ( int Vw ) {
	int Prev = View;

	View = Vw;
	curAtt = -1;
	curVal = -1;

	return Prev;
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Получить текщий вид                                              │ GetView │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetView ( VOID ) {
	return View;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Получить описание модуля.                                  │ GetModuleHead │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetModuleHead ( LPMOD_DESCR pModDescr ) {

	* pModDescr = lpHost->ModDescr;

	return LKE_SUCCESS;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Получить описание модуля.                                  │ SetModuleHead │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetModuleHead ( LPMOD_DESCR pModDescr ) {

	lpHost->ModDescr = * pModDescr;
	lpHost->isChanged = TRUE;

	return LKE_SUCCESS;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Групповые функции (списочные) ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Возвращает число открытых файлов.                          │ KBFilesNumber │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::KBFilesNumber ( VOID ) {
	return KBFILE::KBFilesNumber();
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Возвращает число открытых окон для данного файла. Учитывается│ KBaseNumber │
│ также и данное окно.                                         └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::KBaseNumber ( VOID ) {
	return lpHost->KBaseNumber();
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования и доступа к атрибутам ░░░░░░░░░░░░░░░░░ KBASE ░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Получить описание атрибута по указанному адресу.            │ GetAttribute │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetAttribute ( int Att, LPATT_REC lpAttRec ) {
	if ( Att < 0 || Att >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );

	LPATT_REC pAttRec = lpHost->GetAttribute ( lpHost->AttI2N ( Att, curAttSort ) );

	*lpAttRec = *pAttRec;
	return ( Error = LKE_SUCCESS );
	}

int KBASE::GetAttribute ( LPATT_REC lpAttRec ) {
	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );

	LPATT_REC pAttRec = lpHost->GetAttribute ( curAtt );

	*lpAttRec = *pAttRec;
	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Сохранить описание атрибута из записи в индексе атрибутов   │ PutAttribute │
│ вместо текущего атрибута (старая информация теряется).      └──────────────┤
│ Поле numVal не меняется. Остальные поля должны быть правильно              │
│ инициализированы (система ничего этого не проверяет). После сохранения     │
│ производится досортировка. Текущим  остается прежний атрибут, но его       │
│ относительный номер (т.е. curAtt) может измениться.                        │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::PutAttribute ( LPATT_REC lpAttRec ) {

	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );

	Error = lpHost->PutAttribute ( curAtt, lpAttRec );

	if ( Error != LKE_SUCCESS ) return Error;

	// Оповестить всех (в т.ч. и себя) об изменении параметров атрибута
	lpHost->forEachWindow ( PUT_ATTRIBUTE, (DWORD) curAtt );

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Вставляет атрибут перед текущим. Поле numVal значения не    │ InsAttribute │
│ имеет. Остальные должны быть правильно инициализированы.    └──────────────┤
│ После вставки текущим остается прежний атрибут, однако, его положение в    │
│ списке (т.е. curAtt) может быть любым (не обязательно после нового)        │
│ и зависит от параметров. В других окнах текущий атрибут не меняется.       │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InsAttribute ( LPATT_REC lpAttRec ) {
	// Основная проблема -- это сохранение текущих номеров атрибута
	// когда общее их количество увеличивается. Если вставленный
	// атрибут имеет абсолютный номер меньший либо равный данного,
	// текущий атрибут необходимо увеличить на 1, а в противном случае
	// (если новый атрибут имеет номер меньший текущего) текущий номер
	// не меняется. Абсолютный номер вставленного атрибута равен
	// номеру текущего атрибута, а все большие номера увеличиваются на 1.
	// Необходимо учитывать, что в данном окне, в котором вставляется
	// атрибут, текущий номер не меняется.

	Error = lpHost->InsAttribute ( curAtt, lpAttRec );

	if ( Error != LKE_SUCCESS ) return Error;

	// Необходимо пройтись по всем окнам включая данное и если нужно (если их
	// текущий атрибут не меньше вставленного) увеличить номер атрибута.

	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase->curAtt >= curAtt ) lpKBase->curAtt++;

		}

	lpHost->forEachWindow ( INS_ATTRIBUTE, (DWORD) curAtt );

	return Error;                                               // LKE_SUCCESS
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Удаляет текущий атрибут. Вместе с ним удаляются все его     │ DelAttribute │
│ значения. После удаления текущим становится следующий в     └──────────────┤
│ упорядоченном списке атрибут c -1-ым текущим значением.                    │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::DelAttribute ( VOID ) {
	// При удалении может уменьшиться база знаний

	int curAttOld = curAtt;

	// Пройтись по всем окнам и в каждом изменить текущий атрибут. Если он
	// равен удаляемому, то текущим сделать следующий атрибут в данном упорядоченном
	// списке (он может иметь любое абсолютное значение). Далее надо уменьшить
	// на 1 текущий номер если он больше номера удаляемого атрибута.
	// Удалить высказывания об удаленном атрибуте из данных и заключений.

	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {
		if ( lpKBase->curAtt == curAttOld ) lpKBase->NextAttPos();
		if ( lpKBase->curAtt >= curAttOld ) lpKBase->curAtt--;
		}

	Error = lpHost->DelAttribute ( curAttOld );

	if ( Error != LKE_SUCCESS ) return Error;

	/* curVal = -1; */                   // Это делается функцией NextAttPos()

	// Заново установить указатель на текущее правило, поскольку база изменилась

	for ( lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		lpKBase->lpRul = lpHost->GetRuleHead ( lpKBase->curRul );

		}

	lpHost->forEachWindow ( DEL_ATTRIBUTE, (DWORD) curAttOld );

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Получить относительный (отсортированный) номер текущего        │ GetCurAtt │
│ атрибута.                                                      └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetCurAtt ( VOID ) {
	return lpHost->AttN2I ( curAtt, curAttSort );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Установить новый текущий атрибут.                              │ SetCurAtt │
│ Возвращает предыдущее значение. Номер значения равен -1.       └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetCurAtt ( int Att ) {
	// Для просмотра структуры все понятно.
	// В других случаях необходимо найти позицию, которую занимает указанный
	// атрибут с помощью Att2Prop(), после чего просто установить эту
	// позицию.

	LPHEAD_RUL pRul;

	if ( Att < -1 )                  { curAtt = -1; return Att; }
	else if ( Att > lpHost->numAtt ) { curAtt = lpHost->numAtt; return Att; }

	switch ( View ) {
		case VIEW_STRUCT     : {
			curAtt = lpHost->AttI2N ( Att, curAttSort );
			return Att;
			}
		case VIEW_KNOWLEDGE  : pRul = lpRul;          break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		}

	SetCurAttPos ( lpHost->Att2Prop ( Att, pRul, curAttSort ) );

	return Att;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Получить позицию текущего атрибута в их неполном            │ GetCurAttPos │
│ отсортированном списке.                                     └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetCurAttPos ( VOID ) {
	// Мы знаем абсолютный номер атрибута. По нему необходимо определить
	// его положение в отсортированном списке с дырками. Сперва необходимо
	// абсолютный номер преобразовать в упорядоченный. Далее по нему
	// можно найти положение в списке. Если абсолютный номер атрибута
	// равен их количеству, то необходимо вернуть общее количество пунктов.

	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_STRUCT     : return lpHost->AttN2I ( curAtt, curAttSort );
		case VIEW_KNOWLEDGE  : pRul = lpRul;          break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		}

	return lpHost->Att2Prop ( lpHost->AttN2I ( curAtt, curAttSort ), pRul, curAttSort );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Установить позицию текущего атрибута в неполном             │ SetCurAttPos │
│ отсортированном списке. Возвращается предыдущая позиция.    └──────────────┤
│ Если она не равна новой, то текущее значение сбрасывается.                 │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetCurAttPos ( int Pos ) {
	// Для просмотра структуры все понятно: позиция -- это просто упорядоченный номер.
	// В других случаях необходимо заданнцю позицию преобразовать в относительный
	// номер атрибута с помощью Prop2Att(), после чего относительный номер
	// преобразовать в абсолютный.

	if ( Pos < 0 ) { curAtt = -1; return Pos; }

	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_STRUCT     : {
			curAtt = lpHost->AttI2N ( Pos, curAttSort );
			return Pos;
			}
		case VIEW_KNOWLEDGE  : pRul = lpRul;          break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		}

	curAtt = lpHost->AttI2N ( lpHost->Prop2Att ( Pos, pRul, curAttSort ), curAttSort );

	return Pos;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Получить текущую сортировку атрибутов                         │ GetAttSort │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetAttSort ( VOID ) {
	return curAttSort;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Установить текущую сортировку атрибутов. Текущим остается     │ SetAttSort │
│ прежний атрибут, однако, его упорядоченный номер и положение  └────────────┤
│ в списке могут измениться. Возвращается предыдущая сортировка.             │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetAttSort ( int Sort ) {
	int Prev = curAttSort;
	curAttSort = Sort;
	return Prev;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Позиция в упорядоченном списке с дырками преобразуется в номер  │ Prop2Att │
│ атрибута (также упорядоченный). Зависит от сортировки и         └──────────┤
│ текущего вида. Для структуры возвращает Pos.                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::Prop2Att ( int Pos ) {

	if ( Pos < 0 ) return -1;
	if ( Pos >= PropNumber() ) return lpHost->numAtt;

	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  :
			if ( curRul < 0 || curRul >= lpHost->numRul ) return -1;
			pRul = lpRul;
			break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		case VIEW_STRUCT     :
		default              : return Pos;
		}

	return lpHost->Prop2Att ( Pos, pRul, curAttSort );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Номер атрибута (упорядоченный) преобразуется в упорядоченную    │ Att2Prop │
│ позицию в списке с дырками. Зависит от сортировки и             └──────────┤
│ текущего вида. Для структуры возвращает Att.                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::Att2Prop ( int Att ) {

	if ( Att < 0 ) return -1;
	if ( Att >= lpHost->numAtt ) return PropNumber();

	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  :
			if ( curRul < 0 || curRul >= lpHost->numRul ) return -1;
			pRul = lpRul;
			break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		case VIEW_STRUCT     :
		default              : return Att;
		}

	return lpHost->Att2Prop ( Att, pRul, curAttSort );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ По заданной строке имени атрибута находится его номер в         │ Name2Att │
│ соответствии с текущей сортировкой. Если такой строки нет, то   └──────────┤
│ возвращается -1.                                                           │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::Name2Att ( LPSTR lpName ) {

	for ( int i = 0; i < lpHost->numAtt; i++ ) {
		if ( 0 == lstrcmp ( (LPSTR) lpHost->lpAttIdx[ lpHost->AttI2N(i,curAttSort) ].AttRec.Name, (LPSTR) lpName ) )
		return i;
		}
	return -1;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Проверяет входит ли данный атрибут в упорядоченный список   │ isAttEnabled │
│ с дырками, т.е., не попадает ли он в дырку. Зависит от      └──────────────┤
│ сортировки и текущего вида. Для структуры всегда истинно.                  │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::isAttEnabled ( int Att ) {

	if ( View == VIEW_STRUCT || Att < 0 || Att >= AttNumber() ) return TRUE;

	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  :
			if ( curRul < 0 || curRul >= lpHost->numRul ) return FALSE;
			pRul = lpRul;
			break;
		case VIEW_DATA       : pRul = lpHost->lpData; break;
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;
		default              : return TRUE;             // Не бывает -- ошибка
		}

	if ( ! lpHost->GetPropHead ( pRul, lpHost->AttI2N ( Att, curAttSort ) ) )
		return FALSE;
	else
		return TRUE;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования и доступа к значениям ░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Получить описание значения по указанному адресу.                │ GetValue │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetValue ( int Att, int Val, LPVAL_REC lpValRec ) {
	int i = lpHost->AttI2N ( Att, curAttSort );
	int j = lpHost->ValI2N ( i, Val, curValSort );

	if ( Att < 0 || Att >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );
	if ( Val < 0 || Val >= lpHost->lpAttIdx [ i ].AttRec.numVal )
		return ( Error = LKE_SUCCESS );

	LPVAL_REC pValRec = lpHost->GetValue ( i, j );

	*lpValRec = *pValRec;

	return ( Error = LKE_SUCCESS );
	}

int KBASE::GetValue ( LPVAL_REC lpValRec ) {
	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );
	if ( curVal < 0 || curVal >= lpHost->lpAttIdx [ curAtt ].AttRec.numVal )
		return ( Error = LKE_SUCCESS );

	LPVAL_REC pValRec = lpHost->GetValue ( curAtt, curVal );

	*lpValRec = *pValRec;

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Сохранить описание значения из записи в индексе значений        │ PutValue │
│ вместо текущего значения (старая информация теряется).          └──────────┤
│ Поля записи должны быть правильно                                          │
│ инициализированы (система ничего этого не проверяет). После сохранения     │
│ производится досортировка. Текущим остается это же значение.               │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::PutValue ( LPVAL_REC lpValRec ) {
	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );
	if ( curVal < 0 || curVal >= lpHost->lpAttIdx [ curAtt ].AttRec.numVal )
		return ( Error = LKE_SUCCESS );

	Error = lpHost->PutValue ( curAtt, curVal, lpValRec );
	if ( Error != LKE_SUCCESS ) return Error;

	lpHost->forEachWindow ( PUT_VALUE, MAKELONG ( curAtt, curVal ) );

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Вставляет значение перед текущим.                               │ InsValue │
│ Поля должны быть правильно инициализированы.                    └──────────┤
│ После вставки текущим остается старое значение, однако, оно может          │
│ находиться в любом месте списка в зависимости от                           │
│ параметров. Поскольку меняется и количество значений, то может измениться  │
│ и текущий атрибут (только при сортировке по количеству значений). Вернее   │
│ сам атрибут не меняется, а меняется его положение в списке.                │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InsValue ( LPVAL_REC lpValRec ) {
	// Основная проблема -- это сохранение текущих номеров значений во всех
	// окнах. Понятно, что об этом надо заботиться только если текущий
	// атрибут окна совпадает с номером атрибута вставленного значения.
	// Для данного окна номер значения не меняется, а для всех остальных окон
	// с таким же номером атрибута номер значения увеличивается на 1 если
	// он больше либо равен номера вставленного значения.

	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return ( Error = LKE_SUCCESS );
	if ( curVal < 0 ) return ( Error = LKE_SUCCESS );

	Error = lpHost->InsValue ( curAtt, curVal, lpValRec );

	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Необходимо пройтись по всем окнам включая данное с таким же текущим
	// атрибутом и если нужно (если их текущее значение не меньше вставленного)
	// увеличить номер значения.

	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( /* lpKBase != this && */ lpKBase->curAtt == curAtt ) {
			if ( lpKBase->curVal >= curVal ) lpKBase->curVal++;
			}

		// Установить заново указатель на текущее правило.
		lpKBase->lpRul = lpHost->GetRuleHead ( lpKBase->curRul );
		}

	lpHost->forEachWindow ( INS_VALUE, MAKELONG ( curAtt, curVal ) );

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Удаляет текущее значение.                                       │ DelValue │
│ После удаления текущим становится следующее в                   └──────────┤
│ упорядоченном списке значение (т.е. curVal не меняется). Текущий атрибут   │
│ не меняется, однако при сортировке по количеству значений может изменить   │
│ свое положение в списке.                                                   │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::DelValue ( VOID ) {
	int curValOld = curVal;

	// Пройтись по всем окнам с таким же текущим атрибутом и в каждом изменить
	// текущее значение. Если оно равно удаляемому, то текущим сделать следующее
	// значение в данном упорядоченном списке (оно может иметь любое абсолютное
	// значение). Далее надо уменьшить на 1 текущий номер если он больше номера
	// удаляемого значения (это сохраняет текущее значение).

	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase->curAtt == curAtt ) {
			if ( lpKBase->curVal == curValOld ) lpKBase->NextVal();
			if ( lpKBase->curVal >= curValOld ) lpKBase->curVal--;
			}

		}

	Error = lpHost->DelValue ( curAtt, curValOld );

	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Заново установить указатель на текущее правило, поскольку база изменилась

	for ( lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		lpKBase->lpRul = lpHost->GetRuleHead ( lpKBase->curRul );
		}

	lpHost->forEachWindow ( DEL_VALUE, MAKELONG ( curAtt, curValOld ) );

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Количество значений указанного атрибута.                       │ ValNumber │
│                                                                └───────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::ValNumber ( int Att ) {
	if ( Att < 0 || Att >= lpHost->numAtt ) return -1;

	return lpHost->lpAttIdx [ lpHost->AttI2N ( Att, curAttSort ) ].AttRec.numVal;
	}
int KBASE::ValNumber ( VOID ) {
	if ( curAtt < 0 || curAtt >= lpHost->numAtt ) return -1;

	return lpHost->lpAttIdx [ curAtt ].AttRec.numVal;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Суммарное количество значений с нулевой до указанной позиции в │ ValPosSum │
│ упорядоченном списке атрибутов. Учитывается текущий вид и      └───────────┤
│ сортировка. Параметр -- номер атрибута.                                    │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::ValPosSum ( int Att ) {
	if ( Att < 0 ) return -1;

	if ( View == VIEW_STRUCT ) return lpHost->ValSum ( Att, curAttSort );

	LPHEAD_RUL pRul;
	if ( View == VIEW_KNOWLEDGE )       pRul = lpRul;
	else if ( View == VIEW_DATA )       pRul = lpHost->lpData;
	else if ( View == VIEW_CONCLUSION ) pRul = lpHost->lpConc;

	return lpHost->ValSum ( lpHost->Att2Prop ( Att, pRul, curAttSort ), pRul, curAttSort );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Получить текущий номер значения.                               │ GetCurVal │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetCurVal ( VOID ) {
	return lpHost->ValN2I ( curAtt, curVal, curValSort );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Установить текущее значение. Если атрибут не выделен, то       │ SetCurVal │
│ ничего не делается. Возвращает предыдущее.                     └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetCurVal ( int Val ) {
	int Prev = curVal;

	if ( curAtt < 0 || curAtt >= lpHost->numAtt )   { curVal = -1; return Prev; }

	if ( Val < 0 )                                  curVal = -1;
	else if ( Val >= lpHost->ValNumber ( curAtt ) ) curVal = lpHost->ValNumber ( curAtt );
	else                                            curVal = lpHost->ValI2N ( curAtt, Val, curValSort );
	return Prev;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Получить текущую сортировку значений                          │ GetValSort │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetValSort ( VOID ) {
	return curValSort;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Установить сортировку значений. Текущим остается прежнее      │ SetValSort │
│ значение, однако, его номер может измениться.                 └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetValSort ( int Sort ) {
	int Prev = curValSort;
	curValSort = Sort;
	return Prev;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования правилами ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Вставить пустое правило перед указанным. Это правило не содержит │ InsRule │
│ никаких высказываний (один заголовок). Число правил увеличивается└─────────┤
│ на 1. Текущим становится новое правило. В других окнах ничего не меняется. │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InsRule ( VOID ) {
	// Сперва необходимо просто вставить новое пустое правило (только заголовок).
	// Затем надо пройтись по всем окнам и для каждого скорректировать текущий
	// номер правила и указатель на него. Если текущий номер больше либо равен
	// заданного (номер правила в исходном окне), то он увеличивается на 1,
	// а указатель увеличивается на длину пустого правила (заголовка), иначе
	// ничего делать не надо. В принципе, для корректировки указателя (после
	// того как скорректирован номер) можно использовать функцию его установки
	// по номеру, но это дольше (функция GetRuleHead).

	// Вставить заголовок правила в базу
	Error = lpHost->InsRule ( lpRul );
	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Пройтись по всем окнам кроме данного и если необходимо, то
	// увеличить номер правила и указатель на него.
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this ) continue;

		if ( lpKBase->curRul >= curRul ) {
			lpKBase->curRul++;
			lpKBase->lpRul += sizeof(HEAD_RUL);
			}

		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Удалить текущее правило. Количество правил уменьшается на 1.     │ DelRule │
│ Текущим становится следующее правило. Если в другом окне         └─────────┤
│ текущим является это самое правило, то ему посылается сообщение об удалении│
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::DelRule ( VOID ) {

	// Запомнить длину удаляемого правила
	int Len = lpRul->lenRul;

	// Удалить текущее правило
	Error = lpHost->DelRule ( lpRul );
	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Пройтись по всем окнам кроме данного и если необходимо, то
	// уменьшить номер правила и указатель на него.
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this ) continue;

		if ( lpKBase->curRul == curRul && lpKBase->View == VIEW_KNOWLEDGE ) {
			// Оповестить окно, что правило, которое в нем отображается
			// удалено из другого окна и необходимо отобразить на экране
			// следующее правило, которое стало текущим
			if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, DEL_RULE, curRul );
			}
		else if ( lpKBase->curRul > curRul ) {
			lpKBase->curRul--;
			lpKBase->lpRul -= Len;
			}

		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Установить текущее правило.                                   │ SetCurRule │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetCurRule ( int Rul ) {
	int Prev = curRul;

	if ( Rul < 0 )                   curRul = 0;
	else if ( Rul > lpHost->numRul ) curRul = lpHost->numRul;
	else                             curRul = Rul;

	lpRul = lpHost->GetRuleHead ( curRul );

	return Prev;
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Вернуть дизъюнктивность правила.                          │ isRuleDisjunct │
│                                                           └────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::isRuleDisjunct ( VOID ) {
	return (lpRul->Style & RUL_DISJUNCT);
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Установить дизъюнктивность правила.                      │ SetRuleDisjunct │
│                                                          └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::SetRuleDisjunct ( BOOL isDis ) {
	BOOL Prev = isRuleDisjunct();

	if ( isDis ) lpRul->Style |= RUL_DISJUNCT;
	else         lpRul->Style &= ~ RUL_DISJUNCT;

	return Prev;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования высказываниями ░░░░░░░░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Вставить тривиальное высказывание о заданном упорядоченным       │ InsProp │
│ номером атрибуте. Если такой атрибут уже есть, то возвращается   └─────────┤
│ ошибка.                                                                    │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InsProp ( int Att ) {

	// В зависимости от текущего вида вставлять высказывание надо либо
	// в текущее правило, либо в данные, либо в заключения

	switch ( View ) {
		case VIEW_STRUCT     : return ( Error = LKE_SUCCESS );
		case VIEW_KNOWLEDGE  : {
			Error = lpHost->InsPropInRule ( lpRul, lpHost->AttI2N ( Att, curAttSort ), curAtt );
			lpHost->isChanged = TRUE;
			break;
			}
		case VIEW_DATA       : {
			Error = lpHost->InsPropInData ( lpHost->AttI2N ( Att, curAttSort ), curAtt );
			break;
			}
		case VIEW_CONCLUSION : {
			Error = lpHost->InsPropInConc ( lpHost->AttI2N ( Att, curAttSort ), curAtt );
			break;
			}
		default              : return ( Error = LKE_SUCCESS );
		}

	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Пройтись по всем окнам кроме данного и если необходимо, то
	// передать им сообщение
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this ) continue;
		if ( View == VIEW_KNOWLEDGE ) {
			// Мог измениться указатель на текущее правило (на длину нового высказывания)
			if ( lpKBase->curRul > curRul ) lpKBase->lpRul += sizeof(HEAD_PROP) + sizeof(COMP) * ValNumber ( Att );

			if ( lpKBase->View == VIEW_KNOWLEDGE && lpKBase->curRul != curRul ) continue;
			}

		if ( lpKBase->View != View ) continue;

		// Всем окнам с таким же видом (если знания, то с таким же текущим
		// правилом) послать сообщение
		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, INS_PROP, Att );
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Удалить текущее высказывание в правиле, данных или заключении    │ DelProp │
│ (в зависимости от вида).                                         └─────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::DelProp ( VOID ) {

	// В зависимости от текущего вида удалять высказывание надо либо
	// из текущего правила, либо из данных, либо из заключений
	int PrevPos = GetCurAttPos ();

	switch ( View ) {
		case VIEW_STRUCT     : return ( Error = LKE_SUCCESS );
		case VIEW_KNOWLEDGE  : {
			Error = lpHost->DelPropInRule ( lpRul, curAtt );
			lpHost->isChanged = TRUE;
			break;
			}
		case VIEW_DATA       : {
			Error = lpHost->DelPropInData ( curAtt );
			break;
			}
		case VIEW_CONCLUSION : {
			Error = lpHost->DelPropInConc ( curAtt );
			break;
			}
		default              : return ( Error = LKE_SUCCESS );
		}

	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	// Пройтись по всем окнам кроме данного и если необходимо, то
	// передать им сообщение, а также скорректировать указатель на текущее правило
	// Кроме этого, необходимо перейти к следующей позиции атрибута, если удален именно он
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this ) continue;

		if ( View == VIEW_KNOWLEDGE ) {
			// Мог измениться указатель на текущее правило (на длину нового высказывания)
			if ( lpKBase->curRul > curRul ) lpKBase->lpRul -= sizeof(HEAD_PROP) + sizeof(COMP) * lpHost->ValNumber ( curAtt );

			if ( lpKBase->View == VIEW_KNOWLEDGE && lpKBase->curRul != curRul ) continue;
			}

		if ( lpKBase->View != View ) continue;

		// Перейти к следующей позиции атрибута (т.е., сохранить позицию)
		lpKBase->SetCurAttPos ( PrevPos );

		// Всем окнам с таким же видом (если знания, то с таким же текущим
		// правилом) послать сообщение
		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, DEL_PROP, lpHost->AttN2I ( curAtt, curAttSort ) );
		}

	SetCurAttPos ( PrevPos );

	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Инвертировать высказывание о текущем атрибуте в правиле,      │ InvertProp │
│ данных или заключении (в зависимости от вида).                └────────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InvertProp ( int Att ) {

	// В зависимости от текущего вида инвертировать высказывание надо либо
	// из текущего правила, либо из данных, либо из заключений
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_STRUCT     : return ( Error = LKE_SUCCESS );
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		default              : return ( Error = LKE_SUCCESS );
		}

	Error = lpHost->InvertProp ( pRul, lpHost->AttI2N ( Att, curAttSort ) );
	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	if ( View == VIEW_KNOWLEDGE ) lpHost->isChanged = TRUE;

	// Пройтись по всем окнам кроме данного и если необходимо, то передать им сообщение
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this || lpKBase->View != View )            continue;
		if ( View == VIEW_KNOWLEDGE && lpKBase->curRul != curRul ) continue;

		// Всем окнам с таким же видом (если знания, то с таким же текущим
		// правилом) послать сообщение
		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, INVERT_PROP, lpHost->AttN2I ( curAtt, curAttSort ) );
		}

	return ( Error = LKE_SUCCESS );
	}

int KBASE::InvertProp ( VOID ) {

	return InvertProp ( lpHost->AttN2I ( curAtt, curAttSort ) );

	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Изменить все компоненты высказывания на противоположные без  │ InvertComps │
│ изменения кон/диз высказываания.                             └─────────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InvertComps ( VOID ) {

	// В зависимости от текущего вида изменять высказывание надо либо
	// из текущего правила, либо из данных, либо из заключений
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_STRUCT     : return ( Error = LKE_SUCCESS );
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		default              : return ( Error = LKE_SUCCESS );
		}

	Error = lpHost->InvertComps ( pRul, curAtt );
	if ( lpHost->Error != LKE_SUCCESS ) return (Error = lpHost->Error);

	if ( View == VIEW_KNOWLEDGE ) lpHost->isChanged = TRUE;

	// Пройтись по всем окнам кроме данного и если необходимо, то передать им сообщение
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this || lpKBase->View != View )            continue;
		if ( View == VIEW_KNOWLEDGE && lpKBase->curRul != curRul ) continue;

		// Всем окнам с таким же видом (если знания, то с таким же текущим
		// правилом) послать сообщение
		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, INVERT_PROP, lpHost->AttN2I ( curAtt, curAttSort ) );
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Количество высказываний в правиле, данных или заключениях (в  │ PropNumber │
│ зависимости от текущего вида). Фактически это количество      └────────────┤
│ позиций в списке с дырками.                                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::PropNumber ( VOID ) {
	// Количество высказываний определяется в зависимости от текущего вида
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		case VIEW_STRUCT     :
		default              : return lpHost->numAtt;
		}

	return lpHost->PropNumber ( pRul );
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Вернуть конъюнктивность правила, данных или заключений.   │ isPropConjunct │
│ Бессмыслена при просмотре структуры. Номер атрибута       └────────────────┤
│ относительный.                                                             │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::isPropConjunct ( int Att ) {
	// Конъюнктивность определяется в зависимости от текущего вида
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		case VIEW_STRUCT     :
		default              : return -1;
		}

	LPHEAD_PROP pProp = lpHost->GetPropHead ( pRul, lpHost->AttI2N ( Att, curAttSort ) );

	return (pProp->Style & PROP_CONJUNCT);
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Установить конъюнктивность правила, данных или заключения│ SetPropConjunct │
│                                                          └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBASE::SetPropConjunct ( int Att, BOOL isCon ) {
	// Конъюнктивность определяется в зависимости от текущего вида
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		case VIEW_STRUCT     :
		default              : return -1;
		}

	LPHEAD_PROP pProp = lpHost->GetPropHead ( pRul, lpHost->AttI2N ( Att, curAttSort ) );
	BOOL Prev = (pProp->Style & PROP_CONJUNCT);

	if ( isCon ) pProp->Style |= PROP_CONJUNCT;
	else         pProp->Style &= ~ PROP_CONJUNCT;

	return Prev;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования компонентами ░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBASE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Получить компоненту заданную упорядоченным номером атрибута и    │ GetComp │
│ значения (каждый со своей сортировкой).                          └─────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::GetComp ( int Att, int Val, LPCOMP pComp ) {
	// Компонента выбирается в зависимости от текущего вида из текущего
	// правила, данных или заключения
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		case VIEW_STRUCT     :
		default              : return (Error = LKE_GENERAL);
		}

	int numAtt = lpHost->AttI2N ( Att, curAttSort );
	int numVal = lpHost->ValI2N ( numAtt, Val, curValSort );
	LPCOMP lpComp = lpHost->GetComp ( pRul, numAtt, numVal );
	if ( lpHost->Error != LKE_SUCCESS  ) return (Error = lpHost->Error);

	*pComp = *lpComp;

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Записать компоненту заданную упорядоченным номером атрибута и    │ SetComp │
│ значения (каждый со своей сортировкой).                          └─────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::SetComp ( int Att, int Val, LPCOMP pComp ) {
	// Компонента определяется в зависимости от текущего вида из текущего
	// правила, данных или заключения
	LPHEAD_RUL pRul;

	switch ( View ) {
		case VIEW_KNOWLEDGE  : pRul = lpRul; break;         // текущее правило
		case VIEW_DATA       : pRul = lpHost->lpData; break;         // данные
		case VIEW_CONCLUSION : pRul = lpHost->lpConc; break;     // заключения
		case VIEW_STRUCT     :
		default              : return (Error = LKE_GENERAL);
		}

	int numAtt = lpHost->AttI2N ( Att, curAttSort );
	int numVal = lpHost->ValI2N ( numAtt, Val, curValSort );
	LPCOMP lpComp = lpHost->GetComp ( pRul, numAtt, numVal );
	if ( lpHost->Error != LKE_SUCCESS  ) return (Error = lpHost->Error);

	if ( lpComp->pos == pComp->pos ) {
		return ( Error = LKE_SUCCESS );
		}
	if ( View == VIEW_KNOWLEDGE ) lpHost->isChanged = TRUE;

	*lpComp = *pComp;
	// Передать всем таким же окнам сообщение об изменении компоненты
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {

		if ( lpKBase == this || lpKBase->View != View ) continue;
		if ( View == VIEW_KNOWLEDGE && lpKBase->curRul != curRul ) continue;

		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, CHANGE_COMPONENT, MAKELONG ( Att, Val ) );
		}

	return ( Error = LKE_SUCCESS );
	}








