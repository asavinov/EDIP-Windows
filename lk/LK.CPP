#include <windows.h>
#include <alloc.h>                                                   // size_t
#include <mem.h>
#include <dos.h>                                                     // FP_SEG
#include "lk.h"

/*
▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐▐ GLOBAL▐▐▐
*/

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│ Входная точка для DLL, запускается когда DLL загружается в память│ LibMain │
│                                                                  └─────────┤
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
*/
int far pascal LibMain ( HANDLE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine ) {

	if ( wHeapSize > 0 ) {
		UnlockData (0);
		}
	return 1;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Функция выполняет раздвижение или схлопывание памяти, т.е.     │ InsDelMem │
│ в блоке памяти по адресу pMem и длиной lenMem начиная с адреса └───────────┤
│ BeginAt выполняется добавление (вставление) или удаление байтов памяти в   │
│ количестве Offset байт (в зависимости от знака).                           │
└────────────────────────────────────────────────────────────────────────────┘
*/
int InsDelMem ( LPVOID pMem, long lenMem, LPVOID BeginAt, int Offset ) {

	// Блоки памяти не могут пересекаться
	// void far *far _fmemcpy ( void far *dest, const void far *src, size_t n );

	// Блоки могут перекрываться
	// void* memmove ( void* dest, const void* src, size_t n )   // Стандартная
	// void  movmem (void* src, void* dest, unsigned length )   // Уникальна для Borland C++

	// Копирование блоков памяти независимо от модели памяти:
	// void  movedata ( unsigned srcseg, unsigned srcoff, unsigned destseg, unsigned destoff, size_t n );

	if ( Offset > 0 )
		memmove ( (LPVOID) ((LPSTR) BeginAt + Offset), BeginAt, lenMem - (long) ((LPSTR) BeginAt - (LPSTR) pMem) );
	else if ( Offset < 0 )
		memmove ( BeginAt, (LPVOID) ((LPSTR) BeginAt + (-Offset)), lenMem - (long) ((LPSTR) BeginAt - (LPSTR) pMem) - (-Offset) );
	else return LKE_SUCCESS;

	return LKE_SUCCESS;
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│ Определяет размер памяти по указателю на нее. Берет сегмент   │ MemorySize │
│ и считает, что это дескриптор.                                └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
long MemorySize ( LPVOID ptr ) {
	return GlobalSize ( FP_SEG ( ptr ) );
	}


/*
┌─────────────────────────────────────────────────────────────────┬──────────┐
│ Прочитать содержимое файла по указанному смещению указанной     │ ReadFile │
│ длины и записать его по указанному адресу.                      └──────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int ReadFile ( int hFile, long Ofs, long Len, LPSTR To ) {
	int i;
	int n = Len / BUF_SIZE;                              // Необходимо буферов
	int r = Len % BUF_SIZE;                             // Останется прочитать
	_llseek ( hFile, Ofs, 0 );                       // Переместиться к началу

	for ( i = 0; i < n; i++ ) {
		// Прочитать i-ую порцию
		if ( BUF_SIZE != _lread ( hFile, (LPSTR) ( To + (long) i * (long) BUF_SIZE ), BUF_SIZE ) ) {
			return LKE_CANTREAD;
			}
		}

	// Прочитать оставшиеся r байт
	if ( r != _lread ( hFile, (LPSTR) ( To + (long) n * (long) BUF_SIZE ), r ) ) {
		return LKE_CANTREAD;
		}

	return LKE_SUCCESS;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│ Записать содержимое файла по указанному смещению указанной     │ WriteFile │
│ длины.                                                         └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int WriteFile ( int hFile, long Ofs, long Len, LPSTR From ) {
	int i;
	int n = Len / BUF_SIZE;                              // Необходимо буферов
	int r = Len % BUF_SIZE;                              // Останется записать
	_llseek ( hFile, Ofs, 0 );                       // Переместиться к началу

	for ( i = 0; i < n; i++ ) {
		// Записать i-ую порцию
		if ( BUF_SIZE != _lwrite ( hFile, (LPSTR) ( From + (long) i * BUF_SIZE ), BUF_SIZE ) ) {
			return LKE_CANTWRITE;
			}
		}

	// Записать оставшиеся r байт
	if ( r != _lwrite ( hFile, (LPSTR) ( From + (long) n * BUF_SIZE ), r ) ) {
		return LKE_CANTWRITE;
		}

	return LKE_SUCCESS;
	}

/*
┌──────────────────────────────────────────────────────────────┬─────────────┐
│ Правилен ли формат файла? Проверяется существует ли файл и   │ isFileValid │
│ если да, то правилен ли его формат.                          └─────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
int isFileValid ( LPSTR lpPath ) {
	// Открыть файл
	int hFile = _lopen ( lpPath, READ );
	if ( hFile == HFILE_ERROR ) {
	  return LKE_CANTOPEN;
	  }

	int Ret = isFileValid ( hFile );

	// Закрыть файл
	_lclose ( hFile );
	return Ret;
	}

int isFileValid ( int hFile ) {

	// Прочитать заголовок
	HEAD_REC Head;
//	_llseek ( hFile, 0, 0 );
	if ( sizeof(HEAD_REC) != _lread ( hFile, (LPSTR) & Head, sizeof(HEAD_REC) ) ) {
		return LKE_CANTREAD;
}

	if ( 0 != lstrcmp ( (LPSTR) Head.CopyWrite, (LPSTR) LK_COPYWRITE  ) ) {
		return LKE_NOTEDIPFILE;
}

	if ( Head.Version != LK_VERSION ) {
		return LKE_NOTEDIPFILE;
}

	return LKE_SUCCESS;
	}

/*
┌──────────────────────────────────────────────────────────┬─────────────────┐
│ Выделить имя файла из пути. Возвращается указатель на    │ GetNameFromPath │
│ начало имени файла в строке пути.                        └─────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPSTR GetNameFromPath ( LPSTR lpPath ) {
	if ( lpPath == 0 )    return 0;
	if ( lpPath[0] == 0 ) return lpPath;

	LPSTR ptr = lpPath + lstrlen ( lpPath );
	for ( ; ( ( ptr != lpPath - 1 ) && ( *ptr != '\\') && ( *ptr != ':') ); ptr-- );

    return ptr + 1;
	}

/*
┌─────────────────────────────────────────────────────────┬──────────────────┐
│                                                         │ InitFilesCounter │
│                                                         └──────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/

static LPKBFILE lpNextKBFile;

VOID InitFilesCounter ( VOID ) {
	lpNextKBFile = KBFILE::lpFirstKBFile;
	}

/*
┌────────────────────────────────────────────────────────┬───────────────────┐
│                                                        │ GetNextFileObject │
│                                                        └───────────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPKBASE GetNextFileObject ( VOID ) {
	LPKBASE pKBase;

	// Будет возвращен указатель на первое открытое окно текущего файла
	if ( lpNextKBFile ) pKBase = lpNextKBFile->lpFirstKBase;
	else                return NULL;

	lpNextKBFile = lpNextKBFile->lpNextKBFile;

	return pKBase;
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ AddBakExt │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
static char bakExt[] = ".bak";

VOID AddBakExt ( LPSTR Name ) {
	LPSTR ptr;

    for ( ptr = Name; *ptr && *ptr != '.'; ptr++ ) ;
	* ptr = 0;
	lstrcat ( Name, bakExt );
	}

/*
┌────────────────────────────────────────────────────────────────┬───────────┐
│                                                                │ LK_malloc │
│                                                                └───────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPVOID LK_malloc ( unsigned long nbytes ) {

	HGLOBAL hMem = GlobalAlloc ( GHND, nbytes );

	if ( ! hMem ) return NULL;

	return GlobalLock ( hMem );
	}

/*
┌───────────────────────────────────────────────────────────────┬────────────┐
│                                                               │ LK_realloc │
│                                                               └────────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
LPVOID LK_realloc ( LPVOID ptr, unsigned long nbytes ) {

	GlobalUnlock ( FP_SEG(ptr) );

	HGLOBAL hMem = GlobalReAlloc ( FP_SEG(ptr), nbytes, GHND );

	if ( ! hMem ) return NULL;

	return GlobalLock ( hMem );
	}

/*
┌──────────────────────────────────────────────────────────────────┬─────────┐
│                                                                  │ LK_free │
│                                                                  └─────────┤
└────────────────────────────────────────────────────────────────────────────┘
*/
HGLOBAL LK_free ( LPVOID ptr ) {

	GlobalUnlock ( FP_SEG(ptr) );

	return GlobalFree ( FP_SEG(ptr) );
	}

