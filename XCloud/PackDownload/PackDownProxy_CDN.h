#ifndef _PACKDOWNPROXY_CDN_H
#define _PACKDOWNPROXY_CDN_H

#include "IPackDownProxy.h"	
#include "IPackResDown.h"
#include "PackLogSender.h"
#include "C3BaseFuncInternal.h"

namespace c3pack_down
{

	class CPackDownProxy_CDN : public IPackDownProxy, public IResVertifyStatus
	{
	public:
		CPackDownProxy_CDN(const char* pszRootPath, bool bUseInitObb = false);
		virtual ~CPackDownProxy_CDN(void);

		virtual void Release(void) { delete this; }
		virtual bool Init(void);
		virtual void VersionVertify(void);
		virtual int GetVertifyCode(void);
		virtual int GetVertifyStatus(void);
		virtual int GetVertifyStatusAndSubStatus(int& nSubStatus);
		virtual int GetStatusProgress(void);

		virtual bool SendRequest(const char* pszPack);
		virtual bool ProcessResult(void);
		virtual const char* GetLastError(void);

	public:
		// У����Ȼص�
		virtual void OnVertifyFile(int nVertifyStatus, int nProgress, int nVertifyType, const char* pszErrInfo);
		virtual void OnVertifyProgressMsgs(const char *pszMsg);
		virtual void OnDeleteFile(int nCurrentCount, int nTotalCount);
		virtual void RelatePackInfo(void);
		virtual void ApplyPackInfo(void);
		virtual void SpringLoadPackInfo(void);

	public:
		//�Ƿ����ú�̨����,Ĭ�������� ini/app_config.xml
		void EnableBgDownload(bool bEnable);

		//
		void SetPackDownCallback(PackDownCallback pfnCallback);

		bool SwitchNetWorks(void);

		//��̨�����Ƿ���
		bool IsBgDownloadEnable(void);

		//�Ƿ�֧�ִ�ĳ�ʼ��
		void EnableInitResPack(bool bEnable);

		//���������ַ�Ĵ���,Ҫ��init����ǰ����
		void SetNetWorkAddress(const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr);

		IPackDownloadTask* GetDownloadTask(void);
	public:
		virtual void OnPackInfoDown(const char* pszFile, bool bSuc);

	private:
		bool ShouldStartBgDown();
		void DeleteInvalidPack();
		void OnDelInvalidPackOver(bool bTimeout = false);
		bool DealDelListFile(const char* pszPackDelList);
		void LoadDownLoadList();

	private:
		IDownloadManage * m_pDownloadMgr;
		IPackDownloadTask* m_pPackDown;
		IVertifyInstance* m_pVerifyInst;
		IPackResDownloadItem* m_pResDownItem;
		IPackResDownloadListen* m_pResDownListen;
		std::set<std::string> m_setBgDownItem;
		int m_nVertifyStatus;
		int m_nVertifySubStatus;
		int m_nStatusProgress;
		int m_nLastResVersion;
		int m_nCurResVersion;
		int m_nIsUpdate;
		int m_nFgThreadCreateNum;
		int m_nBgThreadCreateNum;
		DWORD m_dwTimeStamp;
		std::string m_strPackInfoFile;
		std::string m_strPackInfoPack;
		std::string m_strRootPath;
		std::string m_strCdnDomainAddr;
		std::string m_strSrcDomainAddr;
		std::string m_strSrcIpAddr;
		std::string m_strFeedBackAddr;
		DWORD m_dwTimeBeginDelPack;
		DWORD m_dwTimeBeginVertify;
		DWORD m_dwTimeBeginPackInfo;
		bool m_bDelInvalidPackOver;
		bool m_bDownPackInfo;
		bool m_bStartTask;
		bool m_bWaitBgDownOver;
		bool m_bEnableBgDown;
		bool m_bEnableFgDown;
		bool m_bUseInitObb;
	};

	// �������
	class CPackResDownloadItem : public IPackResDownloadItem
	{
	public:
		CPackResDownloadItem();
		virtual ~CPackResDownloadItem();
		// �����ʼִ��
		virtual void OnPackDownloadBegin(const char *pszFileName);

		// ������ִ�й����е�״̬��Ϣ
		virtual void OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte);

		// ���������
		virtual void OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr);
	};

	// �����������
	class CPackResDownloadListen : public IPackResDownloadListen
	{
	public:
		CPackResDownloadListen();
		virtual ~CPackResDownloadListen();
		// ����������ļ��� ��������
		virtual void OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo);

		// ��ȡ���������е������ļ����������ٶȣ������߳���
		virtual void GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount);
	};
}
#endif