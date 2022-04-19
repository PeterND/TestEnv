#include "ResOperation.h"
#include "PackLogSender.h"
#include "PackDownManager.h"
#include "AppConfigInfo.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include "C3BaseFuncInternal.h"
namespace c3pack_down
{
	#define BACKUP_FILE_COUNT   4

	#define ACTION_BACKUP   1
	#define ACTION_REVERT   2
	#define MAX_PATH_LEN   512

	CResOperation::CResOperation()
	{
		m_pThreadDeleteRes = NULL;
		m_pThreadEvent = NULL;
	}

	CResOperation::~CResOperation()
	{
		if (m_pThreadDeleteRes)
		{
			m_pThreadDeleteRes->EndThread();
			SAFE_DELETE(m_pThreadDeleteRes);
		}
		SAFE_DELETE (m_pThreadEvent);
	}

	bool CResOperation::HasFile(const char* pszFile)
	{
	#ifdef _WIN32
		DWORD dwAttrib = GetFileAttributes(pszFile);
		return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
	#else
		struct stat s;
		if(stat(pszFile, &s) == 0)
		{
			if(s.st_mode & S_IFREG)
			{
				return true;
			}
		}
		return false;
	#endif
	}

	bool CResOperation::HasDir(const char* pszDir)
	{
	#ifdef _WIN32
		DWORD dwAttrib = GetFileAttributes(pszDir);
		return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
	#else
		struct stat s;
		if(stat(pszDir, &s) == 0)
		{
			if(s.st_mode & S_IFDIR)
			{
				return true;
			}
		}
		return false;
	#endif
	}

	void CResOperation::_MakeDir(const char* pszPath)
	{
		std::string strPath = pszPath;
		for(int i = 0; i < strPath.size(); i++) 
		{	
			if(strPath[i] == '\\')
			{
				strPath[i] = '/';
			}
		}

		if (strPath[strPath.size()-1] != '/')
		{
			strPath += "/";
		}

		int nR = 0;
		do 
		{
			nR = strPath.find_first_of('/', nR);
			if (std::string::npos == nR)
			{
				break;
			}
			std::string strSub = strPath.substr(0, nR);
			nR++;
	#ifdef _WIN32
			_mkdir(strSub.c_str());
	#else
			mkdir(strSub.c_str(), S_IRWXU);
	#endif
		} while (nR < strPath.size());
	}

	int CResOperation::RmDir(const char* pszDirPath)
	{
		if (NULL == pszDirPath)
		{
			return -1;
		}
#ifdef _WIN32

		char szPath[MAX_PATH_LEN] = {0};

		int nResultone = 0;
		int nNoFile = 0;
	 
		strcpy(szPath, pszDirPath);
		strcat(szPath, "/*");
#ifdef _WIN64
		struct _finddatai64_t fb;
		__int64	lHandle = _findfirst64(szPath, &fb);
		if (lHandle == -1) return 0;
		if (lHandle > 0)
		{
			while (0 == _findnext64(lHandle, &fb))
			{
				nNoFile = strcmp(fb.name, "..");
				if (0 != nNoFile)
				{
					memset(szPath, 0, sizeof(szPath));
					strcpy(szPath, pszDirPath);
					strcat(szPath, "\\");
					strcat(szPath, fb.name);
					if (fb.attrib == 16)
					{
						 RmDir(szPath);	
					}
					else
					{
						::remove(szPath);
					}
				}	
			}
			_findclose(lHandle);
		}

		nResultone = _rmdir(pszDirPath);
		return nResultone;
#else
		struct _finddata_t fb;
		long lHandle  = _findfirst(szPath, &fb);
		if (lHandle == -1) return 0;
		if (lHandle > 0)
		{
			while (0 == _findnext(lHandle, &fb))
			{
				nNoFile = strcmp(fb.name, "..");
				if (0 != nNoFile)
				{
					memset(szPath, 0, sizeof(szPath));
					strcpy(szPath, pszDirPath);
					strcat(szPath, "\\");
					strcat(szPath, fb.name);
					if (fb.attrib == 16)
					{
						RmDir(szPath);
					}
					else
					{
						::remove(szPath);
					}
				}
			}
			_findclose(lHandle);
		}

		nResultone = _rmdir(pszDirPath);
		return nResultone;
#endif	
	#else
		int nRet = 0;
		DIR *pDir = NULL;
		char szBuf[MAX_PATH_LEN] = {0};
		struct dirent *ptr;
		pDir = opendir(pszDirPath);
		if(NULL == pDir)
		{
			return 0;
		}

		while((ptr = readdir(pDir)) != NULL)
		{
			if(0 == strcmp(ptr->d_name, ".") || 0 == strcmp(ptr->d_name, ".."))
			{
				continue;
			}
			snprintf(szBuf, MAX_PATH_LEN, "%s/%s", pszDirPath, ptr->d_name);
			if(HasDir(szBuf))
			{
				 RmDir(szBuf);
			}
			else
			{
				remove(szBuf);
			}
		}
		closedir(pDir);

		nRet = remove(pszDirPath);
		return nRet;
	#endif
	}

