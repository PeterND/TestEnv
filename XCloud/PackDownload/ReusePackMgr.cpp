#include "ReusePackMgr.h"
#include "ITqPackage.h"
#include <algorithm>
#include "IPackBaseDef.h"
#include "utility/Utility_pack.h"
#include "pugixml.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "C3BaseFuncInternal.h"
using namespace std;

CReusePackMgr* CReusePackMgr::s_pReusePackMgr = NULL;

#define PACKINFOPATH "c3pack/pack_info.xml"
#define PACKFILEUID	"uid"
#define PACKFILENAME "file"
#define PACKFILETYPE "File"
#define MAX_PATH_LEN   512

CReusePackMgr* CReusePackMgr::GetInstance()
{
	if (NULL == s_pReusePackMgr)
	{
		s_pReusePackMgr = new CReusePackMgr();
	}
	return s_pReusePackMgr;
}

void CReusePackMgr::ReleaseInstance()
{
	if (s_pReusePackMgr)
	{
		delete s_pReusePackMgr;
		s_pReusePackMgr = NULL;
	}
}

CReusePackMgr::~CReusePackMgr(void)
{
	m_mapReusePack.clear();
	m_mapOldPackInfo.clear();
	m_setPackDel.clear();
}

bool CReusePackMgr::LoadPackDelList(const char* pszPackDelList)
{
	static bool s_hasPackDel = false;
	if (s_hasPackDel)
		return true;
	if (NULL == pszPackDelList || !strcmp(pszPackDelList,"")) return false;

	std::string  strEnv = c3pack_down::getAppCachePath();
	strEnv.append("/");

	std::string  strPackDelList = strEnv;
	strPackDelList.append(pszPackDelList);
	if (!c3pack_down::IsFileExist(strPackDelList.c_str()))
	{
		return false;
	}

	pugi::xml_document doc;
	if (!doc.load_file(strPackDelList.c_str()))
	{
		::LogMsg(" %s doc open fail", strPackDelList.c_str());
		return false;
	}
	pugi::xml_node rootNode = doc.root();
	pugi::xml_node nodePackage = rootNode.child("PackageInfo");
	pugi::xml_node nodePack = nodePackage.child("Pack");
	m_setPackDel.clear();
	for (; !nodePack.empty(); nodePack=nodePack.next_sibling("Pack"))
	{
		pugi::xml_node nodeFileName = nodePack.child("FileName");
		std::string pack = nodeFileName.text().as_string();
		m_setPackDel.insert(pack);
	}
	s_hasPackDel = true;
	return true;
}

bool CReusePackMgr::SaveReusePackList(const char* pszReusePackName)
{
	if (NULL == pszReusePackName || !strcmp(pszReusePackName,"")) return false;
	if (m_mapReusePack.empty()) // 空数据不保存
	{
		return true;
	}

	pugi::xml_document doc;
	doc.reset();
	pugi::xml_node nodeDoc = doc.root();
	pugi::xml_node nodeC3Pack = nodeDoc.append_child("C3Pack");

	PACKINFO_MAP::iterator iter = m_mapReusePack.begin();
	for (; iter != m_mapReusePack.end(); iter++)
	{
		pugi::xml_node nodePack = nodeC3Pack.append_child("Pack");

		nodePack.append_attribute("file",iter->first);

		set<FileInfo>::iterator setiter = iter->second.begin();
		for (; setiter != iter->second.end(); setiter++)
		{
			pugi::xml_node nodeFile = nodePack.append_child(PACKFILETYPE);
			if (NULL == nodeFile) continue;
			nodeFile.append_attribute(PACKFILENAME,setiter->file);
			nodeFile.append_attribute(PACKFILEUID,setiter->uid);
		}
	}

	std::string strFile = c3pack_down::getAppCachePath();
	strFile.append("/");
	strFile.append(pszReusePackName);
	bool bSuc = doc.save_file(strFile.c_str());
	if (!bSuc) DebugMsg("xml resuepack  save fail");
	return bSuc;
}

bool CReusePackMgr::LoadReusePackList(const char* pszReusePackList)
{
	static bool s_hasReusePack = false;
	if (s_hasReusePack)
		return true;
	if (NULL == pszReusePackList || !strcmp(pszReusePackList,"")) return false;

	std::string  strEnv = c3pack_down::getAppCachePath();
	strEnv.append("/");

	std::string  strReusePackList = strEnv;
	strReusePackList.append(pszReusePackList);
	if (!c3pack_down::IsFileExist(strReusePackList.c_str()))
	{
		return false;
	}

	pugi::xml_document doc;
	if (!doc.load_file(strReusePackList.c_str()))
	{
		::LogMsg(" %s doc open fail", strReusePackList.c_str());
		return false;
	}
	pugi::xml_node rootNode = doc.root();
	pugi::xml_node nodeC3Pack = rootNode.child("C3Pack");
	pugi::xml_node nodePack = nodeC3Pack.child("Pack");
	m_mapReusePack.clear();
	for (; !nodePack.empty(); nodePack=nodePack.next_sibling("Pack"))
	{
		std::string packfile = nodePack.attribute(PACKFILENAME).as_string();
		set<FileInfo> files;
		pugi::xml_node nodeFile = nodeC3Pack.child("File");
		for (; !nodeFile.empty(); nodeFile=nodeFile.next_sibling("File"))
		{
			FileInfo info;
			info.file = nodeFile.attribute(PACKFILENAME).as_string();
			info.uid  = (unsigned int)nodeFile.attribute(PACKFILEUID).as_uint();
			files.insert(info);
		}
		m_mapReusePack[packfile] = files;
	}

	s_hasReusePack = true;
	return true;
}

