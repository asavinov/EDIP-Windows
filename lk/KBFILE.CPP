#include <windows.h>
#include <stdlib.h>                                                  // abs
#include <alloc.h>                                                   // size_t
#include <mem.h>
#include <dos.h>                                                     // FP_SEG
#include <stdio.h>                                                   // rename
#include "lk.h"

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ Глобальные переменные                                             │        │
│                                                                   └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
// Определение статических членов класса KBFILE
int      KBFILE::Error = 0;
LPKBFILE KBFILE::lpFirstKBFile = NULL;
LPKBFILE KBFILE::lpLastKBFile = NULL;

unsigned char UNITY = 100;

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования объектом ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Конструктор класса KBFILE.                                    │ OpenKBFile │
│ Если путь не задан (NULL), то БЗ считается новой (команда New)└────────────┤
│ т.е. не связана ни с каким файлом. Если файл с таким путем уже открыт      │
│ (т.е. для него создан объект класса KBFILE), то новый объект не создается, │
│ а возвращается указатель на уже имеющийся. Если указан путь, то            │
│ предполагается, что этот файл существует и он валиден (т.е. ранее          │
│ вызывалась функция isValid).                                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPKBFILE KBFILE::OpenKBFile ( LPSTR lpPath ) {
	LPKBFILE tis;

	if ( lpPath ) {

		// Открыт ли уже указаннй файл?
		LPKBFILE lpOpened = KBFILE::isAlreadyOpened ( lpPath );
		if ( lpOpened ) { Error = LKE_SUCCESS; return ( lpOpened ); }

		// Выделить место для объекта
		tis = (LPKBFILE) LK_malloc ( sizeof(KBFILE) );
		if ( tis == NULL ) { Error = LKE_ALLOCMEM; return ( NULL ); }

		// Открыть файл БЗ для чтения
		int hFile = _lopen ( lpPath, OF_READ );
		if ( hFile == -1 ) { LK_free (tis); Error = LKE_CANTOPEN; return ( NULL ); }

		// Сохранить путь файла в объекте
		lstrcpy ( (LPSTR) tis->KBFilePath, (LPSTR) lpPath );

		// Выделить из пути имя файла и сохранить его в объекте
		lstrcpy ( (LPSTR) tis->KBFileName, (LPSTR) GetNameFromPath ( lpPath ) );

		// Прочитать заголовок в Head
		HEAD_REC Head;
		_llseek ( hFile, 0, 0 );
		if ( sizeof(HEAD_REC) != _lread ( hFile, (LPSTR) &Head, sizeof(HEAD_REC) ) ) {
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return ( NULL );
			}

		// Скопировать из него нужные поля и инициализировать остальные
		tis->lpFirstKBase = NULL;                               // Первого нет
		tis->lpLastKBase = NULL;                            // Последнего тоже

		tis->numAtt = Head.numAtt;
		tis->numAllVal = Head.numAllVal;

		tis->numRul = Head.numRul;
		tis->numCon = Head.numCon;
		tis->numDis = Head.numDis;

		tis->lenRul = Head.lenRul;
		tis->lenDNF = Head.lenDNF;
		tis->lenCNF = Head.lenCNF;

		tis->isDNF = Head.isDNF;
		tis->isCNF = Head.isCNF;

		tis->ModDescr = Head.ModDescr;

		tis->isChanged = FALSE;

		// Выделить место для атрибутов, значений, правил и буфера
		tis->lpAttIdx = (LPATT_IDX) LK_malloc ( max(1,tis->numAtt) * (long)sizeof(ATT_IDX) );
		tis->lpValIdx = (LPVAL_IDX) LK_malloc ( max(2,tis->numAllVal) * (long)sizeof(VAL_IDX) );

		tis->lpRul    = (LPHEAD_RUL) LK_malloc ( max(64,tis->lenRul) );
		tis->lpDNF    = (LPHEAD_RUL) LK_malloc ( max(64,tis->lenDNF) );
		tis->lpCNF    = (LPHEAD_RUL) LK_malloc ( max(64,tis->lenCNF) );

		LPSTR lpTemp = (LPSTR) LK_malloc ( BUF_SIZE );

		if ( ! tis->lpAttIdx || ! tis->lpValIdx || ! tis->lpRul || ! tis->lpDNF || ! tis->lpCNF || ! lpTemp ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_ALLOCMEM; return NULL;
			}

		// Прочитать атрибуты
		int k = BUF_SIZE / sizeof(ATT_REC);              // Атрибутов в буфере
		int m = k * sizeof(ATT_REC);                // Еденица чтения в байтах
		int n = tis->numAtt / k;                         // Необходимо буферов
		int r = tis->numAtt % k;                        // Останется атрибутов
		int i, l;
		_llseek ( hFile, Head.ofsAtt, 0 ); // Переместиться к началу атрибутов
		for ( i = 0; i < n; i++ ) {
			// Прочитать i-ую порцию атрибутов
			if ( m != _lread ( hFile, (LPSTR) lpTemp, m ) ) {
				LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
				_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
				}
			for ( l = 0; l < k; l++ ) {
				// Записать l-ый атрибут из буфера на i*k+l-ое место в индексе
				tis->lpAttIdx[ i*k + l ].AttRec = ( (LPATT_REC) lpTemp )[l];
				}
			}
		// Прочитать оставшиеся r атрибутов
		if ( r*sizeof(ATT_REC) != _lread( hFile, (LPSTR) lpTemp, r*sizeof(ATT_REC) ) ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
			}
		for ( l = 0; l < r; l++ ) {
			// Записать l-ый атрибут из буфера на n*k+l-ое место в индексе
			tis->lpAttIdx[ n*k + l ].AttRec = ( (LPATT_REC) lpTemp )[l];
			}

		// Прочитать значения
		k = BUF_SIZE / sizeof(VAL_REC);                   // Значений в буфере
		m = k * sizeof(VAL_REC);                    // Еденица чтения в байтах
		n = tis->numAllVal / k;                          // Необходимо буферов
		r = tis->numAllVal % k;                          // Останется значений
		_llseek( hFile, Head.ofsVal, 0 );   // Переместиться к началу значений
		for ( i = 0; i < n; i++ ) {
			// Прочитать i-ую порцию значений
			if ( m != _lread( hFile, (LPSTR) lpTemp, m ) ) {
				LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
				_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
				}
			for ( l = 0; l < k; l++ ) {
				// Записать l-ое значений из буфера на i*k+l-ое место в индексе
				tis->lpValIdx[ i*k + l ].ValRec = ( (LPVAL_REC) lpTemp )[l];
				}
			}
		// Прочитать оставшиеся r значений
		if ( r*sizeof(VAL_REC) != _lread( hFile, (LPSTR) lpTemp, r*sizeof(VAL_REC) ) ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
			}
		for ( l = 0; l < r; l++ ) {
			// Записать l-ое значений из буфера на n*k+l-ое место в индексе
			tis->lpValIdx[ n*k + l ].ValRec = ( (LPVAL_REC) lpTemp )[l];
			}

		// Прочитать правила непосредственно на место
		if ( LKE_SUCCESS != ReadFile ( hFile, Head.ofsRul, tis->lenRul, (LPSTR) tis->lpRul ) ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
			}

		// Прочитать ДНФ непосредственно на место
		if ( LKE_SUCCESS != ReadFile ( hFile, Head.ofsDNF, tis->lenDNF, (LPSTR) tis->lpDNF ) ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
			}

		// Прочитать КНФ непосредственно на место
		if ( LKE_SUCCESS != ReadFile ( hFile, Head.ofsCNF, tis->lenCNF, (LPSTR) tis->lpCNF ) ) {
			LK_free (lpTemp); LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			_lclose ( hFile ); LK_free (tis); Error = LKE_CANTREAD; return NULL;
			}

		LK_free ( lpTemp );
		_lclose ( hFile );

		// Отсортировать атрибуты и значения
		tis->SortAttributes ( );
		tis->SortAllValues ( );
		}

	else {                                       // Создаем новый (new) объект
		// Выделить место для объекта
		tis = (LPKBFILE) LK_malloc ( sizeof(KBFILE) );
		if ( tis == NULL ) { Error = LKE_ALLOCMEM; return NULL; }

		// Инициализировать поля
		tis->KBFilePath[0] = 0;
		tis->KBFileName[0] = 0;

		tis->lpFirstKBase = NULL;                               // Первого нет
		tis->lpLastKBase = NULL;                            // Последнего тоже

		tis->numAtt = 0;
		tis->numAllVal = 0;

		tis->numRul = 0;
		tis->numCon = 0;
		tis->numDis = 0;

		tis->lenRul = 0;
		tis->lenDNF = 0;
		tis->lenCNF = 0;

		tis->isDNF = FALSE;
		tis->isCNF = FALSE;

		tis->ModDescr.Name[0] = tis->ModDescr.Question[0] =
		tis->ModDescr.Comment[0] = tis->ModDescr.IconFile[0] =
		tis->ModDescr.HyperFile[0] = 0;
		tis->ModDescr.HyperIndex = 0;

		tis->isChanged = FALSE;

		// Выделить место для атрибутов, значений и правил
		tis->lpAttIdx = (LPATT_IDX) LK_malloc ( sizeof(ATT_IDX) );
		tis->lpValIdx = (LPVAL_IDX) LK_malloc ( sizeof(VAL_IDX) );
		tis->lpRul    = (LPHEAD_RUL) LK_malloc ( 64 );
		tis->lpDNF    = (LPHEAD_RUL) LK_malloc ( 64 );
		tis->lpCNF    = (LPHEAD_RUL) LK_malloc ( 64 );
		if ( ! tis->lpAttIdx || ! tis->lpValIdx || ! tis->lpRul || ! tis->lpDNF || ! tis->lpCNF ) {
			LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
			LK_free (tis); Error = LKE_ALLOCMEM; return NULL;
			}

		}



	// Выделить место для данных и заключений с инициализацией
	tis->lpData = (LPHEAD_RUL) LK_malloc ( sizeof(HEAD_RUL) );
	tis->lpConc = (LPHEAD_RUL) LK_malloc ( sizeof(HEAD_RUL) );
	if ( tis->lpData == NULL || tis->lpConc == NULL ) {
		LK_free (tis->lpCNF); LK_free (tis->lpDNF); LK_free (tis->lpRul); LK_free (tis->lpValIdx); LK_free (tis->lpAttIdx);
		LK_free ( tis->lpConc ); LK_free ( tis->lpData ); LK_free ( tis );
		Error = LKE_ALLOCMEM; return NULL;
		}

	// Инициализация данных
	tis -> lpData -> numProp = 0;
	tis -> lpData -> lenRul = sizeof ( HEAD_RUL );
	tis -> lpData -> Const.pos = UNITY;
	tis -> lpData -> Const.nec = 0;
	tis -> lpData -> Style = 0;    // Данные действуют через И (как несколько односекционных дизъюнктов)
	tis -> lpData -> numParent1 = 0;
	tis -> lpData -> numParent2 = 0;
	tis -> lpData -> RuleProp1.pos = tis -> lpData -> RuleProp2.pos = 0;
	tis -> lpData -> RuleProp1.nec = tis -> lpData -> RuleProp2.nec = 0;

	// Инициализация заключений
	tis -> lpConc -> numProp = 0;
	tis -> lpConc -> lenRul = sizeof ( HEAD_RUL );
	tis -> lpConc -> Const.pos = UNITY;
	tis -> lpConc -> Const.nec = 0;
	tis -> lpConc -> Style = 0;
	tis -> lpConc -> numParent1 = 0;
	tis -> lpConc -> numParent2 = 0;
	tis -> lpConc -> RuleProp1.pos = tis -> lpData -> RuleProp2.pos = 0;
	tis -> lpConc -> RuleProp1.nec = tis -> lpData -> RuleProp2.nec = 0;

	// Внести в конец списка
	tis->lpPrevKBFile = lpLastKBFile;               // Передо мной экс-крайний
	tis->lpNextKBFile = NULL; lpLastKBFile = tis; // Я крайний (за мной никого нет)
	if ( tis->lpPrevKBFile )                          // За экс-крайним стою я
		tis->lpPrevKBFile->lpNextKBFile = tis;
	else
		lpFirstKBFile                   = tis;

	Error = LKE_SUCCESS;
	return tis;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Деструктор класса KBFILE. Закрываться может только файл,     │ CloseKBFile │
│ который был успешно открыт. Эта функция самостоятельно       └─────────────┤
│ не применяется, т.к. останется мусор (надо закрыть все KBASE'ы).           │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::CloseKBFile ( void ) {
/*
	// Закрыть все созданные объекты класса KBASE (в обратном порядке)
	while ( lpLastKBase ) {
		lpLastKBase->CloseKBase ( );
		}
*/
	// Вычеркнуть из списка
	if ( lpPrevKBFile )
		lpPrevKBFile->lpNextKBFile = lpNextKBFile;
	else
		lpFirstKBFile              = lpNextKBFile;

	if ( lpNextKBFile )
		lpNextKBFile->lpPrevKBFile = lpPrevKBFile;
	else
		lpLastKBFile               = lpPrevKBFile;

	// Освободить место занятое под индексы атрибутов и значений и правила
	LK_free ( lpAttIdx );
	LK_free ( lpValIdx );
	LK_free ( lpRul );
	LK_free ( lpDNF );
	LK_free ( lpCNF );
	LK_free ( lpData );
	LK_free ( lpConc );

	LK_free ( this );
	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Сохранить БЗ. Путь к ней не должен быть пустым, т.е. это не   │ SaveKBFile │
│ должна быть новая БЗ.                                         └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::SaveKBFile ( void ) {

	if ( KBFilePath[0] == 0 ) { return ( Error = LKE_GENERAL ); }

	{
	// Удалить старое расширение и добавить новое *.bak
	char bakPath[128];
	lstrcpy ( bakPath, KBFilePath );
	AddBakExt ( bakPath );
	rename ( KBFilePath, bakPath );
	}

	HEAD_REC Head;
	int      hFile;
	LPSTR    lpTemp;

	lstrcpy ( (LPSTR) Head.CopyWrite, (LPSTR) LK_COPYWRITE );
	Head.Version = LK_VERSION;

	Head.numAtt = numAtt;
	Head.numAllVal = numAllVal;

	Head.numRul = numRul;
	Head.numCon = numCon;
	Head.numDis = numDis;

	Head.lenRul = lenRul;
	Head.lenDNF = lenDNF;
	Head.lenCNF = lenCNF;

	Head.isDNF = isDNF;
	Head.isCNF = isCNF;

	Head.ModDescr = ModDescr;

	// Смещение до раздела равно смещению до предыдущего раздела плюс
	// длина предыдущего раздела
	Head.ofsAtt = sizeof ( HEAD_REC );
	Head.ofsVal = Head.ofsAtt + numAtt * (long) sizeof ( ATT_REC );
	Head.ofsRul = Head.ofsVal + numAllVal * (long) sizeof ( VAL_REC );
	Head.ofsDNF = Head.ofsRul + Head.lenRul;
	Head.ofsCNF = Head.ofsDNF + Head.lenDNF;

	lpTemp = (LPSTR) LK_malloc ( BUF_SIZE );       // Выделить временный буфер
	if ( ! lpTemp ) return ( Error = LKE_ALLOCMEM );

	// Открыть файл БЗ только для записи (его содержимое теряется)
	hFile = _lopen ( KBFilePath, OF_WRITE );
	if ( hFile == -1 ) {
		hFile = _lcreat ( KBFilePath, 0 );
		if ( hFile == -1 ) { LK_free (lpTemp); return ( Error = LKE_CANTOPEN ); }
		}

	// Записать заголовок в файл
	_llseek( hFile, 0, 0 );
	if ( sizeof(HEAD_REC) != _lwrite ( hFile, (LPSTR) &Head, sizeof(HEAD_REC) ) ) {
		_lclose ( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	// Записать атрибуты
	int k = BUF_SIZE / sizeof(ATT_REC);                  // Атрибутов в буфере
	int m = k * sizeof(ATT_REC);                    // Единица записи в байтах
	int n = numAtt / k;                                  // Необходимо буферов
	int r = numAtt % k;                                 // Останется атрибутов
	int i, l;
	_llseek( hFile, Head.ofsAtt, 0 );      // Переместиться к началу атрибутов
	for ( i = 0; i < n; i++ ) {
		// Подготовить i-ую порцию атрибутов в буфере
		for ( l = 0; l < k; l++ ) {
			// Записать i*k+l-ый атрибут из индекса на l-ое место в буфере
			((LPATT_REC)lpTemp)[l] = lpAttIdx [ i*k + l ].AttRec;
			}
		if ( m != _lwrite ( hFile, (LPSTR) lpTemp, m ) ) {
			_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
			}
		}
	// Записать оставшиеся r атрибутов
	for ( l = 0; l < r; l++ ) {
		// Записать n*k+l-ый атрибут из индекса на l-ое место в буфере
		((LPATT_REC)lpTemp)[l] = lpAttIdx [ n*k + l ].AttRec;
		}
	if ( r*sizeof(ATT_REC) != _lwrite( hFile, (LPSTR) lpTemp, r*sizeof(ATT_REC) ) ) {
		_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	// Записать значения
	k = BUF_SIZE / sizeof(VAL_REC);                       // Значений в буфере
	m = k * sizeof(VAL_REC);                        // Еденица записи в байтах
	n = numAllVal / k;                                   // Необходимо буферов
	r = numAllVal % k;                                   // Останется значений
	_llseek( hFile, Head.ofsVal, 0 );       // Переместиться к началу значений
	for ( i = 0; i < n; i++ ) {
		// Подготовить i-ую порцию значений в буфере
		for ( l = 0; l < k; l++ ) {
			// Записать i*k+l-ое значений из индекса на l-ое место в буфере
			( (LPVAL_REC) lpTemp )[l] = lpValIdx [ i*k + l ].ValRec;
			}
		if ( m != _lwrite ( hFile, (LPSTR) lpTemp, m ) ) {
			_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
			}
		}
	// Записать оставшиеся r значений
	for ( l = 0; l < r; l++ ) {
		// Записать n*k+l-ое значений из индекса на l-ое место в буфере
		( (LPVAL_REC) lpTemp)[l] = lpValIdx [ n*k + l ].ValRec;
		}
	if ( r*sizeof(VAL_REC) != _lwrite( hFile, (LPSTR) lpTemp, r*sizeof(VAL_REC) ) ) {
		_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	// Записать правила
	if ( LKE_SUCCESS != WriteFile ( hFile, Head.ofsRul, lenRul, (LPSTR) lpRul ) ) {
		_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	// Записать ДНФ
	if ( LKE_SUCCESS != WriteFile ( hFile, Head.ofsDNF, lenDNF, (LPSTR) lpDNF ) ) {
		_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	// Записать КНФ
	if ( LKE_SUCCESS != WriteFile ( hFile, Head.ofsCNF, lenCNF, (LPSTR) lpCNF ) ) {
		_lclose( hFile ); LK_free (lpTemp); return ( Error = LKE_CANTWRITE );
		}

	_lclose ( hFile );
	LK_free ( lpTemp );
	isChanged = FALSE;
	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ RenameKBFile │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::RenameKBFile ( LPSTR lpPath ) {
	// Необходимо проверить все открытые в данный момент файлы
	// в поисках того, который имеет имя lpPath
	// Если такой открытый файл не найден, то все нормально,
	// иначе необходимо все окна из найденного файла перекинуть
	// в данный, а сам объект уничтожить.
	// В конце надо просто переименовать

	LPKBFILE lpKBFile;

	for ( lpKBFile = lpFirstKBFile; lpKBFile; lpKBFile = lpKBFile->lpNextKBFile ) {
		if ( lpKBFile == this ) continue;
		// Прервать цикл поиска если найден открытый файл с заданным именем
		// В результате переменная цикла lpKBFile не будет равна нулю после цикла
		if ( ! lstrcmp ( (LPSTR) lpPath, (LPSTR) ( lpKBFile->KBFilePath ) ) ) break;
		}

	lstrcpy ( (LPSTR) KBFilePath, (LPSTR) lpPath );
	lstrcpy ( (LPSTR) KBFileName, (LPSTR) GetNameFromPath ( lpPath ) );

	forEachWindow ( CHANGE_TITLE, 0L );

	if ( lpKBFile != 0 ) {
		// Перекинуть все окна из lpKBFile в this
		for ( int i = 0, n = lpKBFile->lpLastKBase->KBaseNumber(); i < n; i++ ) {
			lpKBFile->lpLastKBase->MoveKBase ( this );
			}
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Сбросить КНФ в начальное некомпилированное состояние.           │ ResetCNF │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::ResetCNF ( void ) {
	numDis = 0; isCNF = FALSE; lenCNF = 0;
	lpCNF = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpCNF, 64 );
	return;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Сбросить ДНФ в начальное некомпилированное состояние.           │ ResetDNF │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::ResetDNF ( void ) {
	numCon = 0; isDNF = FALSE; lenDNF = 0;
	lpDNF = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpDNF, 64 );
	return;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ isKBFileNamed │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::isKBFileNamed ( void ) {
	if ( KBFilePath[0] == 0 ) return FALSE;
	return TRUE;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│                                                            │ isKBFileSaved │
│                                                            └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::isKBFileSaved ( void ) {
	return ( ! isChanged );
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Открыт ли уже файл? Путь к файлу должен содержаться в    │ isAlreadyOpened │
│ lpPath. Если среди созданных объектов класса KBFILE      └─────────────────┤
│ существует объект с таким же путем, то возвращается указатель на           │
│ него, в противном случае (файл еще не открыт) возвращается NULL.           │
│ Путь должен быть полным (получен из FileOpen).                             │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPKBFILE KBFILE::isAlreadyOpened ( LPSTR lpPath ) {
	LPKBFILE lpKBFile;

	for ( lpKBFile = lpFirstKBFile; lpKBFile; lpKBFile = lpKBFile->lpNextKBFile ) {
		if ( ! lstrcmp ( (LPSTR) lpPath, (LPSTR) ( lpKBFile->KBFilePath ) ) )
			return lpKBFile;
		}
	return NULL;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции сортировки ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/


/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Сортировка атрибутов. Меняются только номера в индексе    │ SortAttributes │
│ атрибутов. Функция вызывается при любых изменениях        └────────────────┤
│ параметров атрибутов (которые контролируются LK), при изменении их         │
│ количества и при открытии файла. Функция должна правильно работать при     │
│ количестве атрибутов 0 и 1.                                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::SortAttributes ( void ) {
	// Сбросить индекс в начальное состояние с естественным порядком атрибутов
	for ( int i = 0; i < numAtt; i++ ) {
		lpAttIdx[i].idxName = lpAttIdx[i].idxPrice = lpAttIdx[i].idxImportance = lpAttIdx[i].idxNumVal = i;
		}

	BOOL isEnd = FALSE;           // При стабилизации процесса становится TRUE
	int  idxTemp;                                 // Для перестановки индексов

	// Преставляем элементы пока процесс не стабилизируется,
	// т.е. нечего будет переставлять, т.к. все будет на своем месте.
	while ( ! isEnd ) {
		// Очередной проход начинается с предположения, что все хорошо
		isEnd = TRUE;

		for ( int i = 0; i < numAtt - 1; i++ ) {
			// Берем очередной элемент и сравниваем его со следующим;
			// если они стоят неправильно, то переставляем их

			if ( 0 < lstrcmp( (LPSTR) lpAttIdx[ lpAttIdx[i].idxName ].AttRec.Name,
                              (LPSTR) lpAttIdx[ lpAttIdx[i+1].idxName ].AttRec.Name ) ) {
				idxTemp = lpAttIdx[i].idxName;
				lpAttIdx[i].idxName = lpAttIdx[i+1].idxName;
				lpAttIdx[i+1].idxName = idxTemp;
				isEnd = FALSE;
				}
			if ( lpAttIdx[ lpAttIdx[i  ].idxPrice ].AttRec.Price >
                 lpAttIdx[ lpAttIdx[i+1].idxPrice ].AttRec.Price ) {
				idxTemp = lpAttIdx[i].idxPrice;
				lpAttIdx[i].idxPrice = lpAttIdx[i+1].idxPrice;
				lpAttIdx[i+1].idxPrice = idxTemp;
				isEnd = FALSE;
				}
			if ( lpAttIdx[ lpAttIdx[i  ].idxImportance ].AttRec.Importance >
                 lpAttIdx[ lpAttIdx[i+1].idxImportance ].AttRec.Importance ) {
				idxTemp = lpAttIdx[i].idxImportance;
				lpAttIdx[i].idxImportance = lpAttIdx[i+1].idxImportance;
				lpAttIdx[i+1].idxImportance = idxTemp;
				isEnd = FALSE;
				}
			if ( lpAttIdx[ lpAttIdx[i  ].idxNumVal ].AttRec.numVal >
                 lpAttIdx[ lpAttIdx[i+1].idxNumVal ].AttRec.numVal ) {
				idxTemp = lpAttIdx[i].idxNumVal;
				lpAttIdx[i].idxNumVal = lpAttIdx[i+1].idxNumVal;
				lpAttIdx[i+1].idxNumVal = idxTemp;
				isEnd = FALSE;
				}
			}
		}
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Сортировка значений. Меняются только номера в индексе      │ SortAllValues │
│ значений. Должна правильно работать при количестве         └───────────────┤
│ значений 0 и 1.                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::SortAllValues ( void ) {

	for ( int i = 0; i < numAtt; i++ ) SortValues ( i );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Сортировка значений одного атрибута, с заданным абсолютным    │ SortValues │
│ номером. Должна правильно работать при количестве             └────────────┤
│ значений 0 и 1.                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::SortValues ( int Att ) {
	if ( Att < 0 || Att >= numAtt ) return;

	int i;
	int m = lpAttIdx[Att].AttRec.numVal;
	int k = ValSum ( Att );

	// Сбросить индекс в начальное состояние с естественным порядком значений
	for ( i = 0; i < m; i++ ) {
		lpValIdx [ k+i ].idxName = lpValIdx [ k+i ].idxPrice =
		lpValIdx [ k+i ].idxImportance = lpValIdx [ k+i ].idxCharacter = i;
		}

	BOOL isEnd = FALSE;           // При стабилизации процесса становится TRUE
	int  idxTemp;                                 // Для перестановки индексов

	// Преставляем значения одного атрибута пока процесс не стабилизируется,
	// т.е. нечего будет переставлять, т.к. все будет на своем месте.
	while ( ! isEnd ) {
		// Очередной проход начинается с предположения, что все хорошо
		isEnd = TRUE;

		for ( i = 0; i < m-1; i++ ) {
			// Берем очередной элемент и сравниваем его со следующим;
			// если они стоят неправильно, то переставляем их

			if ( 0 < lstrcmp( (LPSTR) lpValIdx[ k + lpValIdx[k+i  ].idxName ].ValRec.Name,
                              (LPSTR) lpValIdx[ k + lpValIdx[k+i+1].idxName ].ValRec.Name ) ) {
				idxTemp = lpValIdx[k+i].idxName;
				lpValIdx[k+i].idxName = lpValIdx[k+i+1].idxName;
				lpValIdx[k+i+1].idxName = idxTemp;
				isEnd = FALSE;
				}
			if ( lpValIdx[ k + lpValIdx[k+i  ].idxPrice ].ValRec.Price >
                 lpValIdx[ k + lpValIdx[k+i+1].idxPrice ].ValRec.Price ) {
				idxTemp = lpValIdx[k+i].idxPrice;
				lpValIdx[k+i].idxPrice = lpValIdx[k+i+1].idxPrice;
				lpValIdx[k+i+1].idxPrice = idxTemp;
				isEnd = FALSE;
				}
			if ( lpValIdx[ k + lpValIdx[k+i  ].idxImportance ].ValRec.Importance >
                 lpValIdx[ k + lpValIdx[k+i+1].idxImportance ].ValRec.Importance ) {
				idxTemp = lpValIdx[k+i].idxImportance;
				lpValIdx[k+i].idxImportance = lpValIdx[k+i+1].idxImportance;
				lpValIdx[k+i+1].idxImportance = idxTemp;
				isEnd = FALSE;
				}
			if ( lpValIdx[ k + lpValIdx[k+i  ].idxCharacter ].ValRec.Character >
                 lpValIdx[ k + lpValIdx[k+i+1].idxCharacter ].ValRec.Character ) {
				idxTemp = lpValIdx[k+i].idxCharacter;
				lpValIdx[k+i].idxCharacter = lpValIdx[k+i+1].idxCharacter;
				lpValIdx[k+i+1].idxCharacter = idxTemp;
				isEnd = FALSE;
				}
			}
		}
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ По индексу (т.е. отсортированному номеру) выдает абсолютный       │ AttI2N │
│ номер в зависимости от сортировки.                                └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::AttI2N ( int idxAtt, int Sort ) {

	if ( idxAtt < 0 || idxAtt >= numAtt ) return idxAtt;

	BOOL isRev;
	if ( Sort > 0 ) isRev = FALSE; else isRev = TRUE;
	Sort = abs ( Sort );

	if ( Sort == SORT_ASINRULE ) return idxAtt;
	if ( isRev ) idxAtt = numAtt - 1 - idxAtt;

	switch ( Sort ) {
		case SORT_UNSORTED   : return idxAtt;
		case SORT_ALPHA      : return lpAttIdx [ idxAtt ].idxName;
		case SORT_VALNUM     : return lpAttIdx [ idxAtt ].idxNumVal;
		case SORT_PRICE      : return lpAttIdx [ idxAtt ].idxPrice;
		case SORT_IMPORTANCE : return lpAttIdx [ idxAtt ].idxImportance;
		default              : return idxAtt;
		}
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ По абсолютному номеру выдает относительный (индекс).              │ AttN2I │
│ Т.е. на каком месте упорядоченного списка стоит атрибут с данным  └────────┤
│ абсолютным номером.                                                        │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::AttN2I ( int Att, int Sort ) {
	int i;
	int n = numAtt;

	if ( Att < 0 || Att >= n ) return Att;

	BOOL isRev;
	if ( Sort > 0 ) isRev = FALSE; else isRev = TRUE;
	Sort = abs ( Sort );

	if ( Sort == SORT_ASINRULE ) return Att;

	switch ( Sort ) {
		case SORT_UNSORTED   : i = Att; break;
		case SORT_ALPHA      :
			for ( i = 0; i < n; i++ ) { if ( lpAttIdx [ i ].idxName == Att ) break; }
			break;
		case SORT_VALNUM     :
			for ( i = 0; i < n; i++ ) { if ( lpAttIdx [ i ].idxNumVal == Att ) break; }
			break;
		case SORT_PRICE      :
			for ( i = 0; i < n; i++ ) { if ( lpAttIdx [ i ].idxPrice == Att ) break; }
			break;
		case SORT_IMPORTANCE :
			for ( i = 0; i < n; i++ ) { if ( lpAttIdx [ i ].idxImportance == Att ) break; }
			break;
		default              : i = Att;
		}
	if ( isRev ) return n - 1 - i;
	else                 return i;
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ По индексу (т.е. отсортированному номеру) значения и абсолютному  │ ValI2N │
│ номеру атрибута выдает абсолютный номер значения в зависимости от └────────┤
│ сортировки. Номер атрибута является абсолютным.                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::ValI2N ( int Att, int idxVal, int Sort ) {

	if ( Att < 0 || Att >= numAtt ) return idxVal;

	int m = lpAttIdx [ Att ].AttRec.numVal;

	if ( idxVal < 0 || idxVal >= m ) return idxVal;

	BOOL isRev;
	if ( Sort > 0 ) isRev = FALSE; else isRev = TRUE;
	Sort = abs ( Sort );

	if ( isRev ) idxVal = m - 1 - idxVal;

	int s = ValSum ( Att ) + idxVal;

	switch ( Sort ) {
		case SORT_UNSORTED   : return idxVal;
		case SORT_ALPHA      : return lpValIdx [ s ].idxName;
		case SORT_CHARACTER  : return lpValIdx [ s ].idxCharacter;
		case SORT_PRICE      : return lpValIdx [ s ].idxPrice;
		case SORT_IMPORTANCE : return lpValIdx [ s ].idxImportance;
		default              : return idxVal;
		}
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ По абсолютному номеру атрибута и значения выдает относительный    │ ValN2I │
│ номер значения, т.е. на каком месте упорядоченного списка стоит   └────────┤
│ значение с данным абсолютным номером.                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::ValN2I ( int Att, int Val, int Sort ) {

	int n = numAtt;
	if ( Att < 0 || Att >= n ) return Val;

	int m = lpAttIdx [ Att ].AttRec.numVal;
	if ( Val < 0 || Val >= m ) return Val;

	int s = ValSum ( Att );
	int j;
	BOOL isRev;
	if ( Sort > 0 ) isRev = FALSE; else isRev = TRUE;
	Sort = abs ( Sort );

	switch ( Sort ) {
		case SORT_UNSORTED   : j = Val; break;
		case SORT_ALPHA      :
			for ( j = 0; j < m; j++ ) { if ( lpValIdx [ s + j ].idxName == Val ) break; }
			break;
		case SORT_CHARACTER  :
			for ( j = 0; j < m; j++ ) { if ( lpValIdx [ s + j ].idxCharacter == Val ) break; }
			break;
		case SORT_PRICE      :
			for ( j = 0; j < m; j++ ) { if ( lpValIdx [ s + j ].idxPrice == Val ) break; }
			break;
		case SORT_IMPORTANCE :
			for ( j = 0; j < m; j++ ) { if ( lpValIdx [ s + j ].idxImportance == Val ) break; }
			break;
		default              : j = Val;
		}
	if ( isRev ) return m - 1 - j;
	else         return j;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Возвращает номер атрибута (относительный), который соответствует│ Prop2Att │
│ указанному высказыванию в правиле, т.е. на каком месте в        └──────────┤
│ упорядоченном списке всех атрибутов стоит высказывание, стоящее на Prop    │
│ месте в упорядоченном списке высказываний данного правила.                 │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::Prop2Att ( int Prop, LPHEAD_RUL pRul, int Sort ) {
	// Необходимо пройтись по полному упорядоченному списку при этом
	// подсчитывая атрибуты, входящие в правило. Как только будет
	// встречен Prop-ый атрибут, входящий в правило, то его сквозной
	// номер в списке и есть искомый.
	// Если указана сортировка "как в правиле", то необходимо просто вернуть
	// абсолютный номер атрибута, который стоит на Prop-ом месте попорядку в правиле.
	// Необходимо помнить, что обратность сортировки в этом случае относится
	// только к номеру высказывания, а не к атрибуту.

	if ( Prop < 0 )                    return -1;
	if ( Prop >= PropNumber ( pRul ) ) return numAtt;

	if ( abs ( Sort ) == SORT_ASINRULE ) {
		return AbsProp2Att ( Sort < 0 ? PropNumber ( pRul ) - Prop - 1 : Prop, pRul );
		}

	for ( int i = 0, p = -1; i < numAtt; i++ ) {
		if ( GetPropHead ( pRul, AttI2N ( i, Sort ) ) ) if ( ++p == Prop ) break;
		}

	return i;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Возвращает номер высказывания (упорядоченный), который          │ Att2Prop │
│ соответствует указанному атрибуту, т.е. на каком месте в        └──────────┤
│ упорядоченном списке всех высказываний стоит атрибут с номером Att.        │
│ Если указанный атрибут отсутствует в правиле, то выдается -1.              │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::Att2Prop ( int Att, LPHEAD_RUL pRul, int Sort ) {
	// Необходимо пройтись по полному упорядоченному списку атрибутов,
	// одновременно подсчитывая те из них, которые входят в правило.
	// После того, как встретится Att-ый атрибут (т.е., цикл состоит из
	// Att шагов) процесс прекращается, а подсчитанное число вхождений
	// в правило и есть номер высказывания. Предполагается, что Att-ый
	// атрибут входит в правило.
	// Если указана сортировка "как в правиле", то, считая, что номер атрибута
	// задан абсолютно, необходимо пройтись по всем высказываниям (с начала или
	// с конца) и остановиться на том, которое имеет данный номер. Индекс цикла
	// и будет номером высказывания. Необходимо помнить, что в этом случае
	// обратность сортировки относится только к номеру высказвания, а не к атрибуту.

	if ( Att < 0 )       return -1;
	if ( Att >= numAtt ) return PropNumber ( pRul );

	if ( abs ( Sort ) == SORT_ASINRULE ) {
		int p = AbsAtt2Prop ( Att, pRul );
		return Sort < 0 ? PropNumber ( pRul ) - p - 1 : p;
		}

	for ( int i = 0, p = -1; i <= Att; i++ ) {
		if ( GetPropHead ( pRul, AttI2N ( i, Sort ) ) ) p++;
		}

	return p;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Групповые функции (списочные) ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Каждому окну данного открытого файла послать указанное     │ forEachWindow │
│ сообщение.                                                 └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::forEachWindow ( int Msg, DWORD Prm ) {

	for ( LPKBASE lpKBase = lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {
		if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, Msg, Prm );
		}

	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Возвращает число открытых файлов.                          │ KBFilesNumber │
│ Статическая функция.                                       └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::KBFilesNumber ( VOID ) {

	int i = 0;
	for ( LPKBFILE lpKBFile = lpFirstKBFile; lpKBFile; lpKBFile = lpKBFile->lpNextKBFile ) {
		i++;
		}
	return i;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Возвращает число открытых окон для данного файла.            │ KBaseNumber │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::KBaseNumber ( VOID ) {

	int i = 0;
	for ( LPKBASE lpKBase = lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {
		i++;
		}
	return i;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования и доступа к атрибутам ░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Получить описание атрибута по указанному адресу.            │ GetAttribute │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPATT_REC KBFILE::GetAttribute ( int Att ) {
	if ( Att < 0 || Att >= numAtt ) return 0;

	return (LPATT_REC) & ( lpAttIdx [ Att ].AttRec );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Сохранить описание атрибута из записи в индексе атрибутов   │ PutAttribute │
│ вместо текущего атрибута (старая информация теряется).      └──────────────┤
│ Поле numVal менять нельзя. Остальные поля должны быть правильно            │
│ инициализированы (система ничего этого не проверяет). После сохранения     │
│ производится досортировка. Текущим  остается прежний атрибут, но его       │
│ относительный номер (т.е. curAtt) может измениться.                        │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::PutAttribute ( int Att, LPATT_REC lpAttRec ) {

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	for ( int i = 0; i < numAtt; i++ ) {
		if ( i != Att )
			if ( ! lstrcmp ( (LPSTR) lpAttIdx [ i ].AttRec.Name, (LPSTR) lpAttRec->Name ) )
				return ( Error = LKE_NOTUNIQUE );
		}

	int numValues = lpAttIdx [ Att ].AttRec.numVal;  // Сохранить поле numVal
	lpAttIdx [ Att ].AttRec = *lpAttRec;
	lpAttIdx [ Att ].AttRec.numVal = numValues;   // Восстановить поле numVal

	SortAttributes();
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Вставляет атрибут перед текущим. Поле numVal значения не    │ InsAttribute │
│ имеет. Остальные должны быть правильно инициализированы.    └──────────────┤
│ После вставки текущим становится новый атрибут, однако, его положение в    │
│ списке (т.е. curAtt) может быть любым (не обязательно перед предыдущим)    │
│ и зависит от параметров.                                                   │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsAttribute ( int Att, LPATT_REC lpAttRec ) {
	LPATT_IDX New;

	if ( Att < 0 ) return ( Error = LKE_SUCCESS );

	for ( int i = 0; i < numAtt; i++ ) {
		if ( ! lstrcmp ( (LPSTR) lpAttIdx [ i ].AttRec.Name, (LPSTR) lpAttRec->Name ) )
			return ( Error = LKE_NOTUNIQUE );
		}

	// Выделить место для нового атрибута
	New = (LPATT_IDX) LK_realloc ( lpAttIdx, ( numAtt + 1 ) * sizeof(ATT_IDX) );
	if ( ! New ) return ( Error = LKE_ALLOCMEM );

	lpAttIdx = New;

	// Раздвинуть записи и освободить место перед атрибутом Att
	for ( i = numAtt; i > Att; i-- ) lpAttIdx [ i ] = lpAttIdx [ i - 1 ];

	lpAttIdx [ Att ].AttRec = *lpAttRec;
	lpAttIdx [ Att ].AttRec.numVal = 0;

	numAtt++;

	// Скомпенсировать увеличение абсолютных номеров некоторых атрибутов
	InsAttInKBase ( Att );
	InsAttInCNF ( Att );
	InsAttInDNF ( Att );
	InsAttInRule ( Att, lpData );
	InsAttInRule ( Att, lpConc );

	SortAttributes();
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Удаляет текущий атрибут. Вместе с ним удаляются все его     │ DelAttribute │
│ значения. После удаления текущим становится следующий в     └──────────────┤
│ упорядоченном списке атрибут c -1-ым текущим значением.                    │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelAttribute ( int Att ) {

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	// Удалить все высказывания о данном атрибуте из базы знаний (и подстроить указатели на правио для всех окон)
	DelPropInKBase ( Att );
	// а также из данных и заключений
	DelPropInData ( Att );
	DelPropInConc ( Att );

	// Удалить все значения текущего атрибута из индекса значений (схлопнуть)
	int s = lpAttIdx [ Att ].AttRec.numVal;
	if ( s ) {
		for ( int i = ValSum ( Att + 1 ); i < numAllVal; i++ ) {
			lpValIdx [ i - s ] = lpValIdx [ i ];
			}
		}

	// Удалить атрибут из индекса (сдвинуть записи на место атрибута Att, схлопнуть)
	for ( int i = Att + 1; i < numAtt; i++ ) {
		lpAttIdx [ i - 1 ] = lpAttIdx [ i ];
		}

	numAtt--;
	numAllVal -= s;

	// Скомпенсировать уменьшение абсолютных номеров некоторых атрибутов
	DelAttInKBase ( Att );
	DelAttInRule ( Att, lpData );
	DelAttInRule ( Att, lpConc );

	SortAttributes();
	isChanged = TRUE;

	// Сбросить скомпилированные части
	ResetDNF(); ResetCNF();

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Вернуть количество атрибутов в базе знаний.                    │ AttNumber │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::AttNumber ( void ) {
	return numAtt;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования и доступа к значениям ░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Получить описание значения по указанному адресу.                │ GetValue │
│                                                                 └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPVAL_REC KBFILE::GetValue ( int Att, int Val ) {
	if ( Att < 0 || Att >= numAtt ) return 0;
	if ( Val < 0 || Val >= ValNumber ( Att ) ) return 0;

	return (LPVAL_REC) & ( lpValIdx [ ValSum ( Att ) + Val ].ValRec );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Сохранить описание значения из записи в индексе значений        │ PutValue │
│ вместо текущего значения (старая информация теряется).          └──────────┤
│ Поля записи должны быть правильно                                          │
│ инициализированы (система ничего этого не проверяет). После сохранения     │
│ производится досортировка.                                                 │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::PutValue ( int Att, int Val, LPVAL_REC lpValRec ) {
	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );
	if ( Val < 0 || Val >= ValNumber ( Att ) ) return ( Error = LKE_SUCCESS );

	int m = ValNumber ( Att );
	int k = ValSum ( Att );
	for ( int j = 0; j < m; j++ ) {
		if ( j != Val )
			if ( ! lstrcmp ( (LPSTR) lpValIdx [ k + j ].ValRec.Name, (LPSTR) lpValRec->Name ) )
				return ( Error = LKE_NOTUNIQUE );
		}

	lpValIdx [ k + Val ].ValRec = *lpValRec;

	// Количество значений не меняется, поэтому атрибуты не сортируются
	SortValues ( Att );
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Вставляет значение перед указанным.                             │ InsValue │
│ Поля должны быть правильно инициализированы.                    └──────────┤
│ После вставки текущим становится новое значение, однако, оно может         │
│ находиться в любом месте списка в зависимости от                           │
│ параметров. Поскольку меняется и количество значений, то может измениться  │
│ и текущий атрибут (только при сортировке по количеству значений). Вернее   │
│ сам атрибут не меняется, а меняется его положение в списке.                │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsValue ( int Att, int Val, LPVAL_REC lpValRec ) {

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );
	if ( Val < 0 ) return ( Error = LKE_SUCCESS );

	int m = ValNumber ( Att );
	int k = ValSum ( Att );
	for ( int j = 0; j < m; j++ ) {
		if ( ! lstrcmp ( (LPSTR) lpValIdx [ k + j ].ValRec.Name, (LPSTR) lpValRec->Name ) )
			return ( Error = LKE_NOTUNIQUE );
		}

	// Выделить место для нового значения
	LPVAL_IDX New = (LPVAL_IDX) LK_realloc ( lpValIdx, ( numAllVal + 1 ) * sizeof(VAL_IDX) );
	if ( ! New ) return ( Error = LKE_ALLOCMEM );
	lpValIdx = New;

	// Вставить одну тривиальную компоненту во все правила базы знаний для
	// высказывания об атрибуте Att
	InsValInKBase ( Att, Val );
	InsValInCNF ( Att, Val );
	InsValInDNF ( Att, Val );
	InsValInData ( Att, Val );
	InsValInConc ( Att, Val );

	// Раздвинуть записи и освободить место перед значением Val
	for ( j = numAllVal; j > k + Val; j-- ) {
		lpValIdx [ j ] = lpValIdx [ j - 1 ];
		}

	lpValIdx [ k + Val ].ValRec = *lpValRec;

	lpAttIdx [ Att ].AttRec.numVal++;
	numAllVal++;

	SortAttributes();     // Меняется только сортировка по количеству значений
	SortValues ( Att );
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Удаляет указанное значение.                                     │ DelValue │
│ После удаления текущим становится следующее в                   └──────────┤
│ упорядоченном списке значение (т.е. curVal не меняется). Текущий атрибут   │
│ не меняется, однако при сортировке по количеству значений может изменить   │
│ свое положение в списке.                                                   │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelValue ( int Att, int Val ) {

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );
	if ( Val < 0 || Val >= ValNumber ( Att ) ) return ( Error = LKE_SUCCESS );

	// Скомпенсировать удаление значения для базы знаний
	DelValInKBase ( Att, Val );
	DelValInData ( Att, Val );
	DelValInConc ( Att, Val );

	// Сдвинуть записи на место значения Val
	for ( int j = ValSum ( Att ) + Val + 1; j < numAllVal; j++ ) {
		lpValIdx [ j - 1 ] = lpValIdx [ j ];
		}

	numAllVal--;
	lpAttIdx [ Att ].AttRec.numVal--;

	SortAttributes ( );
	SortValues ( Att );
	isChanged = TRUE;

	// Сбросить скомпилированные части
	ResetDNF(); ResetCNF();

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Возвращает количество значений указанного атрибута.            │ ValNumber │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::ValNumber ( int Att ) {
	if ( Att < 0 || Att >= numAtt ) return -1;
	return lpAttIdx [ Att ].AttRec.numVal;
	}

/*
┌───────────────────────────────────────────────────────────────────┬────────┐
│ Возвращает суммарное количество значений до указанного            │ ValSum │
│ атрибута: без сортировки, с сортировкой, по правилу.              └────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::ValSum ( int Att ) {
	if ( Att < 0 ) return -1;
	if ( Att >= numAtt ) return numAllVal;
	for ( int i = 0, sum = 0; i < Att; i++ )
		sum += ValNumber ( i );
	return sum;
	}
int KBFILE::ValSum ( int Att, int Sort ) {
	if ( Att < 0 ) return -1;
	if ( Att >= numAtt ) return numAllVal;

	for ( int i = 0, sum = 0; i < Att; i++ )
		sum += ValNumber ( AttI2N ( i, Sort ) );
	return sum;
	}
int KBFILE::ValSum ( int Prop, LPHEAD_RUL pRul, int Sort ) {
	if ( Prop < 0 ) return -1;
	if ( Prop > PropNumber ( pRul ) ) Prop = PropNumber ( pRul );

	for ( int p = 0, sum = 0; p < Prop; p++ )
		sum += ValNumber ( AttI2N ( Prop2Att(p,pRul,Sort), Sort ) );
	return sum;
	}

/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
░░░ Функции манипулирования и доступа к правилам ░░░░░░░░░░░░░░░░░░ KBFILE ░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Возвращает количество правил в БЗ.                            │ RuleNumber │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::RuleNumber ( void ) {
	return numRul;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Возвращает указатель на заголовок правила, номер которог     │ GetRuleHead │
│ передается в качестве параметра.                             └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::GetRuleHead ( int Rul ) {
	// Необходимо отсчитать от начала базы знаний необходимое количество
	// правил и остановиться.

	if ( Rul >= numRul )             // Вернуть указатель на конец базы знаний
		return (LPHEAD_RUL) ((LPSTR) lpRul + lenRul);

	if ( Rul <= 0 )                    // Вернуть указатель на нулевое правило
		return lpRul;

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < Rul; r++ ) {
		// Перейти к следующему правилу
		pRul = (LPHEAD_RUL) ((LPSTR) pRul + pRul->lenRul);
		}

	return pRul;
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Вставить пустое правило перед указанным. Это правило не содержит │ InsRule │
│ никаких высказываний (один заголовок). Общая длина правил        └─────────┤
│ увеличивается на длину заголовка. Число правил увеличивается на 1 (numRul).│
│ Корректность указателя на правило не проверяется!                          │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsRule ( LPHEAD_RUL pRul ) {

	// Выделить память для нового правила
	LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpRul, lenRul + sizeof(HEAD_RUL) );
	if ( ! New ) return ( Error = LKE_ALLOCMEM );
	lpRul = New;

	// Раздвинуть правила начиная с текущего
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pRul, sizeof(HEAD_RUL) ) )
		return  ( Error = LKE_ALLOCMEM );

	// Инициализировать заголовок правила
	pRul->numProp = 0;
	pRul->lenRul = sizeof(HEAD_RUL);
	pRul->Const.nec = 0;
	pRul->Const.pos = 0;
	pRul->Style = 0;
	pRul->numParent1 = 0;
	pRul->numParent2 = 0;
	pRul->RuleProp1.nec = pRul->RuleProp1.pos = 0;
	pRul->RuleProp2.nec = pRul->RuleProp2.pos = 0;

	// Подстроить информацию о базе знаний
	lenRul += sizeof(HEAD_RUL);
	numRul++;
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Удалить указанное правило. Если указатель указывает на самый     │ DelRule │
│ конец базы, то ничего не происходит. Общая длина уменьшается на  └─────────┤
│ длину удаляемого правила, а количество правил - на 1. Корректность         │
│ указателя на правило не проверяется.                                       │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelRule ( LPHEAD_RUL pRul ) {

	int Len = pRul->lenRul;

	// Сдвинуть правила на место текущего
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pRul, - Len ) )
		return ( Error = LKE_ALLOCMEM );

	// Подстроить информацию о базе знаний
	lenRul -= Len;
	numRul--;
	isChanged = TRUE;

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Скомпенсировать добавление атрибута для всей базы знаний.  │ InsAttInKBase │
│ Происходит увеличение некоторых номеров высказываний.      └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsAttInKBase ( int Att ) {
	// Выполнить операцию для всех правил

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < numRul; r++ ) {
		pRul = InsAttInRule ( Att, pRul );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Скомпенсировать добавление атрибута для ДНФ.                 │ InsAttInDNF │
│ Происходит увеличение некоторых номеров высказываний.        └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsAttInDNF ( int Att ) {
	// Выполнить операцию для всех утверждений

	LPHEAD_RUL pRul = lpDNF;
	for ( int r = 0; r < numCon; r++ ) {
		pRul = InsAttInRule ( Att, pRul );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Скомпенсировать добавление атрибута для КНФ.                 │ InsAttInCNF │
│ Происходит увеличение некоторых номеров высказываний.        └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsAttInCNF ( int Att ) {
	// Выполнить операцию для всех утверждений

	LPHEAD_RUL pRul = lpCNF;
	for ( int r = 0; r < numDis; r++ ) {
		pRul = InsAttInRule ( Att, pRul );
		}
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Увеличить на 1 те номера атрибутов высказываний, которые    │ InsAttInRule │
│ имеют номер больше лмбо равный указанного. Вернуть ссылку   └──────────────┤
│ на следующее правило.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::InsAttInRule ( int Att, LPHEAD_RUL pRul ) {
	// Необходимо пройтись по всем высказываниям в правиле и увеличить на 1
	// номер тех, которые имеют номер атрибута больший либо равный Att

	// Пропускаем заголовок правила, считая, что далее начинается первое высказывание
	LPHEAD_PROP lpProp = (LPHEAD_PROP) ( pRul + 1 );

	// Берем попорядку все высказывания из правила и сравниваем его номер с заданным
	for ( int p = 0; p < pRul->numProp; p++ ) {
		if ( lpProp->curAtt >= Att ) lpProp->curAtt++;
		// Перейти к следующему высказыванию
		lpProp = (LPHEAD_PROP) ( (LPSTR) lpProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ lpProp->curAtt ].AttRec.numVal );
		}
	return (LPHEAD_RUL) lpProp;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Скомпенсировать удаление атрибута для всей базы знаний.    │ DelAttInKBase │
│ Некоторые номера высказываний могут уменьшиться.           └───────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::DelAttInKBase ( int Att ) {
	// Выполнить операцию для всех правил

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < numRul; r++ ) {
		pRul = DelAttInRule ( Att, pRul );
		}
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Уменьшить на 1 те номера атрибутов высказываний, которые    │ DelAttInRule │
│ имеют номер строго больший указанного. Вернуть ссылку       └──────────────┤
│ на следующее правило.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::DelAttInRule ( int Att, LPHEAD_RUL pRul ) {
	// Необходимо пройтись по всем высказываниям в правиле и уменьшить на 1
	// номер тех, которые имеют номер атрибута больший Att

	// Пропускаем заголовок правила, считая, что далее начинается первое высказывание
	LPHEAD_PROP lpProp = (LPHEAD_PROP) ( pRul + 1 );

	// Берем попорядку все высказывания из правила и сравниваем его номер с заданным
	for ( int p = 0; p < pRul->numProp; p++ ) {
		if ( lpProp->curAtt > Att ) lpProp->curAtt--;
		// Перейти к следующему высказыванию
		lpProp = (LPHEAD_PROP) ( (LPSTR) lpProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ lpProp->curAtt ].AttRec.numVal );
		}
	return (LPHEAD_RUL) lpProp;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Скомпенсировать добавление значения для всей базы знаний.  │ InsValInKBase │
│ В соответствующие высказывания добавляется по одной        └───────────────┤
│ тривиальной компоненте.                                                    │
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsValInKBase ( int Att, int Val ) {
	// Выполнить операцию для всех правил

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < numRul; r++ ) {
		pRul = InsValInRule ( Att, Val, pRul );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Скомпенсировать добавление атрибута для ДНФ. В соответствую- │ InsValInDNF │
│ щие высказывания добавляется по одной тривиальной компоненте.└─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsValInDNF ( int Att, int Val ) {
	// Выполнить операцию для всех утверждений

	LPHEAD_RUL pRul = lpDNF;
	for ( int r = 0; r < numCon; r++ ) {
		pRul = InsValInRule ( Att, Val, pRul );
		}
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Скомпенсировать добавление атрибута для КНФ. В соответствую- │ InsValInCNF │
│ щие высказывания добавляется по одной тривиальной компоненте.└─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::InsValInCNF ( int Att, int Val ) {
	// Выполнить операцию для всех утверждений

	LPHEAD_RUL pRul = lpCNF;
	for ( int r = 0; r < numDis; r++ ) {
		pRul = InsValInRule ( Att, Val, pRul );
		}
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывание в номером Att вставить новую компоненту перед│ InsValInRule │
│ имеющей номер Val. Для этого необходимо раздвинуть правило. └──────────────┤
│ Возвращается указатель на следующее правило.                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::InsValInRule ( int Att, int Val, LPHEAD_RUL pRul ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. Далее проверить есть ли память для новой компоненты.
	// Если нет, то выделить. После этого раздвинуть базу знаний на длину
	// одной компоненты и инициализировать образовавшуюся дырку тривиальным
	// в данном контексте значением.

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	if ( lenRul + sizeof(COMP) > MemorySize ( (LPVOID) lpRul ) ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( lpRul, lenRul + sizeof(COMP) );
		if ( ! New ) { Error = LKE_ALLOCMEM; return 0; }                // ???
		lpRul = New;
		}

	// Раздвинуть базу знаний, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pComp, sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                      // ???

	pRul->lenRul += sizeof(COMP);
	lenRul += sizeof(COMP);

	// Инициализировать компоненту высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	pComp->nec = 0;
	if (pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
	else                               pComp->pos = 0;

	return (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывание в номером Att вставить новую компоненту перед│ InsValInData │
│ имеющей номер Val. Для этого необходимо раздвинуть правило. └──────────────┤
│ Возвращается указатель на следующее правило.                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::InsValInData ( int Att, int Val ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. Далее проверить есть ли память для новой компоненты.
	// Если нет, то выделить. После этого раздвинуть данные на длину
	// одной компоненты и инициализировать образовавшуюся дырку тривиальным
	// в данном контексте значением.

	LPHEAD_PROP pProp = GetPropHead ( lpData, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) lpData + lpData->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	if ( lpData->lenRul + sizeof(COMP) > MemorySize ( (LPVOID) lpData ) ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( lpData, lpData->lenRul + sizeof(COMP) );
		if ( ! New ) { Error = LKE_ALLOCMEM; return 0; }                // ???
		lpData = New;
		}

	// Раздвинуть данные, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpData, lpData->lenRul, (LPVOID) pComp, sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                      // ???

	lpData->lenRul += sizeof(COMP);

	// Инициализировать компоненту высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	pComp->nec = 0;
	if (pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
	else                               pComp->pos = 0;

	return (LPHEAD_RUL) ( (LPSTR) lpData + lpData->lenRul );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывание в номером Att вставить новую компоненту перед│ InsValInConc │
│ имеющей номер Val. Для этого необходимо раздвинуть правило. └──────────────┤
│ Возвращается указатель на следующее правило.                               │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::InsValInConc ( int Att, int Val ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. Далее проверить есть ли память для новой компоненты.
	// Если нет, то выделить. После этого раздвинуть заключения на длину
	// одной компоненты и инициализировать образовавшуюся дырку тривиальным
	// в данном контексте значением.

	LPHEAD_PROP pProp = GetPropHead ( lpConc, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) lpConc + lpConc->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	if ( lpConc->lenRul + sizeof(COMP) > MemorySize ( (LPVOID) lpConc ) ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( lpConc, lpConc->lenRul + sizeof(COMP) );
		if ( ! New ) { Error = LKE_ALLOCMEM; return 0; }                // ???
		lpConc = New;
		}

	// Раздвинуть заключения, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpConc, lpConc->lenRul, (LPVOID) pComp, sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                      // ???

	lpConc->lenRul += sizeof(COMP);

	// Инициализировать компоненту высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	pComp->nec = 0;
	if (pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
	else                               pComp->pos = 0;

	return (LPHEAD_RUL) ( (LPSTR) lpConc + lpConc->lenRul );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Скомпенсировать удаление значения для всей базы знаний.    │ DelValInKBase │
│ Для этого необходимо удалить компоненту из некоторых       └───────────────┤
│ высказываний.                                                              │
└────────────────────────────────────────────────────────────────────────────┘
*/
void KBFILE::DelValInKBase ( int Att, int Val ) {
	// Выполнить операцию для всех правил

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < numRul; r++ ) {
		pRul = DelValInRule ( Att, Val, pRul );
		}
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывании с номером Att удалить компоненту с номером   │ DelValInRule │
│ Val, а освободившееся место схлопнуть. Вернуть ссылку       └──────────────┤
│ на следующее правило.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::DelValInRule ( int Att, int Val, LPHEAD_RUL pRul ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. После этого сдвинуть базу знаний на длину
	// одной компоненты.

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	// Сдвинуть базу знаний, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pComp, - sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                         // ???

	lenRul -= sizeof(COMP);
	pRul->lenRul -= sizeof(COMP);

	return (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывании с номером Att удалить компоненту с номером   │ DelValInData │
│ Val, а освободившееся место схлопнуть. Вернуть ссылку       └──────────────┤
│ на следующее правило.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::DelValInData ( int Att, int Val ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. После этого сдвинуть данные на длину
	// одной компоненты.

	LPHEAD_PROP pProp = GetPropHead ( lpData, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) lpData + lpData->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	// Сдвинуть данные, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpData, lpData->lenRul, (LPVOID) pComp, - sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                         // ???

	lpData->lenRul -= sizeof(COMP);

	return (LPHEAD_RUL) ( (LPSTR) lpData + lpData->lenRul );
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ В высказывании с номером Att удалить компоненту с номером   │ DelValInConc │
│ Val, а освободившееся место схлопнуть. Вернуть ссылку       └──────────────┤
│ на следующее правило.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_RUL KBFILE::DelValInConc ( int Att, int Val ) {
	// Необходимо найти высказывание с абсолютным номером Att, а в нем
	// Val'ую компоненту. После этого сдвинуть данные на длину
	// одной компоненты.

	LPHEAD_PROP pProp = GetPropHead ( lpConc, Att );
	if ( pProp == 0 ) return (LPHEAD_RUL) ( (LPSTR) lpConc + lpConc->lenRul );
	LPCOMP pComp = (LPCOMP) (pProp + 1) + Val;

	// Сдвинуть данные, начиная с заданной компоненты
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpConc, lpConc->lenRul, (LPVOID) pComp, - sizeof(COMP) ) )
		{ Error = LKE_ALLOCMEM; return 0; }                         // ???

	lpConc->lenRul -= sizeof(COMP);

	return (LPHEAD_RUL) ( (LPSTR) lpConc + lpConc->lenRul );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Возвращает количество высказываний в указанном правиле.       │ PropNumber │
│ Корректность указателя на правило не проверяется.             └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::PropNumber ( LPHEAD_RUL pRul ) {
	return pRul->numProp;
	}
int KBFILE::PropNumber ( int Rul ) {
	if ( Rul < 0 )             Rul = 0;
	else if ( Rul >= numRul )  Rul = numRul - 1;
	return PropNumber ( GetRuleHead ( Rul ) );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Вернуть указатель на заголовок высказывания по абсолютному   │ GetPropHead │
│ номеру атрибута.                                             └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPHEAD_PROP KBFILE::GetPropHead ( LPHEAD_RUL pRul, int Att ) {
	if ( Att < 0 )       return 0;
	if ( Att >= numAtt ) return (LPHEAD_PROP) ( (LPSTR) pRul + pRul->lenRul );

	// Пропускаем заголовок правила, считая, что далее начинается первое высказывание
	LPHEAD_PROP lpProp = (LPHEAD_PROP) ( pRul + 1 );
	int p, m;

	// Берем попорядку все высказывания из правила и сравниваем его номер с заданным
	for ( p = 0; p < pRul->numProp; p++ ) {
		m = lpProp->curAtt;
		if ( m == Att ) return lpProp;
		// Перейти к следующему высказыванию
		lpProp = (LPHEAD_PROP) ( (LPSTR) lpProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ m ].AttRec.numVal );
		}
	return 0;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Какой атрибут стоит на указанном месте в правиле.            │ AbsProp2Att │
│ Возвращается абсолютный номер атрибута.                      └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::AbsProp2Att ( int Prop, LPHEAD_RUL pRul ) {
	if ( Prop < 0 )              return -1;
	if ( Prop >= pRul->numProp ) return numAtt;

	// Пропускаем заголовок правила, считая, что далее начинается первое высказывание
	LPHEAD_PROP lpProp = (LPHEAD_PROP) ( pRul + 1 );

	// Отсчитываем от начала Prop высказываний
	for ( int i = 0; i < Prop; i++ ) {
		// Перейти к следующему высказыванию
		lpProp = (LPHEAD_PROP) ( (LPSTR) lpProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ lpProp->curAtt ].AttRec.numVal );
		}
	return lpProp->curAtt;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ По абсолютному номеру атрибута найти номер высказывания, т.е.│ AbsAtt2Prop │
│ на каком месте в правиле стоит данный атрибут.               └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::AbsAtt2Prop ( int Att, LPHEAD_RUL pRul ) {
	if ( Att < 0 )       return -1;
	if ( Att >= numAtt ) return PropNumber ( pRul );

	// Пропускаем заголовок правила, считая, что далее начинается первое высказывание
	LPHEAD_PROP lpProp = (LPHEAD_PROP) ( pRul + 1 );
	int i, m;

	// Берем попорядку все высказывания из правила и сравниваем его номер с заданным
	for ( i = 0; i < pRul->numProp; i++ ) {
		m = lpProp->curAtt;
		if ( m == Att ) return i;
		// Перейти к следующему высказыванию
		lpProp = (LPHEAD_PROP) ( (LPSTR) lpProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ m ].AttRec.numVal );
		}
	return -1;
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция вставляет в указанное правило высказывание об      │ InsPropInData │
│ атрибуте Att перед атрибутом BeforeAtt, считая, что это    └───────────────┤
│ правило единственное. Номера атрибутов абсолютные.                         │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsPropInData ( int Att, int BeforeAtt ) {
	// Найти указатель на высказывание об атрибуте BeforeAtt. Если он не найден, то
	// закончить. Далее необходимо проверить существует ли уже в правиле
	// атрибут Att. Если да, то вернуть ошибку, иначе продолжить. Далее необходимо
	// раздвинуть это место, т.е., отодвинуть подальше хвост правила. При этом
	// необходимо увеличить количество высказываний в правиле и его длину.

	if ( BeforeAtt < 0 ) return ( Error = LKE_SUCCESS );
	if ( BeforeAtt > numAtt ) BeforeAtt = numAtt;
	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Есть ли в этом правиле уже такое высказывание
	if ( GetPropHead ( lpData, Att ) ) return ( Error = LKE_NOTUNIQUE );

	LPHEAD_PROP pProp = GetPropHead ( lpData, BeforeAtt );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	// Если необходимо, перевыделить память данных, чтобы вместилось новое высказывание
	if ( MemorySize ( (LPVOID) lpData ) < lpData->lenRul + Len ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpData, lpData->lenRul + Len );
		if ( ! New ) return ( Error = LKE_ALLOCMEM );
		lpData = New;
		}

	// Раздвинуть данные начиная с BeforeAtt (pProp) высказывания
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpData, lpData->lenRul, (LPVOID) pProp, Len ) )
		return  ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок данных
	lpData->numProp++;
	lpData->lenRul += Len;

	// Инициализировать высказывание
	pProp->curAtt = Att;               // Это высказывание об Att-ом атрибуте
	pProp->Style = 0;
	if ( lpData->Style & RUL_DISJUNCT ) pProp->Style |= PROP_CONJUNCT;

	// Инициализировать компоненты высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	LPCOMP pComp = (LPCOMP) (pProp + 1);         // Указать на первую компоненту
	for ( int j = 0; j < m; j++, pComp++ ) {
		pComp->nec = 0;
		if ( pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
		else                                pComp->pos = 0;
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция вставляет в указанное правило высказывание об      │ InsPropInConc │
│ атрибуте Att перед атрибутом BeforeAtt, считая, что это    └───────────────┤
│ правило единственное. Номера атрибутов абсолютные.                         │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsPropInConc ( int Att, int BeforeAtt ) {
	// Найти указатель на высказывание об атрибуте BeforeAtt. Если он не найден, то
	// закончить. Далее необходимо проверить существует ли уже в правиле
	// атрибут Att. Если да, то вернуть ошибку, иначе продолжить. Далее необходимо
	// раздвинуть это место, т.е., отодвинуть подальше хвост правила. При этом
	// необходимо увеличить количество высказываний в правиле и его длину.

	if ( BeforeAtt < 0 ) return ( Error = LKE_SUCCESS );
	if ( BeforeAtt > numAtt ) BeforeAtt = numAtt;
	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Есть ли в этом правиле уже такое высказывание
	if ( GetPropHead ( lpConc, Att ) ) return ( Error = LKE_NOTUNIQUE );

	LPHEAD_PROP pProp = GetPropHead ( lpConc, BeforeAtt );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	// Если необходимо, перевыделить память заключений, чтобы вместилось новое высказывание
	if ( MemorySize ( (LPVOID) lpConc ) < lpConc->lenRul + Len ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpConc, lpConc->lenRul + Len );
		if ( ! New ) return ( Error = LKE_ALLOCMEM );
		lpConc = New;
		}

	// Раздвинуть данные начиная с BeforeAtt (pProp) высказывания
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpConc, lpConc->lenRul, (LPVOID) pProp, Len ) )
		return  ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок данных
	lpConc->numProp++;
	lpConc->lenRul += Len;

	// Инициализировать высказывание
	pProp->curAtt = Att;               // Это высказывание об Att-ом атрибуте
	pProp->Style = 0;
	if ( lpConc->Style & RUL_DISJUNCT ) pProp->Style |= PROP_CONJUNCT;

	// Инициализировать компоненты высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	LPCOMP pComp = (LPCOMP) (pProp + 1);         // Указать на первую компоненту
	for ( int j = 0; j < m; j++, pComp++ ) {
		pComp->nec = 0;
		if ( pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
		else                                pComp->pos = 0;
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция вставляет в указанное правило высказывание об      │ InsPropInRule │
│ атрибуте Att перед атрибутом BeforeAtt считая, что это     └───────────────┤
│ правило находится в базе знаний. Номера атрибутов абсолютные.              │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InsPropInRule ( LPHEAD_RUL pRul, int Att, int BeforeAtt ) {
	// Найти указатель на высказывание об атрибуте BeforeAtt. Если он не найден, то
	// закончить. Далее необходимо проверить существует ли уже в правиле
	// атрибут Att. Если да, то вернуть ошибку, иначе продолжить. Далее необходимо
	// раздвинуть это место, т.е., отодвинуть подальше хвост базы. При этом
	// необходимо увеличить количество высказываний в правиле и его длину,
	// а также общую длину базы.

	if ( BeforeAtt < 0 ) return ( Error = LKE_SUCCESS );
	if ( BeforeAtt > numAtt ) BeforeAtt = numAtt;
	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Есть ли в этом правиле уже такое высказывание
	if ( GetPropHead ( pRul, Att ) ) return ( Error = LKE_NOTUNIQUE );

	LPHEAD_PROP pProp = GetPropHead ( pRul, BeforeAtt );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	// Если необходимо, перевыделить память базы, чтобы вместилось новое высказывание
	if ( MemorySize ( (LPVOID) lpRul ) < lenRul + Len ) {
		LPHEAD_RUL New = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpRul, lenRul + Len );
		if ( ! New ) return ( Error = LKE_ALLOCMEM );
		lpRul = New; // Если селектор меняется при перевыделении памяти, то указатели
                     // pProp и pRul становятся неверными!!!???
		}

	// Раздвинуть правила начиная с BeforeAtt (pProp) высказывания
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pProp, Len ) )
		return  ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок базы знаний
	lenRul += Len;

	// Подстроить заголовок правила
	pRul->numProp++;
	pRul->lenRul += Len;

	// Инициализировать высказывание
	pProp->curAtt = Att;               // Это высказывание об Att-ом атрибуте
	pProp->Style = 0;
	if ( pRul->Style & RUL_DISJUNCT ) pProp->Style |= PROP_CONJUNCT;

	// Инициализировать компоненты высказывания
	//   Rule  Proposition  Comp (не влияет на результат)
	//   CON   CON          1
	//   CON   DIS          0
	//   DIS   CON          1
	//   DIS   DIS          0
	LPCOMP pComp = (LPCOMP) (pProp + 1);         // Указать на первую компоненту
	for ( int j = 0; j < m; j++, pComp++ ) {
		pComp->nec = 0;
		if ( pProp->Style & PROP_CONJUNCT ) pComp->pos = UNITY;
		else                                pComp->pos = 0;
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция удаляет из правила высказывание об атрибуте Att,   │ DelPropInData │
│ считая, что это правило единственно. Номер атрибута        └───────────────┤
│ абсолютный.                                                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelPropInData ( int Att ) {
	// Найти указатель на высказывание об атрибуте Att. Если он не найден, то
	// закончить. Если такой атрибут найден, то необходимо его вычеркнуть и
	// схлопнуть это место, т.е., сдвинуть хвост правила. При этом
	// необходимо уменьшить количество высказываний в правиле и его длину.

	LPHEAD_PROP pProp = GetPropHead ( lpData, Att );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Сдвинуть правила начиная с текущего высказывания и тем самым удалить высказывание
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpData, lpData->lenRul, (LPVOID) pProp, - Len ) )
		return ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок правила
	lpData->numProp--;
	lpData->lenRul -= Len;

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция удаляет из правила высказывание об атрибуте Att,   │ DelPropInConc │
│ считая, что это правило единственно. Номер атрибута        └───────────────┤
│ абсолютный.                                                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelPropInConc ( int Att ) {
	// Найти указатель на высказывание об атрибуте Att. Если он не найден, то
	// закончить. Если такой атрибут найден, то необходимо его вычеркнуть и
	// схлопнуть это место, т.е., сдвинуть хвост правила. При этом
	// необходимо уменьшить количество высказываний в правиле и его длину.

	LPHEAD_PROP pProp = GetPropHead ( lpConc, Att );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Сдвинуть правила начиная с текущего высказывания и тем самым удалить высказывание
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpConc, lpConc->lenRul, (LPVOID) pProp, - Len ) )
		return ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок правила
	lpConc->numProp--;
	lpConc->lenRul -= Len;

	return ( Error = LKE_SUCCESS );
	}

/*
┌────────────────────────────────────────────────────────────┬───────────────┐
│ Функция удаляет из правила высказывание об атрибуте Att,   │ DelPropInRule │
│ считая, что это правило находится в базе знаний. Номер     └───────────────┤
│ атрибута абсолютный.                                                       │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelPropInRule ( LPHEAD_RUL pRul, int Att ) {
	// Найти указатель на высказывание об атрибуте Att. Если он не найден, то
	// закончить. Если такой атрибут найден, то необходимо его вычеркнуть и
	// схлопнуть это место, т.е., сдвинуть хвост базы знаний. При этом
	// необходимо уменьшить количество высказываний и длину правила, а также
	// длину всех правил.

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( pProp == 0 ) return ( Error = LKE_NOCURRENTPROP );

	int m = lpAttIdx [ Att ].AttRec.numVal;
	int Len = sizeof(HEAD_PROP) + sizeof(COMP) * m;

	// Сдвинуть правила начиная с текущего высказывания и тем самым удалить высказывание
	if ( LKE_SUCCESS != InsDelMem ( (LPVOID) lpRul, lenRul, (LPVOID) pProp, - Len ) )
		return ( Error = LKE_ALLOCMEM );

	// Подстроить заголовок правила
	pRul->numProp--;
	pRul->lenRul -= Len;

	lenRul -= Len;

	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Функция удаляет из всех правил в базе знаний высказывание │ DelPropInKBase │
│ об атрибуте с абсолютным номером Att.                     └────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DelPropInKBase ( int Att ) {
	// Выполнить цикл по правилам и из каждого вычеркнуть указанный атрибут

	LPHEAD_RUL pRul = lpRul;
	for ( int r = 0; r < numRul; r++ ) {
		DelPropInRule ( pRul, Att );
		pRul = (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
		}
	return ( Error = LKE_SUCCESS );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Инвертировать высказывание                                    │ InvertProp │
│ Корректность указателя не проверяется. Атрибут абсолютный.    └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InvertProp ( LPHEAD_RUL pRul, int Att ) {
	// Найти в указанном правиле атрибут Att. Если его там нет, то вернуть
	// ошибку. Инвертировать компоненты этого высказывания.

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	int m = lpAttIdx [ Att ].AttRec.numVal;

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( ! pProp ) return ( Error = LKE_NOCURRENTPROP );

	if ( ! (pProp->Style & PROP_CONJUNCT) ) pProp->Style |= PROP_CONJUNCT;
	else                                    pProp->Style &= ~PROP_CONJUNCT;

	// Инвертировать компоненты высказывания
	LPCOMP pComp = (LPCOMP) (pProp + 1);         // Указать на первую компоненту
	for ( int j = 0; j < m; j++, pComp++ ) {
		pComp->pos = UNITY - pComp->pos;
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Изменить компоненты высказывания на противоположные.         │ InvertComps │
│ Корректность указателя не проверяется. Атрибут абсолютный.   └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::InvertComps ( LPHEAD_RUL pRul, int Att ) {
	// Найти в указанном правиле атрибут Att. Если его там нет, то вернуть
	// ошибку. Инвертировать компоненты этого высказывания.

	if ( Att < 0 || Att >= numAtt ) return ( Error = LKE_SUCCESS );

	int m = lpAttIdx [ Att ].AttRec.numVal;

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( ! pProp ) return ( Error = LKE_NOCURRENTPROP );

	// Инвертировать компоненты высказывания
	LPCOMP pComp = (LPCOMP) (pProp + 1);         // Указать на первую компоненту
	for ( int j = 0; j < m; j++, pComp++ ) {
		pComp->pos = UNITY - pComp->pos;
		}

	return ( Error = LKE_SUCCESS );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Получить указатель на компоненту по указателю на высказывание и  │ GetComp │
│ абсолютному номеру значения. Корректность указателя на высказы-  └─────────┤
│ вание не проверяется.                                                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
LPCOMP KBFILE::GetComp ( LPHEAD_PROP pProp, int Comp ) {

	if ( Comp < 0 || Comp >= lpAttIdx [ pProp->curAtt ].AttRec.numVal )
		{ Error = LKE_GENERAL; return 0; }

	return (LPCOMP) (pProp + 1) + Comp;
	}

LPCOMP KBFILE::GetComp ( LPHEAD_RUL pRul, int Att, int Comp ) {

	if ( Att < 0 || Att >= numAtt ) { Error = LKE_GENERAL; return 0; }

	LPHEAD_PROP pProp = GetPropHead ( pRul, Att );
	if ( ! pProp ) { Error = LKE_NOCURRENTPROP; return 0; }

	if ( Comp < 0 || Comp >= lpAttIdx [ pProp->curAtt ].AttRec.numVal )
		{ Error = LKE_GENERAL; return 0; }

	return (LPCOMP) (pProp + 1) + Comp;
	}







