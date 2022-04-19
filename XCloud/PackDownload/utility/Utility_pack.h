#ifndef __C3PACK_UTILITY_PACK_H__
#define __C3PACK_UTILITY_PACK_H__

#include <string> 
#include <vector> 

namespace c3pack_down
{
	const char* getAppInnerPath(void);
	const char* getAppCachePath(void);

	void setAppInnerPath(const char* pszPath);
	void setAppCachePath(const char* pszPath);
	void setVersion(int nVersion);
	int getVersion(void);

	int	Double2Int(double dValue);
	bool IsFileExist(const char* pszFile);

	std::string GetFileName(const std::string& strFile);
	std::string GetFilePath(const std::string& strFile);
	std::string GetResVersion(const char* pszFileName);
	std::string ReplaceChar(const char* pszSource, char cFindChar, char cNewChar);
	const char* GetPlatform();
	void SplitString(const std::string& strSrc, std::vector<std::string>& tokens, const std::string& delims);

	int MyGetFreeSpace();
	void MySetFreeSpace(int nSpaceMb);
	bool MyDeleteFile(const char *pszDirectory);
	bool MyCopyFile(const char* pszDes, const char* pszSrc);
	void MyCreateDirectory(const char *pszDirectory);

} 
#endif // __C3PACK_UTILITY_H__
