#ifndef _REUSEPACK_H_
#define _REUSEPACK_H_
#include "IPackResDown.h"
#include <set>
#include <string>
#include <map>

namespace c3pack_down
{
	#define REUSEPACKPATH  "c3pack/reuse_pack_info.xml"
	#define LAST_RES_VERSION  "tp_autopatch/resversion.dat"
	#define	PACKDELPATH	 "tp_autopatch/packdel.xml"
	#define RES_VERSION  "version.dat"

	class CReusePackMgr
	{
	public:
		struct FileInfo
		{
			std::string file;
			unsigned int  uid;
			bool operator < (const FileInfo& rInfo) const
			{
				return this->uid > rInfo.uid;
			}
		};
		virtual ~CReusePackMgr(void);
		static CReusePackMgr* GetInstance();
		static void ReleaseInstance();
		//更新复用表
		void Update(int nOldVersion);
		//保存复用表
		void Save();
		//资源全下载完就删除复用表
		bool Delete();
	private:
		bool LoadPackDelList(const char* pszPackDelList);
		bool LoadReusePackList(const char* pszReusePackList);
		bool LoadOldPackInfoList(const char* pszPackInfoFile,const char* pszPackInfoPack);
		bool SaveReusePackList(const char* pszReusePackName);
		int  GetOldVersion();
		const char* Find(const char* pszFile);
		typedef std::map<std::string,std::set<FileInfo> > PACKINFO_MAP;
		PACKINFO_MAP m_mapReusePack;
		PACKINFO_MAP m_mapOldPackInfo;
		std::set<std::string> m_setPackDel;
		static CReusePackMgr* s_pReusePackMgr;

	};
}
#endif