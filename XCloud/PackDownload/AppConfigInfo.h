#ifndef _APP_CONFIG_INFO_H_
#define _APP_CONFIG_INFO_H_
#include "utility/Utility_pack.h"
#include <map>
#include <string>
#include <vector>

namespace c3pack_down
{
typedef std::map<std::string, std::vector<std::string> >  MAP_PRIORITY;
	class CAppConfigInfo
	{
	public:
		static CAppConfigInfo* GetInstance();
		static void ReleaseInstance();
	private:
		bool ParseAppConfig(const char* pszAppDir, const char* pszCacheDir);

	public:
		bool isPackDownEnable(){ return m_bEnablePackDown; }
		bool isFgDownEnable() { return m_bEnableFgDown; }
		bool isBgDownEnalbe() { return m_bEnableBgDown; }
		int  getLogEnable() { return m_nLogEnable; }
		int  getFgThreadNum() { return m_nFgThreadNum; }
		int  getBgThreadNum() { return m_nBgThreadNum; }
		int  getBackupRes() { return m_nBackupRes; }
		int  getDelRes() { return m_nDelRes; }
		int  getOffline() { return m_nOffline; }
		int  getRenameC3pack() { return m_nRenameC3pack; }
		const char* getPackinfoXml() { return m_strPackinfoXml.c_str(); }
		const char* getExcludeFolders() { return m_strExcludeFolders.c_str(); }
		std::vector<std::string> getDealFolders() { return m_vecFolders; }
		MAP_PRIORITY getMapPriority() { return m_mapPriority; }

	private:
		bool m_bEnablePackDown;
		bool m_bEnableFgDown;
		bool m_bEnableBgDown;
		int m_nLogEnable;
		int m_nFgThreadNum;
		int m_nBgThreadNum;
		int m_nBackupRes;
		int m_nDelRes;
		int m_nOffline;
		int m_nRenameC3pack;
		std::string m_strPackinfoXml;
		std::string m_strExcludeFolders;
		std::vector<std::string> m_vecFolders;
		MAP_PRIORITY m_mapPriority;

	private:
		CAppConfigInfo();
		virtual ~CAppConfigInfo();
		
	private:
		static CAppConfigInfo* s_pAppConfigInfo;
	};
}
#endif
