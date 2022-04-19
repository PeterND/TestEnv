#ifndef _PACK_LOG_SENDER_H_
#define _PACK_LOG_SENDER_H_
#include "IPackBaseDef.h"
#include "IPackResDown.h"
#include "IPackDownManager.h"
#include "utility/Utility_pack.h"
#include <string> 
#include <vector> 
#include <set>

#define ERROR_TYPE_NONE				10
#define ERROR_TYPE_VERIFY			ERROR_TYPE_NONE + 1
#define ERROR_TYPE_CONNECT		ERROR_TYPE_NONE + 2
#define ERROR_TYPE_DOWNLOAD		ERROR_TYPE_NONE + 3


//下载反馈
#define PACK_MSG_NONE							1000
#define PACK_MSG_UPDATE_BEGIN					1001 // update begin verify
#define PACK_MSG_UPDATE_FINISH					1002 // update end verify
#define PACK_MSG_DOWN_PACKINFO_FINISH			1003 // download pack_info finish
#define PACK_MSG_DOWNLOAD_10					1004 // download file progress
#define PACK_MSG_DOWNLOAD_20					1005
#define PACK_MSG_DOWNLOAD_30					1006
#define PACK_MSG_DOWNLOAD_40					1007
#define PACK_MSG_DOWNLOAD_50					1008
#define PACK_MSG_DOWNLOAD_60					1009
#define PACK_MSG_DOWNLOAD_70					1010
#define PACK_MSG_DOWNLOAD_80					1011
#define PACK_MSG_DOWNLOAD_90					1012
#define PACK_MSG_DOWNLOAD_100					1013

#define PACK_MSG_ERROR_NONE						1100 //error message id begin
#define PACK_MSG_VERIFY_ERROR					1101 // verify error
#define PACK_MSG_CREATE_LIB_ERROR				1102 // init download lib error
#define PACK_MSG_PACKINFO_ERROR					1103 // pack_info error
#define PACK_MSG_DOWN_FILE_ERROR				1104 // download file error
#define PACK_MSG_BACK_SRCIP_ERROR				1105 // cdn server down file error

#define PACK_MSG_VERIFY_START					1106
#define PACK_MSG_VERIFY_OVER					1107

enum
{
	Flag_progress_none,
	Flag_progress_write,
	Flag_progress_read,
};
enum
{
	Flag_Thread_Count_None,
	Flag_Fg_Thread,
	Flag_Bg_Thread
};


#define TQ_AUTOPATCH  "tp_autopatch"
#define DOWNLOAD_PROGRESS  "tp_autopatch/progess.dat"
/************************************************************************/
/* 1. 错误收集
	2. 下载log反馈
	3. 下载信息统计*/
/************************************************************************/

namespace c3pack_down
{
	class CPackLogSender
	{
	public:
		CPackLogSender();
		virtual ~CPackLogSender(void);
		static CPackLogSender* GetInstance();
		static void ReleaseInstance();
		void SetDownloadAddr(const char* pCdnAddr, const char* pSrcAddr, const char* pBakAddr);
		std::string GetDownAddr(EnDownloadAddr enAddr);
		void SetFeedbackAddr(const char* pAddr);
		void SetFeebackSendFuntion(PFnSendDataToHttp  pFn);
		void SetPackDownError(int nType, int nErrorCode);
		const char* GetLastError(void);
		std::string GetDownErrorFileName(int nVersion, EnDownloadAddr enAddr, const char* pFile);
		void SendPackDownFeedbackMsg(int nStep, const char* pszPack, int nErrorId, bool bBg = false, int nResVersion = -1);
		void SetPackDownInfo(int nFinishCount, int nTotalCount, std::string strCurDownFileName);
		void NetDownLog(const char* pszFolder, const char* fmt, ...);
		void OnPackDownCallback(int nStatus, const char* pszPack);
		void SetCallback(PackDownCallback pfnCallback);
		void SetPlatformCode(int nGameType, int nCooperator);
		void SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
			const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion,int nFreeSpaceMb);
		int GetErrorCode(void);
	private:
		static CPackLogSender* s_pCPackLogSender;
		int m_nResVersion;
		int m_nGameTypeId;
		int m_nCooperatorId;
		int m_nErrorCode;
		std::string m_strMsgError;
		std::string m_strFeedbackAddr;
		std::string m_strCdnAddr;
		std::string m_strSrcAddr;
		std::string m_strBakAddr;
		std::set<std::string> m_setPackDownFileName;
		PackDownCallback m_pfnPackDownCallback;

		int m_nServerID;
		int m_nAccountID;
		std::string m_strAccountName;
		std::string m_strChannelID;
		std::string m_strDeviceID;
		std::string m_strDeviceName;
		std::string m_strOSVersion;
		PFnSendDataToHttp m_pfnSendData;
	};
}
#endif
