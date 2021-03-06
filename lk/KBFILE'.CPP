
/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴커
�                                                             � CompileToDNF �
�                                                             읕컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
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

	// 띲ゥ¡� 與か (�□젩졻猶젰恂� �乘ⅱ�� ��젪Œ�)
	if ( tis->lpRul->Style & RUL_DISJUNCT ) { Con0IntoCon ( tis ); }    // (+)
	else                                    { Dis0IntoCon ( tis ); }    // (-)

	if ( lpProcOut ) {
		isBreak = (*lpProcOut) ( STAT_GENERATION, 0, 0, 0, tis->numCon );
		if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
		}

	pCon = tis -> lpDNF;             // 뜝�졻� ��＋�耀��� � �乘ⅱ�． ぎ�惟�も�
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



	// 롟��˛�� �昔璵遜 (췅葉췅β碎 � ��舒�． ��젪Œ�)
	pDis = (HEAD_RUL*) ( (char*) tis->lpRul + tis->lpRul->lenRul );
	for ( i = 1; i < tis->numRul; i++ ) {                // 뼥か �� ㄸ㏐縡も젹

		if ( lpProcOut ) {
			isBreak = (*lpProcOut) ( STAT_GENERATION, i, 0, tis->numCon, tis->numCon );
			if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
			}

		if ( pDis->Style & RUL_DISJUNCT ) {                    // (+) ぎ�惟�も
			ConIntoCon ( tis, pCon ); // ??? pDis ???
			}
		else {                                                 // (-) ㄸ㏐縡も

			// 댂ⓩ쯄エ쭬與� ㄻ� 與か� �� ぎ�惟�も젹
			sz = tis->lenDNF;                      // 룼췅ㄾ〃恂� �黍 �揖�夕Ð젺Ŀ
			n = tis -> numCon;                   // 댾若ㄽ�� ぎエ曄飡¡ ぎ�惟�も��
			pCon = tis -> lpDNF;                    // 뜝�졻� � �乘ⅱ�． ぎ�惟�も�
			for ( j = 0; j < n; j++ ) {

				if ( lpProcOut ) {
					isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
					if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
					}

				DisIntoCon ( tis, pDis, pCon, n, sz );

				// 룯誓⒱� � 笹ⅳ莘耀с ぎ�惟�も�
				pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );

				}
			// 룼笹� 將�． 與か� ㄸ㏐縡も ˚ヮ曄� ¡ ㏇� ぎ�惟�も�, � 誓㎯レ�졻�
			// �젳Д� 꼵� (率耀飡´���) �乙��, �.�., n << tis->numCon.

			if ( lpProcOut ) {
				isBreak = (*lpProcOut) ( STAT_GENERATION, i, j, n, tis->numCon );
				if ( isBreak ) { ResetDNF ( tis ); return FALSE; }
				}

			// 뮙��閃 ���▼�ㄸМ ⓤ若ㄽ瑜 n ぎ�惟�も�� 蝨젷ⓥ� �� 꼵�
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF, (void*) tis->lpDNF, - sz );
			tis->numCon -= n;
			tis->lenDNF -= sz;
			}


		// 눃α� 若昔溫 〓 ��＋�殊筍 ぎ�惟�も�, ぎ獸贍� 笹ⅳ莘� �� 꼵�,
		// �.�., 쵟めº젷彛� 承昔飡ⓥ� 꼵�, �ㄽ젶�, щ ��첓 僧ⅴ� ��＋��졻�
		// 獸レぎ 收, ぎ獸贍� 笹ⅳ莘� �� ㅰ膝ⓨ.
		// 꽑ゥ� М┃� ▲ㄵ� ���昔‘쥯筍 ��＋��졻� 收 ぎ�惟�も�, ぎ獸贍�
		// 笹ⅳ莘� �� 캙� (循�ⅹ � �.�.) ㅰ膝ⓨ ぎ�惟�も��.

		pCon = tis -> lpDNF;         // 뜝�졻� ��＋�耀��� � �乘ⅱ�． ぎ�惟�も�
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

		// 룯誓⒱� � 笹ⅳ莘耀с ��젪Œ�
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
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
� 띲ゥ¡� ㄸ㏐縡も ˚ヮ�젰恂� � �信栒� 꼵�.                    � Dis0IntoCon �
�                                                              읕컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
int Dis0IntoCon ( KBFILE* tis ) {
	// 뤲�´黍筍 罌졻ⓥ エ Д飡� � 꼵� ㄻ� ㄾ��キⓥ�レ�音 numProp (� 0-�� ㄸ㏐縡も�)
	// ぎ�惟�も�� ㄻÞ� 1 �吟첓㏓쥯��� �ヮ� �ㄽ� ㄻÞ� pDis
	long sz = ( tis->lpRul->numProp * sizeof(HEAD_RUL) ) + tis->lpRul->lenRul;
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, sz + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;

		}
	// 뮙��閃 캙э筍 α筍


	HEAD_RUL  *pEndCon  = tis->lpDNF;
	HEAD_PROP *pConProp = (HEAD_PROP*) (pEndCon + 1);
	HEAD_PROP *pDisProp = (HEAD_PROP*) (tis->lpRul + 1);
	int i, m;

	// 뼥か �� �吟첓㏓쥯�⑨� ㄸ㏐縡も�
	for ( i = 0; i < tis->lpRul->numProp; i++ ) {

		m = tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;

		// 댂ⓩ쯄エ㎤昔쥯筍 쭬．ギ¡� ��¡． ぎ�惟�も� pEndCon
		pEndCon->numProp = 1;
		pEndCon->lenRul = sizeof(HEAD_RUL) + sizeof(HEAD_PROP) + m * sizeof(COMP);
		pEndCon->Const.nec = 0; pEndCon->Const.pos = UNITY;
		pEndCon->Style = RUL_DISJUNCT;

		// 뫇��ⓣ�쥯筍 ��笹� 쭬．ギ˚� 收ゃ耀� �吟첓㏓쥯��� ㄸ㏐縡も�
		_fmemcpy ( (void*) pConProp, (void*) pDisProp, sizeof(HEAD_PROP) + m * sizeof(COMP) );
		if ( pDisProp->Style & PROP_CONJUNCT ) {           // 끷エ 將� ��頌オ�
			pConProp->Style = 0;
			}
		else {
			pConProp->Style = PROP_CONJUNCT;
			}

		// 뤲Ðα殊 ���硫 ぎ�惟�も (� Þ´設ⓣ�쥯����)
		ReduceCon ( tis, pEndCon );
		if ( pEndCon->Const.pos != 0 ) {          // 끷エ ぎ�惟�も �β黍˘젷��
			// 궕ヮ葉筍 ��飡昔���硫 ぎ�惟�も � 꼵�, �.�., �㎚��ⓥ� pEndCon,
			// ぎエ曄飡¡ ぎ�惟�も�� � ⓨ ㄻÞ�
			tis->numCon++;
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			pConProp = (HEAD_PROP*) (pEndCon + 1);
			}

		// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧ � ㄸ㏐縡も�
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}

	return LKE_SUCCESS;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
