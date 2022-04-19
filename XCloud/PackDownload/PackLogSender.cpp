#include "PackLogSender.h"
#include "IPackResDown.h"
#include "PackDownManager.h"
#include "AppConfigInfo.h"


namespace c3pack_down
{
	CPackLogSender* CPackLogSender::s_pCPackLogSender = NULL;

	CPackLogSender* CPackLogSender::GetInstance()
	{
		if (NULL == s_pCPackLogSender)
		{
			s_pCPackLogSender = new CPackLogSender();
		}
		return s_pCPackLogSender;
	}

	void CPackLogSender::ReleaseInstance()
	{
		if (s_pCPackLogSender)
		{
			delete s_pCPackLogSender;
			s_pCPackLogSender = NULL;
		}
	}

	CPackLogSender::CPackLogSender()
	{
		m_nResVersion = 1000;
		m_nGameTypeId = 442;//默认是手机魔域
		m_nCooperatorId = 1075;
		m_nErrorCode = 0;
		m_strCdnAddr = "";
		m_strSrcAddr = "";
		m_strBakAddr = "";
		m_strFeedbackAddr="http://datacollect.99.com/InstallStep.ashx?";

		m_nServerID = 0;
		m_nAccountID = 0;
		m_strAccountName = "";
		m_strChannelID = "";
		m_strDeviceID = "";
		m_strDeviceName = "";
		m_strOSVersion = "";
		m_pfnPackDownCallback = NULL;
		m_pfnSendData = NULL;
	}

	CPackLogSender::~CPackLogSender(void)
	{
		m_setPackDownFileName.clear();
	}

	void CPackLogSender::SetDownloadAddr(const char* pCdnAddr, const char* pSrcAddr, const char* pBakAddr)
	{
		if (pCdnAddr)
		{
			m_strCdnAddr = pCdnAddr;
		}
		if (pSrcAddr)
		{
			m_strSrcAddr = pSrcAddr;
		}
		if (pBakAddr)
		{
			m_strBakAddr = pBakAddr;
		}
	}

	std::string CPackLogSender::GetDownAddr(EnDownloadAddr enAddr)
	{
		if (enAddr ==  EnAddr_Cdn)
		{
			return m_strCdnAddr;
		}else if (enAddr == EnAddr_Ip)
		{
			return m_strSrcAddr;
		}else if (enAddr == EnAddr_BakIp)
		{
			return m_strBakAddr;
		}
		return "";
	}


	std::string CPackLogSender::GetDownErrorFileName(int nVersion, EnDownloadAddr enAddr, const char* pFile)
	{
		if (NULL == pFile) return "";

		std::string strOs = c3pack_down::GetPlatform();

		//std::string strAddr = GetDownAddr(enAddr);
		char szDownError[256];
		sprintf(szDownError, "%s%s/%d/%s", m_strBakAddr.c_str(), strOs.c_str(), nVersion, pFile);
		std::string strFileName = szDownError;
		return strFileName;
	}

	void CPackLogSender::SetFeedbackAddr(const char* pAddr)
	{
		if (pAddr)
		{
			m_strFeedbackAddr = pAddr;
		}
	}

	void CPackLogSender::SetPackDownError(int nType, int nErrorCode)
	{
		m_strMsgError = "none";

		switch(nType)
		{
		case ERROR_TYPE_VERIFY:
			{
				switch(nErrorCode)
				{
				case EnVertify_Fail:			m_strMsgError = "EnVertify_Fail"; break;
				case EnVertify_DownPackFail:	m_strMsgError = "EnVertify_DownPackFail"; break;
				case EnVertify_PackHashFail:	m_strMsgError = "EnVertify_PackHashFail"; break;
				case EnVertify_ParsePackFail:	m_strMsgError = "EnVertify_ParsePackFail"; break;
				default:break;
				}
				break;
			}
		case ERROR_TYPE_CONNECT:
			{
				switch(nErrorCode)
				{
				case EnNet_Empty:				m_strMsgError = "EnNet_Empty"; break;
				case EnNet_UnConnect:			m_strMsgError = "EnNet_UnConnect"; break;
				case EnNet_Error:				m_strMsgError = "EnNet_Error"; break;
				default:break;
				}
				break;
			}
		case ERROR_TYPE_DOWNLOAD:
			{
				switch(nErrorCode)
				{
				case EnRes_Empty:				m_strMsgError = "EnRes_Empty"; break;
				case EnRes_NotFount:			m_strMsgError = "EnRes_NotFount"; break;
				case EnRes_NetError:			m_strMsgError = "EnRes_NetError"; break;
				case EnRes_HashError:			m_strMsgError = "EnRes_HashError"; break;
				case EnRes_FromSourceAddr:		m_strMsgError = "EnRes_FromSourceAddr"; break;
				default:break;
				}
				break;
			}
		default:break;
		}

		//NetDownLog(TQ_AUTOPATCH, "NetDown error: %s",m_strMsgError.c_str());
	}

	const char* CPackLogSender::GetLastError(void)
	{
		return m_strMsgError.c_str();
	}

	void CPackLogSender::SetCallback(PackDownCallback pfnCallback)
	{
		m_pfnPackDownCallback = pfnCallback;
		m_setPackDownFileName.clear();
	}

	void CPackLogSender::SetPlatformCode(int nGameType, int nCooperator)
	{
		m_nGameTypeId = nGameType;
		m_nCooperatorId = nCooperator;
	}

