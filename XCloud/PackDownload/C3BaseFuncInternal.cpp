#include "C3BaseFuncInternal.h"

bool g_debug_log_enabled = 1;
char g_pszLogFolderPath[512];
// Log
#ifdef __ANDROID__
#include <android/log.h>
void DebugMsg(const char* fmt, ...)
{
	if (g_debug_log_enabled)
	{
		va_list args;
		va_start(args, fmt);
		__android_log_vprint(ANDROID_LOG_DEBUG, "XCloud", fmt, args);
		va_end(args);
	}
}
#else
void DebugMsg(const char* fmt, ...)
{
	if (g_debug_log_enabled)
	{
		char szMsg[1024] = { 0 };
		szMsg[1023] = 0;

		va_list args;
		va_start(args, fmt);
		vsnprintf(szMsg, 1023, fmt, args);
		va_end(args);

		strcat(szMsg, "\n");
		printf(szMsg);
	}
}
#endif


void LogMsg(const char* fmt, ...)
{
	if (g_debug_log_enabled)
	{
		char szMsg[1024] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf(szMsg, 1023, fmt, args);
		va_end(args);

		::DebugMsg(szMsg); // LogMsg的同时条用DebugMsg输出

		time_t ltime;
		time(&ltime);

		char szLogName[512] = "";
		tm* curTime = localtime(&ltime);
		sprintf(szLogName, "%s/XCloud_%u_%u_%u.log", g_pszLogFolderPath, curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday);

		FILE* fp = fopen(szLogName, "a+");
		if (fp != NULL)
		{
			fprintf(fp, "%s -- %d-%d-%d %d:%d:%d\n", szMsg, curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday, curTime->tm_hour, curTime->tm_min, curTime->tm_sec);
			fclose(fp);
		}
	}
}

DWORD TimeGetTime()
{
	return	 0;
	//ZBTODO
}