� 띲ゥ¡� ��젪Œ�, 琠ワ迹ⅴ碎 ぎ�惟�も�� (��ギ┬收レ�臾        � Con0IntoCon �
� 呻´逝ㄵ����), ˚ヮ�젰恂� � 꼵�.                             읕컴컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
int Con0IntoCon ( KBFILE* tis ) {
	// 뤲�´黍筍 罌졻ⓥ エ Д飡� � 꼵� ㄻ� �ㄽ�． ぎ�惟�も�
	if ( (MemorySize ( (void*) tis->lpDNF ) ) < tis->lpRul->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, tis->lpRul->lenRul + 2048L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// 뮙��閃 캙э筍 α筍

	// 뫇��ⓣ�쥯筍 ��젪Œ� � 꼵� � ㏇�
	_fmemcpy ( (void*) tis->lpDNF, (void*) tis->lpRul, tis->lpRul->lenRul );

	// 뤲Ðα殊 ���硫 ぎ�惟�も
	ReduceCon ( tis, tis->lpDNF );
	// 뚮 �� �昔´涉�� ㎛좂���� ぎ�飡젺瞬, �.�., ���▼�ㄸМ ºβ� ヮ‘� ぎ�惟�も

	// 궕ヮ葉筍 ぎ�惟�も � 꼵�
	tis->numCon++;
	tis->lenDNF += tis->lpRul->lenRul;

	return LKE_SUCCESS;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
� 꽙㏐縡も pDis ˚ヮ�젰恂� � ぎ�惟�も pCon. 룼ャ葉�鼇α�        � DisIntoCon �
� ぎ�惟�も� ㄾ줎˙禎恂� � ぎ�ζ 꼵�. 뱼�エ葉쥯β碎 �↓ⅴ        읕컴컴컴컴컴캑
� ぎエ曄飡¡ ぎ�惟�も�� � ㄻÞ� 꼵�.                                         �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
int DisIntoCon ( KBFILE* tis, HEAD_RUL *pDis, HEAD_RUL *pCon, int numStart, long ofsStart ) {

	// 뤲�´黍筍 罌졻ⓥ エ Д飡� � 꼵� ㄻ� ㄾ��キⓥ�レ�音 numProp (� ㄸ㏐縡も�)
	// ぎ�惟�も�� ㄻÞ� pCon �ヮ� �ㄽ� ㄻÞ� pDis
	long sz = (pDis->numProp * pCon->lenRul) + pDis->lenRul;
	if ( tis->lenDNF + sz > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < sz ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + sz + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// 뮙��閃 캙э筍 α筍

	BOOL      isZero;
	HEAD_PROP *pDisProp = (HEAD_PROP*) (pDis + 1);  // 벆젳졻� 췅 0-�� �吟첓�.
	HEAD_PROP *pConProp;
	COMP      *pDisComp;
	COMP      *pConComp;
	HEAD_RUL  *pEndCon  = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	int i, j, m;

	for ( i = 0; i < pDis->numProp; i++ ) {

		// 뫇��ⓣ�쥯筍 ぎ�惟�も pCon � ぎ�ζ 꼵�, �.�. 誠�쳽筍 쭬．獸˚�
		_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

		// 벁췅筍 ��Д� i-． �吟첓㏓쥯�⑨ � ㄸ㏐縡も� � 췅⒱� �吟첓㏓쥯���
		// � �젶º ┘ ��Д昔� � ぎ�惟�も�. 끷エ �젶�． �吟첓㏓쥯�⑨
		// � ぎ�惟�も� �β (�.�., ��� ⅳÞ①���), 獸 ���▼�ㄸМ
		// ㄾ줎˘筍 ⅲ� � ぎ�ζ ぎ�惟�も�

		// 뤲�⒱ⓤ� �� ㏇�� �吟첓㏓쥯�⑨� ぎ�惟�も�
		pConProp = (HEAD_PROP*) (pEndCon + 1);
		for ( j = 0; j < pEndCon->numProp; j++ ) {
			m = pConProp->curAtt;
			if ( m == pDisProp->curAtt ) break;
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ m ].AttRec.numVal );
			}
		if ( j == pEndCon->numProp ) {
			// 뼥か 쭬ぎ�葉メ�, �� �拾��� �吟첓㏓쥯��� 췅ħ��� �� 〓ギ
			// 뜢�▼�ㄸМ ㄾ줎˘筍 ⅲ� � �젹硫 ぎ�ζ ぎ�惟�も� � Þⓩ쯄エ㎤昔쥯筍
			pConProp->curAtt = pDisProp->curAtt;
			pConProp->Style = PROP_CONJUNCT;

			pConComp = (COMP*) (pConProp + 1);
			m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pConComp->nec = 0; pConComp->pos = UNITY;
				pConComp = pConComp + 1;     // 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
				}
			// 뱼�エ葉筍 ㄻÞ� ぎ�惟�も� 췅 ㄻÞ� �吟첓㏓쥯�⑨
			pEndCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pEndCon->numProp++;
			}
		// 뮙��閃 pConProp 丞젳猶젰� 췅 貰�手β飡㏂迹ⅴ �吟첓㏓쥯��� � ぎ�惟�も�


		// 귣葉笹ⓥ� ��ぎН����狩硫 Ж�º僧 �吟첓㏓쥯�Ł pConProp � pDisProp

		// 뚮 淞ⓥ젰�, 譽� ⓤ若ㄽ硫 ぎ�惟�も (ㄾ 췅ギ┘�⑨ 췅 �ⅲ� 醒ゆĿ)
		// �黍´ㄵ�, �.�., �� 拾� ˚ヮ曄� � 꼵�. 렎췅ぎ, � �黍�與��,
		// ��ャ曄��硫 ぎ�惟�も М┘� �첓쭬筍碎 ���黍´ㄵ��臾, ��稅�レゃ
		// ぎ＄� 췅か젮猶젰恂� 醒ゆ⑨, 獸 �� �笹젩ワβ碎. 뜮 將� М┘� �昔�㎜⒱�
		// 獸レぎ ��-쭬 將�� 醒ゆĿ. 룼將�с �黍 췅ギ┘�Ŀ М┃� �ㄽ�№�Д���
		// ㄵ쳽筍 �黍´ㄵ���, ´惜ⅴ, ㄵ쳽筍 ���▼�ㄸщ� ぎ薛ⅹ與�. 뚮 М┘� 〓筍
		// 瑟�誓��, 譽� ㏇� ぎН����瞬 ⓤ若ㄽ�． ㄸ㏐縡も�, ぎ獸贍� ‘レ蜈
		// エ‘ �젪�� ぎ�飡젺瞬, �젪�� 1. 뤲� 췅ギ┘�Ŀ М╈ 〓筍 ㄲ� 쥯黍젺��:
		// 1. 뒶Н������ 僧��麟Œ졹�, �� �� 飡젷� Д�麟� ぎ�飡젺瞬; � 將�� 笹晨젰
		//    ⅴ ���▼�ㄸМ �飡젪ⓥ� �젪��� 1 (�.�., �� Д�汀�); む�Д 獸．,
		//    αエ �젶좑 ぎН������ 率耀飡㏂β, 獸 �↓좑 ぎ�飡젺�� 拾� ��
		//    �㎚��ⓥ碎 (�� 僧��麟ⓥ碎), ��將�с ���▼�ㄸМ 信�젺�˘筍 貰�手β飡㏂迹Ł
		//    氏젫 (� М┃� �‘⒱ⓤ� � ‥� 氏젫�, � �昔飡� 魚젺ⓥ� ＄�-獸
		//    쵟めº젷彛�� ㎛좂���� 醒ゆĿ, � αエ � ぎ�璵 ��� �첓┘恂�
		//    ‘レ蜈 エ‘ �젪�臾 ぎ�飡젺收, 獸 ㎛좂ⓥ ぎ�飡젺�� �� Д�畑恂�).
		// 2. 뒶Н������ ��信殊쳽刷 �Ĳ� ぎ�飡젺瞬; � 將�� 笹晨젰 ⅴ ���▼�ㄸМ
		//    �黍�젪�汀� 將�с ㎛좂��⑧ � 晨α筍 �黍 췅若┐��Ŀ 쵟めº僧� (ぎ�飡젺瞬)
		//    醒ゆĿ.
		// 귗� �昔璵ㅳ說 М┃� �薯젺�㎜쥯筍 �젶. 궙 №�э 췅ギ┘�⑨ щ ㏇ⅲ쩆
		// 淞ⓥ젰�, 譽� ぎ�飡젺�� ぎ�惟�も� �� Д�畑恂� � �젪췅 ⅴ ⓤ若ㄽ�с
		// ㎛좂��⑧. � 誓㎯レ�졻� ��ャ葉恂� 醒ゆ⑨, �黍´ㄵ�췅� º���� ��
		// 將�� ぎ�飡젺收 (�.�., ㏇�, 譽� �泣� ぎ�飡젺瞬 �黍�젪�Ð젰恂� 1).
		// 렎��№�Д��� �黍 췅ギ┘�Ŀ �揖ⓤワβ碎 ぎ�飡젺�� 醒ゆĿ (�.�., ⅴ
		// 쵟めº僧). � ぎ�璵 ぎ�飡젺�� 醒ゆĿ 蓀젪�Ð젰恂� � ぎ�飡젺獸�
		// ぎ�惟�も�. 끷エ ��舒좑 ‘レ蜈 エ‘ �젪췅 ™�昔�, 獸 ㏇� ��席젷彛�,
		// щ �� ��誓獄� ㄾ�信殊с� (�� 僧��麟Œ� �↓莘 ぎ�飡젺栒) ｀젺ⓩ� �
		// ぎ�惟�も 췅若ㄸ恂� � �黍´ㄵ���� 兒席�. 끷エ ┘ ぎ�飡젺�� 醒ゆĿ
		// Д�麟� ぎ�飡젺瞬 ぎ�惟�も�, 獸 �↓좑 ぎ�飡젺�� �黍�젪�Ð젰恂�
		// ��¡с ㎛좂��⑧, ��笹� 曄． ���▼�ㄸМ �昔⒱ⓤ� �� ㏇�� 醒ゆ⑨�
		// (� �.�. � �� 收ゃ耀�) � �黍´飡� ⓨ � ��¡� ぎ�飡젺收. � 誓㎯レ�졻�
		// �ⅹ�獸贍� ぎН����瞬, ぎ獸贍� ��-쭬 ���Ĳ��⑨ ぎ�飡젺瞬 �첓쭬エ刷
		// ‘レ蜈 エ‘ �젪�臾� ⅸ, ▲ㅳ� �黍�젪���� � 1. 궙㎚�┃�, ��琠汀碎
		// ⅳÞ①�瑜 醒ゆĿ--ⓨ ���▼�ㄸМ �揖�夕�呻�. 뤲�璵ㅳ說 �黍´ㄵ�⑨
		// ㏇ⅲ� ぎ�惟�も� � ��¡� ぎ�飡젺收 (� ¡㎚�┃臾 蝨젷����� 醒ゆŁ)
		// М┃� �萸α殊. 꽞� 貰む좈��⑨ �揖ⓤゥ�Ł, αエ ぎ�飡젺�� 飡젺�˘恂�
		// �젪��� 0, �.�., ぎ�惟�も �①ⅲ� �� ㎛좂ⓥ � ��＋��젰恂� ヮ〓� ㅰ膝º,
		// 獸 щ �� �黍¡ㄸ� ⅲ�, �.�., 쩆ゥ� �昔¡ㄸ恂� 召ζ쯄レ췅� �昔´夕�
		// 將�． 笹晨좑.

		pConComp = (COMP*) (pConProp + 1);
		pDisComp = (COMP*) (pDisProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		COMP NewConst; NewConst.nec = 0; NewConst.pos = 0;
		BOOL Invert = pDisProp->Style & PROP_CONJUNCT;

		for ( j = 0; j < m; j++ ) {
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			if ( pConComp->pos > pDisComp->pos && pDisComp->pos < pEndCon->Const.pos ) {
				// 룯舒�� 信ギ˘� ��첓㏓쥯β ���▼�ㄸМ飡� 僧��麟��⑨, �ㄽ젶�,
				// 獸レぎ αエ �誘�キ畑恂� ™�昔� 信ギ˘�, �.�., 將� 僧��麟����
				// �Ĳ� 侁�˛� ぎ�飡젺瞬
				pConComp->pos = pDisComp->pos;
				}
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			// 귣葉笹ⓥ� ���硫 쵟めº僧 (��㏂� ぎ�飡젺栒)
			if ( pConComp->pos > NewConst.pos ) NewConst.pos = pConComp->pos;
			// 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
			pConComp = pConComp + 1; pDisComp = pDisComp + 1;
			}
		if ( NewConst.pos == 0 ) pEndCon->Const.pos = 0;
		else if ( NewConst.pos < pEndCon->Const.pos ) {
			pEndCon->Const.pos = NewConst.pos;
			// 뤲Ðα殊 ぎ�惟�も � ��¡� ぎ�飡젺收 NewConst.pos
			ReduceConTo ( tis, pEndCon );
			}
		// 뮙��閃 ��笹ⅳ�Ł ぎ�惟�も �黍´ㄵ� (む�Д 笹晨좑 NewConst.pos == 0)


		if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, numStart, ofsStart ) ) {
			// 궕ヮ葉筍 ��飡昔���硫 ぎ�惟�も � 꼵�, �.�., �㎚��ⓥ� pEndCon,
			// ぎエ曄飡¡ ぎ�惟�も�� � ⓨ ㄻÞ�
			tis->lenDNF += pEndCon->lenRul;
			pEndCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
			tis->numCon++;
			}

		// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
		pDisProp = (HEAD_PROP*) ( (char*) pDisProp + sizeof(HEAD_PROP) +
                       sizeof(COMP) * tis->lpAttIdx [ pDisProp->curAtt ].AttRec.numVal );
		}

	return LKE_SUCCESS;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
