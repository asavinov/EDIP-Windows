/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Провести логический вывод на данных и ДНФ и получить           │ Inference │
│ заключение.                                                    └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int Inference ( KBASE* tis ) {
	// Если нет ДНФ либо БЗ изменена, то выдать сообщение и предложить компиляцию
	if ( ! tis->lpHost->isDNF ) return LKE_NOTCOMPILED;

	// Выделить необходимую память
	int i, j, k, m;
	COMP* pDataComp;
	COMP* pConComp;
	HEAD_PROP* pDataProp;
	HEAD_PROP* pConProp;
	long sz = sizeof(HEAD_RUL) + tis->lpHost->numAtt * sizeof(HEAD_PROP) + tis->lpHost->numAllVal * sizeof(COMP);
	HEAD_RUL* pRul;
	HEAD_RUL* pData = (HEAD_RUL*) farmalloc ( sz );
	HEAD_RUL* pCon = (HEAD_RUL*) farmalloc ( sz );
	tis->lpConc = (HEAD_RUL*) farrealloc ( (void*) tis->lpConc, sz );
	if ( ! pData || ! pCon || ! tis->lpConc ) return LKE_ALLOCMEM;

	// Скопировать данные в буфер и привести их к максимальному виду
	_fmemcpy ( (void*) pData, (void*) tis->lpData, tis->lpData->lenRul );
	ReduceConMax ( tis->lpHost, pData );

	// Сбросить заключение
	tis->lpConc->lenRul = sizeof(HEAD_RUL);
	tis->lpConc->numProp = 0;
	tis->lpConc->Const.pos = 0;                         // Начинаем с нулевого
	tis->lpConc->Const.nec = 0;
	tis->lpConc->Style = RUL_DISJUNCT;


	// Цикл по конъюнктам из ДНФ
	pRul = tis->lpHost->lpDNF;
	for ( i = 0; i < tis->lpHost->numCon; i++ ) {

		// Скопировать очередной конъюнкт в буфер и привести его к максимальной форме
		_fmemcpy ( (void*) pCon, (void*) pRul, pRul->lenRul );

		ReduceConMaxTo ( tis->lpHost, pCon );    // Константа уже установлена!


		// Наложить на очередной конъюнкт в буфере данные с помощью покомпонентного минимума
		// Цикл по всем высказываниям данных
		pDataProp = (HEAD_PROP*) (pData + 1);
		for ( j = 0; j < pData->numProp; j++ ) {

			// Пройтись по всем высказываниям конъюнкта
			pConProp = (HEAD_PROP*) (pCon + 1);
			for ( k = 0; k < pCon->numProp; k++ ) {
				m = pConProp->curAtt;
				if ( m == pDataProp->curAtt ) break;
				pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == pCon->numProp ) {
				// Цикл закончился, но нужное высказывание найдено не было
				// Необходимо добавить его в самый конец конъюнкта и инициализировать
				pConProp->curAtt = pDataProp->curAtt;
				pConProp->Style = PROP_CONJUNCT;

				pConComp = (COMP*) (pConProp + 1);
				m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pConComp->nec = 0; pConComp->pos = UNITY;
					pConComp = pConComp + 1;         // К следующей компоненте
					}
				// Увеличить длину конъюнкта на длину высказывания
				pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				pCon->numProp++;
				}
			// Теперь pConProp указывает на соответствующее высказывание в конъюнкте

			// Вычислить покомпонентный минимум высказываний pConProp и pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pConComp->pos = pDataComp->pos;
				// Перейти к следующей компоненте
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal );
			}                     // Наложили последнее высказывание из данных


		// Привести текущий конъюнкт к максимальной форме с нахождением константы
		ReduceConMax ( tis->lpHost, pCon );
		ExtendCon ( tis->lpHost, pCon );        // Вставить пропущенные секции

		// Наложить полученный конъюнкт на конъюнкт выводов с помощью покомпонентного максимума
		// Цикл по всем высказываниям конъюнкта (pDataProp - заключение)
		pConProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {

			// Пройтись по всем высказываниям заключения
			pDataProp = (HEAD_PROP*) (tis->lpConc + 1);
			for ( k = 0; k < tis->lpConc->numProp; k++ ) {
				m = pDataProp->curAtt;
				if ( m == pConProp->curAtt ) break;
				pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == tis->lpConc->numProp ) {
				// Цикл закончился, но нужное высказывание найдено не было
				// Необходимо добавить его в самый конец заключения и инициализировать
				pDataProp->curAtt = pConProp->curAtt;
				pDataProp->Style = PROP_CONJUNCT;

				pDataComp = (COMP*) (pDataProp + 1);
				m = tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pDataComp->nec = 0; pDataComp->pos = 0;
					pDataComp = pDataComp + 1;         // К следующей компоненте
					}
				// Увеличить длину конъюнкта на длину высказывания
				tis->lpConc->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				tis->lpConc->numProp++;
				}
			// Теперь pDataProp указывает на соответствующее высказывание в заключении

			// Вычислить покомпонентный максимум высказываний pConProp и pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pDataComp->pos = pConComp->pos;
				// Перейти к следующей компоненте
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
			}                 // Наложили последнее высказывание на заключение


		pRul = (HEAD_RUL*) ( (char*) pRul + pRul->lenRul );
		}

	// Привести конъюнкт выводов к максимальной форме (с нахождением константы)
	ReduceConMax ( tis->lpHost, tis->lpConc );

	return TRUE;
	}

