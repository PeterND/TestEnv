#include "PackDownManager.h"
#include "utility/Utility_pack.h"
#include "utility/SuperFastHash.h"
#include "BgDownloadMgr.h"
#include "PackLogSender.h"
#include "ResOperation.h"
#include "ReusePackMgr.h"
#include "AppConfigInfo.h"

namespace c3pack_down
{
	CPackDownManager* CPackDownManager::s_pPackDownManager = NULL;

	CPackDownManager::CPackDownManager()
	{
		m_pPackDownProxy = NULL;
		m_bVerifyDataSupport = false;
		m_bEnableInitRes = false;
		m_pszInitResPath = NULL;
		m_pszAppPath = NULL;
		m_pszCachePath = NULL;
		m_pResOp = NULL;
	}

	CPackDownManager::~CPackDownManager(void)
	{
		SAFE_DELETE(m_pResOp);
		CAppConfigInfo::ReleaseInstance();
	//ZBTODO	::DataSupport_Release();
	}

	IPackDownManager& CPackDownManager::getSingleton()
	{
		if (NULL == s_pPackDownManager)
		{
			s_pPackDownManager = new CPackDownManager();
		}

		return *s_pPackDownManager;
	}

	void CPackDownManager::deleteSigleton()
	{
		SAFE_DELETE(CPackDownManager::s_pPackDownManager);
	}

	//////////////////////////////////////////////////////////////////////////

	bool CPackDownManager::Init(int nGameType, int nCooperator, int nResVesion, const char* pszAppDir, const char* pszCacheDir,
								const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr, PFnSendDataToHttp pfnSend)
	{
		if (NULL == pszAppDir || NULL == pszCacheDir)
		{
			LogMsg("AppPath or CachePath NULL");
			return false;
		}

		c3pack_down::setAppInnerPath(pszAppDir);
		c3pack_down::setAppCachePath(pszCacheDir);
		c3pack_down::setVersion(nResVesion);

		m_pszAppPath = pszAppDir;
		m_pszCachePath = pszCacheDir;

		if (!CAppConfigInfo::GetInstance()->isPackDownEnable())
		{
			//ZBTODO	::DataSupport_Init(m_pszCachePath, m_pszAppPath, true, false);
			// 关闭微端时,启用默认PackDowan支持本地微端包加载
			return true;
		}

		CPackLogSender::GetInstance()->SetFeebackSendFuntion(pfnSend);

		if (CAppConfigInfo::GetInstance()->isPackDownEnable())
		{
			CPackLogSender::GetInstance()->SetPlatformCode(nGameType, nCooperator);

			bool bHightPriority = CAppConfigInfo::GetInstance()->getBackupRes() != 0; // 旧资源已经备份，调整为高优先级

			bool bUseDefaultPackDownProxy = CAppConfigInfo::GetInstance()->getOffline() == 1; // 精简配置，也为了避免配置默认下载器导致微端流程无法正常运行
			const char* pszRootPath = m_pszCachePath;
			const char* pszExtendPath = m_bEnableInitRes ? m_pszInitResPath : m_pszAppPath;
			if (NULL == pszExtendPath)
			{
				LogMsg("init packDown failed: ExtendPath null");
				return false;
			}

			//ZBTODO	::DataSupport_Init(pszRootPath, pszExtendPath, bUseDefaultPackDownProxy, bHightPriority);

			//设置代理,由DataSupport负责释放
			if (!bUseDefaultPackDownProxy)
			{
				m_pPackDownProxy = new CPackDownProxy_CDN(pszRootPath, m_bEnableInitRes);
				if (NULL == m_pPackDownProxy) { 
					LogMsg("new m_pPackDownProxy null");
					return false;
				}
				m_pPackDownProxy->SetNetWorkAddress(pszCndAddr, pszSourceAddr, pszBakAddr, pszFeedbackAddr);
				if (!m_pPackDownProxy->Init()) { 
					LogMsg("init m_pPackDownProxy false");
					return false;
				}
				//ZBTODO::DataSupport_SetPackDownProxy(m_pPackDownProxy, true);
			}
		}

		return true;
	}

	void CPackDownManager::SetPackDownCallback(PackDownCallback pFnCallback)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return;