� 룼ギ┬收レ��� 呻´逝ㄵ��� pCon ˚ヮ�젰恂� � 꼵� �呻�� �昔飡�．� ConIntoCon �
� ㄾ줎˙��⑨ ⅲ� � ぎ�ζ 꼵�.                                   읕컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
int ConIntoCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	// 뤲�´黍筍 罌졻ⓥ エ Д飡� � 꼵� ㄻ� �ㄽ�． ぎ�惟�も�
	if ( tis->lenDNF + pCon->lenRul > 65536L ) return LKE_ALLOCMEM;
	if ( (MemorySize ( (void*) tis->lpDNF ) - tis->lenDNF) < pCon->lenRul ) {
		HEAD_RUL* lpNewDNF = (HEAD_RUL*) farrealloc ( (void*) tis->lpDNF, MemorySize ( (void*) tis->lpDNF ) + pCon->lenRul + 512L );
		if ( ! lpNewDNF ) { return LKE_ALLOCMEM; }
		tis->lpDNF = lpNewDNF;
		}
	// 뮙��閃 캙э筍 α筍

	// 뫇��ⓣ�쥯筍 ��젪Œ� � 꼵� � ㏇�
	HEAD_RUL* pEndCon = (HEAD_RUL*) ( (char*) tis->lpDNF + tis->lenDNF );
	_fmemcpy ( (void*) pEndCon, (void*) pCon, pCon->lenRul );

	// 뤲Ðα殊 ���硫 ぎ�惟�も
	ReduceCon ( tis, pEndCon );
	if ( pEndCon->Const.pos != 0 && ! AbsorbLast ( tis, pEndCon, 0, 0 ) ) {
		// 궕ヮ葉筍 ��飡昔���硫 ぎ�惟�も � 꼵�
		tis->numCon++;
		tis->lenDNF += pEndCon->lenRul;
		}

	return LKE_SUCCESS;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴커
