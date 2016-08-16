
/*
����������������������������������������������������������������������������Ŀ
�                                                             � CompileToDNF �
�                                                             ��������������Ĵ
������������������������������������������������������������������������������
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

	// �㫥��� 横� (��ࠡ��뢠���� �㫥��� �ࠢ���)
	if ( tis->lpRul->Style & RUL_DISJUNCT ) { Con0IntoCon ( tis ); }    // (+)
	else                                    { Dis0IntoCon ( tis ); }    // (-)

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_GENERATION, 0, 0, 0, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}

	pCon = tis -> lpDNF;             // ����� �����饭�� � �㫥���� �����
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



	// �᭮���� ����� (��稭����� � ��ࢮ�� �ࠢ���)
	pDis = (HEAD_RUL*) ( (char*) tis->lpRul + tis->lpRul->lenRul );
	for ( i = 1; i < tis->numRul; i++ ) {                // ���� �� ����⠬

		if ( lpProcOut ) {
			isBreak = (*lpProcOut) ( STAT_GENERATION, i, 0, tis->numCon, tis->numCon );
			if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
			}

		if ( pDis->Style & RUL_DISJUNCT ) {                    // (+) �����
			ConIntoCon ( tis, pCon ); // ??? pDis ???
			}
		else {                                                 // (-) �����

			// ���樠������ ��� 横�� �� ����⠬
			sz = tis->lenDNF;                      // ����������� �� ���ન�����
			n = tis -> numCon;                   // ��室��� ������⢮ ����⮢
			pCon = tis -> lpDNF;                    // ����� � �㫥���� �����
			for ( j = 0; j < n; j++ ) {

				if ( lpProcOut ) {
					isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
					if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
					}

				DisIntoCon ( tis, pDis, pCon, n, sz );

				// ��३� � ᫥���饬� ������
				pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );

				}
			// ��᫥ �⮣� 横�� ����� ����祭 �� �� ������, � १����
			// ࠧ��� ��� (����⢥���) ����, �.�., n << tis->numCon.

			if ( lpProcOut ) {
				isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
				if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
				}

			// ������ ����室��� ��室�� n ����⮢ 㤠���� �� ���
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF, (void*) tis->lpDNF, - sz );
			tis->numCon -= n;
			tis->lenDNF -= sz;
			}


		// ����� ��� �� �������� ������, ����� ᫥���� �� ���,
		// �.�., ���ᨬ��쭮 ������ ���, ������, �� ���� 㬥�� ��������
		// ⮫쪮 �, ����� ᫥���� �� ��㣨�.
		// ����� ����� �㤥� ���஡����� �������� � ������, �����
		// ᫥���� �� ��� (�஥� � �.�.) ��㣨� ����⮢.

		pCon = tis -> lpDNF;         // ����� �����饭�� � �㫥���� �����
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

		// ��३� � ᫥���饬� �ࠢ���
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
����������������������������������������������������������������������������Ŀ
� �㫥��� ����� ����砥��� � ������ ���.                    � Dis0IntoCon �
�                                                              �������������Ĵ
������������������������������������������������������������������������������
*/
int Dis0IntoCon ( KBFILE* tis ) {
	// �஢���� 墠�� �� ���� � ��� ��� �������⥫��� numProp (� 0-�� �����)
	// ����⮢ ����� 1 ��᪠�뢠��� ���� ���� ����� pDis
	long sz = ( tis->lpRul->numProp * sizeof(HEAD_RUL) ) + tis->lpRul->lenRul;
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, sz + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;

		}
	// ������ ������ ����


	HEAD_RUL  *pEndCon  = tis->lpDNF;
	HEAD_PROP *pConProp = (HEAD_PROP*) (pEndCon + 1);
	HEAD_PROP *pDisProp = (HEAD_PROP*) (tis->lpRul + 1);
	int i, m;

	// ���� �� ��᪠�뢠��� �����
	for ( i = 0; i < tis->lpRul->numProp; i++ ) {

		m = tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;

		// ���樠����஢��� ��������� ������ ����� pEndCon
		pEndCon->numProp = 1;
		pEndCon->lenRul = sizeof(HEAD_RUL) + sizeof(HEAD_PROP) + m * sizeof(COMP);
		pEndCon->Const.nec = 0; pEndCon->Const.pos = UNITY;
		pEndCon->Style = RUL_DISJUNCT;

		// �����஢��� ��᫥ ��������� ⥪�饥 ��᪠�뢠��� �����
		_fmemcpy ( (void*) pConProp, (void*) pDisProp, sizeof(HEAD_PROP) + m * sizeof(COMP) );
		if ( pDisProp->Style & PROP_CONJUNCT ) {           // �᫨ �� ���뫪�
			pConProp->Style = 0;
			}
		else {
			pConProp->Style = PROP_CONJUNCT;
			}

		// �ਢ��� ���� ����� (� ������஢�����)
		ReduceCon ( tis, pEndCon );
		if ( pEndCon->Const.pos != 0 ) {          // �᫨ ����� ���ਢ�����
			// ������� ����஥��� ����� � ���, �.�., �������� pEndCon,
			// ������⢮ ����⮢ � �� �����
			tis->numCon++;
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			pConProp = (HEAD_PROP*) (pEndCon + 1);
			}

		// ��३� � ᫥���饬� ��᪠�뢠��� � �����
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}

	return LKE_SUCCESS;
	}

