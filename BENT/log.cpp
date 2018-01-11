#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "log.h"

using namespace std;

CRITICAL_SECTION log_cs;
string logbuf;
char lPath[MAX_PATH] = "BentLog.txt";
char pBuffer[4096 * 8];

void InitializeLog() {
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LogThread, 0, 0, 0);
	Sleep(50);
}

bool _Log(char *szLog, const char *data, size_t len) {
	FILE *f = NULL;

	fopen_s(&f, szLog, "a");

	if (!f) {
		fopen_s(&f, szLog, "a");
	}

	if (f) {
		fwrite(data, len, 1, f);
		fclose(f);
		return true;
	}

	return false;
}

void FlushLog(bool enterCS = true) {
	if (enterCS) {
		EnterCriticalSection(&log_cs);
	}
	size_t len = logbuf.length();

	if (len > 0) {
		if (_Log(lPath, logbuf.c_str(), len)) {
			logbuf.clear();
		}
	}

	if (enterCS) {
		LeaveCriticalSection(&log_cs);
	}
}

DWORD WINAPI LogThread(void *) {
	InitializeCriticalSection(&log_cs);

	DeleteFileA(lPath);
	Sleep(250);

	while (1) {
		Sleep(10);

		FlushLog();
	}

	return 0;
}

void __Log(char *data) {
	EnterCriticalSection(&log_cs);
	logbuf += data;
	if (logbuf.size() > 2000) {
		FlushLog(false);
	}
	LeaveCriticalSection(&log_cs);
}

void Log(char *pText, ...) {
	va_list valist;

	va_start(valist, pText);
	vsprintf_s(pBuffer, sizeof(pBuffer), pText, valist);
	va_end(valist);

	__Log(pBuffer);
}
