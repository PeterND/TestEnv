#include "PackDownProxy_CDN.h"
#include "C3BaseFuncInternal.h"
#include "utility/Utility_pack.h"
#include "pugixml.hpp"
#include "BgDownloadMgr.h"
#include "ReusePackMgr.h"
#include "PackDownManager.h"
#include "ResOperation.h"
#include "AppConfigInfo.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma comment(lib,"PackResUpdate.lib")
#endif

namespace c3pack_down
{
	#define HIGH_PRIORITY_FOLDER	"c3pack/data/map"
	#define HIGH_PRIORITY_FOLDER2	"c3pack/data/3dmap"

	#define INIT_RES_PACK_INFO  "pack_info_init.xml"
	#define INIT_RES_PACK_OBB   "init.obb"

	#define MAX_PACK_PATH   512
	#define TIME_START_BG_DOWN  5000

	#define		CHECK(x)	{ if(!(x)) { return;} }
	#define		CHECKF(x)	{ if(!(x)) { return 0;} }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CPackDownProxy_CDN::CPackDownProxy_CDN(const char* pszRootPath, bool bUseInitObb)
	{
		m_pDownloadMgr = NULL;
		m_pPackDown = NULL;
		m_pVerifyInst = NULL;
		m_nVertifyStatus = DS_VERTIFY_NONE;
		m_nVertifySubStatus = DS_SUB_STATUS_NONE;
		m_nStatusProgress = 0;
		m_dwTimeStamp = 0;
		m_pResDownItem = NULL;
		m_pResDownListen = NULL;
		m_dwTimeBeginDelPack = 0;
		m_dwTimeBeginVertify = 0;
		m_dwTimeBeginPackInfo = 0;
		m_bDelInvalidPackOver = false;
		m_bStartTask = false;
		m_bDownPackInfo = false;
		m_bWaitBgDownOver = false;
		m_bEnableBgDown = false;
		m_bEnableFgDown = true;
		m_bUseInitObb = bUseInitObb;
		m_nLastResVersion = 1000;
		m_nCurResVersion = 1000;
		m_nFgThreadCreateNum = 1;
		m_nBgThreadCreateNum = 1;
		m_nIsUpdate = -1;

		m_strCdnDomainAddr = "";
		m_strSrcDomainAddr = "";
		m_strSrcIpAddr = "";
		m_strFeedBackAddr = "";

		if (pszRootPath)
		{
			m_strRootPath = pszRootPath;
		}
	}

	CPackDownProxy_CDN::~CPackDownProxy_CDN(void)
	{
		CBgDownloadMgr::ReleaseInstance();
		if (m_pDownloadMgr)
		{
			m_pDownloadMgr->DestoryTask();
			SAFE_RELEASE(m_pDownloadMgr);
		}
		CReusePackMgr::ReleaseInstance();
		SAFE_DELETE(m_pResDownItem);
		SAFE_DELETE(m_pResDownListen);
	}

	const char* CPackDownProxy_CDN::GetLastError(void)
	{
		return CPackLogSender::GetInstance()->GetLastError();
	}

	void CPackDownProxy_CDN::SetNetWorkAddress(const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr)
	{
		m_strCdnDomainAddr = pszCndAddr;
		m_strSrcDomainAddr = pszSourceAddr;
		m_strSrcIpAddr = pszBakAddr;
		m_strFeedBackAddr = pszFeedbackAddr;
	}