/*
����������������������������������������������������������������������������Ŀ
� �㫥��� �ࠢ���, ��饥�� ����⮬ (������⥫��        � Con0IntoCon �
� �⢥ত�����), ����砥��� � ���.                             �������������Ĵ
������������������������������������������������������������������������������
*/
int Con0IntoCon ( KBFILE* tis ) {
	// �஢���� 墠�� �� ���� � ��� ��� ������ �����
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < tis->lpRul->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, tis->lpRul->lenRul + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// ������ ������ ����

	// �����஢��� �ࠢ��� � ��� � ��
	_fmemcpy ( (void*) tis->lpDNF, (void*) tis->lpRul, tis->lpRul->lenRul );

	// �ਢ��� ���� �����
	ReduceCon ( tis, tis->lpDNF );
	// �� �� �஢��塞 ���祭�� ����⠭��, �.�., ����室��� ����� �� �����

	// ������� ����� � ���
	tis->numCon++;
	tis->lenDNF += tis->lpRul->lenRul;

	return LKE_SUCCESS;
	}

/*
����������������������������������������������������������������������������Ŀ
� ����� pDis ����砥��� � ����� pCon. ����稢訥��        � DisIntoCon �
� ������ ����������� � ����� ���. �����稢����� ��饥        ������������Ĵ
� ������⢮ ����⮢ � ����� ���.                                         �
������������������������������������������������������������������������������
*/
int DisIntoCon ( KBFILE* tis, HEAD_RUL *pDis, HEAD_RUL *pCon, int numStart, long ofsStart ) {

	// �஢���� 墠�� �� ���� � ��� ��� �������⥫��� numProp (� �����)
	// ����⮢ ����� pCon ���� ���� ����� pDis
	long sz = (pDis->numProp * pCon->lenRul) + pDis->lenRul;
	if ( tis->lenDNF + sz > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + sz + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// ������ ������ ����

	BOOL      isZero;
	HEAD_PROP *pDisProp = (HEAD_PROP*) (pDis + 1);  // ������� �� 0-�� ��᪠�.
	HEAD_PROP *pConProp;
	COMP      *pDisComp;
	COMP      *pConComp;
	HEAD_RUL  *pEndCon  = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	int i, j, m;

	for ( i = 0; i < pDis->numProp; i++ ) {

		// �����஢��� ����� pCon � ����� ���, �.�. ᤥ���� ����⮢��
		_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

		// ������ ����� i-�� ��᪠�뢠��� � ����� � ���� ��᪠�뢠���
		// � ⠪�� �� ����஬ � �����. �᫨ ⠪��� ��᪠�뢠���
		// � ����� ��� (�.�., ��� �����筮�), � ����室���
		// �������� ��� � ����� �����

		// �ன��� �� �ᥬ ��᪠�뢠��� �����
		pConProp = (HEAD_PROP*) (pEndCon + 1);
		for ( j = 0; j < pEndCon->numProp; j++ ) {
			m = pConProp->curAtt;
			if ( m == pDisProp->curAtt ) break;
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ m ].AttRec.numVal );
			}
		if ( j == pEndCon->numProp ) {
			// ���� �����稫��, �� �㦭�� ��᪠�뢠��� ������� �� �뫮
			// ����室��� �������� ��� � ᠬ� ����� ����� � ���樠����஢���
			pConProp->curAtt = pDisProp->curAtt;
			pConProp->Style = PROP_CONJUNCT;

			pConComp = (COMP*) (pConProp + 1);
			m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pConComp->nec = 0; pConComp->pos = UNITY;
				pConComp = pConComp + 1;     // ��३� � ᫥���饩 ���������
				}
			// �������� ����� ����� �� ����� ��᪠�뢠���
			pEndCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pEndCon->numProp++;
			}
		// ������ pConProp 㪠�뢠�� �� ᮮ⢥�����饥 ��᪠�뢠��� � �����


		// ���᫨�� ������������ ������ ��᪠�뢠��� pConProp � pDisProp

		// �� ��⠥�, �� ��室�� ����� (�� ��������� �� ���� ᥪ樨)
		// �ਢ����, �.�., �� 㦥 ����祭 � ���. ������, � �ਭ樯�,
		// ����祭�� ����� ����� ��������� ���ਢ������, ��᪮���
		// ����� ������뢠���� ᥪ��, � �� �᫠������. �� �� ����� �ந����
		// ⮫쪮 ��-�� �⮩ ᥪ樨. ���⮬� �� ��������� ����� �����६����
		// ������ �ਢ������, ��୥�, ������ ����室��� ���४樨. �� ����� ����
		// 㢥७�, �� �� ���������� ��室���� �����, ����� �����
		// ���� ࠢ�� ����⠭��, ࠢ�� 1. �� ��������� ���� ���� ��� ��ਠ��:
		// 1. ��������� 㬥��訫���, �� �� �⠫� ����� ����⠭��; � �⮬ ��砥
		//    �� ����室��� ��⠢��� ࠢ��� 1 (�.�., �� ������); �஬� ⮣�,
		//    �᫨ ⠪�� ��������� �������, � ���� ����⠭� 㦥 ��
		//    ��������� (�� 㬥������), ���⮬� ����室��� ��⠭����� ᮮ⢥�����騩
		//    䫠� (� ����� ������� � ��� 䫠��, � ���� �࠭��� ���-�
		//    ���ᨬ��쭮� ���祭�� ᥪ樨, � �᫨ � ���� ��� ��������
		//    ����� ���� ࠢ�� ����⠭�, � ����� ����⠭� �� �������).
		// 2. ��������� ����⨫��� ���� ����⠭��; � �⮬ ��砥 �� ����室���
		//    ��ࠢ���� �⮬� ���祭�� � ����� �� ��宦����� ���ᨬ㬠 (����⠭��)
		//    ᥪ樨.
		// ��� ��楤��� ����� �࣠�������� ⠪. �� �६� ��������� �� �ᥣ��
		// ��⠥�, �� ����⠭� ����� �� ������� � ࠢ�� �� ��室����
		// ���祭��. � १���� �������� ᥪ��, �ਢ������� ������ ��
		// �⮩ ����⠭� (�.�., ��, �� ��� ����⠭�� ��ࠢ�������� 1).
		// �����६���� �� ��������� �������� ����⠭� ᥪ樨 (�.�., ��
		// ���ᨬ�). � ���� ����⠭� ᥪ樨 �ࠢ�������� � ����⠭⮩
		// �����. �᫨ ��ࢠ� ����� ���� ࠢ�� ��ன, � �� ��ଠ�쭮,
		// �� �� ���諨 �����⨬�� (�� 㬥��訫� ����� ����⠭��) �࠭��� �
		// ����� ��室���� � �ਢ������� �ଥ. �᫨ �� ����⠭� ᥪ樨
		// ����� ����⠭�� �����, � ���� ����⠭� ��ࠢ��������
		// ������ ���祭��, ��᫥ 祣� ����室��� �ன��� �� �ᥬ ᥪ��
		// (� �.�. � �� ⥪�饩) � �ਢ��� �� � ����� ����⠭�. � १����
		// ������� ����������, ����� ��-�� ��������� ����⠭�� ���������
		// ����� ���� ࠢ�묨 ��, ���� ��ࠢ���� � 1. ��������, �������
		// ������� ᥪ樨--�� ����室��� ���ભ���. ��楤��� �ਢ������
		// �ᥣ� ����� � ����� ����⠭� (� �������� 㤠������ ᥪ権)
		// ����� �뭥��. ��� ᮪�饭�� ���᫥���, �᫨ ����⠭� �⠭������
		// ࠢ��� 0, �.�., ����� ��祣� �� ����� � �����頥��� ��� ��㣨�,
		// � �� �� �ਢ���� ���, �.�., ����� �஢������ ᯥ樠�쭠� �஢�ઠ
		// �⮣� ����.

		pConComp = (COMP*) (pConProp + 1);
		pDisComp = (COMP*) (pDisProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		COMP NewConst; NewConst.nec = 0; NewConst.pos = 0;
		BOOL Invert = pDisProp->Style & PROP_CONJUNCT;

		for ( j = 0; j < m; j++ ) {
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			if ( pConComp->pos > pDisComp->pos && pDisComp->pos < pEndCon->Const.pos ) {
				// ��ࢮ� �᫮��� �����뢠�� ����室������ 㬥��襭��, ������,
				// ⮫쪮 �᫨ �믮������ ��஥ �᫮���, �.�., �� 㬥��襭��
				// ���� �஢�� ����⠭��
				pConComp->pos = pDisComp->pos;
				}
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			// ���᫨�� ���� ���ᨬ� (����� ����⠭��)
			if ( pConComp->pos > NewConst.pos ) NewConst.pos = pConComp->pos;
			// ��३� � ᫥���饩 ���������
			pConComp = pConComp + 1; pDisComp = pDisComp + 1;
			}
		if ( NewConst.pos == 0 ) pEndCon->Const.pos = 0;
		else if ( NewConst.pos < pEndCon->Const.pos ) {
			pEndCon->Const.pos = NewConst.pos;
			// �ਢ��� ����� � ����� ����⠭� NewConst.pos
			ReduceConTo ( tis, pEndCon );
			}
		// ������ ��᫥���� ����� �ਢ���� (�஬� ���� NewConst.pos == 0)


		if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, numStart, ofsStart ) ) {
			// ������� ����஥��� ����� � ���, �.�., �������� pEndCon,
			// ������⢮ ����⮢ � �� �����
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			tis->numCon++;
			}

		// ��३� � ᫥���饬� ��᪠�뢠���
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal );
		}

	return LKE_SUCCESS;
	}

