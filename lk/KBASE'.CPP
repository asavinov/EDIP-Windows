/*
����������������������������������������������������������������������������Ŀ
� �஢��� �����᪨� �뢮� �� ������ � ��� � �������           � Inference �
� �����祭��.                                                    �����������Ĵ
������������������������������������������������������������������������������
*/
int Inference ( KBASE* tis ) {
	// �᫨ ��� ��� ���� �� ��������, � �뤠�� ᮮ�饭�� � �।������ ���������
	if ( ! tis->lpHost->isDNF ) return LKE_NOTCOMPILED;

	// �뤥���� ����室���� ������
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

	// �����஢��� ����� � ���� � �ਢ��� �� � ���ᨬ��쭮�� ����
	_fmemcpy ( (void*) pData, (void*) tis->lpData, tis->lpData->lenRul );
	ReduceConMax ( tis->lpHost, pData );

	// ������ �����祭��
	tis->lpConc->lenRul = sizeof(HEAD_RUL);
	tis->lpConc->numProp = 0;
	tis->lpConc->Const.pos = 0;                         // ��稭��� � �㫥����
	tis->lpConc->Const.nec = 0;
	tis->lpConc->Style = RUL_DISJUNCT;


	// ���� �� ����⠬ �� ���
	pRul = tis->lpHost->lpDNF;
	for ( i = 0; i < tis->lpHost->numCon; i++ ) {

		// �����஢��� ��।��� ����� � ���� � �ਢ��� ��� � ���ᨬ��쭮� �ଥ
		_fmemcpy ( (void*) pCon, (void*) pRul, pRul->lenRul );

		ReduceConMaxTo ( tis->lpHost, pCon );    // ����⠭� 㦥 ��⠭������!


		// �������� �� ��।��� ����� � ���� ����� � ������� ����������⭮�� �����㬠
		// ���� �� �ᥬ ��᪠�뢠��� ������
		pDataProp = (HEAD_PROP*) (pData + 1);
		for ( j = 0; j < pData->numProp; j++ ) {

			// �ன��� �� �ᥬ ��᪠�뢠��� �����
			pConProp = (HEAD_PROP*) (pCon + 1);
			for ( k = 0; k < pCon->numProp; k++ ) {
				m = pConProp->curAtt;
				if ( m == pDataProp->curAtt ) break;
				pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == pCon->numProp ) {
				// ���� �����稫��, �� �㦭�� ��᪠�뢠��� ������� �� �뫮
				// ����室��� �������� ��� � ᠬ� ����� ����� � ���樠����஢���
				pConProp->curAtt = pDataProp->curAtt;
				pConProp->Style = PROP_CONJUNCT;

				pConComp = (COMP*) (pConProp + 1);
				m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pConComp->nec = 0; pConComp->pos = UNITY;
					pConComp = pConComp + 1;         // � ᫥���饩 ���������
					}
				// �������� ����� ����� �� ����� ��᪠�뢠���
				pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				pCon->numProp++;
				}
			// ������ pConProp 㪠�뢠�� �� ᮮ⢥�����饥 ��᪠�뢠��� � �����

			// ���᫨�� ������������ ������ ��᪠�뢠��� pConProp � pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pConComp->pos = pDataComp->pos;
				// ��३� � ᫥���饩 ���������
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal );
			}                     // �������� ��᫥���� ��᪠�뢠��� �� ������


		// �ਢ��� ⥪�騩 ����� � ���ᨬ��쭮� �ଥ � ��宦������ ����⠭��
		ReduceConMax ( tis->lpHost, pCon );
		ExtendCon ( tis->lpHost, pCon );        // ��⠢��� �ய�饭�� ᥪ樨

		// �������� ����祭�� ����� �� ����� �뢮��� � ������� ����������⭮�� ���ᨬ㬠
		// ���� �� �ᥬ ��᪠�뢠��� ����� (pDataProp - �����祭��)
		pConProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {

			// �ன��� �� �ᥬ ��᪠�뢠��� �����祭��
			pDataProp = (HEAD_PROP*) (tis->lpConc + 1);
			for ( k = 0; k < tis->lpConc->numProp; k++ ) {
				m = pDataProp->curAtt;
				if ( m == pConProp->curAtt ) break;
				pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == tis->lpConc->numProp ) {
				// ���� �����稫��, �� �㦭�� ��᪠�뢠��� ������� �� �뫮
				// ����室��� �������� ��� � ᠬ� ����� �����祭�� � ���樠����஢���
				pDataProp->curAtt = pConProp->curAtt;
				pDataProp->Style = PROP_CONJUNCT;

				pDataComp = (COMP*) (pDataProp + 1);
				m = tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pDataComp->nec = 0; pDataComp->pos = 0;
					pDataComp = pDataComp + 1;         // � ᫥���饩 ���������
					}
				// �������� ����� ����� �� ����� ��᪠�뢠���
				tis->lpConc->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				tis->lpConc->numProp++;
				}
			// ������ pDataProp 㪠�뢠�� �� ᮮ⢥�����饥 ��᪠�뢠��� � �����祭��

			// ���᫨�� ������������ ���ᨬ� ��᪠�뢠��� pConProp � pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pDataComp->pos = pConComp->pos;
				// ��३� � ᫥���饩 ���������
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
			}                 // �������� ��᫥���� ��᪠�뢠��� �� �����祭��


		pRul = (HEAD_RUL*) ( (char*) pRul + pRul->lenRul );
		}

	// �ਢ��� ����� �뢮��� � ���ᨬ��쭮� �ଥ (� ��宦������ ����⠭��)
	ReduceConMax ( tis->lpHost, tis->lpConc );

	return TRUE;
	}