� 뤲Ðⅳ���� ��젪Œ�. 뙫┘� �㎚��ⓥ藺� ぎエ曄飡¡ �吟첓㏓쥯�Ł    쿝educeCon �
� � ㄻÞ� ��젪Œ� (僧��麟ⓥ藺�), � �젶┘ 信�젺젪エ쥯β碎 ぎ�飡젺�졓컴컴컴컴컴�
� 렎췅ぎ �� 晨ⓥ猶젰恂�, 譽� 쭬 ��젪Œ�� Мｃ� 笹ⅳ�쥯筍 ㅰ膝�� � �Ø젶ⓨ    �
� ����졿Ł � 캙э筍� �� �昔�㎖�ㄸ恂� (�.�., �誓ㄿ�쳽짛β碎, 譽� ��젪Œ�      �
� ��笹ⅳ�ⅴ). 끷エ ��젪Œ� 獸┐α手���� ⓤ殊���, 獸 ��� �� �揖�夕Ð젰恂�,    �
� � �昔飡� �� �ⅲ� 蝨젷禎恂� ㏇� �吟첓㏓쥯�⑨ � 信�젺젪エ쥯β碎 貰�手β飡㏂�-�
� �좑 ぎ�飡젺��. 룼將�с ��笹� �黍´ㄵ�⑨ ���▼�ㄸМ �昔´涉筍 ぎ�飡젺栒.    �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void ReduceCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// 뼥か �� �吟첓㏓쥯�⑨� ㄻ� 췅若┐��⑨ ぎ�飡젺瞬, �.�., Ж�º젷彛�． 쵟めº僧� � 첓┐�� 醒ゆĿ
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// 뼥か �� ぎН�����젹 收ゃ耀． �吟첓㏓쥯�⑨�
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// 뮙��閃 щ ºⅴ� ��젪Œ彛莘 ぎ�飡젺栒

	ReduceConTo ( tis, pCon );

	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