/*
����������������������������������������������������������������������������Ŀ
� ������⥫쭮� �⢥ত���� pCon ����砥��� � ��� ��⥬ ���⮣�� ConIntoCon �
� ���������� ��� � ����� ���.                                   ������������Ĵ
������������������������������������������������������������������������������
*/
int ConIntoCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	// �஢���� 墠�� �� ���� � ��� ��� ������ �����
	if ( tis->lenDNF + pCon->lenRul > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < pCon->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + pCon->lenRul + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// ������ ������ ����

	// �����஢��� �ࠢ��� � ��� � ��
	HEAD_RUL* pEndCon = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

	// �ਢ��� ���� �����
	ReduceCon ( tis, pEndCon );
	if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, 0, 0 ) ) {
		// ������� ����஥��� ����� � ���
		tis->numCon++;
		tis->lenDNF += pEndCon->lenRul;
		}

	return LKE_SUCCESS;
	}

/*
����������������������������������������������������������������������������Ŀ
� �ਢ������ �ࠢ���. ����� ���������� ������⢮ ��᪠�뢠���    �ReduceCon �
� � ����� �ࠢ��� (㬥�������), � ⠪�� ��⠭���������� ����⠭�����������Ĵ
� ������ �� ���뢠����, �� �� �ࠢ���� ����� ᫥������ ��㣨� � �������    �
� ����権 � ������� �� �ந�������� (�.�., �।����������, �� �ࠢ���      �
� ��᫥����). �᫨ �ࠢ��� ⮦���⢥��� ��⨭��, � ��� �� ���ન������,    �
� � ���� �� ���� 㤠������ �� ��᪠�뢠��� � ��⠭���������� ᮮ⢥�����-�
� �� ����⠭�. ���⮬� ��᫥ �ਢ������ ����室��� �஢����� ����⠭��.    �
������������������������������������������������������������������������������
*/
void ReduceCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// ���� �� ��᪠�뢠��� ��� ��宦����� ����⠭��, �.�., �������쭮�� ���ᨬ㬠 � ������ ᥪ樨
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// ���� �� ��������⠬ ⥪�饣� ��᪠�뢠���
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // ��३� � ᫥���饩 ���������
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// ��३� � ᫥���饬� ��᪠�뢠���
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// ������ �� ����� �ࠢ����� ����⠭��

	ReduceConTo ( tis, pCon );

	return;
	}