	bool CPackDownProxy_CDN::Init(void)
	{

		const char* pszAppPath = c3pack_down::getAppInnerPath();
		const char* pszCachePath = c3pack_down::getAppCachePath();

		std::string strPlatform = c3pack_down::GetPlatform();

		m_strPackInfoFile = CAppConfigInfo::GetInstance()->getPackinfoXml();

		m_bEnableBgDown = CAppConfigInfo::GetInstance()->isBgDownEnalbe();
		m_bEnableFgDown = CAppConfigInfo::GetInstance()->isFgDownEnable();
		m_nFgThreadCreateNum =  CAppConfigInfo::GetInstance()->getFgThreadNum();
		m_nBgThreadCreateNum =  CAppConfigInfo::GetInstance()->getBgThreadNum();

		if (m_strPackInfoFile.length() == 0) return false;
		if (m_strRootPath.length() == 0) return false;
		if (m_strCdnDomainAddr.length() == 0) return false;
		if (m_strSrcDomainAddr.length() == 0) return false;
		if (m_strSrcIpAddr.length() == 0) return false;
		if (m_strFeedBackAddr.length() == 0) return false;

		m_pDownloadMgr = CreatePackResDown();
		CHECKF(m_pDownloadMgr);
		m_pVerifyInst = m_pDownloadMgr->GetVertifyInstance();
		CHECKF(m_pVerifyInst);
		m_pVerifyInst->InitConfig(m_strRootPath.c_str(), pszAppPath, m_strCdnDomainAddr.c_str(), m_strSrcDomainAddr.c_str(),m_strSrcIpAddr.c_str());
		m_pVerifyInst->InitResVertifyCallback(this);
		
		m_pPackDown = m_pDownloadMgr->CreateTask();
		CHECKF(m_pPackDown);
		m_pResDownItem = new CPackResDownloadItem();
		m_pResDownListen = new CPackResDownloadListen();

		CPackLogSender::GetInstance()->SetDownloadAddr(m_strCdnDomainAddr.c_str(), m_strSrcDomainAddr.c_str(),m_strSrcIpAddr.c_str());
		CPackLogSender::GetInstance()->SetFeedbackAddr(m_strFeedBackAddr.c_str());
		
		m_nLastResVersion = atoi(c3pack_down::GetResVersion(LAST_RES_VERSION).c_str());
		return true;
	}

	void CPackDownProxy_CDN::SetPackDownCallback(PackDownCallback pfnCallback)
	{
		CPackLogSender::GetInstance()->SetCallback(pfnCallback);
	}

	bool CPackDownProxy_CDN::SwitchNetWorks(void)
	{
		CHECKF(m_pDownloadMgr);
		return m_pDownloadMgr->SwitchNetWorks();
	}

	void CPackDownProxy_CDN::DeleteInvalidPack()
	{
		m_dwTimeBeginDelPack = ::TimeGetTime();

		std::string  strEnv = c3pack_down::getAppCachePath();
		strEnv.append("/");

		std::string  strUpdateList = strEnv;
		strUpdateList.append(DOWNLOAD_UPDATE_SORT);
		bool bUpdateListExist = c3pack_down::IsFileExist(strUpdateList.c_str());

		std::string  strPackDelPath = strEnv;
		strPackDelPath.append(DOWNLOAD_PACK_DEL);
		bool bPackDelExist = c3pack_down::IsFileExist(strPackDelPath.c_str());

		std::string  strFlagUpdateFinish = strEnv;
		strFlagUpdateFinish.append(UPDATE_FINISH_FLAG);
		bool bFlagUpdateFinishExist = c3pack_down::IsFileExist(strFlagUpdateFinish.c_str());

		if (!bUpdateListExist && bPackDelExist && bFlagUpdateFinishExist)
		{
			//�������ʱ�����б������Ҵ���ɾ���б�,��ɾ���ɰ汾��Դ
			m_nVertifySubStatus = DS_SUB_STATUS_DEL_OLD_PACK;
			
			if (CReusePackMgr::GetInstance()->Delete())
			{
				DealDelListFile(PACKDELPATH);
				DebugMsg("DelListFile:%d ms", ::TimeGetTime() - m_dwTimeBeginDelPack);
			}
			if (m_pVerifyInst)
			{
				m_nStatusProgress = 0;
				m_pVerifyInst->DeleteDifferFile();
			}
		}
		else
		{
			this->OnDelInvalidPackOver();
		}
	}

	void CPackDownProxy_CDN::OnDelInvalidPackOver(bool bTimeout)
	{
		if (m_bDelInvalidPackOver){
			return;
		}
		m_bDelInvalidPackOver = true;

		if (bTimeout)
		{
			if (m_pVerifyInst)
			{
				m_pVerifyInst->StopDeleteDifferFile();
			}
			char szVerifyError[128];
			sprintf(szVerifyError, "cdn:%s,DelInvalidPack Timeout",m_strCdnDomainAddr.c_str());
			CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_ERROR, szVerifyError, VERIFY_ERR_INNER_CODE_DEL_PACK_TIMEOUT);
		}
		