	bool CResOperation::CopyResFile(const char* pszSrcFile, const char* pszDestFile)
	{
		if (NULL == pszSrcFile || NULL == pszDestFile)
		{
			return false;
		}
		FILE* pFileDes = fopen(pszDestFile, "wb");
		if (NULL == pFileDes) return false;
	    
		FILE* pFileSrc = fopen(pszSrcFile, "rb");
		if (NULL == pFileSrc)
		{
			if (pFileDes)
			{
				fclose(pFileDes);
			}
			return false;
		}
	    
		fseek(pFileSrc, 0, SEEK_END);
		DWORD nFileSize = ftell(pFileSrc);
		fseek(pFileSrc, 0, SEEK_SET);
	    
		char* pszBuffRead = (char*) malloc(nFileSize + 1);
		if (pszBuffRead != NULL)
		{
			fread(pszBuffRead, sizeof(char), nFileSize, pFileSrc);
			pszBuffRead[nFileSize] = '\0';
			fclose(pFileSrc);
	        
			fwrite(pszBuffRead, sizeof(char), nFileSize, pFileDes);
			fclose(pFileDes);
	        
			free(pszBuffRead);
			pszBuffRead = NULL;
			return true;
		}
		fclose(pFileSrc);
		fclose(pFileDes);
		return false;
	}

	bool CResOperation::CopyFolder(const char* pszSrcDir, const char* pszDest)
	{
		if (NULL == pszSrcDir || NULL == pszDest)
		{
			return false;
		}
#ifdef _WIN32
#ifdef _WIN64
		std::string strSrcDir = pszSrcDir;
		std::string strDestDir = pszDest;
	    
		char cLastChar = strSrcDir[strSrcDir.size() - 1];
		if (cLastChar != '\\')
		{
			strSrcDir.append("\\");
		}
		cLastChar = strDestDir[strDestDir.size() - 1];
		if (cLastChar != '\\')
		{
			strDestDir.append("\\");
		}
	    
		std::string strFindFile = strSrcDir;
		strFindFile += "*.*";
		_finddatai64_t findData;
		__int64	handle = _findfirst64(strFindFile.c_str(), &findData);
		if (handle == -1) return true; //没有此文件目录,认为是正常的
	    
		_MakeDir(strDestDir.c_str());
	    
		do
		{
			if (findData.attrib & _A_SUBDIR)
			{
				if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				{
					continue;
				}
				std::string strNewSrcDir = strSrcDir;
				strNewSrcDir += findData.name;
				std::string strNewDestDir = strDestDir;
				strNewDestDir += findData.name;
				CopyFolder(strNewSrcDir.c_str(), strNewDestDir.c_str());
			}
			else
			{
				std::string strDestFile = strDestDir + findData.name;
				std::string strSrcFile = strSrcDir + findData.name;
				CopyResFile(strSrcFile.c_str(), strDestFile.c_str());
			}
		} while (_findnext64(handle, &findData) == 0);
	    
		_findclose(handle);
	    
		return true;
#else
		std::string strSrcDir = pszSrcDir;
		std::string strDestDir = pszDest;

		char cLastChar = strSrcDir[strSrcDir.size() - 1];
		if (cLastChar != '\\')
		{
			strSrcDir.append("\\");
		}
		cLastChar = strDestDir[strDestDir.size() - 1];
		if (cLastChar != '\\')
		{
			strDestDir.append("\\");
		}

		std::string strFindFile = strSrcDir;
		strFindFile += "*.*";
		_finddata_t findData;
		long handle = _findfirst(strFindFile.c_str(), &findData);
		if (handle == -1) return true; //没有此文件目录,认为是正常的

		_MakeDir(strDestDir.c_str());

		do
		{
			if (findData.attrib & _A_SUBDIR)
			{
				if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				{
					continue;
				}
				std::string strNewSrcDir = strSrcDir;
				strNewSrcDir += findData.name;
				std::string strNewDestDir = strDestDir;
				strNewDestDir += findData.name;
				CopyFolder(strNewSrcDir.c_str(), strNewDestDir.c_str());
			}
			else
			{
				std::string strDestFile = strDestDir + findData.name;
				std::string strSrcFile = strSrcDir + findData.name;
				CopyResFile(strSrcFile.c_str(), strDestFile.c_str());
	}
		} while (_findnext(handle, &findData) == 0);

		_findclose(handle);

		return true;
#endif	
#else
		DIR *pDir = NULL;
		struct dirent *pEntry;
		struct stat statbuf;
	    
		pDir = opendir(pszSrcDir);
		if(!pDir) return true;
	    
		_MakeDir(pszDest);
	    
		chdir(pszSrcDir);
		while((pEntry = readdir(pDir)) != NULL)
		{
			lstat(pEntry->d_name, &statbuf);
			if (S_ISDIR(statbuf.st_mode))
			{
				if(0 == strcmp(".", pEntry->d_name) || 0 == strcmp("..", pEntry->d_name))
				{
					continue;
				}
				char szNewSrcDir[MAX_PATH_LEN] = {0};
				char szNewDestDir[MAX_PATH_LEN] = {0};
				sprintf(szNewSrcDir, "%s/%s", pszSrcDir, pEntry->d_name);
				sprintf(szNewDestDir, "%s/%s", pszDest, pEntry->d_name);
				CopyFolder(szNewSrcDir, szNewDestDir);
			}
			else
			{
				char szSrcFile[MAX_PATH_LEN] = {0};
				sprintf(szSrcFile, "%s/%s", pszSrcDir, pEntry->d_name);
				char szDestFile[MAX_PATH_LEN] = {0};
				sprintf(szDestFile, "%s/%s", pszDest, pEntry->d_name);
				CopyResFile(szSrcFile, szDestFile);
			}
		}
	    
		chdir("..");
		closedir(pDir);
		return true;
#endif
	}

