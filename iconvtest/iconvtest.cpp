#ifdef _WIN32
#include <vld.h>
#endif

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>
#include <errno.h>
#include <clocale>


const int BufferSize = 32;
char MS949_String[BufferSize] = {0};

void Test1();
void Test2();
void Test3();
int main()
{
	// test.txt는 CP949로 쓰여진 데이터.
	FILE* fp = fopen("test.txt", "r");
	fread(MS949_String, BufferSize, 1, fp);
	
	setlocale(LC_ALL, "");
	Test1();
	Test2();
	Test3();
	fclose(fp);
	return 0;
}

void PrintBuffer(const char* buf, int size)
{
	for (int i=0; i<size; ++i) {
		printf(" %02x", (unsigned char)buf[i]);
	}
	printf("\n");
}

void Test1()
{
	// 기본적인 사용법 확인

	int ret = 0;
	int err = 0;
	printf("start Test1\n");

	// CP949에서 UTF-8로 변환
	iconv_t icd = iconv_open("UTF-8", "CP949"); // MS949를 넣으면 실패.
	if (icd == (iconv_t)-1) {
		err = errno;
		printf("fail iconv_open(),  err : %d\n", err);
		return;
	}

	char inBuf[BufferSize];
	char outBuf[BufferSize];

	// valgrind가 시끄러우니 초기화를 해준다.
	memset(inBuf, 0xff, BufferSize);
	memset(outBuf, 0xff, BufferSize);

	size_t inSize = strlen(MS949_String) + 1;
	size_t outSize = BufferSize;

	strcpy(inBuf, MS949_String);
	inBuf[inSize] = '\0'; // Null문자 확인 필

	try {
		// iconv 함수가 인자로 받은 버퍼의 포인터값을 바꿔버린다.
		// 따라서 임시 변수를 만들어서 넘겨줘야 한다.
		char* inArg = (char*)inBuf;
		char* outArg = (char*)outBuf;
		printf("inArg : %p\n", inArg);
		PrintBuffer(inBuf, BufferSize);
		printf("outArg : %p\n", outArg);
		PrintBuffer(outBuf, BufferSize);

		printf("convert!\n");
		ret = iconv(icd, &inArg, &inSize, &outArg, &outSize);
		err = errno;

		// inArg와 outArg의 값이 바뀌었지만
		// 굳이 이것들을 관리해줄 필요는 없다.
		// vargrind를 돌려봤지만 메모리릭은 없었다.
		printf("inArg : %p\n", inArg);
		printf("outArg : %p\n", outArg);
		printf("\n");
	}
	catch(...) {
		printf("unknown exception!!\n");
		goto _End;
	}

	if (ret < 0) {
		printf("ret : %d,  err : %d\n", ret, err);
	}
	else {
		// outSize는 iconv() 호출 시 넘겼던 값에서 실제로 쓰인 값을 뺀 값이다.
		// 예를들어 10을 넘겼고 결과로 3글자가 쓰였다면 7이된다.
		printf("ret : %d,  size : %d\n", ret, outSize); 
		printf("%s\n", outBuf);
		PrintBuffer(outBuf, BufferSize);
	}

_End:
	iconv_close(icd);
	printf("\nend\n\n");
}

void Test2()
{
	// Wide 문자 변환
	int ret = 0;
	int err = 0;
	printf("start Test2\n");

	// UTF16 or UTF32에서 UTF-8로 변환
#ifdef _WIN32
	iconv_t icd = iconv_open("UTF-8", "UTF-16LE");
#else
	iconv_t icd = iconv_open("UTF-8", "UTF-32LE");
#endif
	if (icd == (iconv_t)-1) {
		err = errno;
		printf("fail iconv_open(),  err : %d\n", err);
		return;
	}

	wchar_t wstr[] = L"abc 가나다";
	char buf[BufferSize];
	memset(buf, 0xff, sizeof(buf));

	char* inBuf = (char*)wstr;
	char* outBuf = (char*)buf;
	size_t inSize = sizeof(wstr);
	size_t outSize = sizeof(buf);

	PrintBuffer(inBuf, sizeof(wstr));
	
	ret = iconv(icd, &inBuf, &inSize, &outBuf, &outSize);
	
	if (ret < 0) {
		printf("ret : %d,  err : %d\n", ret, err);
	}
	else {
		printf("ret : %d,  size : %d\n", ret, outSize);
		printf("%s\n", buf);
		PrintBuffer((char*)buf, sizeof(buf));
	}

_End:
	iconv_close(icd);
	printf("\nend\n\n");
}

void Test3()
{
	// UTF8에서 Wide로 변환
	int ret = 0;
	int err = 0;
	printf("start Test3\n");

#ifdef _WIN32
	iconv_t icd = iconv_open("UTF-16LE", "CP949");
#else
	iconv_t icd = iconv_open("UTF-32LE", "UTF-8");
#endif
	if (icd == (iconv_t)-1) {
		err = errno;
		printf("fail iconv_open(),  err : %d\n", err);
		return;
	}

	char str[] = "abc 가나다";
	wchar_t buf[BufferSize];
	memset(buf, 0xff, sizeof(buf));

	char* inBuf = (char*)str;
	char* outBuf = (char*)buf;
	size_t inSize = sizeof(str);
	//size_t outSize = sizeof(buf);
	size_t outSize = 36;
	
	PrintBuffer(inBuf, sizeof(str));

	ret = iconv(icd, &inBuf, &inSize, &outBuf, &outSize);

	if (ret < 0) {
		printf("ret : %d,  err : %d\n", ret, err);
	}
	else {
		printf("ret : %d,  size : %d\n", ret, outSize);
		wprintf(L"%ls\n", buf);
		PrintBuffer((char*)buf, sizeof(buf));
	}

_End:
	iconv_close(icd);
	printf("\nend\n\n");
}

