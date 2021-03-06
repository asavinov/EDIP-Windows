/*
旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컫컴컴컴컴컴커
� 뤲�´飡� ギ（曄稅Ł �猶�� 췅 쩆��音 � 꼵� � ��ャ葉筍           � Inference �
� 쭬か蹟����.                                                    읕컴컴컴컴컴�
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
int Inference ( KBASE* tis ) {
	// 끷エ �β 꼵� エ‘ 걞 �㎚���췅, 獸 �琉졻� 貰�↓���� � �誓ㄻ�┬筍 ぎНŒ湞⑧
	if ( ! tis->lpHost->isDNF ) return LKE_NOTCOMPILED;

	// 귣ㄵエ筍 ���▼�ㄸс� 캙э筍
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

	// 뫇��ⓣ�쥯筍 쩆��瑜 � ▲芯� � �黍´飡� ⓨ � 쵟めº젷彛�с ˘ㅳ
	_fmemcpy ( (void*) pData, (void*) tis->lpData, tis->lpData->lenRul );
	ReduceConMax ( tis->lpHost, pData );

	// 몼昔歲筍 쭬か蹟����
	tis->lpConc->lenRul = sizeof(HEAD_RUL);
	tis->lpConc->numProp = 0;
	tis->lpConc->Const.pos = 0;                         // 뜝葉췅�� � �乘ⅱ�．
	tis->lpConc->Const.nec = 0;
	tis->lpConc->Style = RUL_DISJUNCT;


	// 뼥か �� ぎ�惟�も젹 �� 꼵�
	pRul = tis->lpHost->lpDNF;
	for ( i = 0; i < tis->lpHost->numCon; i++ ) {

		// 뫇��ⓣ�쥯筍 �曄誓ㄽ�� ぎ�惟�も � ▲芯� � �黍´飡� ⅲ� � 쵟めº젷彛�� 兒席�
		_fmemcpy ( (void*) pCon, (void*) pRul, pRul->lenRul );

		ReduceConMaxTo ( tis->lpHost, pCon );    // 뒶�飡젺�� 拾� 信�젺�˙�췅!


		// 뜝ギ┬筍 췅 �曄誓ㄽ�� ぎ�惟�も � ▲芯誓 쩆��瑜 � ��М雩� ��ぎН����狩�． Ж�º僧�
		// 뼥か �� ㏇�� �吟첓㏓쥯�⑨� 쩆��音
		pDataProp = (HEAD_PROP*) (pData + 1);
		for ( j = 0; j < pData->numProp; j++ ) {

			// 뤲�⒱ⓤ� �� ㏇�� �吟첓㏓쥯�⑨� ぎ�惟�も�
			pConProp = (HEAD_PROP*) (pCon + 1);
			for ( k = 0; k < pCon->numProp; k++ ) {
				m = pConProp->curAtt;
				if ( m == pDataProp->curAtt ) break;
				pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == pCon->numProp ) {
				// 뼥か 쭬ぎ�葉メ�, �� �拾��� �吟첓㏓쥯��� 췅ħ��� �� 〓ギ
				// 뜢�▼�ㄸМ ㄾ줎˘筍 ⅲ� � �젹硫 ぎ�ζ ぎ�惟�も� � Þⓩ쯄エ㎤昔쥯筍
				pConProp->curAtt = pDataProp->curAtt;
				pConProp->Style = PROP_CONJUNCT;

				pConComp = (COMP*) (pConProp + 1);
				m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pConComp->nec = 0; pConComp->pos = UNITY;
					pConComp = pConComp + 1;         // � 笹ⅳ莘耀� ぎН����收
					}
				// 뱼�エ葉筍 ㄻÞ� ぎ�惟�も� 췅 ㄻÞ� �吟첓㏓쥯�⑨
				pCon->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				pCon->numProp++;
				}
			// 뮙��閃 pConProp 丞젳猶젰� 췅 貰�手β飡㏂迹ⅴ �吟첓㏓쥯��� � ぎ�惟�も�

			// 귣葉笹ⓥ� ��ぎН����狩硫 Ж�º僧 �吟첓㏓쥯�Ł pConProp � pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pConComp->pos = pDataComp->pos;
				// 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal );
			}                     // 뜝ギ┬エ ��笹ⅳ�ⅴ �吟첓㏓쥯��� �� 쩆��音


		// 뤲Ðα殊 收ゃ蟯� ぎ�惟�も � 쵟めº젷彛�� 兒席� � 췅若┐����� ぎ�飡젺瞬
		ReduceConMax ( tis->lpHost, pCon );
		ExtendCon ( tis->lpHost, pCon );        // 귗�젪ⓥ� �昔�申���瑜 醒ゆĿ

		// 뜝ギ┬筍 ��ャ曄��硫 ぎ�惟�も 췅 ぎ�惟�も �猶�ㄾ� � ��М雩� ��ぎН����狩�． 쵟めº僧�
		// 뼥か �� ㏇�� �吟첓㏓쥯�⑨� ぎ�惟�も� (pDataProp - 쭬か蹟����)
		pConProp = (HEAD_PROP*) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {

			// 뤲�⒱ⓤ� �� ㏇�� �吟첓㏓쥯�⑨� 쭬か蹟��⑨
			pDataProp = (HEAD_PROP*) (tis->lpConc + 1);
			for ( k = 0; k < tis->lpConc->numProp; k++ ) {
				m = pDataProp->curAtt;
				if ( m == pConProp->curAtt ) break;
				pDataProp = (HEAD_PROP*) ( (char*) pDataProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ m ].AttRec.numVal );
				}
			if ( k == tis->lpConc->numProp ) {
				// 뼥か 쭬ぎ�葉メ�, �� �拾��� �吟첓㏓쥯��� 췅ħ��� �� 〓ギ
				// 뜢�▼�ㄸМ ㄾ줎˘筍 ⅲ� � �젹硫 ぎ�ζ 쭬か蹟��⑨ � Þⓩ쯄エ㎤昔쥯筍
				pDataProp->curAtt = pConProp->curAtt;
				pDataProp->Style = PROP_CONJUNCT;

				pDataComp = (COMP*) (pDataProp + 1);
				m = tis->lpHost->lpAttIdx [ pDataProp->curAtt ].AttRec.numVal;
				for ( k = 0; k < m; k++ ) {
					pDataComp->nec = 0; pDataComp->pos = 0;
					pDataComp = pDataComp + 1;         // � 笹ⅳ莘耀� ぎН����收
					}
				// 뱼�エ葉筍 ㄻÞ� ぎ�惟�も� 췅 ㄻÞ� �吟첓㏓쥯�⑨
				tis->lpConc->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
				tis->lpConc->numProp++;
				}
			// 뮙��閃 pDataProp 丞젳猶젰� 췅 貰�手β飡㏂迹ⅴ �吟첓㏓쥯��� � 쭬か蹟��Ŀ

			// 귣葉笹ⓥ� ��ぎН����狩硫 쵟めº僧 �吟첓㏓쥯�Ł pConProp � pDataProp
			pConComp = (COMP*) (pConProp + 1);
			pDataComp = (COMP*) (pDataProp + 1);
			m = tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( k = 0; k < m; k++ ) {
				if ( pConComp->pos > pDataComp->pos ) pDataComp->pos = pConComp->pos;
				// 룯誓⒱� � 笹ⅳ莘耀� ぎН����收
				pConComp = pConComp + 1; pDataComp = pDataComp + 1;
				}

			pConProp = (HEAD_PROP*) ( (char*) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * tis->lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
			}                 // 뜝ギ┬エ ��笹ⅳ�ⅴ �吟첓㏓쥯��� 췅 쭬か蹟����


		pRul = (HEAD_RUL*) ( (char*) pRul + pRul->lenRul );
		}

	// 뤲Ðα殊 ぎ�惟�も �猶�ㄾ� � 쵟めº젷彛�� 兒席� (� 췅若┐����� ぎ�飡젺瞬)
	ReduceConMax ( tis->lpHost, tis->lpConc );

	return TRUE;
	}