	int CPackLogSender::GetErrorCode(void)
	{
		return m_nErrorCode;
	}

	void CPackLogSender::SetFeebackSendFuntion(PFnSendDataToHttp  pFn)
	{
		m_pfnSendData = pFn;
	}

	void CPackLogSender::SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
						 const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion, int nFreeSpaceMb)
	{
		m_nServerID = nServerID;
		m_nAccountID = nAccountID;
		m_strAccountName = pszAccountName;
		m_strChannelID = pszChannelID;
		m_strDeviceID = pszDeviceID;
		m_strDeviceName = pszDeviceName;
		m_strOSVersion = pszOSVersion;
		MySetFreeSpace(nFreeSpaceMb);
	}

	void CPackLogSender::OnPackDownCallback(int nStatus, const char* pszPack)
	{
		if (pszPack)
		{
			std::set<std::string>::iterator it = m_setPackDownFileName.find(pszPack);
			if (it == m_setPackDownFileName.end())
			{
				m_setPackDownFileName.insert(pszPack);
				if (m_pfnPackDownCallback)
				{
					m_pfnPackDownCallback(nStatus);
				}
			}
			else
			{
				DebugMsg("repeat download pack=%s", pszPack);
			}
		}
	}

	void CPackLogSender::SetPackDownInfo(int nFinishCount, int nTotalCount, std::string strCurDownFileName)
	{
		 if (nFinishCount == nTotalCount)
		{
			SendPackDownFeedbackMsg(PACK_MSG_DOWNLOAD_100, strCurDownFileName.c_str(), 0);
			NetDownLog(TQ_AUTOPATCH, "Download finish file count = %d", nFinishCount);
		}
	}

	void CPackLogSender::SendPackDownFeedbackMsg(int nStep, const char* pszPack, int nErrorId, bool bBg, int nResVersion)
	{
		char szData1[512]={0};
		sprintf(szData1, "\"Gametype\":\"%d\", \"Cooperator\":\"%d\", \"ServerID\":\"%d\", \"AccountName\":\"%s\", \"AccountId\":\"%d\",\"Step\":\"%d\", \"Channel\":\"%s\", \"Mac\":\"%s\"",
			m_nGameTypeId,m_nCooperatorId,m_nServerID,m_strAccountName.c_str(),m_nAccountID,nStep,	m_strChannelID.c_str(),m_strDeviceID.c_str());

		if (nStep == PACK_MSG_DOWN_FILE_ERROR)
		{
			m_nErrorCode = nErrorId;
		}

		//version
		if (1000 == m_nResVersion)
		{
			m_nResVersion = c3pack_down::getVersion();
		}

		if (nResVersion == -1)
		{
			nResVersion = m_nResVersion;
		}

		// time
		time_t ltime;
		time( &ltime );
		struct tm* curdata = localtime( &ltime );
		std::string strTime;
		if (curdata)
		{
			char szTime[64]={0};
			sprintf(szTime,"%02d%02d%02d%02d%02d%02d", curdata->tm_year + 1900, curdata->tm_mon + 1, curdata->tm_mday , curdata->tm_hour , curdata->tm_min, curdata->tm_sec);
			strTime = szTime;
		}


		std::string strOs = c3pack_down::GetPlatform();

		char szData2[512]={0};
		sprintf(szData2, "\"Time\":\"%s\", \"platform\":\"%s\", \"devname\":\"%s\", \"systemversion\":\"%s\", \"Version\":\"%d\", \"IsBg\":\"%d\", \"Pack\":\"%s\", \"ErrorId\":\"%d\"",
			strTime.c_str(),strOs.c_str(),m_strDeviceName.c_str(),m_strOSVersion.c_str(),
			nResVersion,bBg ? 1 : 0,pszPack ? pszPack : "",nErrorId);

		char szData[1024]={0};
		sprintf(szData,"{%s, %s}", szData1, szData2);

		if (m_strFeedbackAddr.length() > 0)
		{
			NetDownLog(TQ_AUTOPATCH, "pack feedback strData=%s", szData);
			if (m_pfnSendData)
			{
				m_pfnSendData(m_strFeedbackAddr.c_str(), szData);
			}
		}
	}

	void CPackLogSender::NetDownLog(const char* pszFolder, const char* fmt, ...)
	{
		char szMsg[1024] = { 0 };
		va_list args;
		va_start(args, fmt);
		vsnprintf(szMsg, 1023, fmt, args);
		va_end(args);

		DebugMsg(szMsg);

		if (1 == CAppConfigInfo::GetInstance()->getLogEnable())
		{
			if (NULL == pszFolder) return;

			std::string strEnv = c3pack_down::getAppCachePath();
			strEnv += "/";
			strEnv += pszFolder;
			if (!c3pack_down::IsFileExist(strEnv.c_str())) 
			{
				c3pack_down::MyCreateDirectory(strEnv.c_str());
			}

			time_t ltime;
			time(&ltime);

			char szLogName[512] = "";
			tm* curTime = localtime(&ltime);
			sprintf(szLogName, "%s/wd_%u_%u_%u.log", strEnv.c_str(), curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday);

			FILE* fp = fopen(szLogName, "a+");
			if(fp != NULL)
			{
				fprintf(fp, "%s ----%d:%d:%d\n", szMsg, curTime->tm_hour, curTime->tm_min, curTime->tm_sec);
				fclose(fp);	
			}
		}
	}
}