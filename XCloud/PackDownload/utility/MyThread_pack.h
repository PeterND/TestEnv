// MyThread.h: interface for the CMyThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MYTHREAD_PACK_H_
#define _MYTHREAD_PACK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef _WIN32
#include "windows.h"
#else
#include <pthread.h>
#define INFINITE 10000
typedef void *              LPVOID;
typedef unsigned int   DWORD;
#endif

namespace c3pack_down
{
	const DWORD TIME_WAITINGCLOSE	= 3000;		// 3000ms


	class IThreadEvent  
	{
	public:
		virtual ~IThreadEvent() {}

		virtual void OnThreadProcess(void) {} //返回true的时候跳出循环
	};

	class CMyThread  
	{
	public:
		CMyThread(IThreadEvent& event);
		virtual ~CMyThread();
		void EndThread();
	public:
		enum { SUSPEND = true, RUN = false };
	#ifdef _WIN32
		bool	Create	(bool bSuspend=SUSPEND, DWORD dwWorkInterval=INFINITE);
	#else
		bool	Create	(bool bSuspend=SUSPEND, DWORD dwWorkInterval=1);
	#endif


	protected:
		DWORD			m_dwWorkInterval;
		IThreadEvent&	m_event;

	private:
		DWORD Run(void);
		
	#ifdef _WIN32
		HANDLE		m_hThread;
	#else
		pthread_t		m_hThread;
	#endif

	#ifdef _WIN32
		static DWORD WINAPI RunThread(LPVOID pThreadParameter);
	#else
		static void* RunThread(LPVOID pThreadParameter);
	#endif

	public:
		static CMyThread*	CreateNew(IThreadEvent& refEvent, bool bSuspend = SUSPEND, DWORD dwWorkInterval = INFINITE);
	};
}
#endif