bool CReusePackMgr::LoadOldPackInfoList(const char* pszPackInfoFile,const char* pszPackInfoPack)
{
	static bool s_hasOldPackInfo = false;

	if (s_hasOldPackInfo)
		return true;

	std::string strRoot = c3pack_down::getAppCachePath();
	IDataPack* pPack = DataPackCreate();
	pPack->SetPackFile(pszPackInfoPack);
	if (!pPack->LoadPack(strRoot.c_str()))
	{
		SAFE_RELEASE(pPack);
		return false;
	}
	pPack->SetPackState(IDataPack::PACK_STATE_READY);

	if (!pPack->IsPackReady(pPack->GetPackState()))
	{
	    SAFE_RELEASE(pPack);
		return false;
	}
	bool bSuc = false;
	string strSave ="";
	int nAmount = pPack->GetFileAmount();
	for (int i = 0; i < nAmount; i++)
	{
		DWORD nSize = 0;
		string strFile = pPack->GetFileName(i);
		if (0 == strcmp(strFile.c_str(),pszPackInfoFile))
		{
			pPack->GetFileLoader()->CheckFile(strFile.c_str(), &nSize);
			char* pBuf = (char*)malloc(nSize);
			pPack->GetFileLoader()->LoadFile(strFile.c_str(), pBuf, nSize);
			bSuc = true;
			
			strSave = strRoot + "/" + strFile;

			string strSavePath = c3pack_down::GetFilePath(strSave);
			c3pack_down::MyCreateDirectory(strSavePath.c_str());
			FILE* fp = fopen(strSave.c_str(), "wb");
			if (NULL == fp) {
				SAFE_RELEASE(pPack);
				free(pBuf);
				LogMsg("save file error, fileName=%s",strSave.c_str());
				return false;
			}
			fwrite( pBuf, sizeof( char ), nSize, fp );
			fclose(fp);
			free(pBuf);

			if (!c3pack_down::IsFileExist(strSave.c_str())) return false;

			pugi::xml_document doc;
			if (!doc.load_file(strSave.c_str()))
			{
				c3pack_down::MyDeleteFile(strSave.c_str());
				SAFE_RELEASE(pPack);
				::LogMsg("open xml file %s fail", strSave.c_str());
				return false;
			}

			pugi::xml_node rootNode = doc.root();
			pugi::xml_node nodePackage = rootNode.child("C3Pack");

			pugi::xml_node nodePack = nodePackage.child("Pack");
			m_mapOldPackInfo.clear();
			for (; !nodePack.empty(); nodePack=nodePack.next_sibling("Pack"))
			{
				std::string packfile = nodePack.attribute(PACKFILENAME).as_string();
				set<FileInfo> files;
				pugi::xml_node nodeFile = nodePack.child("File");
				for (; !nodeFile.empty(); nodeFile=nodeFile.next_sibling("File"))
				{
					FileInfo info;
					info.file = nodeFile.attribute(PACKFILENAME).as_string();
					info.uid  = nodeFile.attribute(PACKFILEUID).as_uint();
					files.insert(info);
				}
				m_mapOldPackInfo[packfile] = files;
			}
		}

	}
	s_hasOldPackInfo = true;
	c3pack_down::MyDeleteFile(strSave.c_str());
	SAFE_RELEASE(pPack);
	return bSuc;
}

const char* CReusePackMgr::Find(const char* pszFile)
{
	PACKINFO_MAP::iterator iter = m_mapReusePack.begin();
	for (; iter != m_mapReusePack.end(); iter++)
	{
		set<FileInfo>::iterator setiter = iter->second.begin();
		for (; setiter != iter->second.end(); setiter++)
		{
			if (0 == strcmp((setiter->file).c_str(),pszFile) )
			{
				return iter->first.c_str();
			}
		}
	}
	return NULL;
}

