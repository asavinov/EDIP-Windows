#include <windows.h>
#include <stdlib.h>                                                  // abs
#include <mem.h>
#include <dos.h>                                                     // FP_SEG
#include "lk.h"

/*
▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
▌▌▌ Функции компиляции ▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
*/

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Основная процедура для компиляци.                           │ CompileToCNF │
│                                                             └──────────────┤
└────────────────────────────────────────────────────────────────────────────┘
Исходные правила, состоящие из дизъюнктов (отрицательных утверждений, обозначаемых
знаком [-]) и конъюнктов (положительных утверждений, обозначаемых знаком [+])
преобразуются в КНФ, состоящую только из дизъюнктов [-], каждый из которых
приведен к минимальной форме и не следует из другого.

Т.о., основной цикл -- это цикл по исходным утверждениям. На каждом его шаге
берется очередное i-ое утверждение и включается в выходную КНФ. Процедура
включения, естественно, существенно отличается для дизъюнктов [-] и
конъюнктов [+], поэтому прежде всего определяется знак утверждения, после
чего процесс продолжается в одном из двух направлений.

Обработка (включение в выходную КНФ) дизъюнкта существенно проще обработки
конъюнкта и заключается в (i) его копировании в конец выходной КНФ,
(ii) приведении дизъюнкта, (iii) поглощении данного дизъюнкта и (iv) поглощении
предыдущих дизъюнктов данным.

Обработка конъюнкта [+] сложнее и заключается в том, что он приводится, далее
включается в каждый и уже существующих выходных дизъюнктов (они уже приведены)
с одновременным поглощением незначимых дизъюнктов (новых или уже существующих).
В результате выходная КНФ может сильно разраститсь, поэтому процедура
поглощения очень важна. После включения конъюнкта, как и после дизъюнкта,
выходная КНФ находится в правильном состоянии, т.е., состоит из приведенных
дизъюнктов, которые не следуют друг из друга.