		if (m_pPackDownProxy && pFnCallback)
		{
			m_pPackDownProxy->SetPackDownCallback(pFnCallback);
		}
	}

	void CPackDownManager::SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
								 const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion,int nFreeSpaceMb)
	{
		CPackLogSender::GetInstance()->SetFeedbackInfo(nServerID,nAccountID,pszAccountName, pszChannelID, pszDeviceID,pszDeviceName,pszOSVersion,nFreeSpaceMb);
	}

	bool CPackDownManager::ProcessPackVerify(void)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
		{
			return true; // 不启动微端直接跳过该流程，进入登录阶段
		}

		if (CAppConfigInfo::GetInstance()->getOffline() == 1)
		{
			return true;
		}

		static DWORD s_dwTimeBegin = 0;

		// 触发微端版本验证
		if (!m_bVerifyDataSupport)
		{
			s_dwTimeBegin = ::TimeGetTime();
			//ZBTODO	::DataSupport_Vertify();
			m_bVerifyDataSupport = true;
		}

		// 检测校验状态
		//ZBTODO	int nStatus = ::DataSupport_GetVertifyStatus();
		int nStatus = 0;
		bool bRet = false;
		switch (nStatus)
		{
		case DS_VERTIFY_SUC:
			bRet = true;
			DebugMsg("CPackDownManager::DS_VERTIFY_SUC %dms", ::TimeGetTime() - s_dwTimeBegin);
			break;
		case DS_VERTIFY_ING_VERSION:
		case DS_VERTIFY_ING_PACK_INFO:
			bRet = false;
			break;
		case DS_VERTIFY_FAIL_VERSION:
		case DS_VERTIFY_FAIL_DOWN_PACK_INFO:
		case DS_VERTIFY_FAIL_LOAD_PACK_INFO:
			// DebugMsg("CPackDownManager::DataSupport_Vertify() DS_VERTIFY_fail %d", nStatus);
			bRet = false;
			break;
		default:
			break;
		}
		return bRet;
	}

	void CPackDownManager::Process(void)
	{
		// 关闭微端的情况下也需要Process保证默认的DataSupport执行
	//ZBTODO	::DataSupport_Process(); // 数据下载驱动
	}

	int CPackDownManager::GetVertifyCode(void)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return 0;

		if (NULL == m_pPackDownProxy)
		{
			return 0;
		}

		return m_pPackDownProxy->GetVertifyCode();
	}

	int CPackDownManager::GetVertifyStatus(int& nSubStatus)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return 0;

		if (NULL == m_pPackDownProxy)
		{
			nSubStatus = 0;
			return 0;
		}

		return m_pPackDownProxy->GetVertifyStatusAndSubStatus(nSubStatus);
	}

	int CPackDownManager::GetStatusProgress(void)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return 0;

		if (NULL == m_pPackDownProxy)
		{
			return 0;
		}

		int nProgress = m_pPackDownProxy->GetStatusProgress();
		if (nProgress < 0)
		{
			nProgress = 0;
		}
		if (nProgress > 100)
		{
			nProgress = 100;
		}
		return nProgress;
	}

	//////////////////////////////////////////////////////////////////////////

	void CPackDownManager::EnableBgDownload(bool bEnable)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return;

		if (m_pPackDownProxy)
		{
			m_pPackDownProxy->EnableBgDownload(bEnable);
		}
	}

	void CPackDownManager::EnableInitRes(bool bEnable, const char* pszInitResPath)
	{
		m_bEnableInitRes = bEnable;
		m_pszInitResPath = pszInitResPath;
	}

	bool CPackDownManager::IsPackDownEnable(void)
	{
		return CAppConfigInfo::GetInstance()->isPackDownEnable();
	}

	bool CPackDownManager::IsFgDownloadEnable(void)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return false;

		return CAppConfigInfo::GetInstance()->isFgDownEnable();
	}

	bool CPackDownManager::IsBgDownloadEnable(void)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return false;

		if (m_pPackDownProxy)
		{
			return m_pPackDownProxy->IsBgDownloadEnable();
		}
		return false;
	}

	void CPackDownManager::SetDownloadOrder(int nStrategy)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return;
		if (nStrategy < 0) nStrategy = 0;
		CBgDownloadMgr::GetInstance()->ReLoad(nStrategy);
	}

	bool CPackDownManager::GetDownLoadInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uFinishSize, int& nCurDownSpeed, char* pszCurDownFileName, int& nDownStatus)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return false;

		CBgDownloadMgr::GetInstance()->GetPackDownInfo(nTotalCount, nFinishCount, uTotalSize, uFinishSize, pszCurDownFileName);
		nCurDownSpeed = CBgDownloadMgr::GetInstance()->GetDownloadSpeed();
		nDownStatus = CPackLogSender::GetInstance()->GetErrorCode();
		return true;
	}

	void CPackDownManager::SendC3CallbackMsg(const char* pszMsg)
	{
		if (false == CAppConfigInfo::GetInstance()->isPackDownEnable())
			return;
		// 共用9999的1102段收集C3Callback消息,版本信息放ErrorID字段
		int nResVersion = c3pack_down::getVersion();
		CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_CREATE_LIB_ERROR, pszMsg, nResVersion, false, 9999);
	}

	bool CPackDownManager::SwitchNetWorks(void)
	{
		if (NULL == m_pPackDownProxy)
		{
			return false;
		}
		return m_pPackDownProxy->SwitchNetWorks();
	}

	bool CPackDownManager::IsDownloadFinish()
	{
		int nTotalCount = 0;
		int nFinishCount = 0;
		unsigned int uTotalSize = 0;
		unsigned int uDownSize = 0;
		int nCurDownSpeed = 0;
		int nErrorCode = 0;
		char pszDownFileName[512]={0};
		GetDownLoadInfo(nTotalCount, nFinishCount, uTotalSize, uDownSize, nCurDownSpeed, pszDownFileName, nErrorCode);
		return nTotalCount == 0 || nFinishCount == nTotalCount;
	}

