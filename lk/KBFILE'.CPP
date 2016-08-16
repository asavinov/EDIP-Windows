
/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│                                                             │ CompileToDNF │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL CompileToDNF ( KBFILE* tis, COMPILEOUT lpProcOut ) {
	int i, j, n;
	long sz;
	HEAD_RUL *pDis, *pCon;
	BOOL isBreak;

	ResetDNF ( tis );

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_GENERATION, 0, 0, 0, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}

	// Нулевой цикл (обрабатывается нулевое правило)
	if ( tis->lpRul->Style & RUL_DISJUNCT ) { Con0IntoCon ( tis ); }    // (+)
	else                                    { Dis0IntoCon ( tis ); }    // (-)

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_GENERATION, 0, 0, 0, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}

	pCon = tis -> lpDNF;             // Начать поглощение с нулевого конъюнкта
	for ( j = 0; j < tis->numCon; ) {

		if ( lpProcOut ) {
			isBreak = (*lpProcOut) ( STAT_ABSORPTION, 0, j, tis->numCon, tis->numCon );
			if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
			}

		if ( ! AbsorbFirst ( tis, pCon, j ) ) {
			pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul ); j++;
			}

		}

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_ABSORPTION, 0, j, tis->numCon, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}



	// Основной процесс (начинается с первого правила)
	pDis = (HEAD_RUL*) ( (char*) tis->lpRul + tis->lpRul->lenRul );
	for ( i = 1; i < tis->numRul; i++ ) {                // Цикл по дизъюнктам

		if ( lpProcOut ) {
			isBreak = (*lpProcOut) ( STAT_GENERATION, i, 0, tis->numCon, tis->numCon );
			if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
			}

		if ( pDis->Style & RUL_DISJUNCT ) {                    // (+) конъюнкт
			ConIntoCon ( tis, pCon ); // ??? pDis ???
			}
		else {                                                 // (-) дизъюнкт

			// Инициализация для цикла по конъюнктам
			sz = tis->lenDNF;                      // Понадобится при вычеркивании
			n = tis -> numCon;                   // Исходное количество конъюнктов
			pCon = tis -> lpDNF;                    // Начать с нулевого конъюнкта
			for ( j = 0; j < n; j++ ) {

				if ( lpProcOut ) {
					isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
					if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
					}

				DisIntoCon ( tis, pDis, pCon, n, sz );

				// Перейти к следующему конъюнкту
				pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );

				}
			// После этого цикла дизъюнкт включен во все конъюнкты, в результате
			// размер ДНФ (существенно) вырос, т.е., n << tis->numCon.

			if ( lpProcOut ) {
				isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
				if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
				}

			// Теперь необходимо исходные n конъюнктов удалить из ДНФ
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF, (void*) tis->lpDNF, - sz );
			tis->numCon -= n;
			tis->lenDNF -= sz;
			}


		// Здесь хорошо бы поглотить конъюнкты, которые следуют из ДНФ,
		// т.е., максимально упростить ДНФ, однако, мы пока умеем поглощать
		// только те, которые следуют из других.
		// Далее можно будет попробовать поглощать те конъюнкты, которые
		// следуют из пар (троек и т.д.) других конъюнктов.

		pCon = tis -> lpDNF;         // Начать поглощение с нулевого конъюнкта
		for ( j = 0; j < tis->numCon; ) {

			if ( lpProcOut ) {
				isBreak = (*lpProcOut) ( STAT_ABSORPTION, i, j, tis->numCon, tis->numCon );
				if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
				}

			if ( ! AbsorbFirst ( tis, pCon, j ) ) {
				pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul ); j++;
				}

			}

		if ( lpProcOut ) {
			isBreak = (*lpProcOut) ( STAT_ABSORPTION, i, j, tis->numCon, tis->numCon );
			if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
			}

		// Перейти к следующему правилу
		pDis = (HEAD_RUL*) ( (char*) pDis + pDis->lenRul );
		}

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_ABSORPTION, tis->numRul, tis->numCon, tis->numCon, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}

	tis->isDNF = TRUE;
	tis->isChanged = TRUE;
	return TRUE;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Нулевой дизъюнкт включается в пустую ДНФ.                    │ Dis0IntoCon │
