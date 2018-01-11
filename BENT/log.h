#ifndef LOG_H
#define LOG_H

#ifndef _DEBUG
#define DEBUG_LOG
#else
#define DEBUG_LOG Log
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void InitializeLog();
DWORD WINAPI LogThread(void *);

void Log(char *format, ...);
extern char lPath[MAX_PATH];

#endif
