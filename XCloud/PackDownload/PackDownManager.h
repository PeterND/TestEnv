#pragma once
#include "IPackDownManager.h"
#include "PackDownProxy_CDN.h"

namespace c3pack_down
{
	class CResOperation;
	class C3PACKDOWN_API CPackDownManager :public IPackDownManager
	{
	private:
		static CPackDownManager* s_pPackDownManager;
	public:
		CPackDownManager();
		virtual ~CPackDownManager(void);
		static IPackDownManager& getSingleton();
		static void deleteSigleton();

	public:
		//处理旧版本资源的备份或还原
		virtual bool CheckOldVersionRes(const char* pszAppDir, const char* pszCacheDir);
	public:
		virtual bool Init(int nGameType, int nCooperator, int nResVesion, const char* pszAppDir, const char* pszCacheDir,
			const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr,PFnSendDataToHttp pfnSend);
		virtual bool ProcessPackVerify(void);
		virtual void Process(void);

		virtual int GetVertifyCode(void);    //获取验证码.为定位问题方便,将验证显示在界面上

		virtual int GetVertifyStatus(int& nSubStatus); // 获取验证状态. 返回值为:主状态值,定义在IPluginForMY.h里面的DS_VERTIFY_XXX.
															 // nSubStatus:表示DS_VERTIFY_XXX 过程中的子状态的值,定义在该文件的 DS_SUB_STATUS_XXX
		virtual int GetStatusProgress(void); // 获取状态进度，百分制[0-100]。可以获取进度的状态[DS_SUB_STATUS_DEL_OLD_PACK, DS_SUB_STATUS_UPDATE_VERSION]

		virtual void EnableBgDownload(bool bEnable); //是否启用后台下载,默认配置在 ini/app_config.xml

		virtual void EnableInitRes(bool bEnable, const char* pszInitResPath); //是否启用初始大包,需要传入大包的路径

		virtual bool IsPackDownEnable(void);	//微端下载是否开启

		virtual bool IsBgDownloadEnable(void); //后台下载是否开启

		virtual bool IsFgDownloadEnable(void); //前台下载是否开启

		virtual void SetDownloadOrder(int nStrategy = 0);// 客户端根据需要设置下载排序策略类型

		virtual void SetPackDownCallback(PackDownCallback pFnCallback);//设置下载回调函数

		virtual void SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
			const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion, int nFreeSpaceMb);
		//获取下载信息
		//nTotalCount:本次需要更新的包的总个数. nFinishCount:已经下载好的包的个数
		//uTotalSize:本次需要更新的所有资源的大小总和. uDownSize:已经下载完成的包的大小总和.(这两个的单位都是 kb)
		//nCurDownSpeed:表示当前的下载速度,单位(kb/s).strCurDownFileName:是当前正在下载的包的名字
		virtual bool GetDownLoadInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, int& nCurDownSpeed, char* pszCurDownFileName, int& nDownStatus);

		virtual void SendC3CallbackMsg(const char* pszMsg);

		// 网络切换(移动端下使用)
		// @return: 返回切换结果 false 失败，true成功
		virtual bool SwitchNetWorks(void);

		//下载完成
		virtual bool IsDownloadFinish();

#ifdef _WIN32
		// 设置上下行数据监控时间
		// @param nUpFlowTime	: 上行流量监控时间，单位秒
		// @param nDownFlowTime	: 下行流量监控时间，单位秒
		virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime, bool bBg = true);

		// 获取下载上行数据
		// @param dUpAverageFlow: 上行平均流量，单位KB/S
		// @param dUpMaxFlow	: 上行最大流量，单位KB/S
		// @param dUpMinFlow	: 上行最小流量，单位KB/S
		virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow, bool bBg = true);

		// 获取下载下行数据
		// @param dDownAverageFlow: 下行平均流量，单位KB/S
		// @param dDownMaxFlow	: 下行最大流量，单位KB/S
		// @param dDownMinFlow	: 下行最小流量，单位KB/S
		virtual void GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow, bool bBg = true);
#endif
	private:
		CPackDownProxy_CDN* m_pPackDownProxy;
		CResOperation* m_pResOp;
		bool m_bVerifyDataSupport;
		bool m_bEnableInitRes;
		const char* m_pszInitResPath;
		const char* m_pszAppPath;
		const char* m_pszCachePath;
	};
}