/*
����������������������������������������������������������������������������Ŀ
� �ਢ������ ����� � ����� ����⠭�, ����� ������ ����  � ReduceConTo �
� ��⠭������. �।����������, �� ࠭�� ����� �� �ਢ���� �������������Ĵ
� � ��㣮� (����襩) ����⠭�. �� �ਢ������ � ����� ����⠭� �          �
� ����������, ����� ����� ���� ࠢ�� �� ��ࠢ�������� 1. � १����    �
� ����� ��������� ⮦���⢥��� ��⨭�� ᥪ樨, ����� ���ન������, ��  �
� �ਢ���� � 㬥��襭�� ������⢠ ��᪠�뢠��� � ����� �����.           �
������������������������������������������������������������������������������
*/
void ReduceConTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// ���� �� ��᪠�뢠���
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// ���� �� ��������⠬ ⥪�饣� ��᪠�뢠���
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// �᫨ ⥪��� ��������� ����� ���� ࠢ�� ����⠭�, � ��ࠢ���� �� 1
			// ���� �⬥���, �� ᥪ�� ���ਢ���쭠
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = UNITY; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // ��३� � ᫥���饩 ���������
			}

		if ( isTrivSec ) {
			// ������� ��� ᥪ��
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			// ??????? lpDNF
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// ��३� � ᫥���饬� ��᪠�뢠���
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
����������������������������������������������������������������������������Ŀ
� �ਢ��� ����� � ���ᨬ��쭮� �ଥ � �।���⥫��    � ReduceConMax �
� ��宦������ ����⠭��.                                      ��������������Ĵ
������������������������������������������������������������������������������
*/
void ReduceConMax ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// ���� �� ��᪠�뢠��� ��� ��宦����� ����⠭��, �.�., �������쭮�� ���ᨬ㬠 � ������ ᥪ樨
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// ���� �� ��������⠬ ⥪�饣� ��᪠�뢠���
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // ��३� � ᫥���饩 ���������
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// ��३� � ᫥���饬� ��᪠�뢠���
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// ������ �� ����� �ࠢ����� ����⠭��

	ReduceConMaxTo ( tis, pCon );

	return;
	}