#ifdef _WIN32
	void CPackDownManager::SetFlowTime(int nUpFlowTime, int nDownFlowTime, bool bBg)
	{
		IPackDownloadTask* pTask = NULL;
		if (bBg)
		{
			pTask = CBgDownloadMgr::GetInstance()->GetDownloadTask();
		}else
		{
			if (m_pPackDownProxy)
			{
				pTask = m_pPackDownProxy->GetDownloadTask();
			}
		}
		if (pTask)
		{
			pTask->SetFlowTime(nUpFlowTime, nDownFlowTime);
		}
	}

	void CPackDownManager::GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow, bool bBg)
	{
		IPackDownloadTask* pTask = NULL;
		if (bBg)
		{
			pTask = CBgDownloadMgr::GetInstance()->GetDownloadTask();
		}else
		{
			if (m_pPackDownProxy)
			{
				pTask = m_pPackDownProxy->GetDownloadTask();
			}
		}
		if (pTask)
		{
			pTask->GetUpFlowInfo(dUpAverageFlow, dUpMaxFlow, dUpMinFlow);
		}
	}

	void CPackDownManager::GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow, bool bBg)
	{
		IPackDownloadTask* pTask = NULL;
		if (bBg)
		{
			pTask = CBgDownloadMgr::GetInstance()->GetDownloadTask();
		}else
		{
			if (m_pPackDownProxy)
			{
				pTask = m_pPackDownProxy->GetDownloadTask();
			}
		}
		if (pTask)
		{
			pTask->GetDownFlowInfo(dDownAverageFlow, dDownMaxFlow, dDownMinFlow);
		}
	}
#endif
	bool CPackDownManager::CheckOldVersionRes(const char* pszAppDir, const char* pszCacheDir)
	{
		bool bRet = false;
		if (pszAppDir && pszCacheDir)
		{
			c3pack_down::setAppInnerPath(pszAppDir);
			c3pack_down::setAppCachePath(pszCacheDir);
			m_pResOp = new CResOperation();
			if (m_pResOp)
			{
				m_pResOp->CheckRes();
				bRet = true;
			}
		}
		return bRet;
	}
}

IPackDownManager& GetPackDownManger()
{
	return c3pack_down::CPackDownManager::getSingleton();
}

void RelesePackDownManger()
{
	c3pack_down::CPackDownManager::deleteSigleton();
}