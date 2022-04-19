// MyThread.cpp: implementation of the CMyThread class.
//
//////////////////////////////////////////////////////////////////////
#include "MyThread_pack.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#endif

namespace c3pack_down
{
	#ifdef _WIN32
	typedef unsigned (__stdcall *MYTHREAD_FUNC)(void *);
	#else
	typedef void* (*MYTHREAD_FUNC)(void*);
	#endif

	//////////////////////////////////////////////////////////////////////
	#ifdef _WIN32
	DWORD WINAPI CMyThread::RunThread(LPVOID pThreadParameter)
	#else
	void* CMyThread::RunThread(LPVOID pThreadParameter)
	#endif
	{
		CMyThread* pThread = (CMyThread*)pThreadParameter;
		if (!pThread) {
			return NULL;
		}
		
	#ifdef _WIN32
		return pThread->Run();
	#else
		return (void*)(pThread->Run());
	#endif
	}

	DWORD CMyThread::Run(void)
	{
		m_event.OnThreadProcess();

		return 0;
	}


	#ifdef _WIN32
	CMyThread::CMyThread(IThreadEvent& event)
		: m_event(event), m_hThread(NULL)
	{
		m_dwWorkInterval	= 0;
	}
	#else
	CMyThread::CMyThread(IThreadEvent& event)
	:m_event(event)
	{
		m_dwWorkInterval = 0;
	}
	#endif

	CMyThread::~CMyThread()
	{
	#ifdef _WIN32
		if (m_hThread)
		{
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	#endif
	}

	void CMyThread::EndThread()
	{
	#ifdef _WIN32
		if (m_hThread)
		{
			::MsgWaitForMultipleObjects(1, &m_hThread, false, INFINITE, QS_ALLINPUT);//WaitForSingleObject导致主线程卡住，消息循环阻塞
		}

	#else
		DWORD testnum = 0;
		int joinret = pthread_join(m_hThread, NULL);
		if (joinret != 0)
		{
			
		}
	#endif
	}


	//////////////////////////////////////////////////////////////////////
	bool CMyThread::Create(bool bSuspend/*=true*/, DWORD dwWorkInterval/*=INFINITE*/)
	{

	#ifdef _WIN32

		DWORD dwCreationFlags = 0;
		if (bSuspend)
			dwCreationFlags	=CREATE_SUSPENDED;
		int id = 0;
		m_hThread	= (HANDLE)_beginthreadex (	NULL,			// default security
												0,				// default stack size
												(MYTHREAD_FUNC)RunThread,		// pointer to thread routine
												this,			// argument for thread
												dwCreationFlags,// start it right away if 0, else suspend
												(unsigned *)&id);
		if (!m_hThread)
		{
			return false;
		}

		m_dwWorkInterval = dwWorkInterval;

		return true;
	#else
		
		if (0 != pthread_create(&m_hThread, NULL, (MYTHREAD_FUNC)RunThread, this))
		{
			return false;
		}
		
		m_dwWorkInterval = dwWorkInterval;
		
		return true;
	#endif
	}

	//////////////////////////////////////////////////////////////////////
	CMyThread*	CMyThread::CreateNew(IThreadEvent& refEvent, bool bSuspend/* = SUSPEND*/, DWORD dwWorkInterval/* = INFINITE*/)
	{
		CMyThread* pThread = new CMyThread(refEvent);
		if (!pThread)
			return NULL;

		if (!pThread->Create(bSuspend, dwWorkInterval))
		{
			delete pThread;
			return NULL;
		}

		return pThread;
	}
}