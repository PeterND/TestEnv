#include "Utility_pack.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace c3pack_down
{

	std::string g_AppInnerPath = "";
	std::string g_AppCachePath = "";
	int g_nFreeSpaceMb = 110;
	int g_nCurResVersion = 1000;

	const char* getAppInnerPath(void)
	{
		return g_AppInnerPath.c_str();
	}

	const char* getAppCachePath(void)
	{
		return g_AppCachePath.c_str();
	}

	void setAppInnerPath(const char* pszPath)
	{
		if (pszPath)
		{
			g_AppInnerPath = pszPath;
		}
	}

	void setAppCachePath(const char* pszPath)
	{
		if (pszPath)
		{
			g_AppCachePath = pszPath;
		}
	}

	void setVersion(int nVersion)
	{
		g_nCurResVersion = nVersion;
	}

	int getVersion(void)
	{
		return g_nCurResVersion;
	}

	int	Double2Int(double dValue)
	{
		if ((int)(dValue + 0.5) > (int)dValue)
			return int(dValue) + 1;
		else
			return int(dValue);
	}

	bool IsFileExist(const char* pszFile)
	{
#ifdef _WIN32
		return (GetFileAttributes(pszFile) != INVALID_FILE_ATTRIBUTES);
#else
		struct stat s;
		if (stat(pszFile, &s) == 0) {
			return true;
		}
		return false;
#endif
	}

	std::string GetFileName(const std::string& strFile)
	{
		std::string strFileName;
		int index = strFile.find_last_of("\\/");

		if (index == -1)
			strFileName = strFile;

		if (index == ((int)strFile.length() - 1))
			return "";

		strFileName = strFile.substr(index + 1);

		index = strFileName.find_last_of(".");
		if (index != -1)
		{
			strFileName = strFileName.substr(0, index);
		}

		return strFileName;
	}

	std::string GetFilePath(const std::string& strFile)
	{
		std::string strPath;
		int index = strFile.find_last_of("\\/");

		if (index == -1)
		{
			return "";
		}

		strPath = strFile.substr(0, index + 1);
		return strPath;
	}

	std::string GetResVersion( const char* pszFileName )
	{
		std::string strVersion;
		std::string strVersionFile = getAppCachePath();
		strVersionFile.append("/");
		strVersionFile.append(pszFileName);
		FILE* fp = fopen(strVersionFile.c_str(), "r");
		if (fp) 
		{
			char szTemp[64] = { 0 };
			fgets(szTemp, sizeof(szTemp) - 1, fp);
			int nStrlen = strlen(szTemp);
			if (nStrlen > 0 && '\n' == szTemp[nStrlen-1])
			{
				szTemp[nStrlen-1] = 0;
			}
			strVersion = szTemp;
			fclose(fp);
			if (strVersion.empty())
			{
				//ZBTODO
				//::LogMsg("fgets %s fail", pszFileName);
			}
		}
		else
		{
			strVersion = "1000";
		}

		return strVersion;
	}

	void SplitString(const std::string& strSrc, std::vector<std::string>& tokens, const std::string& delims)
	{
		std::string::size_type lastPos = strSrc.find_first_not_of(delims, 0);
		std::string::size_type pos = strSrc.find_first_of(delims, lastPos);
		while(std::string::npos != pos || std::string::npos != lastPos)
		{
			tokens.push_back(strSrc.substr(lastPos, pos - lastPos));
			lastPos = strSrc.find_first_not_of(delims, pos);
			pos = strSrc.find_first_of(delims, lastPos);
		}
	}

	int MyGetFreeSpace()
	{
		return g_nFreeSpaceMb;
	}

	void MySetFreeSpace(int nSpaceMb)
	{
		g_nFreeSpaceMb = nSpaceMb;
	}

	bool	MyCopyFile(const char* pszDes, const char* pszSrc)
	{
		FILE* pFileDes = fopen(pszDes, "wb");
		if (NULL == pFileDes)
			return false;
		FILE* pFileSrc = fopen(pszSrc, "rb");
		if (NULL == pFileSrc)
		{
			fclose(pFileDes);
			return false;
		}

		const char* pszRet = NULL;
		char szLine[1024] = {0};
		do 
		{
			pszRet = fgets(szLine, sizeof(szLine)-1, pFileSrc);
			if (NULL == pszRet)
				break;
			fputs(szLine, pFileDes);
		} while (true);
		fclose(pFileDes);
		fclose(pFileSrc);
		return true;
	}

#if defined(_WIN32) || defined(__ANDROID__)
	bool MyDeleteFile(const char *pszDirectory)
	{
		return 0 == remove(pszDirectory);
	}
#endif

	void MyCreateDirectory(const char *pszDirectory)
	{
#ifdef _WIN32
		mkdir(pszDirectory);
#else
		mkdir(pszDirectory, S_IRWXU | S_IRWXG | S_IRWXO);
		chmod(pszDirectory, 0777);
#endif
	}

	const char* GetPlatform()
	{
		static std::string strOs;
#if defined(_WIN32)
		strOs = "win";
#elif defined(__ANDROID__)
		strOs = "android";
#elif defined(__APPLE__)
		strOs = "ios";
#else
		strOs = "other";
#endif
		return strOs.c_str();
	}

	std::string ReplaceChar(const char* pszSource, char cFindChar, char cNewChar)
	{
		if (NULL == pszSource)
		{
			return "";
		}
		std::string strSource = pszSource;
		std::string str;
		str = "";
		for (unsigned int i = 0; i < strSource.size(); i++)
		{
			if (pszSource[i] == cFindChar)
			{
				str += cNewChar;
			}
			else {
				str += pszSource[i];
			}
		}
		return str;
	}
}