/*
����������������������������������������������������������������������������Ŀ
� �ਢ��� ����� � ���ᨬ��쭮� �ଥ �� ��� ��࠭��     � ReduceConMaxTo �
� ��⠭�������� ����⠭�. �� ����砥�, �� �� ����������,����������������Ĵ
� �����, ����� ����⠭��, ��ࠢ�������� ��.                              �
������������������������������������������������������������������������������
*/
void ReduceConMaxTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// ���� �� ��᪠�뢠���
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// ���� �� ��������⠬ ⥪�饣� ��᪠�뢠���
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// �᫨ ⥪��� ��������� ����� ����⠭��, � ��ࠢ���� �� ����⠭�
			// ���� �⬥���, �� ᥪ�� ���ਢ���쭠
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = pCon->Const.pos; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // ��३� � ᫥���饩 ���������
			}

		if ( isTrivSec ) {
			// ������� ��� ᥪ��
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			InsDelMem ( (void*) pCon, pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// ��३� � ᫥���饬� ��᪠�뢠���
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
����������������������������������������������������������������������������Ŀ
� �㭪�� ��⠥��� �������� ����� pCon ����⠬�, ����륳 AbsorbFirst �
� ��室���� ��᫥ ����. �᫨ ����� �����頥���, � ��       �������������Ĵ
� ���ન������, 㬥��蠥��� ������⢮ ����⮢ � �� �����.               �
������������������������������������������������������������������������������
*/
BOOL AbsorbFirst ( KBFILE* tis, HEAD_RUL *pEndCon, int numCon ) {
	int i;
	HEAD_RUL *pCon;

	// ����� � numCon+1-�� �����
	pCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
	for ( i = numCon+1; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) break;

		// �த������ �஢��� �����饭�� ��� ᫥����� ����⮢
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}
	if ( i == tis->numCon ) return FALSE;

	// ���ભ��� �����
	tis->numCon--;
	tis->lenDNF -= pEndCon->lenRul;
	InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pEndCon->lenRul, (void*) pEndCon, - pEndCon->lenRul );

	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
� �㭪�� ��⠥��� �������� �����, ����� ᫥��� ��       � AbsorbLast �
� ��᫥���� (�.�., �� �� �� �ਭ������� ���).                  ������������Ĵ
� � �� ��砥, ������ᨬ� �� �����頥����,                               �
� ��祣� �� �������, �஬� �����頥���� ���祭��. �஢�ઠ �� �����饭��   �
� ��稭����� � ����� numStart, ᬥ饭�� �� ���ண� ofsStart.            �
������������������������������������������������������������������������������
*/
BOOL AbsorbLast ( KBFILE* tis, HEAD_RUL* pEndCon, int numStart, long ofsStart ) {
	int i;
	HEAD_RUL*  pCon;

	// ����� � numStart-�� �����
	pCon = (HEAD_RUL*) ( (char*) tis->lpDNF + ofsStart );
	for ( i = numStart; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) return TRUE;

		// �த������ �஢��� �����饭�� ��� ᫥����� ����⮢
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}

	return FALSE;
	}

