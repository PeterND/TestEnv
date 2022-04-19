#ifndef _RES_OPERATION_H_
#define _RES_OPERATION_H_
#include "utility/Utility_pack.h"
#include "utility/MyThread_pack.h"

namespace c3pack_down
{
	class CThreadEvenDeleteRes : public IThreadEvent
	{
	public:
		virtual ~CThreadEvenDeleteRes() {}
	protected:
		virtual void OnThreadProcess(void);
	};

	class CResOperation
	{
	public:
		CResOperation();
		virtual ~CResOperation();
		bool CheckRes();
	private:
		bool RenameRes(int nBackupOrRevert);
		bool RenameFile(const char* pszOldFile, const char* pszNewFile, int nAction);
		bool RenameFolder(const char* pszOldFolder, const char* pszNewFolder, int nAction);
		bool IsNeedDelete();
		bool CheckFolderPermission(const char* pszFolderName);

	public:
		static bool HasFile(const char* pszFile);
		static bool HasDir(const char* pszDir);
		static void _MakeDir(const char* pszPath);
		static int RmDir(const char* pszDirPath);
		static bool CopyResFile(const char* pszSrcFile, const char* pszDestFile);
		static bool CopyFolder(const char* pszSrcDir, const char* pszDest);
		static bool DeleteRes();

	private:
		CMyThread*	m_pThreadDeleteRes;
		CThreadEvenDeleteRes* m_pThreadEvent;
	};
}
#endif
