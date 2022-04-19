#ifndef _BGDOWNLOADMGR_H_
#define _BGDOWNLOADMGR_H_

#include "IPackResDown.h"
#include "IPackBaseDef.h"
#include "utility/Utility_pack.h"
#include <set>
#include <map>

namespace c3pack_down
{
#define DOWNLOAD_UPDATE_SORT  "tp_autopatch/update_sort.xml"
#define DOWNLOAD_PACK_DEL  "tp_autopatch/packdel.xml"
#define FILE_PRIOR  "tp_autopatch/prior.dat"
#define UPDATE_FINISH_FLAG  "tp_autopatch/flag_update_finish.dat"

#define VERIFY_ERR_INNER_CODE_NONE					0
#define VERIFY_ERR_INNER_CODE_DEL_PACK_TIMEOUT		1
#define VERIFY_ERR_INNER_CODE_VERIFY_TIMEOUT		2
#define VERIFY_ERR_INNER_CODE_USE_BAK_PACK 			3  //use bak pack
#define VERIFY_ERR_INNER_CODE_LOW_FREE_SPACE 		20

	// �ռ䲻�����С��ֵ
#define FREE_SPACE_MIN_SIZE 100

	class CBgDownloadMgr
	{
	public:
		virtual ~CBgDownloadMgr();
		static CBgDownloadMgr* GetInstance();
		static void ReleaseInstance();
	public:
		struct UpdateListInfo
		{
			unsigned int uFileSize;
			int nVersion;
			unsigned int uFileHash;
			std::string strPack;
			bool bHasDown;
			bool operator < (UpdateListInfo& rInfo)
			{
				return this->nVersion > rInfo.nVersion;
			}
			UpdateListInfo()
			{
				uFileSize = 0;
				nVersion = 0;
				uFileHash = 0;
				strPack = "";
				bHasDown = false;
			}
		};

		void Init(IDownloadManage* pDownloadMgr, int nBgThreadCreateNum);
		void ReLoad(int nStrategyType);
		bool LoadUpdateList(const char* pszUpdataList, bool bUpdate);
		bool SaveUpdateList(const char* pszRelName, int nStrategySave);
		void SortUpdateList(int nStrategyType);
		void StartDownloadRes();
		void PauseDownloadRes();
		void ResumeDownloadRes();
		void StopBgDownloadRes();
		void ProcessBgDown();
		//ǰ̨ѯ�ʺ�̨�Ƿ����������Դ
		bool IsDownloadingRes(const char* pszRes);
		//����Դ�������
		bool IsDownloadItemOver(const char* pszRes);
		unsigned int GetFileSizeByName(const char* pszFileName);
		CBgDownloadMgr::UpdateListInfo* GetUpdateInfo(const char* pszFileName); // ��ȡ���ļ�������Ϣ
		void SetFinishInfo(CBgDownloadMgr::UpdateListInfo* pInfo);
		void GetPackDownInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, char* pszCurFileName);
		void SetDownloadSpeed(int uSpeed);
		int GetDownloadSpeed(void);
		int GetSaveStategy(void);
		void SetUpdateFinishFlag(int nVersion);
		bool ResetUpdateFinishFlag(void);
		void SetExcludeFolder(const char* pszFolderNames);
		bool IsExistUpdateFinishFlag();
		bool CheckFreeSpaceLowerThan(int nCheckMb);
		bool CheckDownloadFinish();
		bool IsDownloadExcludeFile(const char* pszFileName);
		IPackDownloadTask* GetDownloadTask(void);
	private:
		CBgDownloadMgr();
		const char* GetUnDownLoadItem();
		bool LoadPriorityConfig(int nStrategy = 0);
		static bool PriorityStrategy(const UpdateListInfo& info1, const UpdateListInfo& info2);
		void SaveStategy(int nStrategy);
	private:
		static CBgDownloadMgr* s_pBgDownloadMgr;
		typedef std::vector<UpdateListInfo> UpdateListInfo_VEC;
		typedef std::map<std::string, int> StrInt_Map;
		IPackDownloadTask* m_pResDownload;
		IVertifyInstance* m_pVerify;
		IPackResDownloadItem* m_pResDownItem;
		IPackResDownloadListen* m_pResDownListen;
		UpdateListInfo_VEC m_vecUpdateListInfo;
		std::set<std::string> m_setDowningItem;
		std::vector<std::string> m_vecExcludeFolders;
		StrInt_Map m_mapFileNameIndex;
		std::string m_strUpdateListPath;
		std::string m_strEnvPath;
		std::string m_strCurDownFileName;
		std::string m_strNextDownFile;
		int m_nCurUndownIndex;
		int m_nBgThreadCreateNum;
		bool m_bPauseDownload;
		bool m_bStopDownload;
		int64 m_nFinishSize;
		int64 m_uTotalSize;
		int m_nFinishCount;
		int m_nTotalCount;
		int m_uDownLoadSpeed;
	};

	// �������
	class CBgResDownloadItem : public IPackResDownloadItem
	{
	public:
		// �����ʼִ��
		virtual void OnPackDownloadBegin(const char *pszFileName);

		// ������ִ�й����е�״̬��Ϣ
		virtual void OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte);

		// ���������
		virtual void OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr);
	};

	// �����������
	class CBgResDownloadListen : public IPackResDownloadListen
	{
	public:
		// ����������ļ��� ��������
		virtual void OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo);

		// ��ȡ���������е������ļ����������ٶȣ������߳���
		virtual void GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount);
	};
}
#endif