│                                                              └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int Dis0IntoCon ( KBFILE* tis ) {
	// Проверить хватит ли места в ДНФ для дополнительных numProp (в 0-ом дизъюнкте)
	// конъюнктов длины 1 высказывание плюс одна длина pDis
	long sz = ( tis->lpRul->numProp * sizeof(HEAD_RUL) ) + tis->lpRul->lenRul;
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, sz + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;

		}
	// Теперь память есть


	HEAD_RUL  *pEndCon  = tis->lpDNF;
	HEAD_PROP *pConProp = (HEAD_PROP*) (pEndCon + 1);
	HEAD_PROP *pDisProp = (HEAD_PROP*) (tis->lpRul + 1);
	int i, m;

	// Цикл по высказываниям дизъюнкта
	for ( i = 0; i < tis->lpRul->numProp; i++ ) {

		m = tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;

		// Инициализировать заголовок нового конъюнкта pEndCon
		pEndCon->numProp = 1;
		pEndCon->lenRul = sizeof(HEAD_RUL) + sizeof(HEAD_PROP) + m * sizeof(COMP);
		pEndCon->Const.nec = 0; pEndCon->Const.pos = UNITY;
		pEndCon->Style = RUL_DISJUNCT;

		// Скопировать после заголовка текущее высказывание дизъюнкта
		_fmemcpy ( (void*) pConProp, (void*) pDisProp, sizeof(HEAD_PROP) + m * sizeof(COMP) );
		if ( pDisProp->Style & PROP_CONJUNCT ) {           // Если это посылка
			pConProp->Style = 0;
			}
		else {
			pConProp->Style = PROP_CONJUNCT;
			}

		// Привести новый конъюнкт (с инвертированием)
		ReduceCon ( tis, pEndCon );
		if ( pEndCon->Const.pos != 0 ) {          // Если конъюнкт нетривиален
			// Включить построенный конъюнкт в ДНФ, т.е., изменить pEndCon,
			// количество конъюнктов и их длину
			tis->numCon++;
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			pConProp = (HEAD_PROP*) (pEndCon + 1);
			}

		// Перейти к следующему высказыванию в дизъюнкте
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}

	return LKE_SUCCESS;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Нулевое правило, являющееся конъюнктом (положительным        │ Con0IntoCon │
│ утверждением), включается в ДНФ.                             └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int Con0IntoCon ( KBFILE* tis ) {
	// Проверить хватит ли места в ДНФ для одного конъюнкта
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < tis->lpRul->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, tis->lpRul->lenRul + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// Теперь память есть

	// Скопировать правило в ДНФ и все
	_fmemcpy ( (void*) tis->lpDNF, (void*) tis->lpRul, tis->lpRul->lenRul );

	// Привести новый конъюнкт
	ReduceCon ( tis, tis->lpDNF );
	// Мы не проверяем значение константы, т.к., необходимо иметь любой конъюнкт

	// Включить конъюнкт в ДНФ
	tis->numCon++;
	tis->lenDNF += tis->lpRul->lenRul;

	return LKE_SUCCESS;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Дизъюнкт pDis включается в конъюнкт pCon. Получившиеся        │ DisIntoCon │
│ конъюнкты добавляются в конец ДНФ. Увеличивается общее        └────────────┤
│ количество конъюнктов и длина ДНФ.                                         │
└────────────────────────────────────────────────────────────────────────────┘
*/
int DisIntoCon ( KBFILE* tis, HEAD_RUL *pDis, HEAD_RUL *pCon, int numStart, long ofsStart ) {

	// Проверить хватит ли места в ДНФ для дополнительных numProp (в дизъюнкте)
	// конъюнктов длины pCon плюс одна длина pDis
	long sz = (pDis->numProp * pCon->lenRul) + pDis->lenRul;
	if ( tis->lenDNF + sz > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + sz + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// Теперь память есть

	BOOL      isZero;
	HEAD_PROP *pDisProp = (HEAD_PROP*) (pDis + 1);  // Указать на 0-ое высказ.
	HEAD_PROP *pConProp;
	COMP      *pDisComp;
	COMP      *pConComp;
	HEAD_RUL  *pEndCon  = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	int i, j, m;

	for ( i = 0; i < pDis->numProp; i++ ) {

		// Скопировать конъюнкт pCon в конец ДНФ, т.е. сделать заготовку
		_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

		// Узнать номер i-го высказывания в дизъюнкте и найти высказывание
		// с таким же номером в конъюнкте. Если такого высказывания
		// в конъюнкте нет (т.е., оно единичное), то необходимо
		// добавить его в конец конъюнкта

		// Пройтись по всем высказываниям конъюнкта
		pConProp = (HEAD_PROP*) (pEndCon + 1);
		for ( j = 0; j < pEndCon->numProp; j++ ) {
			m = pConProp->curAtt;
			if ( m == pDisProp->curAtt ) break;
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ m ].AttRec.numVal );
			}
		if ( j == pEndCon->numProp ) {
			// Цикл закончился, но нужное высказывание найдено не было
			// Необходимо добавить его в самый конец конъюнкта и инициализировать
			pConProp->curAtt = pDisProp->curAtt;
			pConProp->Style = PROP_CONJUNCT;

			pConComp = (COMP*) (pConProp + 1);
			m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pConComp->nec = 0; pConComp->pos = UNITY;
				pConComp = pConComp + 1;     // Перейти к следующей компоненте
				}
			// Увеличить длину конъюнкта на длину высказывания
			pEndCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pEndCon->numProp++;
			}
		// Теперь pConProp указывает на соответствующее высказывание в конъюнкте


		// Вычислить покомпонентный минимум высказываний pConProp и pDisProp

		// Мы считаем, что исходный конъюнкт (до наложения на него секции)
		// приведен, т.к., он уже включен в ДНФ. Однако, в принципе,
		// полученный конъюнкт может оказаться неприведенным, поскольку
		// когда накладывается секция, то он ослабляется. Но это может произойти
		// только из-за этой секции. Поэтому при наложении можно одновременно
		// делать приведение, вернее, делать необходимые коррекции. Мы можем быть
		// уверены, что все компоненты исходного дизъюнкта, которые больше
		// либо равны константы, равны 1. При наложении можт быть два варианта:
		// 1. Компонента уменьшилась, но не стала меньше константы; в этом случае
		//    ее необходимо оставить равной 1 (т.е., не менять); кроме того,
		//    если такая компонента существует, то общая константа уже не
		//    изменится (не уменьшится), поэтому необходимо установить соответствующий
		//    флаг (а можно обойтись и без флага, а просто хранить где-то
		//    максимальное значение секции, и если в конце оно окажется
		//    больше либо равным константе, то значит константа не меняется).
		// 2. Компонента опустилась ниже константы; в этом случае ее необходимо
		//    приравнять этому значению и учесть при нахождении максимума (константы)
		//    секции.
		// Всю процедуру можно организовать так. Во время наложения мы всегда
		// считаем, что константа конъюнкта не меняется и равна ее исходному
		// значению. В результате получится секция, приведенная именно по
		// этой константе (т.е., все, что выше константы приравнивается 1).
		// Одновременно при наложении вычисляется константа секции (т.е., ее
		// максимум). В конце константа секции сравнивается с константой
		// конъюнкта. Если первая больше либо равна второй, то все нормально,
		// мы не перешли допустимую (не уменьшили общую константу) границу и
		// конъюнкт находится в приведенной форме. Если же константа секции
		// меньше константы конъюнкта, то общая константа приравнивается
		// новому значению, после чего необходимо пройтись по всем секциям
		// (в т.ч. и по текущей) и привести их к новой константе. В результате
		// некоторые компоненты, которые из-за понижения константы оказались
		// больше либо равными ей, будут приравнены к 1. Возможно, появятся
		// единичные секции--их необходимо вычеркнуть. Процедуру приведения
		// всего конъюнкта к новой константе (с возможным удалением секций)
		// можно вынести. Для сокращения вычислений, если константа становится
		// равной 0, т.е., конъюнкт ничего не значит и поглощается любым другим,
		// то мы не приводим его, т.к., далее проводится специальная проверка
		// этого случая.

		pConComp = (COMP*) (pConProp + 1);
		pDisComp = (COMP*) (pDisProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		COMP NewConst; NewConst.nec = 0; NewConst.pos = 0;
		BOOL Invert = pDisProp->Style & PROP_CONJUNCT;

		for ( j = 0; j < m; j++ ) {
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			if ( pConComp->pos > pDisComp->pos && pDisComp->pos < pEndCon->Const.pos ) {
				// Первое условие показывает необходимость уменьшения, однако,
				// только если выполняется второе условие, т.е., это уменьшение
				// ниже уровня константы
				pConComp->pos = pDisComp->pos;
				}
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			// Вычислить новый максимум (новую константу)
			if ( pConComp->pos > NewConst.pos ) NewConst.pos = pConComp->pos;
			// Перейти к следующей компоненте
			pConComp = pConComp + 1; pDisComp = pDisComp + 1;
			}
		if ( NewConst.pos == 0 ) pEndCon->Const.pos = 0;
		else if ( NewConst.pos < pEndCon->Const.pos ) {
			pEndCon->Const.pos = NewConst.pos;
			// Привести конъюнкт к новой константе NewConst.pos
			ReduceConTo ( tis, pEndCon );
			}
		// Теперь последний конъюнкт приведен (кроме случая NewConst.pos == 0)


		if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, numStart, ofsStart ) ) {
			// Включить построенный конъюнкт в ДНФ, т.е., изменить pEndCon,
			// количество конъюнктов и их длину
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			tis->numCon++;
			}

		// Перейти к следующему высказыванию
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal );
		}

	return LKE_SUCCESS;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Положительное утверждение pCon включается в ДНФ путем простого│ ConIntoCon │