╔═══╤══════════════════════════════╗
║ ─ │        Compiling             ║
╟───┴──────────────────────────────╢
║  Status: <Status>                ║
║                                  ║
║          Current   Total         ║
║        ┌───────────────────────┐ ║<Status><curInput><curOutput><totInput><totOutput>
║   Input│<curInput>  <totInput> │ ║
║  Output│<curOutput> <totOutput>│ ║
║        └───────────────────────┘ ║
║                     <?????????>  ║
║                                  ║
╚══════════════════════════════════╝
*/
BOOL KBFILE::CompileToCNF ( COMPILEOUT lpProcOut ) {
	int i, j;
	LPHEAD_RUL pRul, pDis;

	ResetCNF();
	if ( (*lpProcOut) ( STAT_GENERATION, 0, 0, numRul, 0 ) ) { ResetCNF(); return FALSE; }

	for ( i = 0, pRul = lpRul; i < numRul; i++ ) {

		if ( (*lpProcOut) ( STAT_GENERATION, i, 0, numRul, numDis ) ) { ResetCNF(); return FALSE; }

		if ( pRul->Style & RUL_DISJUNCT ) { // Обработать текущий конъюнкт [+]
			// Последовательно включаем конъюнкт во все дизъюнкты из КНФ
			for ( j = 0; j < numDis; j++ ) {
				if ( (*lpProcOut) ( STAT_GENERATION, i, j, numRul, numDis ) ) { ResetCNF(); return FALSE; }

				; // Включить конъюнкт i в дизъюнкт j

				}
			}
		else {                              // Обработать текущий дизъюнкт [-]
			// (i) Скопировать на место
			// (ii) Привести к минимальной форме (вычислить константу и т.п.
			// (iii) Поглотить данный дизъюнкт
			DisIntoDis ( pRul );
			}

		// Перейти к следующему правилу
		pRul = (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
		}

	if ( (*lpProcOut) ( STAT_ABSORPTION, numRul, numDis, numRul, numDis ) ) { ResetCNF(); return FALSE; }

	isCNF = TRUE;
	isChanged = TRUE;
	return TRUE;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Отрицательное утверждение pDis включается в КНФ путем простого│ DisIntoDis │
│ добавления его в конец КНФ.                                   └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::DisIntoDis ( LPHEAD_RUL pDis ) {

	// Проверить хватит ли места в КНФ для еще одного дизъюнкта
	if ( lenCNF + pDis->lenRul > 65536L ) return LKE_ALLOCMEM;
	if ( ( MemorySize ( (LPVOID) lpCNF ) - lenCNF ) < pDis->lenRul ) {
		LPHEAD_RUL lpNewCNF = (LPHEAD_RUL) LK_realloc ( (LPVOID) lpCNF, MemorySize ( (LPVOID) lpCNF ) + pDis->lenRul + 512L );
		if ( ! lpNewCNF ) { return LKE_ALLOCMEM; }
		lpCNF = lpNewCNF;
		}
	// Теперь память есть

	// Скопировать правило в КНФ и все
	LPHEAD_RUL pEndDis = (LPHEAD_RUL) ( (LPSTR) lpCNF + lenCNF );
	_fmemcpy ( (LPVOID) pEndDis, (LPVOID) pDis, pDis->lenRul );

	// Привести новый дизъюнкт
	ReduceDis ( pEndDis );
	if ( pEndDis->Const.pos != UNITY && ! AbsorbLast ( pEndDis, 0, 0 ) ) {
		// Включить построенный конъюнкт в ДНФ
		numDis++;
		lenCNF += pEndDis->lenRul;
		}

	return LKE_SUCCESS;
	}

/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Приведение правила. Может измениться количество высказываний    │ReduceDis │
│ и длина правила (уменьшиться), а также устанавливается константа└──────────┤
│ Однако не учитывается, что за правилом могут следовать другие и никаких    │
│ операций с памятью не производится (т.е., предполагается, что правило      │
│ последнее). Если правило тождественно истинно, то оно не вычеркивается,    │
│ а просто из него удаляются все высказывания и устанавливается соответствую-│
│ щая константа. Поэтому после приведения необходимо проверять константу.    │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ReduceDis ( LPHEAD_RUL pDis ) {
	LPHEAD_PROP pDisProp;
	LPCOMP      pDisComp;
	COMP        ConstSec;
	BOOL        Invert;
	int i, j, m;

	// Цикл по высказываниям для нахождения константы, т.е., максимального минимума в каждой секции
	// Начинаем со следующего за заголовком высказывания и нулевой константы
	pDisProp = (LPHEAD_PROP) (pDis + 1);
	pDis->Const.pos = 0;
	for ( i = 0; i < pDis->numProp; i++ ) {
		Invert = pDisProp->Style & PROP_CONJUNCT;
		if ( Invert ) pDisProp->Style &= ~ PROP_CONJUNCT;

		// Цикл по компонентам текущего высказываниям для нахождения минимальной
		// Начинаем с нулевой компоненты
		pDisComp = (LPCOMP) (pDisProp + 1);
		m = lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;
		ConstSec.pos = UNITY;
		for ( j = 0; j < m; j++ ) {
			if ( Invert ) pDisComp->pos = UNITY - pDisComp->pos;
			if ( ConstSec.pos > pDisComp->pos ) ConstSec.pos = pDisComp->pos;
			pDisComp = pDisComp + 1;         // Перейти к следующей компоненте
			}

		if ( pDis->Const.pos < ConstSec.pos ) pDis->Const.pos = ConstSec.pos;

		// Перейти к следующему высказыванию
		pDisProp = (LPHEAD_PROP) pDisComp;
//		pDisProp = (LPHEAD_PROP) ( (LPSTR) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// Теперь мы имеем правильную константу

	ReduceDisTo ( pDis );

	return;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Приведение дизъюнкта к новой константе, которая должна быть  │ ReduceDisTo │
│ установлена. Предполагается, что ранее дизъюнкт был приведен └─────────────┤
│ к другой (меньшей) константе. При приведении к новой константе те          │
│ компоненты, которые меньше либо равны ей приравниваются 0. В результате    │
│ могут получиться тождественно ложные секции, которые вычеркиваются, что    │
│ приводит к уменьшению количества высказываний и длины дизъюнкта.           │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ReduceDisTo ( LPHEAD_RUL pDis ) {
	LPHEAD_PROP pDisProp;
	LPCOMP      pDisComp;
	int i, j, m;
	BOOL isTrivSec;

	// Цикл по высказываниям
	pDisProp = (LPHEAD_PROP) (pDis + 1);
	for ( i = 0; i < pDis->numProp; ) {

		// Цикл по компонентам текущего высказываниям
		pDisComp = (LPCOMP) (pDisProp + 1);
		m = lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// Если текущая компонента меньше либо равна константе, то приравнять ее 0
			// иначе отметить, что секция нетривиальна
			if ( pDisComp->pos <= pDis->Const.pos ) { pDisComp->pos = 0; }
			else                                    { isTrivSec = FALSE; }
			pDisComp = pDisComp + 1;         // Перейти к следующей компоненте
			}

		if ( isTrivSec ) {
			// Удалить эту секцию
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			InsDelMem ( (LPVOID) pDis, pDis->lenRul, (LPVOID) pDisProp, -sz );
			pDis->numProp--; pDis->lenRul -= sz;
			}
		else {
			// Перейти к следующему высказыванию
			pDisProp = (LPHEAD_PROP) pDisComp;
//			pDisProp = (LPHEAD_PROP) ( (LPSTR) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Привести конъюнкт к максимальной форме с предварительным    │ ReduceConMax │
│ нахождением константы. Константа находится как обычно для   └──────────────┤
│ конъюнктов, т.е., как минимальный максимум в каждой секции. Далее все      │
│ компоненты, которые больше константы, приравниваются ей. Если имеются      │
│ тривиальные (тождественно равные константе) секции, то они вычеркиваются.  │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ReduceConMax ( LPHEAD_RUL pCon ) {
	LPHEAD_PROP pConProp;
	LPCOMP      pConComp;
	COMP        ConstSec;
	BOOL        noInvert;
	int i, j, m;

	// Цикл по высказываниям для нахождения константы, т.е., минимального максимума в каждой секции
	pConProp = (LPHEAD_PROP) (pCon + 1);
	pCon->Const.pos = UNITY;
	for ( i = 0; i < pCon->numProp; i++ ) {
		noInvert = pConProp->Style & PROP_CONJUNCT;
		if ( ! noInvert ) pConProp->Style |= PROP_CONJUNCT;

		// Цикл по компонентам текущего высказываниям
		pConComp = (LPCOMP) (pConProp + 1);
		m = lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		ConstSec.pos = 0;
		for ( j = 0; j < m; j++ ) {
			if ( ! noInvert ) pConComp->pos = UNITY - pConComp->pos;
			if ( ConstSec.pos < pConComp->pos ) ConstSec.pos = pConComp->pos;
			pConComp = pConComp + 1;         // Перейти к следующей компоненте
			}

		if ( pCon->Const.pos > ConstSec.pos ) pCon->Const.pos = ConstSec.pos;

		// Перейти к следующему высказыванию
		pConProp = (LPHEAD_PROP) pConComp;
//		pConProp = (LPHEAD_PROP) ( (LPSTR) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
		}
	// Теперь мы имеем правильную константу

	ReduceConMaxTo ( pCon );

	return;
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Привести конъюнкт к максимальной форме по его заранее     │ ReduceConMaxTo │
│ установленной константе. Это означает, что все компоненты,└────────────────┤
│ которые, больше константы, приравниваются ей (поджимаются сверху),         │
│ а тривиальные секции вычеркиваются.                                        │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ReduceConMaxTo ( LPHEAD_RUL pCon ) {
	LPHEAD_PROP pConProp;
	LPCOMP      pConComp;
	int i, j, m;
	BOOL isTrivSec;

	// Цикл по высказываниям
	pConProp = (LPHEAD_PROP) (pCon + 1);
	for ( i = 0; i < pCon->numProp; ) {

		// Цикл по компонентам текущего высказывания
		pConComp = (LPCOMP) (pConProp + 1);
		m = lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
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
			InsDelMem ( (LPVOID) pCon, pCon->lenRul, (LPVOID) pConProp, -sz );
			pCon->numProp--; pCon->lenRul -= sz;
			}
		else {
			// Перейти к следующему высказыванию
			pConProp = (LPHEAD_PROP) pConComp;
//			pConProp = (LPHEAD_PROP) ( (LPSTR) pConProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Привести дизъюнкт к максимальной форме по его заранее     │ ReduceDisMaxTo │
│ установленной константе. Это означает, что все компоненты,└────────────────┤
│ которые, меньше константы, приравниваются ей (поджимаются снизу),          │
│ а тривиальные секции вычеркиваются.                                        │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ReduceDisMaxTo ( LPHEAD_RUL pDis ) {
	LPHEAD_PROP pDisProp;
	LPCOMP      pDisComp;
	int i, j, m;
	BOOL isTrivSec;

	// Цикл по высказываниям
	pDisProp = (LPHEAD_PROP) (pDis + 1);
	for ( i = 0; i < pDis->numProp; ) {

		// Цикл по компонентам текущего высказывания
		pDisComp = (LPCOMP) (pDisProp + 1);
		m = lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;
		isTrivSec = TRUE;
		for ( j = 0; j < m; j++ ) {
			// Если текущая компонента меньше либо равна константы, то приравнять
			// ее константе, иначе отметить, что секция нетривиальна
			if ( pDisComp->pos <= pDis->Const.pos ) { pDisComp->pos = pDis->Const.pos; }
			else                                    { isTrivSec = FALSE; }
			pDisComp = pDisComp + 1;         // Перейти к следующей компоненте
			}

		if ( isTrivSec ) {
			// Удалить эту секцию
			int sz = sizeof(HEAD_PROP) + m * sizeof(COMP);
			InsDelMem ( (LPVOID) pDis, pDis->lenRul, (LPVOID) pDisProp, -sz );
			pDis->numProp--; pDis->lenRul -= sz;
			}
		else {
			// Перейти к следующему высказыванию
			pDisProp = (LPHEAD_PROP) pDisComp;
//			pDisProp = (LPHEAD_PROP) ( (LPSTR) pDisProp + sizeof(HEAD_PROP) + m * sizeof(COMP) );
			i++;
			}
		}

	return;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Расширить конъюнкт. Отсутствующие в конъюнкте секции           │ ExtendCon │
│ пристыковываются в конец и ее компоненты приравниваются        └───────────┤
│ константе, которая должна быть установлена.                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ExtendCon ( LPHEAD_RUL pCon ) {
	int i, j, m;
	LPHEAD_PROP pProp;
	LPCOMP      pComp;

	// Цикл по всем номерам высказываний, т.е., проверяем каждую секцию
	for ( i = 0; i < numAtt; i++ ) {

		// Пройтись по всем высказываниям конъюнкта в поисках i-го
		pProp = (LPHEAD_PROP) (pCon + 1);
		for ( j = 0; j < pCon->numProp; j++ ) {
			if ( i == pProp->curAtt ) break;
			pProp = (LPHEAD_PROP) ( (LPSTR) pProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pProp->curAtt ].AttRec.numVal );
			}
		if ( j == pCon->numProp ) {
			// Цикл закончился, но i-ое высказывание найдено не было
			// Необходимо добавить его в самый конец конъюнкта и инициализировать
			pProp->curAtt = i;
			pProp->Style = PROP_CONJUNCT;

			pComp = (LPCOMP) (pProp + 1);
			m = lpAttIdx [ pProp->curAtt ].AttRec.numVal;
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

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Расширить дизъюнкт. Отсутствующие в дизъюнкте секции           │ ExtendDis │
│ пристыковываются в конец и ее компоненты приравниваются        └───────────┤
│ константе, которая должна быть установлена.                                │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBFILE::ExtendDis ( LPHEAD_RUL pDis ) {
	int i, j, m;
	LPHEAD_PROP pProp;
	LPCOMP      pComp;

	// Цикл по всем номерам высказываний, т.е., проверяем каждую секцию
	for ( i = 0; i < numAtt; i++ ) {

		// Пройтись по всем высказываниям дизъюнкта в поисках i-го
		pProp = (LPHEAD_PROP) (pDis + 1);
		for ( j = 0; j < pDis->numProp; j++ ) {
			if ( i == pProp->curAtt ) break;
			pProp = (LPHEAD_PROP) ( (LPSTR) pProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pProp->curAtt ].AttRec.numVal );
			}
		if ( j == pDis->numProp ) {
			// Цикл закончился, но i-ое высказывание найдено не было
			// Необходимо добавить его в самый конец дизъюнкта и инициализировать
			pProp->curAtt = i;
			pProp->Style = 0;

			pComp = (LPCOMP) (pProp + 1);
			m = lpAttIdx [ pProp->curAtt ].AttRec.numVal;
			for ( j = 0; j < m; j++ ) {
				pComp->nec = 0; pComp->pos = pDis->Const.pos;
				pComp = pComp + 1;                   // К следующей компоненте
				}
			// Увеличить длину дизъюнкта на длину высказывания
			pDis->lenRul += sizeof(HEAD_PROP) + sizeof(COMP) * m;
			pDis->numProp++;
			}
		}
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Функция пытается поглотить дизъюнкт, который следует за       │ AbsorbLast │
│ последним (т.е., он еще не принадлежит КНФ).                  └────────────┤
│ В любом случае, независимо от поглощаемости,                               │
│ ничего не меняется, кроме возвращаемого значения. Проверка на поглощение   │
│ начинается с дизъюнкта numStart, смещение до которого ofsStart.            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::AbsorbLast ( LPHEAD_RUL pEndDis, int numStart, long ofsStart ) {
	int i;
	LPHEAD_RUL  pDis;

	// Начать с numStart-го конъюнкта
	pDis = (LPHEAD_RUL) ( (LPSTR) lpCNF + ofsStart );
	for ( i = numStart; i < numDis; i++ ) {

		if ( AbsorbDis ( pDis, pEndDis ) ) return TRUE;

		// Продолжить проверку поглощения для следующих конъюнктов
		pDis = (LPHEAD_RUL) ( (LPSTR) pDis + pDis->lenRul );
		}

	return FALSE;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Функция проверяет выполнение условия pFstDis |= pSecDis, что   │ AbsorbDis │
│ означает поглощение дизъюнкта pSecDis (т.е., он м.б. вычеркнут)└───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::AbsorbDis ( LPHEAD_RUL pFstDis, LPHEAD_RUL pSecDis ) {
	int i, j, k, m;
	LPHEAD_PROP pFstDisProp;
	LPHEAD_PROP pSecDisProp;
	LPCOMP      pFstDisComp;
	LPCOMP      pSecDisComp;

	// Делаем самые элементарные проверки
	if ( pSecDis->numProp  <  pFstDis->numProp || pSecDis->Const.pos < pFstDis->Const.pos ) {
		return FALSE;
		}

	// Теперь надо проверять вложенность высказываний -- если в pFstDis
	// есть хотя бы одно высказывание, которое не присутствует в
	// pSecDis, то условие не выполняется
	pFstDisProp = (LPHEAD_PROP) ( pFstDis + 1 );
	for ( j = 0; j < pFstDis->numProp; j++ ) {

		// Проверить существует ли высказывание с номером pFstDisProp->curAtt
		// в последнем дизъюнкте pSecDis

		pSecDisProp = (LPHEAD_PROP) ( pSecDis + 1 );
		for ( k = 0; k < pSecDis->numProp; k++ ) {
			if ( pSecDisProp->curAtt == pFstDisProp->curAtt ) {

				// Нашлось соответствующее высказывание; проверить вложенность

				// Нужно проверить покомпонентность, т.е., любая
				// компонента из pFstDis должна быть не больше чем в pSecDis
				// Если в pFstDis нашлась компонента, которая больше
				// соответствующей в pSecDis, то условие не выполняется,
				// а если таковой не было найдено, то это
				// означает поглощение.

				pFstDisComp = (LPCOMP) ( pFstDisProp + 1 );
				pSecDisComp = (LPCOMP) ( pSecDisProp + 1 );
				m = lpAttIdx [ pFstDisProp->curAtt ].AttRec.numVal;
				for ( i = 0; i < m; i++ ) {
					if ( pFstDisComp->pos > pSecDisComp->pos ) return FALSE;
					pFstDisComp = pFstDisComp + 1; pSecDisComp = pSecDisComp + 1;
					}
				break;
				}
			pSecDisProp = (LPHEAD_PROP) ( (LPSTR) pSecDisProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pSecDisProp->curAtt ].AttRec.numVal );
			}
		if ( k == pSecDis->numProp ) { return FALSE; }

		// Перейти к следующему высказыванию
		pFstDisProp = (LPHEAD_PROP) ( (LPSTR) pFstDisProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pFstDisProp->curAtt ].AttRec.numVal );
		}

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Функция проверяет выполнение условия pFstCon |= pSecCon, что│ AbsorbConMax │
│ означает поглощение конъюнкта pSecCon.                      └──────────────┤
│ Предполагается, что в обоих конъюнктах есть все секции, возможно           │
│ тривиальные, т.е. они приведены к максимальной форме.                      │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::AbsorbConMax ( LPHEAD_RUL pFstCon, LPHEAD_RUL pSecCon ) {
	int i, j, k, m;
	LPHEAD_PROP pFstConProp;
	LPHEAD_PROP pSecConProp;
	LPCOMP      pFstConComp;
	LPCOMP      pSecConComp;

	// Делаем самые элементарные проверки
	if ( pSecCon->Const.pos < pFstCon->Const.pos ) return FALSE;

	// Теперь надо проверять вложенность высказываний
	pFstConProp = (LPHEAD_PROP) ( pFstCon + 1 );
	for ( j = 0; j < pFstCon->numProp; j++ ) {

		// Найти во втором конъюнкте высказывание с номером pFstConProp->curAtt

		pSecConProp = (LPHEAD_PROP) ( pSecCon + 1 );
		for ( k = 0; k < pSecCon->numProp; k++ ) {
			if ( pSecConProp->curAtt == pFstConProp->curAtt ) {

				// Нашлось соответствующее высказывание; проверить вложенность

				// Нужно проверить покомпонентность, т.е., любая
				// компонента из pFstCon должна быть не больше чем в pSecCon
				// Если в pFstCon нашлась компонента, которая больше
				// соответствующей в pSecCon, то условие не выполняется,
				// а если таковой не было найдено, то это
				// означает поглощение.

				pFstConComp = (LPCOMP) ( pFstConProp + 1 );
				pSecConComp = (LPCOMP) ( pSecConProp + 1 );
				m = lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal;
				for ( i = 0; i < m; i++ ) {
					if ( pFstConComp->pos > pSecConComp->pos ) return FALSE;
					pFstConComp = pFstConComp + 1; pSecConComp = pSecConComp + 1;
					}
				break;
				}
			pSecConProp = (LPHEAD_PROP) ( (LPSTR) pSecConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pSecConProp->curAtt ].AttRec.numVal );
			}

		// Этого не может быть, т.к. означает что какого-то высказывания нет
		if ( k == pSecCon->numProp ) { return FALSE; }

		// Перейти к следующему высказыванию в первом конъюнкте
		pFstConProp = (LPHEAD_PROP) ( (LPSTR) pFstConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal );
		}

	return TRUE;
	}

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Функция накладывает второй конъюнкт на первый с помощью     │ ImposeConMax │
│ операции покомпонентного И. Другими словами, вычисляется    └──────────────┤
│ покомпонентная конъюнкция двух конъюнктов, а результат копируется в первый │
│ из них. Конъюнкты должны быть в максимальной форме. Возвращается TRUE если │
│ первый конъюнкт был изменен в результате операции или, что то же самое,    │
│ если второй следует из первого. Второй конъюнкт м.б. не полным.            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::ImposeConMax ( LPHEAD_RUL pFstCon, LPHEAD_RUL pSecCon ) {
	int i, j, k, m;
	LPHEAD_PROP pFstConProp;
	LPHEAD_PROP pSecConProp;
	LPCOMP      pFstConComp;
	LPCOMP      pSecConComp;
	BOOL        isAbsorbed = TRUE;

	// Пройтись по всем высказываниям (j) первого конъюнкта и на каждый наложить
	// соответствующее высказывание (k) из второго конъюнкта
	pFstConProp = (LPHEAD_PROP) ( pFstCon + 1 );
	for ( j = 0; j < pFstCon->numProp; j++ ) {

		// Найти во втором конъюнкте высказывание с номером pFstConProp->curAtt

		pSecConProp = (LPHEAD_PROP) ( pSecCon + 1 );
		for ( k = 0; k < pSecCon->numProp; k++ ) {
			if ( pSecConProp->curAtt == pFstConProp->curAtt ) {

				// Нашлось соответствующее высказывание; наложить k-е на j-е

				// Если в pFstCon нашлась компонента, которая больше
				// соответствующей в pSecCon, то приравнять ее к pSecCon,
				// а если таковой не было найдено, то ничего не делать

				pFstConComp = (LPCOMP) ( pFstConProp + 1 );
				pSecConComp = (LPCOMP) ( pSecConProp + 1 );
				m = lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal;
				for ( i = 0; i < m; i++ ) {
					if ( pFstConComp->pos > pSecConComp->pos ) {
						pFstConComp->pos = pSecConComp->pos;
						isAbsorbed = FALSE;
						}
					pFstConComp = pFstConComp + 1; pSecConComp = pSecConComp + 1;
					}
				break;
				}
			pSecConProp = (LPHEAD_PROP) ( (LPSTR) pSecConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pSecConProp->curAtt ].AttRec.numVal );
			}

		// Если какого-то высказывания нет во втором конъюнкте (цикл прошел
		// до конца безрезультатно), то просто считать его константным, т.е.,
		// наложить константу на секцию первого конъюнкта
		if ( k == pSecCon->numProp ) {
			pFstConComp = (LPCOMP) ( pFstConProp + 1 );
			m = lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal;
			for ( i = 0; i < m; i++ ) {
				if ( pFstConComp->pos > pSecCon->Const.pos ) {
					pFstConComp->pos = pSecCon->Const.pos;
					isAbsorbed = FALSE;
					}
				pFstConComp = pFstConComp + 1;
				}
			}

		// Перейти к следующему высказыванию в первом конъюнкте
		pFstConProp = (LPHEAD_PROP) ( (LPSTR) pFstConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pFstConProp->curAtt ].AttRec.numVal );
		}

	return isAbsorbed;
	}

/*
┌───────────────────────────────────────────────────────────┬────────────────┐
│ Наложить конъюнкт на дизъюнкт (покомпонентное И). Оба     │ ImposeConOnDis │
│ находятся в максимальной форме. Возвращается TRUE, если   └────────────────┤
│ дизъюнкт хоть немного поменялся, иначе FALSE.                              │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::ImposeConOnDis ( LPHEAD_RUL pDis, LPHEAD_RUL pCon ) {

	// Пройтись в цикле по всем секциям дизъюнкта и для каждой найти
	// соответствующую ей секцию в конъюнкте (она должна быть, поскольку
	// конъюнкт находится в максимальной форме). В цикле по компонентам
	// наложить одну секцию на другую (конъюнкт на дизъюнкт).

	int i, j, k, m;
	LPHEAD_PROP pDisProp;
	LPHEAD_PROP pConProp;
	LPCOMP      pDisComp;
	LPCOMP      pConComp;
	BOOL        isChanged = FALSE;

	// Пройтись по всем высказываниям (j) дизъюнкта и на каждый наложить
	// соответствующее высказывание (k) из конъюнкта
	pDisProp = (LPHEAD_PROP) ( pDis + 1 );
	for ( j = 0; j < pDis->numProp; j++ ) {

		// Найти в конъюнкте высказывание с номером pDisProp->curAtt

		pConProp = (LPHEAD_PROP) ( pCon + 1 );
		for ( k = 0; k < pCon->numProp; k++ ) {
			if ( pConProp->curAtt == pDisProp->curAtt ) {

				// Нашлось соответствующее высказывание; наложить k-е на j-е

				// Если в pDis нашлась компонента, которая больше
				// соответствующей в pCon, то приравнять ее к pCon,
				// а если таковой не было найдено, то ничего не делать

				pDisComp = (LPCOMP) ( pDisProp + 1 );
				pConComp = (LPCOMP) ( pConProp + 1 );
				m = lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;
				for ( i = 0; i < m; i++ ) {
					if ( pDisComp->pos > pConComp->pos ) {
						pDisComp->pos = pConComp->pos;
						isChanged = TRUE;
						}
					pDisComp = pDisComp + 1; pConComp = pConComp + 1;
					}
				break;
				}
			pConProp = (LPHEAD_PROP) ( (LPSTR) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
			}

		// Этого не может быть, т.к. означает что какого-то высказывания нет
		if ( k == pCon->numProp ) { return FALSE; }

		// Перейти к следующему высказыванию в дизъюнкте
		pDisProp = (LPHEAD_PROP) ( (LPSTR) pDisProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pDisProp->curAtt ].AttRec.numVal );
		}

	return isChanged;
	}

/*
┌────────────────────────────────────────────────────┬───────────────────────┐
│ Наложить спроецированный дизъюнкт, содержащий не   │ ImposeProjectDisOnCon │
│ больее одной секции на конъюнкт, находящийся в     └───────────────────────┤
│ произвольной форме, но содержащий все секции. Если конъюнкт поменялся, то  │
│ возвращается TRUE, иначе FALSE.                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
BOOL KBFILE::ImposeProjectDisOnCon ( LPHEAD_RUL pCon, LPHEAD_RUL pDis ) {
/*
Если дизъюнкт не содержит секций, то он константен, поэтому, необходимо
взять первую попавшуюся секцию в конъюнкте и прижать ее сверху константой.

Если дизъюнкт содержит секцию, то надо найти такую же в конъюнкте и наложить
одну на другую учитывая константу дизъюнкта.
*/
	if ( pDis->Const.pos == UNITY ) return FALSE;

	int i, j, m;
	LPHEAD_PROP pConProp;
	LPHEAD_PROP pDisProp;
	LPCOMP      pConComp;
	LPCOMP      pDisComp;
	BOOL        isChanged = FALSE;

	if ( pDis->numProp == 0 ) {

		pConProp = (LPHEAD_PROP) ( pCon + 1 );          // Первое высказывание

		pConComp = (LPCOMP) ( pConProp + 1 );
		m = lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
		for ( i = 0; i < m; i++ ) {
			if ( pConComp->pos > pDis->Const.pos ) {
				pConComp->pos = pDis->Const.pos;
				isChanged = TRUE;
				}
			pConComp = pConComp + 1;
			}

		return isChanged;
		}

	// Иначе дизъюнкт содержит одну секцию

	pDisProp = (LPHEAD_PROP) ( pDis + 1 );

	// Найти в конъюнкте высказывание с номером pDisProp->curAtt

	pConProp = (LPHEAD_PROP) ( pCon + 1 );
	for ( j = 0; j < pCon->numProp; j++ ) {
		if ( pConProp->curAtt == pDisProp->curAtt ) {

			// Нашлось соответствующее высказывание: наложить на него
			// секцию дизъюнкта

			// Если в pCon нашлась компонента, которая больше
			// соответствующей в pDis, то приравнять ее к pDis,
			// а если таковой не было найдено, то ничего не делать

			pDisComp = (LPCOMP) ( pDisProp + 1 );
			pConComp = (LPCOMP) ( pConProp + 1 );
			m = lpAttIdx [ pConProp->curAtt ].AttRec.numVal;
			for ( i = 0; i < m; i++ ) {
				if ( pConComp->pos > pDisComp->pos ) {
					pConComp->pos = pDisComp->pos;
					isChanged = TRUE;
					}
				pDisComp = pDisComp + 1; pConComp = pConComp + 1;
				}

			// Прервать поиск в конъюнкте (уже нашли и все сделали)
			break;
			}

		pConProp = (LPHEAD_PROP) ( (LPSTR) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
		}

	return isChanged;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Весь дизъюнкт проецируется на секцию с максимальной           │ ProjectDis │
│ компонентой. Возвращается номер этой секции либо -1 если      └────────────┤
│ она константна (максимальная компонента равна константе).                  │
│ Устанавливается константа дизъюнкта. Полученный дизъюнкт состоит из одной  │
│ секции либо вообще без секций.                                             │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBFILE::ProjectDis ( LPHEAD_RUL pDis, LPCOMP pMaxComp ) {
/*
Для проецирования дизъюнкта необходимо (i) найти секцию, содержащую максимальную
компоненту, (ii) среди остальных секций найти значение максимальной компоненты
и приравнять ей константу, (iii) обнулить все секции кроме найденной в п.(i),
(iv) привести дизъюнкт к минимальной форме, (v) вернуть значение максимальной
компоненты. Отметим, что если возвращаемая максимальная компонента меньше 1,
то необходимо уменьшить константу конъюнкта выводов до этого уровня и,
возможно, привести конъюнкт. Т.е., фактически, это уровень непротиворечивости
при срабатывании дизъюнкта, который действует на весь конъюнкт выводов. Выходной
дизъюнкт может не содержать ни одной секции (из-за приведения) если полученная
проекция константна. В этом случае возвращаемая максимальная компонента равна
константе. Если при этом константа равна 1, то дизъюнкт не сработал.

Константа дизъюнкта не обязательно должна быть правильно установлена, поэтому
мы ее не используем, а сразу вычисляем новую константу, равную второму найденному
максимуму.
*/

	int i, j, m, numMaxSection;
	LPHEAD_PROP pDisProp;
	LPCOMP      pDisComp;
	COMP        SectionMaxComp;

	// Найти секцию с максимальной компонентой и запомнить вторую максимальную компоненту (в другой секции).
	// Сперва приравняем максимальную компоненту константе, а если найдутся
	// большие, то потом увеличим. Вторая максимальная компонента сразу хранится
	// там где нужно -- на месте константы дизъюнкта.
	pMaxComp->pos = pDis->Const.pos = 0;
	// Цикл по секциям дизъюнкта
	pDisProp = (LPHEAD_PROP) ( pDis + 1 );
	for ( j = 0; j < pDis->numProp; j++ ) {

		// Для очередной секции найти максимальную компоненту
		SectionMaxComp.pos = 0;
		pDisComp = (LPCOMP) ( pDisProp + 1 );
		m = lpAttIdx [ pDisProp->curAtt ].AttRec.numVal;
		for ( i = 0; i < m; i++ ) {
			if ( pDisComp->pos > SectionMaxComp.pos ) {
				SectionMaxComp.pos = pDisComp->pos;
				}
			pDisComp = pDisComp + 1;
			}

		// Теперь мы знаем максимальное значение в этой секции и
		// необходимо установить максимальную компоненту дизъюнкта и константу

		// Во-первых, если новый максимум больше либо равен текущего, то
		// сделать его текущим, а константу приравнять старому
		if ( SectionMaxComp.pos >= pMaxComp->pos ) {
			pDis->Const.pos = pMaxComp->pos;
			pMaxComp->pos = SectionMaxComp.pos;
			numMaxSection = pDisProp->curAtt;
			}
		// Иначе если новый максимум больше текущей константы (и, как выяснилось
		// раньше, меньше текущего максимума), то установить новую текущую константу
		else if ( SectionMaxComp.pos > pDis->Const.pos ) {
			pDis->Const.pos = SectionMaxComp.pos;
			}

		// Перейти к следующему высказыванию в дизъюнкте
		pDisProp = (LPHEAD_PROP) ( (LPSTR) pDisProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpAttIdx [ pDisProp->curAtt ].AttRec.numVal );
		}

	// Если максимальная компонента равна константе, то проекция получается
	// константной, поэтому легче вручную сформировать дизъюнкт, не содержащий
	// ни одной секции.
	if ( pMaxComp->pos == pDis->Const.pos ) {
		pDis->numProp = 0;
		return -1;
		}

	// Иначе константа строго меньше максимума дизъюнкта, поэтому необходимо
	// привести дизъюнкт к максимальной форме. Она будет содержать ровно
	// одну секцию, поскольку все остальные не имеют компонент больших
	// нами же установленной константы и поэтому будут вычеркнуты как
	// тривиальные
	ReduceDisMaxTo ( pDis );
	return numMaxSection;
	}

/*
▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
▌▌▌ Функции логического вывода ▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌
*/

/*
┌─────────────────────────────────────────────────────────────┬──────────────┐
│ Провести логический вывод на данных и получить заключение.  │ InferenceCNF │
│ Для вывода используется КНФ (вывод на дизъюнктах).          └──────────────┤
│ Вывод проводится с помощью нескольких проходов по данным и заключениям     │
│ с занесением результатов в заключения. Процедура останавливается когда     │
│ новых заключений получить уже не удается. В заключения записываются только │
│ те высказывания, которые присутствуют в окне заключений.                   │
└────────────────────────────────────────────────────────────────────────────┘
*/
/*
Процедура вывода заключается в том, что последовательно проводятся проходы
по дизъюнктам. На каждом проходе берется исходный конъюнкт данных, который
содержит все высказывания и находится в максимальной форме, т.е., все его
компоненты минимальны. В процессе логического вывода на всех дизъюнктах
(один проход) полученная информация (полученные в результате срабатывания
дизъюнктов ограничения на атрибуты) записывается в выходной конъюнкт, который
также содержит все высказывания и и находится в максимальной форме.

После каждого прохода выходной конъюнкт приводится, т.е., вычисляется
константа (она может уменьшиться) и если необходимо, то те компоненты, которые
больше ее приравниваются ей. Далее делается проверка, не следуют ли полученные
заключения из данных (следование в смысле дизъюнктов, т.е., фактически,
покомпонентное сравнение). Если заключения следуют из данных, т.е., все компоненты
заключений больше либо равны соответствующих компонент данных, то процедура
вывода прерывается, т.к. это означает, что найдено все что можно. Если
заключения не следуют из данных, т.е., имеется хотя бы одна компонента в
заключениях, которая строго меньше соответствующей компоненты в данных,
то заключения накладываются на данные, т.е., фактически находится их
покомпонентный минимум и начинается новый проход с новыми данными.

По окончании процедуры вывода заключения переносятся в высказывания,
содержащиеся в окне выводов. В конце всем окнам с видом на заключение
передается сообщение об изменении содержания.
*/
int KBASE::InferenceCNF ( VOID ) {

	// Если нет ДНФ либо БЗ изменена, то выдать сообщение и предложить компиляцию
	if ( ! lpHost->isCNF ) return LKE_NOTCOMPILED;
	if ( lpHost->lpConc->numProp == 0 ) return LKE_NOGOAL;

	// Размер полного вектора
	long sz = sizeof(HEAD_RUL) + lpHost->numAtt * sizeof(HEAD_PROP) + lpHost->numAllVal * sizeof(COMP);

	// Выделить память
	LPHEAD_RUL pData = (LPHEAD_RUL) LK_malloc ( sz );         // Вектор данных
	LPHEAD_RUL pConc = (LPHEAD_RUL) LK_malloc ( sz );     // Вектор заключений
	LPHEAD_RUL pDis  = (LPHEAD_RUL) LK_malloc ( sz );   // Буфер для дизъюнкта
	if ( ! pData || ! pConc || ! pDis ) return LKE_ALLOCMEM;

	// Инициализировать вектор данных
	// Скопировать данные в буфер и привести их к максимальному виду (расширить)
	// Одновременно инвертируются посылки в данных
	_fmemcpy ( (LPVOID) pData, (LPVOID) lpHost->lpData, lpHost->lpData->lenRul );
	lpHost->ReduceConMax ( pData );            // Найти и привести к константе
	lpHost->ExtendCon ( pData );                         // Дополнить конъюнкт

	while ( TRUE ) {

		// Провести вывод на дизъюнктах (один проход) и данных
		InferencePassCNF ( pData, pDis, pConc );

		// Проверить, получено ли что-то новое, т.е., неследование заключений из данных
		// Если да, то сформировать новые данные и возобновить цикл
		// иначе (заключения следуют из данных) прервать цикл
		if ( lpHost->ImposeConMax ( pData, pConc ) ) break;

		}

	// Перенести заключения в окно заключений
	TransferConIntoWindow ( pData );

	// Освободить память
	LK_free ( pData );
	LK_free ( pConc );
	LK_free ( pDis );

	// Послать всем окнам заключений сообщение об изменении содержания

	return LKE_SUCCESS;
	}

/*
┌─────────────────────────────────────────────────────────┬──────────────────┐
│ Проводится вывод по данным и одному проходу по          │ InferencePassCNF │
│ дизъюнктам из КНФ. Меняется только вектор заключений,   └──────────────────┤
│ который вначале м.б. не инициализирован (содержать мусор) также как и      │
│ буфер для текущего дизъюнкта. Вектор данных должен содержать данные в      │
│ максимальной приведенной форме. В результате буден получен конъюнкт        │
│ заключений в максимальной форме.                                           │
└────────────────────────────────────────────────────────────────────────────┘
*/
int KBASE::InferencePassCNF ( LPHEAD_RUL pData, LPHEAD_RUL pDis, LPHEAD_RUL pConc ) {

	// Инициализировать вектор заключений (сделать его единичным)
	// Установить поля заголовка и привести к максимальному виду (расширить)
	pConc->numProp = 0;
	pConc->lenRul = sizeof(HEAD_RUL);
	pConc->Const.pos = UNITY;
	pConc->Const.nec = 0;
	pConc->Style = RUL_DISJUNCT;
	pConc->numParent1 = 0;
	pConc->numParent2 = 0;
	lpHost->ExtendCon ( pConc ); // Дополнить заключения тривиальными секциями

	// Цикл по дизъюнктам из КНФ
	LPHEAD_RUL pRul = lpHost->lpCNF;    // Указатель на текущий дизъюнкт в КНФ
	for ( int i = 0; i < lpHost->numDis; i++ ) {

		// Скопировать очередной дизъюнкт в буфер (он уже приведен)
		_fmemcpy ( (LPVOID) pDis, (LPVOID) pRul, pRul->lenRul );

		InferenceDisCNF ( pData, pDis, pConc );

		pRul = (LPHEAD_RUL) ( (LPSTR) pRul + pRul->lenRul );
		}

	// Привести вектор заключений к максимальной форме
	lpHost->ReduceConMax ( pConc );

	return 0;
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Проводится вывод по данным и одному дизъюнкту, который   │ InferenceDisCNF │
│ находится в буфере. Если дизъюнкт сработал, то           └─────────────────┤
│ возвращается TRUE, иначе, если в вектор заключений ничего не было записано,│
│то возвращается FALSE. Вектор заключений должен быть правильно установлен.  │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBASE::InferenceDisCNF ( LPHEAD_RUL pData, LPHEAD_RUL pDis, LPHEAD_RUL pConc ) {
/*
Каждая из секция максимально расширенного конъюнкта представляет один дизъюнкт
в k-ой приведенной форме (константа которого приведена, т.е., прибавлена, к
k-ой секции, а остальные секции нулевые). Фактически, необходимо провести
резолюции по всем секциям. Но перед резолюцией, как известно, константа
дизъюнкта должна быть приведена (прибавлена) к соответствующей секции. Поскольку
первый резольвируемый дизъюнкт (полученный из одной секции конъюнкта) имеет
специфическую форму, а именно, не k-ые секции нулевые (поэтому они не влияют
на резольвенту), можно константу второго дизъюнкта (правила) сразу ввести во
все секции, т.е., найти максимальную приведенную форму. Т.о., процедура наложения
данных на дизъюнкт сводится к (i) расширению дизъюнкта и приведению к максимальной
форме, (ii) нахождению покомпонентного максимума дизъюнкта с данными, и
(iii) приведению полученного дизъюнкта к форме, необходимой для процедуры
проецирования (если нужно).
*/

	// Привести к максимальной форме, т.е., поджать все секции снизу константой
	lpHost->ReduceDisMaxTo ( pDis );

	// Добавить константные секции вместо отсутствующих
	lpHost->ExtendDis ( pDis );

	// Наложить данные на дизъюнкт (фактически, провести n резолюций)
	lpHost->ImposeConOnDis ( pDis, pData );

	// Спроецировать дизъюнкт на секцию с максимальной компонентой
	COMP MaxComp;
	lpHost->ProjectDis ( pDis, & MaxComp );

	// Наложить секцию спроецированного дизъюнкта на конъюнкт
	// заключений (приведение заключений делается после всего прохода).
	// Учитывается константа дизъюнкта
	lpHost->ImposeProjectDisOnCon ( pConc, pDis );
	}

/*
┌────────────────────────────────────────────────────┬───────────────────────┐
│ Записать в вектор заключений данного окна          │ TransferConIntoWindow │
│ информацию из заданного конъюнкта.                 └───────────────────────┤
│                                                                            │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
VOID KBASE::TransferConIntoWindow ( LPHEAD_RUL pCon ) {
/*
Пройтись по всем высказываниям вектора заключений и для каждого найти
соответствующее ему высказывание в конъюнкте. Далее произвести просто
копирование.

Всем окнам с видом на заключения передать сообщение об изменении содержимого.
Если таковых окон не оказалось, то в данном окне перейти к просмотру заключений
(м.б. лучше открыть окно заключений?) и передать ему сообщение об изменении
содержимого.
*/
	int i, j, k, m;
	LPHEAD_PROP pConProp;
	LPHEAD_PROP pWinProp;
	LPCOMP      pConComp;
	LPCOMP      pWinComp;


	// Пройтись по всем высказываниям заключений окна и в каждое скопировать
	// соответствующее высказывание из заданного конъюнкта (возможно, с инвертированием)
	pWinProp = (LPHEAD_PROP) ( lpHost->lpConc + 1 );
	for ( j = 0; j < lpHost->lpConc->numProp; j++ ) {

		// Найти в заданном конъюнкте высказывание с номером pWinProp->curAtt

		pConProp = (LPHEAD_PROP) ( pCon + 1 );
		for ( k = 0; k < pCon->numProp; k++ ) {
			if ( pConProp->curAtt == pWinProp->curAtt ) {

				// Нашлось соответствующее высказывание; скопировать k-е в j-е

				pWinComp = (LPCOMP) ( pWinProp + 1 );
				pConComp = (LPCOMP) ( pConProp + 1 );
				m = lpHost->lpAttIdx [ pWinProp->curAtt ].AttRec.numVal;
				if ( pWinProp->Style & PROP_CONJUNCT ) {
					for ( i = 0; i < m; i++ ) {
						pWinComp->pos = pConComp->pos;
						pWinComp = pWinComp + 1; pConComp = pConComp + 1;
						}
					}
				else {
					for ( i = 0; i < m; i++ ) {
						pWinComp->pos = UNITY - pConComp->pos;
						pWinComp = pWinComp + 1; pConComp = pConComp + 1;
						}
					}
				// Скопировали, поэтому прервать поиск
				break;
				}
			pConProp = (LPHEAD_PROP) ( (LPSTR) pConProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpHost->lpAttIdx [ pConProp->curAtt ].AttRec.numVal );
			}

		// Перейти к следующему высказыванию в конъюнкте заключений окна
		pWinProp = (LPHEAD_PROP) ( (LPSTR) pWinProp + sizeof(HEAD_PROP) + sizeof(COMP) * lpHost->lpAttIdx [ pWinProp->curAtt ].AttRec.numVal );
		}

	// Установить константу заключений (хотя она и не используется)
	lpHost->lpConc->Const = pCon->Const;

	BOOL isConclusionWindow = FALSE;
	// Пройтись по всем окнам заключений
	for ( LPKBASE lpKBase = lpHost->lpFirstKBase; lpKBase; lpKBase = lpKBase->lpNextKBase ) {
		if ( lpKBase->View == VIEW_CONCLUSION ) {
			if ( lpKBase->hWnd ) SendMessage ( lpKBase->hWnd, WM_COMMAND, CHANGE_CONTENT, 0L );
			isConclusionWindow = TRUE;
			}
		}

	if ( ! isConclusionWindow ) {
		SetView ( VIEW_CONCLUSION );
		if ( hWnd ) {
			SendMessage ( hWnd, WM_COMMAND, CHANGE_TITLE, 0L );
			SendMessage ( hWnd, WM_COMMAND, CHANGE_CONTENT, 0L );
			}
		}

	}