	bool CResOperation::DeleteRes()
	{
		const char* szBackupFileSrc[BACKUP_FILE_COUNT] = {"c3.tpd", "c3.tpi", "data.tpd", "data.tpi"};

		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strBak = "bak_";
		for (int i = 0; i < BACKUP_FILE_COUNT; i++)
		{
			std::string strFile = strEnv + strBak + szBackupFileSrc[i];
			if (HasFile(strFile.c_str()))
			{
				c3pack_down::MyDeleteFile(strFile.c_str());
			}
		}

		std::vector<std::string> vecFolder = CAppConfigInfo::GetInstance()->getDealFolders();

		if (vecFolder.size() == 0)
		{
			vecFolder.push_back("c3");
			vecFolder.push_back("data");
		}
		int nSize = vecFolder.size();
		if (nSize > 0)
		{
			for (int i = 0; i < nSize; i++)
			{
				std::string strDir = strEnv + strBak + vecFolder[i];
				if (HasDir(strDir.c_str()))
				{
					RmDir(strDir.c_str());
				}
			}
		}

		std::string strBakVersion = strEnv + "bak_version.dat";
		if (HasFile(strBakVersion.c_str()))
		{
			c3pack_down::MyDeleteFile(strBakVersion.c_str());
		}
	    
		std::string strC3ex = strEnv + "c3ex";
		if (HasDir(strC3ex.c_str()))
		{
			RmDir(strC3ex.c_str());
		}
	    
		std::string strDataex = strEnv + "dataex";
		if (HasDir(strDataex.c_str()))
		{
			RmDir(strDataex.c_str());
		}

		return true;
	}

