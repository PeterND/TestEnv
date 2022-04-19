#include "AppConfigInfo.h"
#include "pugixml.hpp"
#include "C3BaseFuncInternal.h"

namespace c3pack_down
{
	#define FILE_APP_CONFIG_XML				"ini/app_config.xml"

	#define ATTRIB_PACK_INFO			"pack_info"
	#define ATTRIB_FG_DOWN			"fg_download"
	#define ATTRIB_BG_DOWN			"bg_download"
	#define ATTRIB_EXCLUDE_FOLDER  "exclude_folder"
	#define ATTRIB_COUNT				"count"
	#define NODE_FG_THREAD_NUM		"FgThreadNum"
	#define NODE_BG_THREAD_NUM		"BgThreadNum"
	#define NODE_RES_OPERATION		"ResOperation"

	#define ATTRIB_ENABLE			"Enable"
	#define ATTRIB_LOG_ENABLE       "Log_Enable"
	#define ATTRIB_BACKUPRES		"BackupRes"
	#define ATTRIB_DELRES			"DelRes"
	#define ATTRIB_OFFLINE			"Offline"
	#define ATTRIB_RENAME_C3PACK	"RenameC3pack"
	#define ATTRIB_FOLDERS			"Folders"



	CAppConfigInfo* CAppConfigInfo::s_pAppConfigInfo = NULL;

	CAppConfigInfo::CAppConfigInfo()
	{
		m_bEnablePackDown = false;
		m_bEnableFgDown = false;
		m_bEnableBgDown = false;
		m_nLogEnable = 0;
		m_nFgThreadNum = 0;
		m_nBgThreadNum = 0;
		m_nBackupRes = 0;
		m_nDelRes = 0;
		m_nOffline = 0;
		m_nRenameC3pack = 0;
		ParseAppConfig(c3pack_down::getAppInnerPath(), c3pack_down::getAppCachePath());
	}

	CAppConfigInfo::~CAppConfigInfo()
	{

	}

	CAppConfigInfo* CAppConfigInfo::GetInstance()
	{
		if (NULL == s_pAppConfigInfo)
		{
			s_pAppConfigInfo = new CAppConfigInfo();
		}
		return s_pAppConfigInfo;
	}

	void CAppConfigInfo::ReleaseInstance()
	{
		SAFE_DELETE(s_pAppConfigInfo);
	}


	bool CAppConfigInfo::ParseAppConfig(const char* pszAppDir, const char* pszCacheDir)
	{
		if (NULL == pszAppDir || NULL == pszCacheDir) 
		{
			::LogMsg("ParseAppConfig fail pszAppDir pszCacheDir null");
			return false;
		}

		std::string  strEnv = pszCacheDir;
		strEnv.append("/");

		std::string  strAppConfig = strEnv;
		strAppConfig.append(FILE_APP_CONFIG_XML);
		if (!c3pack_down::IsFileExist(strAppConfig.c_str()))
		{
			strEnv = pszAppDir;
			strEnv.append("/");
			strAppConfig = strEnv;
			strAppConfig.append(FILE_APP_CONFIG_XML);
			if (!c3pack_down::IsFileExist(strAppConfig.c_str()))
			{
				::LogMsg("ParseAppConfig fail ini/app_config.xml not found");
				return false;
			}
		}

		pugi::xml_document doc;
		if (!doc.load_file(strAppConfig.c_str()))
		{
			::LogMsg(" %s doc open fail", strAppConfig.c_str());
			return false;
		}
		pugi::xml_node rootNode = doc.root();
		pugi::xml_node nodeApp = rootNode.child("App");
		pugi::xml_node nodeDS = nodeApp.child("DataSupport");

		m_bEnablePackDown = (1 == nodeDS.attribute(ATTRIB_ENABLE).as_int() ? true : false);

		m_nLogEnable = nodeDS.attribute(ATTRIB_LOG_ENABLE).as_int();
	
		m_strPackinfoXml = nodeDS.attribute(ATTRIB_PACK_INFO).as_string();
		m_bEnableFgDown = strcmp("on" , nodeDS.attribute(ATTRIB_FG_DOWN).as_string()) == 0 ? true : false;
		m_bEnableBgDown = strcmp("on" , nodeDS.attribute(ATTRIB_BG_DOWN).as_string())==0 ? true : false;
		std::string strExcludeFolder = nodeDS.attribute(ATTRIB_EXCLUDE_FOLDER).as_string();

		pugi::xml_node nodeDS_Fg = nodeDS.child(NODE_FG_THREAD_NUM);

		m_nFgThreadNum = nodeDS_Fg.attribute(ATTRIB_COUNT).as_int();
		pugi::xml_node nodeDS_Bg = nodeDS.child(NODE_BG_THREAD_NUM);
		m_nBgThreadNum = nodeDS_Bg.attribute(ATTRIB_COUNT).as_int();

		pugi::xml_node nodeDS_Op = nodeDS.child(NODE_RES_OPERATION);

		m_nBackupRes = nodeDS_Op.attribute(ATTRIB_BACKUPRES).as_int();
		m_nDelRes= nodeDS_Op.attribute(ATTRIB_DELRES).as_int();
		m_nOffline = nodeDS_Op.attribute(ATTRIB_OFFLINE).as_int();
		m_nRenameC3pack = nodeDS_Op.attribute(ATTRIB_RENAME_C3PACK).as_int();


		std::string strFolders = nodeDS_Op.attribute(ATTRIB_FOLDERS).as_string();
		if (strFolders.length() > 0)
		{
			std::string strFolders2= "";
			for (int s = 0; s < strFolders.size(); ++s)
			{
				if (strFolders.at(s) == ' ')
				{
					continue;
				}
				strFolders2.push_back(strFolders.at(s));
			}

			if (strFolders2.size() > 0)
			{
				SplitString(strFolders2, m_vecFolders, ",");
			}
		}


		pugi::xml_node nodePriority = nodeApp.child("DownloadPriority");

		std::vector<std::string> vecPackPath;
		for (int i = 0; i < 2; ++i)
		{
			char szName[64] = {0};
			sprintf(szName, "Strategy_%d", i);
			pugi::xml_node nodeStrategy = nodePriority.child(szName);
			pugi::xml_node nodePackPath = nodeStrategy.child("PackPath");
			for ( ; !nodePackPath.empty(); nodePackPath = nodePackPath.next_sibling("PackPath"))
			{
				std::string strPath = nodePackPath.attribute("path").as_string();
				vecPackPath.push_back(strPath.c_str());
			}
			m_mapPriority[szName] = vecPackPath;
		}
		return true;
	}
}