void CReusePackMgr::Update(int nOldVersion)
{
	if(this->LoadPackDelList(PACKDELPATH))
	{
		if (nOldVersion == 1000)
			nOldVersion = GetOldVersion();
		std::string strPack = c3pack_down::GetFilePath(string(PACKINFOPATH));
		strPack += c3pack_down::GetFileName(std::string(PACKINFOPATH));
		char szPackWithVersion[MAX_PATH] = {0};
		sprintf(szPackWithVersion, "%s_%d.pack",strPack.c_str(), nOldVersion);
		if (this->LoadOldPackInfoList(PACKINFOPATH,szPackWithVersion))
		{
			this->LoadReusePackList(REUSEPACKPATH);
			set<std::string>::iterator iter = m_setPackDel.begin();
			for (; iter != m_setPackDel.end(); iter++)
			{
				std::string pack  = *iter;
				PACKINFO_MAP::iterator oldpackiter = this->m_mapOldPackInfo.find(pack);
				if (oldpackiter != this->m_mapOldPackInfo.end())
				{
					if (oldpackiter->first == pack)
					{
						PACKINFO_MAP::iterator reusepackiter = this->m_mapReusePack.find(pack);
						if (reusepackiter != this->m_mapReusePack.end())
						{
							set<FileInfo>::iterator setiter = oldpackiter->second.begin();
							for (; setiter != oldpackiter->second.end(); setiter++)
							{
								FileInfo info;
								info.file = setiter->file;
								info.uid = setiter->uid;
								reusepackiter->second.insert(info);
							}
						}
						else
						{
							set<FileInfo> files;
							set<FileInfo>::iterator setiter = oldpackiter->second.begin();
							for (; setiter != oldpackiter->second.end(); setiter++)
							{
								FileInfo info;
								info.file = setiter->file;
								info.uid = setiter->uid;
								files.insert(info);
							}
							this->m_mapReusePack[pack] = files;
						}

					}


				}
				
			}
		}

	}
}

void CReusePackMgr::Save()
{
	SaveReusePackList(REUSEPACKPATH);
}

bool CReusePackMgr::Delete()
{
	std::string strCachePath = c3pack_down::getAppCachePath();
	strCachePath.append("/");
	strCachePath += REUSEPACKPATH;
	if (c3pack_down::IsFileExist(strCachePath.c_str()))
	{
		c3pack_down::MyDeleteFile(strCachePath.c_str());
		return true;
	}
	return false;
}

static void FindFiles(const char* pszDir, const char* pszExt, std::vector<std::string>& vFiles)
{
	if (NULL == pszDir || NULL == pszExt)
	{
		return;
	}
#ifdef _WIN32
#ifdef _WIN64
	std::string strDir = pszDir;
	std::string strExt = pszExt;

	char cLastChar = strDir[strDir.size() - 1];
	if (cLastChar != '\\')
	{
		strDir.append("\\");
	}

	std::string strFindFile = strDir;
	strFindFile += "*.*";
	_finddatai64_t	 findData;
	__int64 handle = _findfirst64(strFindFile.c_str(), &findData);
	if (handle == -1) return; //没有此文件目录,认为是正常的

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{

		}
		else
		{
			std::string strFile = strDir + findData.name;
			if (strFile.find(pszExt) != -1)
				vFiles.push_back(strFile);
		}
	} while (_findnext64(handle, &findData) == 0);

	_findclose(handle);

	return;
#else
	std::string strDir = pszDir;
	std::string strExt = pszExt;

	char cLastChar = strDir[strDir.size() - 1];
	if (cLastChar != '\\')
	{
		strDir.append("\\");
	}

	std::string strFindFile = strDir;
	strFindFile += "*.*";
	int handle;
	_finddata_t findData;
	handle = _findfirst(strFindFile.c_str(), &findData);
	if (handle == -1) return; //没有此文件目录,认为是正常的

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{

		}
		else
		{
			std::string strFile = strDir + findData.name;
			if (strFile.find(pszExt) != -1)
				vFiles.push_back(strFile);
		}
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);

	return;
#endif	
#else
	DIR *pDir = NULL;
	struct dirent *pEntry;
	struct stat statbuf;

	pDir = opendir(pszDir);
	if (!pDir) return;

	chdir(pszDir);
	while ((pEntry = readdir(pDir)) != NULL)
	{
		lstat(pEntry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode))
		{
		}
		else
		{
			char szFile[MAX_PATH_LEN] = { 0 };
			sprintf(szFile, "%s/%s", pszDir, pEntry->d_name);
			std::string strFile = std::string(szFile);
			if (strFile.find(pszExt) != -1)
				vFiles.push_back(strFile);
		}
	}

	chdir("..");
	closedir(pDir);
	return;
#endif
}

//自定义排序函数
bool Sort(const std::string& v1, const std::string& v2)
{
	return v1 < v2;//升序排列
}

int CReusePackMgr::GetOldVersion()
{
	std::string strEnv = c3pack_down::getAppCachePath();
	strEnv += "/";
	int nOldVersion = 1000;
	std::string strPack = c3pack_down::GetFilePath(std::string(PACKINFOPATH));
	strPack = strEnv + strPack;
	std::vector<std::string> vFiles;
	FindFiles(strPack.c_str(), ".pack", vFiles);
	std::sort(vFiles.begin(), vFiles.end(), Sort);
	if (vFiles.size() >= 2)
	{
		std::string strOldVersion = vFiles[0];
		strOldVersion = strOldVersion.substr(strOldVersion.find_last_of("_") + 1,  strOldVersion.find_last_of(".") - strOldVersion.find_last_of("_") - 1);
		return atoi(strOldVersion.c_str());
	}
	return nOldVersion;
}