/*
����������������������������������������������������������������������������Ŀ
� �㭪�� �஢���� �믮������ �᫮��� pFstCon |= pSecCon, ��   � AbsorbCon �
� ����砥� �����饭�� ����� pSecCon (�.�., �� �.�. ���ભ��)�����������Ĵ
������������������������������������������������������������������������������
*/
BOOL AbsorbCon ( KBFILE* tis, HEAD_RUL *pFstCon, HEAD_RUL *pSecCon ) {
	int i, j, k, m;
	HEAD_PROP* pFstConProp;
	HEAD_PROP* pSecConProp;
	COMP*      pFstConComp;
	COMP*      pSecConComp;

	// ������ ᠬ� ��������� �஢�ન
	if ( pSecCon->numProp  <  pFstCon->numProp || pSecCon->Const.pos > pFstCon->Const.pos ) {
		return FALSE;
		}

	// ������ ���� �஢����� ����������� ��᪠�뢠���--�᫨ � pFstCon
	// ���� ��� �� ���� ��᪠�뢠���, ���஥ �� ��������� �
	// pSecCon, � �᫮��� �� �믮������
	pFstConProp = (HEAD_PROP*) ( pFstCon + 1 );
	for ( j = 0; j < pFstCon->numProp; j++ ) {

		// �஢���� ������� �� ��᪠�뢠��� � ����஬ pFstConProp->curAtt
		// � ��᫥���� ����� pSecCon

		pSecConProp = (HEAD_PROP*) ( pSecCon + 1 );
		for ( k = 0; k < pSecCon->numProp; k++ ) {
			if ( pSecConProp->curAtt == pFstConProp->curAtt ) {

				// ��諮�� ᮮ⢥�����饥 ��᪠�뢠���; �஢���� �����������

				// �㦭� �஢���� ����������⭮���, �.�., ��
				// ��������� �� pFstCon ������ ���� �� ����� 祬 pSecCon
				// �᫨ � pFstCon ��諠�� ���������, ����� �����
				// ᮮ⢥�����饩 � pSecCon, � �᫮��� �� �믮������,
				// � �᫨ ⠪���� �� �뫮 �������, � ��
				// ����砥� �����饭��.

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

		// ��३� � ᫥���饬� ��᪠�뢠���
		pFstConProp = (HEAD_PROP*) ( (char*) pFstConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal );
		}

	return TRUE;
	}

/*
����������������������������������������������������������������������������Ŀ
� ������� �����. ���������騥 � ����� ᥪ樨           � ExtendCon �
� ����몮�뢠���� � ����� � �� ���������� ����������� ��        �����������Ĵ
� ����⠭��, ����� ������ ���� ��⠭������.                                �
������������������������������������������������������������������������������
*/
void ExtendCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	int i, j, m;
	HEAD_PROP* pProp;
	COMP*      pComp;

	// ���� �� �ᥬ ����ࠬ ��᪠�뢠���, �.�., �஢��塞 ������ ᥪ��
	for ( i = 0; i < tis->numAtt; i++ ) {

		// �ன��� �� �ᥬ ��᪠�뢠��� ����� � ���᪠� i-��
		pProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {
			if ( i == pProp->curAtt ) break;
			pProp = (HEAD_PROP*) ( (char*) pProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal );
			}
		if ( j == pCon->numProp ) {
			// ���� �����稫��, �� i-�� ��᪠�뢠��� ������� �� �뫮
			// ����室��� �������� ��� � ᠬ� ����� ����� � ���樠����஢���
			pProp->curAtt = i;
			pProp->Style = PROP_CONJUNCT;

			pComp = (COMP*) (pProp + 1);
			m = tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pComp->nec = 0; pComp->pos = pCon->Const.pos;
				pComp = pComp + 1;                   // � ᫥���饩 ���������
				}
			// �������� ����� ����� �� ����� ��᪠�뢠���
			pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pCon->numProp++;
			}
		}
	}