│ добавления его в конец ДНФ.                                   └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int ConIntoCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	// Проверить хватит ли места в ДНФ для одного конъюнкта
	if ( tis->lenDNF + pCon->lenRul > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < pCon->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + pCon->lenRul + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// Теперь память есть

	// Скопировать правило в ДНФ и все
	HEAD_RUL* pEndCon = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

	// Привести новый конъюнкт
	ReduceCon ( tis, pEndCon );
	if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, 0, 0 ) ) {
		// Включить построенный конъюнкт в ДНФ
		tis->numCon++;
		tis->lenDNF += pEndCon->lenRul;
		}

	return LKE_SUCCESS;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Приведение правила. Может измениться количество высказываний    │ReduceCon │
│ и длина правила (уменьшиться), а также устанавливается константа└──────────┤
│ Однако не учитывается, что за правилом могут следовать другие и никаких    │
│ операций с памятью не производится (т.е., предполагается, что правило      │
│ последнее). Если правило тождественно истинно, то оно не вычеркивается,    │
│ а просто из него удаляются все высказывания и устанавливается соответствую-│
│ щая константа. Поэтому после приведения необходимо проверять константу.    │
└────────────────────────────────────────────────────────────────────────────┘
*/
void ReduceCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// Цикл по высказываниям для нахождения константы, т.е., минимального максимума в каждой секции
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// Цикл по компонентам текущего высказываниям
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // Перейти к следующей компоненте
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// Перейти к следующему высказыванию
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// Теперь мы имеем правильную константу

	ReduceConTo ( tis, pCon );

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Приведение конъюнкта к новой константе, которая должна быть  │ ReduceConTo │
│ установлена. Предполагается, что ранее конъюнкт был приведен └─────────────┤
│ к другой (большей) константе. При приведении к новой константе те          │
│ компоненты, которые больше либо равны ей приравниваются 1. В результате    │
│ могут получиться тождественно истинные секции, которые вычеркиваются, что  │
│ приводит к уменьшению количества высказываний и длины конъюнкта.           │
└────────────────────────────────────────────────────────────────────────────┘
*/
void ReduceConTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// Цикл по высказываниям
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// Цикл по компонентам текущего высказываниям
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// Если текущая компонента больше либо равна константе, то приравнять ее 1
			// иначе отметить, что секция нетривиальна
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = UNITY; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // Перейти к следующей компоненте
			}

		if ( isTrivSec ) {
			// Удалить эту секцию
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			// ??????? lpDNF
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// Перейти к следующему высказыванию
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Привести конъюнкт к максимальной форме с предварительным    │ ReduceConMax │
│ нахождением константы.                                      └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
void ReduceConMax ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// Цикл по высказываниям для нахождения константы, т.е., минимального максимума в каждой секции
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// Цикл по компонентам текущего высказываниям
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // Перейти к следующей компоненте
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// Перейти к следующему высказыванию
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// Теперь мы имеем правильную константу

	ReduceConMaxTo ( tis, pCon );

	return;
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Привести конъюнкт к максимальной форме по его заранее     │ ReduceConMaxTo │
│ установленной константе. Это означает, что все компоненты,└────────────────┤
│ которые, больше константы, приравниваются ей.                              │
└────────────────────────────────────────────────────────────────────────────┘
*/
void ReduceConMaxTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// Цикл по высказываниям
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// Цикл по компонентам текущего высказываниям
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// Если текущая компонента больше константы, то приравнять ее константе
			// иначе отметить, что секция нетривиальна
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = pCon->Const.pos; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // Перейти к следующей компоненте
			}

		if ( isTrivSec ) {
			// Удалить эту секцию
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			InsDelMem ( (void*) pCon, pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// Перейти к следующему высказыванию
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Функция пытается поглотить конъюнкт pCon конъюнктами, которые│ AbsorbFirst │
│ находятся после него. Если конъюнкт поглощается, то он       └─────────────┤
│ вычеркивается, уменьшается количество конъюнктов и их длина.               │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL AbsorbFirst ( KBFILE* tis, HEAD_RUL *pEndCon, int numCon ) {
	int i;
	HEAD_RUL *pCon;

	// Начать с numCon+1-го конъюнкта
	pCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
	for ( i = numCon+1; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) break;

		// Продолжить проверку поглощения для следующих конъюнктов
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}
	if ( i == tis->numCon ) return FALSE;

	// Вычеркнуть конъюнкт
	tis->numCon--;
	tis->lenDNF -= pEndCon->lenRul;
	InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pEndCon->lenRul, (void*) pEndCon, - pEndCon->lenRul );

	return TRUE;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Функция пытается поглотить конъюнкт, который следует за       │ AbsorbLast │
│ последним (т.е., он еще не принадлежит ДНФ).                  └────────────┤
│ В любом случае, независимо от поглощаемости,                               │
│ ничего не меняется, кроме возвращаемого значения. Проверка на поглощение   │
│ начинается с конъюнкта numStart, смещение до которого ofsStart.            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL AbsorbLast ( KBFILE* tis, HEAD_RUL* pEndCon, int numStart, long ofsStart ) {
	int i;
	HEAD_RUL*  pCon;

	// Начать с numStart-го конъюнкта
	pCon = (HEAD_RUL*) ( (char*) tis->lpDNF + ofsStart );
	for ( i = numStart; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) return TRUE;

		// Продолжить проверку поглощения для следующих конъюнктов
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}

	return FALSE;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Функция проверяет выполнение условия pFstCon |= pSecCon, что   │ AbsorbCon │
│ означает поглощение конъюнкта pSecCon (т.е., он м.б. вычеркнут)└───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL AbsorbCon ( KBFILE* tis, HEAD_RUL *pFstCon, HEAD_RUL *pSecCon ) {
	int i, j, k, m;
	HEAD_PROP* pFstConProp;
	HEAD_PROP* pSecConProp;
	COMP*      pFstConComp;
	COMP*      pSecConComp;

	// Делаем самые элементарные проверки
	if ( pSecCon->numProp  <  pFstCon->numProp || pSecCon->Const.pos > pFstCon->Const.pos ) {
		return FALSE;
		}

	// Теперь надо проверять вложенность высказываний--если в pFstCon
	// есть хотя бы одно высказывание, которое не присутствует в
	// pSecCon, то условие не выполняется
	pFstConProp = (HEAD_PROP*) ( pFstCon + 1 );
	for ( j = 0; j < pFstCon->numProp; j++ ) {

		// Проверить существует ли высказывание с номером pFstConProp->curAtt
		// в последнем конъюнкте pSecCon

		pSecConProp = (HEAD_PROP*) ( pSecCon + 1 );
		for ( k = 0; k < pSecCon->numProp; k++ ) {
			if ( pSecConProp->curAtt == pFstConProp->curAtt ) {

				// Нашлось соответствующее высказывание; проверить вложенность

				// Нужно проверить покомпонентность, т.е., любая
				// компонента из pFstCon должна быть не меньше чем pSecCon
				// Если в pFstCon нашлась компонента, которая меньше
				// соответствующей в pSecCon, то условие не выполняется,
				// а если таковой не было найдено, то это
				// означает поглощение.

				pFstConComp = (COMP*) ( pFstConProp + 1 );
				pSecConComp = (COMP*) ( pSecConProp + 1 );
				m = tis->lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal;
				for ( i = 0; i < m; i++ ) {
					if ( pFstConComp->pos < pSecConComp->pos ) return FALSE;
					pFstConComp = pFstConComp + 1; pSecConComp = pSecConComp + 1;
					}
				break;
				}
			pSecConProp = (HEAD_PROP*) ( (char*) pSecConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pSecConProp->curAtt ].AttRec.numVal );
			}
		if ( k == pSecCon->numProp ) { return FALSE; }

		// Перейти к следующему высказыванию
		pFstConProp = (HEAD_PROP*) ( (char*) pFstConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal );
		}

	return TRUE;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Расширить конъюнкт. Отсутствующие в конъюнкте секции           │ ExtendCon │
│ пристыковываются в конец и ее компоненты поднимаются до        └───────────┤
│ константы, которая должна быть установлена.                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
void ExtendCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	int i, j, m;
	HEAD_PROP* pProp;
	COMP*      pComp;

	// Цикл по всем номерам высказываний, т.е., проверяем каждую секцию
	for ( i = 0; i < tis->numAtt; i++ ) {

		// Пройтись по всем высказываниям конъюнкта в поисках i-го
		pProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {
			if ( i == pProp->curAtt ) break;
			pProp = (HEAD_PROP*) ( (char*) pProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal );
			}
		if ( j == pCon->numProp ) {
			// Цикл закончился, но i-ое высказывание найдено не было
			// Необходимо добавить его в самый конец конъюнкта и инициализировать
			pProp->curAtt = i;
			pProp->Style = PROP_CONJUNCT;

			pComp = (COMP*) (pProp + 1);
			m = tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pComp->nec = 0; pComp->pos = pCon->Const.pos;
				pComp = pComp + 1;                   // К следующей компоненте
				}
			// Увеличить длину конъюнкта на длину высказывания
			pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pCon->numProp++;
			}
		}
	}