		DebugMsg("DelInvalidPack:%d ms %s", ::TimeGetTime() - m_dwTimeBeginDelPack, bTimeout ? "timeout" : "");
		this->VersionVertify();
	}

	bool CPackDownProxy_CDN::DealDelListFile(const char* pszPackDelList)
	{
		if (NULL == pszPackDelList || !strcmp(pszPackDelList,"")) return false;

		std::string  strEnv = c3pack_down::getAppCachePath();
		strEnv.append("/");

		std::string  strPackDelList = strEnv;
		strPackDelList.append(pszPackDelList);
		if (!c3pack_down::IsFileExist(strPackDelList.c_str()))
		{
			::LogMsg("%s not found", pszPackDelList);
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
		for (; !nodePack.empty(); nodePack=nodePack.next_sibling("Pack"))
		{
			pugi::xml_node nodeFileName = nodePack.child("FileName");
			std::string pack = nodeFileName.text().as_string();
	//ZBTODO		GetPluginForMY()->OnDelete_ResPack(pack.c_str());
		}
		return true;
	}

	void CPackDownProxy_CDN::VersionVertify(void)
	{
		CHECK(m_pVerifyInst);
		m_nVertifyStatus = DS_VERTIFY_ING_VERSION;

		if (!m_bDelInvalidPackOver)
		{
			CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_START, "", 0);
			this->DeleteInvalidPack();
			return;
		}

		m_nCurResVersion = c3pack_down::getVersion();
		m_pVerifyInst->SetServerVersion(m_nCurResVersion);

		m_dwTimeBeginVertify = ::TimeGetTime();
		m_nVertifySubStatus = DS_SUB_STATUS_CHECK_VERSION;
		m_pVerifyInst->VertifyPackRes(false);
	}

	int CPackDownProxy_CDN::GetVertifyCode(void)
	{
		return m_nIsUpdate;
	}

	int CPackDownProxy_CDN::GetVertifyStatus(void)
	{
		return m_nVertifyStatus;
	}

	int CPackDownProxy_CDN::GetVertifyStatusAndSubStatus(int& nSubStatus)
	{
		nSubStatus = m_nVertifySubStatus;
		return m_nVertifyStatus;
	}

	int CPackDownProxy_CDN::GetStatusProgress(void)
	{
		return m_nStatusProgress;
	}

	bool CPackDownProxy_CDN::SendRequest(const char* pszPack)
	{
		CHECKF(pszPack);
		CHECKF(m_pPackDown);
		if (DS_VERTIFY_ING_VERSION == m_nVertifyStatus)
		{
			DebugMsg("SendRequest on verifying version! pack=%s", pszPack);
		}
		else
		{
			DebugMsg("SendRequest pack=%s", pszPack);
		}

		// �ر�ǰ̨ʱ������pack_info��,������������������
		if (!m_bEnableFgDown)
		{
			if (DS_VERTIFY_ING_PACK_INFO != m_nVertifyStatus)
			{
			//ZBTODO	::DataSupport_PackDownOver(pszPack, false, 0, TimeGetTime());
				return false;
			}
		}

		if (!m_bStartTask)
		{
			if (m_nFgThreadCreateNum < 1) 
			{
				DebugMsg("sendRequest fail FgThreadCreateNum < 1");
			}
			int nThreadNum = m_nFgThreadCreateNum > 0 ? m_nFgThreadCreateNum : 1;
			bool bSuc = m_pPackDown->StartTask(m_pResDownItem, m_pResDownListen, nThreadNum);
			if (!bSuc)
			{
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_CREATE_LIB_ERROR, pszPack, m_nFgThreadCreateNum, false);
				return false;
			}
			m_bStartTask = true;
		}
		//�Ȳ鿴��̨�Ƿ���������,����,��ǰ̨������,�ȴ���̨����
		if (CBgDownloadMgr::GetInstance()->IsDownloadingRes(pszPack))
		{
			m_setBgDownItem.insert(pszPack);
		}
		else
		{
			//ǰ̨������Դʱ,��ͣ��̨����
			CBgDownloadMgr::GetInstance()->PauseDownloadRes();

			// �����ȼ���Ŀ¼��������
			static int nHPFolderLen = strlen(HIGH_PRIORITY_FOLDER);
			static int nHPFolderLen2 = strlen(HIGH_PRIORITY_FOLDER2);
			bool bPrior = (0 == strncmp(HIGH_PRIORITY_FOLDER, pszPack, nHPFolderLen));
			if (!bPrior)
			{
				bPrior |= (0 == strncmp(HIGH_PRIORITY_FOLDER2, pszPack, nHPFolderLen2));
				if (bPrior)
				{
					std::string strPack = pszPack;
					if (strPack.find("model/") != std::string::npos || strPack.find("lightmap/") != std::string::npos)
					{
						bPrior = false;
					}
				}
				if (bPrior)
				{
					::DebugMsg("Height Prior:%s", pszPack);
				}
			}
			m_pPackDown->PushDownloadItem(bPrior, pszPack);
			m_dwTimeStamp = 0;
		}
		return true;
	}

	bool CPackDownProxy_CDN::ProcessResult(void)
	{
		//������֤�汾��Ϣ
		if (m_pVerifyInst && (m_nVertifyStatus == DS_VERTIFY_ING_VERSION))
		{
			m_pVerifyInst->EventProcess();
			if (!m_bDelInvalidPackOver && m_dwTimeBeginDelPack!= 0 && TimeGetTime() - m_dwTimeBeginDelPack > 30000) // ɾ���ļ��Ĳ�����ʱ30�봥����ʱ
			{
				this->OnDelInvalidPackOver(true);
			}
			return true;
		}

		CHECKF(m_pPackDown);
		//�����̨��Դ����
		CBgDownloadMgr::GetInstance()->ProcessBgDown();
		m_pPackDown->EventProcess();
		EnPackCompleteStatus eStatus;
		const char* pFileName = m_pPackDown->PopDownloadItem(eStatus);
		if (pFileName)
		{
			if (m_bDownPackInfo && (0 == strcmp(m_strPackInfoPack.c_str(), pFileName)))
			{
				m_nVertifySubStatus = DS_SUB_STATUS_DOWN_PACK_INFO_END;
				this->OnPackInfoDown(pFileName, eStatus == EnRes_Completed);
			}else
			{
			//ZBTODO	::DataSupport_PackDownOver(pFileName, eStatus == EnRes_Completed, 0, TimeGetTime());
				if (eStatus == EnRes_Completed)
				{
					if (!CBgDownloadMgr::GetInstance()->IsDownloadExcludeFile(pFileName))
					{
						CBgDownloadMgr::GetInstance()->SetFinishInfo(CBgDownloadMgr::GetInstance()->GetUpdateInfo(pFileName));
					}
					CBgDownloadMgr::GetInstance()->CheckDownloadFinish();
				}
			}
		}else
		{
			//������ɺ�,ɾ��ǰ̨���صļ�¼,���ǰ̨���������,�ָ���̨����
			if (ShouldStartBgDown())
			{
				CBgDownloadMgr::GetInstance()->ResumeDownloadRes();
			}
			else
			{
				CBgDownloadMgr::GetInstance()->PauseDownloadRes();
			}
		}

		int nBgDownSize = m_setBgDownItem.size();
		if (nBgDownSize > 0)
		{
			std::set<std::string>::iterator iter = m_setBgDownItem.begin();
			for (; iter != m_setBgDownItem.end(); )
			{
				std::string strBgDownloadRes= *iter;
				if (CBgDownloadMgr::GetInstance()->IsDownloadItemOver(strBgDownloadRes.c_str()))
				{	//ǰ̨�ȴ���̨���ص���Դ��ɺ�����ʹ��
		//ZBTODO			::DataSupport_PackDownOver(strBgDownloadRes.c_str(), true, 0, TimeGetTime());
					m_setBgDownItem.erase(iter++);
				}else
				{
					++iter;
				}
			}
			
		}
		return true;//Ŀǰû��ʹ��
	}

	//////////////////////////////////////////////////////////////////////////

	bool CPackDownProxy_CDN::ShouldStartBgDown()
	{
		int nCdnUnfinishDownItemCount = m_pPackDown->UnFinishDownloadItemCount();
		if ( nCdnUnfinishDownItemCount == 0 && m_dwTimeStamp == 0)
		{
			m_dwTimeStamp = TimeGetTime();
		}
		//ǰ̨������ɺ�5�뿪ʼ��̨����
		if (m_bEnableBgDown && (m_nVertifyStatus == DS_VERTIFY_SUC) 
			&& (nCdnUnfinishDownItemCount == 0) &&  (m_dwTimeStamp != 0 )&& ((TimeGetTime() - m_dwTimeStamp) > TIME_START_BG_DOWN))
		{
			return true;
		}

		return false;
	}

	void CPackDownProxy_CDN::EnableBgDownload(bool bEnable)
	{
		m_bEnableBgDown = bEnable;
		CHECK(m_pPackDown);
		if (bEnable)
		{
			CBgDownloadMgr::GetInstance()->ResumeDownloadRes();
		}
		else
		{
			CBgDownloadMgr::GetInstance()->PauseDownloadRes();
		}
	}

	bool CPackDownProxy_CDN::IsBgDownloadEnable(void)
	{
		return m_bEnableBgDown;
	}

	void CPackDownProxy_CDN::EnableInitResPack(bool bEnable)
	{
		if (bEnable)
		{
			//::DataSupport_RelatePackInfo(INIT_RES_PACK_INFO, INIT_RES_PACK_OBB); // ����Ҫ���������ù�������
		//ZBTODO	::DataSupport_LoadPackInfo_Extend(INIT_RES_PACK_INFO, INIT_RES_PACK_OBB);
		}

	}

	void CPackDownProxy_CDN::OnVertifyFile(int nVertifyStatus, int nProgress, int nVertifyType, const char* pszErrInfo)
	{
		if (EnVertify_Update == nVertifyStatus)
		{
			if (m_nVertifySubStatus != DS_SUB_STATUS_UPDATE_VERSION)
			{
				m_nVertifySubStatus = DS_SUB_STATUS_UPDATE_VERSION;
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_UPDATE_BEGIN, "none", m_nCurResVersion);
				CBgDownloadMgr::GetInstance()->ResetUpdateFinishFlag();//ɾ���ϴΰ汾������ɵı��
			}
		}
		m_nStatusProgress = nProgress;

		if (nProgress < 100) return;
		m_nIsUpdate = nVertifyStatus;
		switch (m_nIsUpdate)
		{
		case EnVertify_Fail:
		case EnVertify_DownPackFail:
		case EnVertify_PackHashFail:
		case EnVertify_ParsePackFail:
		case EnVertify_PackDecompressFail:
		case EnVertify_PathEmptyFail:
		case EnVertify_GetServerFail:
		case EnVertify_CreateConfigFail:
		case EnVertify_SetVersionFail:
		case EnVertify_PackVersionError:
			{
				m_nVertifyStatus = DS_VERTIFY_FAIL_VERSION;
				m_nVertifySubStatus = m_nIsUpdate >= EnVertify_PackVersionError ? DS_SUB_STATUS_VERTIFY_FAIL_LOCAL_ERROR : DS_SUB_STATUS_VERTIFY_FAIL_SERVER_ERROR;

				CPackLogSender::GetInstance()->SetPackDownError(ERROR_TYPE_VERIFY, m_nIsUpdate);
				char szVerifyError[128];
				sprintf(szVerifyError, "verifyType:%d, cdn:%s, err:%s", nVertifyType, m_strCdnDomainAddr.c_str(), pszErrInfo ? pszErrInfo : "");

				//����ֻ��ռ�С��10M,������ʧ����ʶ�ǿռ䲻��
				if (CBgDownloadMgr::GetInstance()->CheckFreeSpaceLowerThan(FREE_SPACE_MIN_SIZE))
				{
					m_nIsUpdate = VERIFY_ERR_INNER_CODE_LOW_FREE_SPACE;
					CPackLogSender::GetInstance()->OnPackDownCallback(STATUS_NOT_ENOUGH_SPACE, m_strPackInfoFile.c_str());
				}
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_ERROR, szVerifyError, m_nIsUpdate);
			}
			break;
		case EnVertify_Update://��Ҫ����
		case EnVertify_UnUpdate://�汾���ø���
			{
				DWORD dwVertifyTime = ::TimeGetTime() - m_dwTimeBeginVertify;
				if (EnVertify_Update == m_nIsUpdate)
				{
					if (dwVertifyTime > 30000)
					{
						char szError[128];
						sprintf(szError, "cdn:%s,update timeout:%d",m_strCdnDomainAddr.c_str(), dwVertifyTime);
						CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_ERROR, szError, VERIFY_ERR_INNER_CODE_VERIFY_TIMEOUT);
					}
					if (nVertifyType != 0)
					{
						char szVerifyType[128] = { 0 };
						sprintf(szVerifyType, "verifyType:%d", nVertifyType);
						CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_ERROR, szVerifyType, VERIFY_ERR_INNER_CODE_USE_BAK_PACK);
					}
					DebugMsg("Vertify Update:%d ms", dwVertifyTime);
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_UPDATE_FINISH, "update finish", m_nIsUpdate);
				}
				else
				{
					DebugMsg("Vertify UnUpdate:%d ms", dwVertifyTime);
				}
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_VERIFY_OVER, "", EnVertify_Update == m_nIsUpdate ? 1 : 2);
				m_dwTimeBeginPackInfo = ::TimeGetTime();
				m_nVertifyStatus = DS_VERTIFY_ING_PACK_INFO;
				m_nVertifySubStatus = DS_SUB_STATUS_NONE;
				this->ApplyPackInfo();

				if (m_bUseInitObb)
				{//���Ӵ���ļ���
					EnableInitResPack(true);
					DebugMsg("enable init res obb");
				}

				CBgDownloadMgr::GetInstance()->Init(m_pDownloadMgr, m_nBgThreadCreateNum);
			}
			break;
		default:
			break;
		}
	}

	void CPackDownProxy_CDN::OnVertifyProgressMsgs(const char *pszMsg)
	{
		if (pszMsg)
		{
			DebugMsg("VertifyProgressMsgs pszMsg=%s", pszMsg);
		}
	}

	void CPackDownProxy_CDN::OnDeleteFile(int nCurrentCount, int nTotalCount)
	{
		if (nTotalCount <= 0 || nCurrentCount < 0)
		{
			this->OnDelInvalidPackOver();
			return;
		}

		DebugMsg("DelPack:%d/%d", nCurrentCount, nTotalCount);
		m_nStatusProgress = nCurrentCount * 100 / nTotalCount;
		if (nCurrentCount == nTotalCount)
		{
			this->OnDelInvalidPackOver();
		}
	}

	void CPackDownProxy_CDN::RelatePackInfo(void)
	{
		CHECK(m_pVerifyInst);
		//��ȡ��ǰ���µİ汾��
		int nCurUpateVersion = m_pVerifyInst->GetServerVersion();
		if (-1 == nCurUpateVersion)
		{
			DebugMsg("get server version fail!!");
			return;
		}

		std::string strPack = c3pack_down::GetFilePath(m_strPackInfoFile);
		strPack += c3pack_down::GetFileName(m_strPackInfoFile);

		char szPackWithVersion[MAX_PACK_PATH] = {0};
		sprintf(szPackWithVersion, "%s_%d.pack",strPack.c_str(), nCurUpateVersion);
		m_strPackInfoPack = szPackWithVersion;
		DebugMsg("RelatePackInfo:%s %s", m_strPackInfoFile.c_str(), m_strPackInfoPack.c_str());
		//::DataSupport_RelatePackInfo(m_strPackInfoFile.c_str(), m_strPackInfoPack.c_str()); // ����Ҫ���������ù�������
	}

	void CPackDownProxy_CDN::ApplyPackInfo(void)
	{
		this->RelatePackInfo();
		
	//ZBTODO	std::string  strFileFullPath = DataSupport_GetRootPath();
		std::string  strFileFullPath = "";
		strFileFullPath.append(m_strPackInfoPack.c_str());

		bool bFileExist = c3pack_down::IsFileExist(strFileFullPath.c_str());
		if (!bFileExist) // �����չĿ¼�д�����Դ����ͬ����Ч
		{
		//ZBTODO	std::string  strFileExtend = DataSupport_GetExtendPath();
			std::string  strFileExtend = "";
			strFileExtend.append(m_strPackInfoPack.c_str());
			bFileExist = c3pack_down::IsFileExist(strFileExtend.c_str());
		}

		if (bFileExist)
		{//�ļ�������ֱ�Ӽ���
			this->SpringLoadPackInfo();

			DWORD dwTime = ::TimeGetTime();
			if (EnVertify_Update == m_nIsUpdate)
			{
				CReusePackMgr::GetInstance()->Update(m_nLastResVersion);
				CReusePackMgr::GetInstance()->Save();
			}
	//ZBTODO		::DataSupport_LoadPackInfo_Extend(REUSEPACKPATH, NULL);
			::DebugMsg("ReusePackInfo:%dms", ::TimeGetTime() - dwTime);
		}else
		{
			m_bDownPackInfo = true;
			m_nVertifySubStatus = DS_SUB_STATUS_DOWN_PACK_INFO_BEGIN;
			this->SendRequest(m_strPackInfoPack.c_str());
		}
	}

	void CPackDownProxy_CDN::SpringLoadPackInfo(void)
	{
		CHECK(m_pVerifyInst);
	//ZBTODO	std::string  strPackFullPath = DataSupport_GetRootPath();
		std::string  strPackFullPath = "";
		strPackFullPath.append(m_strPackInfoPack.c_str());
		bool bFileExist = c3pack_down::IsFileExist(strPackFullPath.c_str());
		bool bHashSame = false;
		
		if (bFileExist)
		{
			int nType = 0;
			bHashSame = m_pVerifyInst->PackFileHashCompare(m_strPackInfoPack.c_str(), nType);
			if (!bHashSame)
			{
				::remove(strPackFullPath.c_str());
				bFileExist = false;
			}
			::DebugMsg("hash compare nType=%d", nType);
		}

		if (!bHashSame)
		{
			// ��֤App���е�PackInfo�Ƿ���Ч
		//ZBTODO	std::string  strFileExtend = DataSupport_GetExtendPath();
			std::string  strFileExtend = "";
			strFileExtend.append(m_strPackInfoPack.c_str());
			if (c3pack_down::IsFileExist(strFileExtend.c_str()))
			{
				::DebugMsg("Use AppPack:%s", m_strPackInfoPack.c_str());
				// PackFileHashCompareֻ�������Ŀ¼�����޷��ж���չĿ¼�е��ļ�
				bHashSame = true; // m_pVerifyInst->PackFileHashCompare(strFileExtend.c_str());
			}
		}
		
		if (bHashSame)
		{
			//ZBTODO!::DataSupport_LoadPackInfo(m_strPackInfoFile.c_str(), m_strPackInfoPack.c_str())
			if (false)
			{
				m_nVertifyStatus = DS_VERTIFY_FAIL_LOAD_PACK_INFO;
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_PACKINFO_ERROR, m_strPackInfoPack.c_str(), 2);
				CPackLogSender::GetInstance()->NetDownLog(TQ_AUTOPATCH, "load pack info failed!!!");
				static bool s_bReDownPackInfo = false;
				if (!s_bReDownPackInfo)
				{
					s_bReDownPackInfo = true;
					CPackLogSender::GetInstance()->NetDownLog(TQ_AUTOPATCH, "down pack info again");
					if (bFileExist)
					{//ɾ�����ص�pack_info_xx.packȻ����������
						::remove(strPackFullPath.c_str());
					}
					m_nVertifyStatus = DS_VERTIFY_ING_PACK_INFO;
					m_bDownPackInfo = true;
					SendRequest(m_strPackInfoPack.c_str());
				}
				return;
			}
		}
		else 
		{
			//pack_info_xx.pack�ԱȲ�һ��ʱ,��������
			CPackLogSender::GetInstance()->NetDownLog(TQ_AUTOPATCH, "down pack info HashCompare false");
			CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_PACKINFO_ERROR, m_strPackInfoPack.c_str(), 3);

			if (bFileExist)
			{//ɾ�����ص�pack_info_xx.packȻ����������
				::remove(strPackFullPath.c_str());
			}
			m_nVertifyStatus = DS_VERTIFY_ING_PACK_INFO;
			m_bDownPackInfo = true;
			SendRequest(m_strPackInfoPack.c_str());
			return;
		}
		m_nVertifySubStatus = DS_SUB_STATUS_LOAD_PACK_INFO_END;
		DWORD dwTime = ::TimeGetTime();
		::DebugMsg("PackInfoLoadOver:%dms", dwTime - m_dwTimeBeginPackInfo);

		//�����ļ����سɹ���,�ټ��������б�
		LoadDownLoadList();
		::DebugMsg("LoadDownLoadList:%dms", ::TimeGetTime() - dwTime);

		m_nVertifyStatus = DS_VERTIFY_SUC;
	}

	void CPackDownProxy_CDN::OnPackInfoDown(const char* pszFile, bool bSuc)
	{
		if (0 == strcmp(m_strPackInfoPack.c_str(), pszFile))
		{
			m_bDownPackInfo = false;
			
			::DebugMsg("PackInfoDownOver:%dms", ::TimeGetTime() - m_dwTimeBeginPackInfo);
			if (bSuc)
			{
				CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_DOWN_PACKINFO_FINISH, m_strPackInfoPack.c_str(), 0);
				this->SpringLoadPackInfo();

				//��֤�������,ִ�кϲ��汾 pack_info_xxxx.pack
				DWORD dwTime = ::TimeGetTime();
				CReusePackMgr::GetInstance()->Update(m_nLastResVersion);
				CReusePackMgr::GetInstance()->Save();
			//ZBTODO	::DataSupport_LoadPackInfo_Extend(REUSEPACKPATH, NULL);
				::DebugMsg("ReusePackInfo With Update:%dms", ::TimeGetTime() - dwTime);
			}
			else
			{
				static int s_nDownRepeatCount = 3;
				if (s_nDownRepeatCount > 0)
				{//pack info ������ʧ��ʱ,����3������
					--s_nDownRepeatCount;
					CPackLogSender::GetInstance()->NetDownLog(TQ_AUTOPATCH, "request packinfo again count=%d", s_nDownRepeatCount);
					m_nVertifyStatus = DS_VERTIFY_ING_PACK_INFO;
					m_nVertifySubStatus = DS_SUB_STATUS_DOWN_PACK_INFO_BEGIN;
					m_bDownPackInfo = true;
					SendRequest(m_strPackInfoPack.c_str());
				}else
				{
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_PACKINFO_ERROR, m_strPackInfoPack.c_str(), 1);
					DebugMsg("download pack info failed!!!");
					m_nVertifyStatus = DS_VERTIFY_FAIL_DOWN_PACK_INFO;
				}
			}
		}
	}

	void CPackDownProxy_CDN::LoadDownLoadList()
	{
		bool bExistUpdateFinishFlag = CBgDownloadMgr::GetInstance()->IsExistUpdateFinishFlag();
		if (EnVertify_Update == m_nIsUpdate || false == bExistUpdateFinishFlag)
		{//��Ҫ����ʱ��update.xml ���Ϊupdate_sort.xml
			std::string strUpdateList = m_pVerifyInst->GetVertifyFilePath();
			if (strUpdateList.length() > 0)
			{
				bool bSuc = CBgDownloadMgr::GetInstance()->LoadUpdateList(strUpdateList.c_str(), true);
				if (bSuc)
				{
					CBgDownloadMgr::GetInstance()->SortUpdateList(0);
					bSuc = CBgDownloadMgr::GetInstance()->SaveUpdateList(DOWNLOAD_UPDATE_SORT, 0);
					if (bSuc)
					{
						CBgDownloadMgr::GetInstance()->SetUpdateFinishFlag(m_nCurResVersion);
					}
				}
			}
		}else if (EnVertify_UnUpdate == m_nIsUpdate)
		{
			CBgDownloadMgr::GetInstance()->LoadUpdateList(DOWNLOAD_UPDATE_SORT, false);
		}
	}

	IPackDownloadTask* CPackDownProxy_CDN::GetDownloadTask(void)
	{
		return m_pPackDown;
	}

	CPackResDownloadItem::CPackResDownloadItem()
	{

	}

	CPackResDownloadItem::~CPackResDownloadItem()
	{

	}

	// �����ʼִ��
	void CPackResDownloadItem::OnPackDownloadBegin(const char *pszFileName)
	{
	}

	// ������ִ�й����е�״̬��Ϣ
	void CPackResDownloadItem::OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte)
	{
		//DebugMsg("NetDown: Downloading FileName=%s packSize=%d, hasDown=%d",pszFileName, nTotalByte, nCurrByte);
	}

	// ���������
	void CPackResDownloadItem::OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr)
	{
		if (enStatus == EnRes_Completed)
		{
			int nAddr = (int)enAddr;
			if (pszFileName)
			{
				CPackLogSender::GetInstance()->NetDownLog("debug","FG Download Addr=%d FileName=%s Version=%d", nAddr, pszFileName, nPackVersion);
				if (enAddr != EnAddr_Cdn)
				{
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_BACK_SRCIP_ERROR, pszFileName, nAddr, false);
				}
			}
		}
		else
		{
			CPackLogSender::GetInstance()->SetPackDownError(ERROR_TYPE_DOWNLOAD, (int)enStatus);
			if (pszFileName)
			{
				int nAddr = (int)enAddr;
				int nStatus = (int)enStatus;

				if (enAddr == EnAddr_BakIp) // 3��ip��ʧ��ʱΪ����ʧ��
				{
					std::string strFileFullPath = CPackLogSender::GetInstance()->GetDownErrorFileName(nPackVersion, enAddr, pszFileName);
					if (enStatus >= EnRes_NotFount)
					{//����ֻ��ռ�С��10M,������ʧ����ʶ�ǿռ䲻��
						if (CBgDownloadMgr::GetInstance()->CheckFreeSpaceLowerThan(FREE_SPACE_MIN_SIZE))
						{
							nStatus = VERIFY_ERR_INNER_CODE_LOW_FREE_SPACE;
						}
					}
					CPackLogSender::GetInstance()->NetDownLog("debug", "FG DownFail Addr=%d Status=%d FileName=%s Version=%d", nAddr, nStatus, pszFileName, nPackVersion);
					CPackLogSender::GetInstance()->SendPackDownFeedbackMsg(PACK_MSG_DOWN_FILE_ERROR, strFileFullPath.c_str(), nStatus, false);
				}
			}
		}
	}


	CPackResDownloadListen::CPackResDownloadListen()
	{

	}
	CPackResDownloadListen::~CPackResDownloadListen()
	{

	}
	// ����������ļ��� ��������
	void CPackResDownloadListen::OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo)
	{
		if (enStatus != EnNet_Connect)
		{
			CPackLogSender::GetInstance()->SetPackDownError(ERROR_TYPE_CONNECT, (int)enStatus);
		}
	}

	void CPackResDownloadListen::GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount)
	{
		CBgDownloadMgr::GetInstance()->SetDownloadSpeed(c3pack_down::Double2Int(dDownloadSpeed));
	}
}