	bool CResOperation::IsNeedDelete()
	{
		const char* szBackupFileSrc[BACKUP_FILE_COUNT] = {"c3.tpd", "c3.tpi", "data.tpd", "data.tpi"};

		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strBak = "bak_";
		for (int i = 0; i < BACKUP_FILE_COUNT; i++)
		{
			std::string strFile =strEnv + strBak + szBackupFileSrc[i];
			if (HasFile(strFile.c_str()))
			{
				return true;
			}
		}

		std::vector<std::string> vecFolder = CAppConfigInfo::GetInstance()->getDealFolders();

		if (vecFolder.size() == 0)
		{
			vecFolder.push_back("c3");
			vecFolder.push_back("data");
			vecFolder.push_back("c3pack");
		}
		int nSize = vecFolder.size();
		if (nSize > 0)
		{
			for (int i = 0; i < nSize; i++)
			{
				std::string strDir =strEnv + strBak + vecFolder[i];
				if (HasDir(strDir.c_str()))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool CResOperation::CheckFolderPermission(const char* pszFolderName)
	{
		if (NULL == pszFolderName)
		{
			return false;
		}
		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strFolder = strEnv + pszFolderName;
		if (HasDir(strFolder.c_str()))
		{
			std::string strPermissionTest = strFolder + "/";
			strPermissionTest += "permission.dat";
			FILE* pFileTest = fopen(strPermissionTest.c_str(), "a+");
			if (NULL == pFileTest)
			{
				::LogMsg("fopen c3pack permission.dat fail");
				//如果手机空间小于100M
				if (c3pack_down::MyGetFreeSpace() < 100)
				{
					::LogMsg("FreeSpace < 100");
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_CREATE_LIB_ERROR, "Free Space less than 100M", 100);
				}
				else
				{
					int nRenameC3pack = CAppConfigInfo::GetInstance()->getRenameC3pack();
					if (nRenameC3pack > 0)
					{
						std::string strFolderBak = strEnv + "bak_";
						strFolderBak += pszFolderName;
						if (HasDir(strFolderBak.c_str()))
						{
							RmDir(strFolderBak.c_str());
						}
						int nRet = ::rename(strFolder.c_str(), strFolderBak.c_str());

						std::string strTpautopack = strEnv + "tp_autopatch";
						if (HasDir(strTpautopack.c_str()))
						{
							RmDir(strTpautopack.c_str());
						}

						if (nRet != 0)
						{//如果重命名失败则删除此文件夹
							RmDir(strFolder.c_str());
						}
					}
					char szBuff[128] = { 0 };
					sprintf(szBuff, "folder=%s, permission deny", pszFolderName);
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_CREATE_LIB_ERROR, szBuff , 101);
				}
			}
			else
			{
				fclose(pFileTest);
				pFileTest = NULL;
				::remove(strPermissionTest.c_str());
			}
		}
		else
		{
			_MakeDir(strFolder.c_str());
		}
		return true;
	}

	bool CResOperation::CheckRes()
	{
	#ifndef _WIN32
		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strFileFolderCheck = strEnv + "ini/folder_check.dat";
		if (false == HasFile(strFileFolderCheck.c_str()))
		{
			CheckFolderPermission("c3pack");
			
			//保存标记
			FILE* pCheckFile = fopen(strFileFolderCheck.c_str(), "w");
			if (pCheckFile)
			{
				fclose(pCheckFile);
				pCheckFile = NULL;
			}
		}
	#endif

		bool bRet = false;
		int nBackupOrRevert = CAppConfigInfo::GetInstance()->getBackupRes();
		if (nBackupOrRevert > 0)
		{
			if (ACTION_REVERT == nBackupOrRevert)
			{
				bRet = RenameRes(ACTION_REVERT);
			}else
			{
				if (CAppConfigInfo::GetInstance()->isPackDownEnable())
				{
					if (ACTION_BACKUP == nBackupOrRevert)
					{
						bRet = RenameRes(ACTION_BACKUP);
					}
				}
			}
		}

		int nDeleteRes = CAppConfigInfo::GetInstance()->getDelRes();
		if (1 ==  nDeleteRes)
		{
			if (IsNeedDelete())
			{
				if (NULL == m_pThreadEvent)
				{
					m_pThreadEvent = new CThreadEvenDeleteRes();
				}
				m_pThreadDeleteRes = CMyThread::CreateNew(*m_pThreadEvent, CMyThread::RUN, 0);
			}
		}

		return bRet;
	}

	bool CResOperation::RenameRes(int nBackupOrRevert)
	{
		const char* szBackupFileSrc[BACKUP_FILE_COUNT] = {"c3.tpd", "c3.tpi", "data.tpd", "data.tpi"};

		bool bRet = true;
		std::string strBak = "bak_";
		for (int i = 0; i < BACKUP_FILE_COUNT; i++)
		{
			std::string strSrcFile = szBackupFileSrc[i];
			std::string strDestFile = strBak + szBackupFileSrc[i];
			if (false == RenameFile(strSrcFile.c_str(), strDestFile.c_str(), nBackupOrRevert))
			{
				DebugMsg("backup_file %s failed", szBackupFileSrc[i]);
				bRet = false;
			}
		}

		std::vector<std::string> vecFolder = CAppConfigInfo::GetInstance()->getDealFolders();

		if (vecFolder.size() == 0)
		{
			vecFolder.push_back("c3");
			vecFolder.push_back("data");
		}
		int nSize = vecFolder.size();
		if ( nSize > 0)
		{
			for (int i = 0; i < nSize; i++)
			{
				std::string strSrcDir = vecFolder.at(i);
				std::string strDestDir = strBak + strSrcDir;
				if (false == RenameFolder(strSrcDir.c_str(), strDestDir.c_str(), nBackupOrRevert))
				{
					DebugMsg("backup_dir %s failed", strSrcDir.c_str());
					bRet = false;
				}
			}
		}
		return bRet;
	}

	bool CResOperation::RenameFile(const char* pszOldFile, const char* pszNewFile, int nAction)
	{
		if (NULL == pszOldFile || NULL == pszNewFile) return false;
		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strSrc = "";
		std::string strDest = "";
		strSrc = strEnv + pszOldFile;
		strDest = strEnv + pszNewFile;
		std::string strVersion = strEnv + "version.dat";
		std::string strBakVersion = strEnv + "bak_version.dat";
		bool bSuc = true;
		if (ACTION_REVERT == nAction)
		{//还原时,若存在 c3.tpd, 则删除 bak_c3.tpd
			if (HasFile(strSrc.c_str()) && HasFile(strDest.c_str()))
			{
				bSuc = c3pack_down::MyDeleteFile(strDest.c_str());
			}
			if(HasFile(strDest.c_str()))
			{
				int nSuc = ::rename(strDest.c_str(), strSrc.c_str());
				bSuc = nSuc == 0 ? true : false;
			}

			if (HasFile(strBakVersion.c_str()))
			{//还原时将 bak_version.dat 
				if (HasFile(strVersion.c_str()))
				{
					c3pack_down::MyDeleteFile(strVersion.c_str());
				}
				int nSuc = ::rename(strBakVersion.c_str(), strVersion.c_str());
				bSuc = nSuc == 0 ? true : false;
			}

		}else if (ACTION_BACKUP == nAction)
		{//备份时,将 c3.tpd重命名成 bak_c3.tpd
			if (HasFile(strSrc.c_str()))
			{
				int nSuc = ::rename(strSrc.c_str(), strDest.c_str());
				bSuc = nSuc == 0 ? true : false;
			}
			
			if (!HasFile(strBakVersion.c_str()))
			{
				if (HasFile(strVersion.c_str()))
				{
					bSuc = c3pack_down::MyCopyFile(strBakVersion.c_str(), strVersion.c_str());
				}
			}
		}
		return bSuc;
	}

	bool CResOperation::RenameFolder(const char* pszOldFolder, const char* pszNewFolder, int nAction)
	{
		if (NULL == pszOldFolder || NULL == pszNewFolder) return false;
		std::string strEnv = c3pack_down::getAppCachePath();
		strEnv += "/";
		std::string strSrc = "";
		std::string strDest = "";
		strSrc = strEnv + pszOldFolder;
		strDest = strEnv + pszNewFolder;
		bool bSuc = true;
		if (HasDir(strSrc.c_str()) && HasDir(strDest.c_str()))
		{
			//不管备份还是还原
			//若已经存在c3和bak_c3,则将c3里的内容拷贝到bak_c3
			bSuc = CopyFolder(strSrc.c_str(), strDest.c_str());
			if (bSuc)
			{//2.拷贝完成后删除c3文件夹
				RmDir(strSrc.c_str());
			}
		}

		int nSuc = -1;
		if (ACTION_REVERT == nAction)
		{
			if (HasDir(strDest.c_str()))
			{
				nSuc = ::rename(strDest.c_str(), strSrc.c_str());
			}
		}else if (ACTION_BACKUP == nAction)
		{
			if (HasDir(strSrc.c_str()))
			{
				nSuc = ::rename(strSrc.c_str(), strDest.c_str());
			}
		}
		bSuc = nSuc == 0 ? true : false;
		
		return bSuc;
	}

	void CThreadEvenDeleteRes::OnThreadProcess(void)
	{
		CResOperation::DeleteRes();
	}
}