� 뤲Ðⅳ���� ぎ�惟�も� � ��¡� ぎ�飡젺收, ぎ獸�좑 ㄾウ췅 〓筍  � ReduceConTo �
� 信�젺�˙�췅. 뤲ⅳ��쳽짛β碎, 譽� �젺ⅴ ぎ�惟�も 〓� �黍´ㄵ� 읕컴컴컴컴컴컴�
� � ㅰ膝�� (‘レ蜈�) ぎ�飡젺收. 뤲� �黍´ㄵ�Ŀ � ��¡� ぎ�飡젺收 收          �
� ぎН����瞬, ぎ獸贍� ‘レ蜈 エ‘ �젪�� ⅸ �黍�젪�Ð좐恂� 1. � 誓㎯レ�졻�    �
� Мｃ� ��ャ葉筍碎 獸┐α手���� ⓤ殊��瑜 醒ゆĿ, ぎ獸贍� �揖�夕Ð좐恂�, 譽�  �
� �黍¡ㄸ� � 僧��麟��⑧ ぎエ曄飡쥯 �吟첓㏓쥯�Ł � ㄻÞ� ぎ�惟�も�.           �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void ReduceConTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// 뼥か �� �吟첓㏓쥯�⑨�
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// 뼥か �� ぎН�����젹 收ゃ耀． �吟첓㏓쥯�⑨�
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// 끷エ 收ゃ�좑 ぎН������ ‘レ蜈 エ‘ �젪췅 ぎ�飡젺收, 獸 �黍�젪�汀� ⅴ 1
			// Þ좂� �燧βⓥ�, 譽� 醒ゆ⑨ �β黍˘젷彛�
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = UNITY; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
			}

		if ( isTrivSec ) {
			// 뱾젷ⓥ� 將� 醒ゆ⑧
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			// ??????? lpDNF
			InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴커
� 뤲Ðα殊 ぎ�惟�も � 쵟めº젷彛�� 兒席� � �誓ㄲ졷ⓥ�レ�臾    � ReduceConMax �
� 췅若┐����� ぎ�飡젺瞬.                                      읕컴컴컴컴컴컴캑
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void ReduceConMax ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	COMP      ConstSec;
	BOOL      noInvert;
	int i, j, m;

	// 뼥か �� �吟첓㏓쥯�⑨� ㄻ� 췅若┐��⑨ ぎ�飡젺瞬, �.�., Ж�º젷彛�． 쵟めº僧� � 첓┐�� 醒ゆĿ
	pConProp = (HEAD_PROP*) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// 뼥か �� ぎН�����젹 收ゃ耀． �吟첓㏓쥯�⑨�
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
		pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// 뮙��閃 щ ºⅴ� ��젪Œ彛莘 ぎ�飡젺栒

	ReduceConMaxTo ( tis, pCon );

	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컴커
� 뤲Ðα殊 ぎ�惟�も � 쵟めº젷彛�� 兒席� �� ⅲ� 쭬�젺ⅴ     � ReduceConMaxTo �
� 信�젺�˙����� ぎ�飡젺收. 앪� �㎛좂젰�, 譽� ㏇� ぎН����瞬,읕컴컴컴컴컴컴컴캑
� ぎ獸贍�, ‘レ蜈 ぎ�飡젺瞬, �黍�젪�Ð좐恂� ⅸ.                              �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void ReduceConMaxTo ( KBFILE* tis, HEAD_RUL *pCon ) {
	HEAD_PROP *pConProp;
	COMP      *pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// 뼥か �� �吟첓㏓쥯�⑨�
	pConProp = (HEAD_PROP*) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// 뼥か �� ぎН�����젹 收ゃ耀． �吟첓㏓쥯�⑨�
		pConComp = (COMP*) (pConProp + 1);
		m = tis->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// 끷エ 收ゃ�좑 ぎН������ ‘レ蜈 ぎ�飡젺瞬, 獸 �黍�젪�汀� ⅴ ぎ�飡젺收
			// Þ좂� �燧βⓥ�, 譽� 醒ゆ⑨ �β黍˘젷彛�
			if ( pConComp->pos >= pCon->Const.pos ) { pConComp->pos = pCon->Const.pos; }
			else                                    { isTrivSec = FALSE; }
			pConComp = pConComp + 1;         // 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
			}

		if ( isTrivSec ) {
			// 뱾젷ⓥ� 將� 醒ゆ⑧
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			InsDelMem ( (void*) pCon, pCon->lenRul, (void*) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴컴커
� 붵�ゆ⑨ �淫젰恂� ��＋�殊筍 ぎ�惟�も pCon ぎ�惟�も젹�, ぎ獸贍Ⅳ AbsorbFirst �
� 췅若ㅿ恂� ��笹� �ⅲ�. 끷エ ぎ�惟�も ��＋��젰恂�, 獸 ��       읕컴컴컴컴컴컴�
� �揖�夕Ð젰恂�, 僧��麟젰恂� ぎエ曄飡¡ ぎ�惟�も�� � ⓨ ㄻÞ�.               �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL AbsorbFirst ( KBFILE* tis, HEAD_RUL *pEndCon, int numCon ) {
	int i;
	HEAD_RUL *pCon;

	// 뜝�졻� � numCon+1-． ぎ�惟�も�
	pCon = (HEAD_RUL*) ( (char*) pEndCon + pEndCon->lenRul );
	for ( i = numCon+1; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) break;

		// 뤲�ㄾウⓥ� �昔´夕� ��＋�耀�⑨ ㄻ� 笹ⅳ莘蟯� ぎ�惟�も��
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}
	if ( i == tis->numCon ) return FALSE;

	// 귣曄夕�呻� ぎ�惟�も
	tis->numCon--;
	tis->lenDNF -= pEndCon->lenRul;
	InsDelMem ( (void*) tis->lpDNF, tis->lenDNF + pEndCon->lenRul, (void*) pEndCon, - pEndCon->lenRul );

	return TRUE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴쩡컴컴컴컴컴커
� 붵�ゆ⑨ �淫젰恂� ��＋�殊筍 ぎ�惟�も, ぎ獸贍� 笹ⅳ濕� 쭬       � AbsorbLast �
� ��笹ⅳ�º (�.�., �� ι� �� �黍췅ㄻⅵⓥ 꼵�).                  읕컴컴컴컴컴캑
� � ヮ‘� 笹晨젰, �ⅶ젪ⓤº� �� ��＋��젰М飡�,                               �
� �①ⅲ� �� Д�畑恂�, む�Д ¡㎖�좈젰М． ㎛좂��⑨. 뤲�´夕� 췅 ��＋�耀���   �
� 췅葉췅β碎 � ぎ�惟�も� numStart, 細ι���� ㄾ ぎ獸昔． ofsStart.            �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL AbsorbLast ( KBFILE* tis, HEAD_RUL* pEndCon, int numStart, long ofsStart ) {
	int i;
	HEAD_RUL*  pCon;

	// 뜝�졻� � numStart-． ぎ�惟�も�
	pCon = (HEAD_RUL*) ( (char*) tis->lpDNF + ofsStart );
	for ( i = numStart; i < tis->numCon; i++ ) {

		if ( AbsorbCon ( tis, pCon, pEndCon ) ) return TRUE;

		// 뤲�ㄾウⓥ� �昔´夕� ��＋�耀�⑨ ㄻ� 笹ⅳ莘蟯� ぎ�惟�も��
		pCon = (HEAD_RUL*) ( (char*) pCon + pCon->lenRul );
		}

	return FALSE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴커
� 붵�ゆ⑨ �昔´涉β �誘�キ���� 信ギ˘� pFstCon |= pSecCon, 譽�   � AbsorbCon �
� �㎛좂젰� ��＋�耀��� ぎ�惟�も� pSecCon (�.�., �� �.�. �揖�夕�呻)읕컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
BOOL AbsorbCon ( KBFILE* tis, HEAD_RUL *pFstCon, HEAD_RUL *pSecCon ) {
	int i, j, k, m;
	HEAD_PROP* pFstConProp;
	HEAD_PROP* pSecConProp;
	COMP*      pFstConComp;
	COMP*      pSecConComp;

	// 꽖쳽�� �젹瑜 姉�Д��졷�瑜 �昔´夕�
	if ( pSecCon->numProp  <  pFstCon->numProp || pSecCon->Const.pos > pFstCon->Const.pos ) {
		return FALSE;
		}

	// 뮙��閃 췅ㄾ �昔´涉筍 ˙�┘���飡� �吟첓㏓쥯�Ł--αエ � pFstCon
	// α筍 若舜 〓 �ㄽ� �吟첓㏓쥯���, ぎ獸昔� �� �黍率恂手濕� �
	// pSecCon, 獸 信ギ˘� �� �誘�キ畑恂�
	pFstConProp = (HEAD_PROP*) ( pFstCon + 1 );
	for ( j = 0; j < pFstCon->numProp; j++ ) {

		// 뤲�´黍筍 率耀飡㏂β エ �吟첓㏓쥯��� � ��Д昔� pFstConProp->curAtt
		// � ��笹ⅳ��� ぎ�惟�も� pSecCon

		pSecConProp = (HEAD_PROP*) ( pSecCon + 1 );
		for ( k = 0; k < pSecCon->numProp; k++ ) {
			if ( pSecConProp->curAtt == pFstConProp->curAtt ) {

				// 뜝獄�刷 貰�手β飡㏂迹ⅴ �吟첓㏓쥯���; �昔´黍筍 ˙�┘���飡�

				// 띲┃� �昔´黍筍 ��ぎН����狩�飡�, �.�., ヮ줎�
				// ぎН������ �� pFstCon ㄾウ췅 〓筍 �� Д�麟� 曄� pSecCon
				// 끷エ � pFstCon 췅獄졹� ぎН������, ぎ獸�좑 Д�麟�
				// 貰�手β飡㏂迹ⅸ � pSecCon, 獸 信ギ˘� �� �誘�キ畑恂�,
				// � αエ �젶�¡� �� 〓ギ 췅ħ���, 獸 將�
				// �㎛좂젰� ��＋�耀���.

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

		// 룯誓⒱� � 笹ⅳ莘耀с �吟첓㏓쥯�⑧
		pFstConProp = (HEAD_PROP*) ( (char*) pFstConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal );
		}

	return TRUE;
	}

/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴커
� 맆訟ⓣⓥ� ぎ�惟�も. 롡率恂手莘蟯� � ぎ�惟�も� 醒ゆĿ           � ExtendCon �
� �黍飡紐��猶좐恂� � ぎ�ζ � ⅴ ぎН����瞬 ��ㄽº좐恂� ㄾ        읕컴컴컴컴컴�
� ぎ�飡젺瞬, ぎ獸�좑 ㄾウ췅 〓筍 信�젺�˙�췅.                                �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
void ExtendCon ( KBFILE* tis, HEAD_RUL *pCon ) {
	int i, j, m;
	HEAD_PROP* pProp;
	COMP*      pComp;

	// 뼥か �� ㏇�� ��Д�젹 �吟첓㏓쥯�Ł, �.�., �昔´涉�� 첓┐莘 醒ゆ⑧
	for ( i = 0; i < tis->numAtt; i++ ) {

		// 뤲�⒱ⓤ� �� ㏇�� �吟첓㏓쥯�⑨� ぎ�惟�も� � ��ⓤ첓� i-．
		pProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {
			if ( i == pProp->curAtt ) break;
			pProp = (HEAD_PROP*) ( (char*) pProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal );
			}
		if ( j == pCon->numProp ) {
			// 뼥か 쭬ぎ�葉メ�, �� i-�� �吟첓㏓쥯��� 췅ħ��� �� 〓ギ
			// 뜢�▼�ㄸМ ㄾ줎˘筍 ⅲ� � �젹硫 ぎ�ζ ぎ�惟�も� � Þⓩ쯄エ㎤昔쥯筍
			pProp->curAtt = i;
			pProp->Style = PROP_CONJUNCT;

			pComp = (COMP*) (pProp + 1);
			m = tis->lpAttIdx [ pProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pComp->nec = 0; pComp->pos = pCon->Const.pos;
				pComp = pComp + 1;                   // � 笹ⅳ莘耀� ぎН����收
				}
			// 뱼�エ葉筍 ㄻÞ� ぎ�惟�も� 췅 ㄻÞ� �吟첓㏓쥯�⑨
			pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pCon->numProp++;
			}
		}
	}
