#include <stdio.h>
#include <wchar.h>
#ifdef _WIN32
#pragma execution_character_set("utf-8")
#endif

#ifdef UNICODE
#define _T(str) L ## str
typedef wchar_t TCHAR;
#else
#define _T(str) str
typedef char TCHAR;
#endif

template<typename T>
void _Print(const T* str, int size)
{
	char* cast = (char*)str;
	for (int i = 0; i < size; ++i) {
		printf(" %02x", (unsigned char)cast[i]);
	}
	printf("\n---------------------------------------------------\n");
}
#define Print(str) _Print(str, sizeof(str));

int main()
{
	// 가 啊 
	//            0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
	// CP949   : 61 62 63 20 b0 a1 b3 aa 8c 63 00
	// UTF8    : 61 62 63 20 ea b0 80 eb 82 98 eb 98 a0 00
	// UTF16LE : 61 00 62 00 63 00 20 00 00 ac 98 b0 20 b6 00 00
	const char mstr[] = "abc 가나똠";
	const wchar_t wstr[] = L"abc 가나똠";
	const TCHAR tstr[] = _T("abc 가나똠");

	Print(mstr);
	Print(wstr);
	Print(tstr);

	printf("%s\n", mstr);
	return 0;
}
