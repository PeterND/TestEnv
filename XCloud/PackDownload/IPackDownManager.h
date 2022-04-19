#ifndef _C3_IPACK_DOWN_MANAGER_H_
#define _C3_IPACK_DOWN_MANAGER_H_

#ifndef C3PACKDOWN_API
	#if defined(_WIN32)
		#define C3PACKDOWN_API __declspec(dllexport)
	#else
		#ifdef __ANDROID__
			#define C3PACKDOWN_API __attribute__ ((visibility("default")))
		#else
			#define C3PACKDOWN_API
		#endif
	#endif
#endif

#define FILE_SERVER_WD_DAT				"ini/server_wd.dat"
#define FILE_SERVER_WD_INI				"ini/server_wd.ini"
#define FILE_SERVER_WD_INI_INSIDE		"ini/server_wd_inside.ini"

//下载回调函数
typedef int(*PackDownCallback)(int nStatus);

//收集信息发送函数
typedef void (*PFnSendDataToHttp)(const char* pszAddr, const char* pszMsg);

enum
{
	DS_SUB_STATUS_NONE,

	//////////////////////////////////////////////////////////////////////////
	// DS_VERTIFY_ING_VERSION 子状态
	DS_SUB_STATUS_DEL_OLD_PACK,		// 删除旧包
	DS_SUB_STATUS_CHECK_VERSION,	// 检查版本号
	DS_SUB_STATUS_UPDATE_VERSION,	// 更新微端版本
	// DS_VERTIFY_ING_PACK_INFO 子状态
	DS_SUB_STATUS_DOWN_PACK_INFO_BEGIN,		// 开始下载pack_info包
	DS_SUB_STATUS_DOWN_PACK_INFO_END,	// 下载pack_info包完成
	DS_SUB_STATUS_LOAD_PACK_INFO_END,	// 加载pack_info包完成
	//DS_VERTIFY_FAIL_VERSION 子状态
	DS_SUB_STATUS_VERTIFY_FAIL_LOCAL_ERROR,  //验证时本地错误,可以尝试重启游戏
	DS_SUB_STATUS_VERTIFY_FAIL_SERVER_ERROR, //验证时服务器错误
	//////////////////////////////////////////////////////////////////////////
};

//文件下载失败码
typedef enum
{
	DownRes_NotFount			= 0x2,
	DownRes_NetError			= 0x3,
	DownRes_HashError			= 0x4,
	DownRes_CreateFileFail		= 0x5,
	DownRes_FromSourceAddr		= 0x6,
	DownRes_LowFreeSpace		= 0x20,
} EDownloadError;

typedef enum
{
	STATUS_INVALID = 0,
	STATUS_NOT_ENOUGH_SPACE = 1,

}CallbackStatus;
//微端回调给客户端逻辑的函数:
//参数 nStatus 由微端内部传入，回调传给客户端
//返回值 是给客户端使用，方便客户端做逻辑

class IPackDownManager
{
public:
	//处理旧版本资源的备份或还原
	virtual bool CheckOldVersionRes(const char* pszAppDir, const char* pszCacheDir) = 0;
public:
	virtual bool Init(int nGameType, int nCooperator, int nResVesion, const char* pszAppDir, const char* pszCacheDir,
		const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr, PFnSendDataToHttp pfnSend) = 0;
	virtual bool ProcessPackVerify(void) = 0;
	virtual void Process(void) = 0;

	virtual int GetVertifyCode(void) = 0;   //获取验证码.为定位问题方便,将验证显示在界面上

	virtual int GetVertifyStatus(int& nSubStatus) = 0; // 获取验证状态. 返回值为:主状态值,定义在IPluginForMY.h里面的DS_VERTIFY_XXX.
	// nSubStatus:表示DS_VERTIFY_XXX 过程中的子状态的值,定义在该文件的 DS_SUB_STATUS_XXX
	virtual int GetStatusProgress(void) = 0; // 获取状态进度，百分制[0-100]。可以获取进度的状态[DS_SUB_STATUS_DEL_OLD_PACK, DS_SUB_STATUS_UPDATE_VERSION]

	virtual void EnableBgDownload(bool bEnable) = 0; //是否启用后台下载,默认配置在 ini/app_config.xml

	virtual void EnableInitRes(bool bEnable, const char* pszInitResPath) = 0; //是否启用初始大包,需要传入大包的路径

	virtual bool IsPackDownEnable(void) = 0;	//微端下载是否开启

	virtual bool IsBgDownloadEnable(void) = 0; //后台下载是否开启

	virtual bool IsFgDownloadEnable(void) = 0; //前台下载是否开启

	virtual void SetDownloadOrder(int nStrategy = 0) = 0;// 客户端根据需要设置下载排序策略类型

	virtual void SetPackDownCallback(PackDownCallback pFnCallback) = 0;//设置下载回调函数

	//登录成功之后设置
	virtual void SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
								 const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion,int nFreeSpaceMb) = 0;

	//获取下载信息
	//nTotalCount:本次需要更新的包的总个数. nFinishCount:已经下载好的包的个数
	//uTotalSize:本次需要更新的所有资源的大小总和. uDownSize:已经下载完成的包的大小总和.(这两个的单位都是 kb)
	//nCurDownSpeed:表示当前的下载速度,单位(kb/s).strCurDownFileName:是当前正在下载的包的名字
	virtual bool GetDownLoadInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, int& nCurDownSpeed, char* pszCurDownFileName, int& nDownStatus) = 0;

	virtual void SendC3CallbackMsg(const char* pszMsg) = 0;

	// 网络切换(移动端下使用)
	// @return: 返回切换结果 false 失败，true成功
	virtual bool SwitchNetWorks(void) = 0;

	//下载完成
	virtual bool IsDownloadFinish() = 0;

#ifdef _WIN32
	// 设置上下行数据监控时间
	// @param nUpFlowTime	: 上行流量监控时间，单位秒
	// @param nDownFlowTime	: 下行流量监控时间，单位秒
	virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime, bool bBg = true) = 0;

	// 获取下载上行数据
	// @param dUpAverageFlow: 上行平均流量，单位KB/S
	// @param dUpMaxFlow	: 上行最大流量，单位KB/S
	// @param dUpMinFlow	: 上行最小流量，单位KB/S
	virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow, bool bBg = true) = 0;

	// 获取下载下行数据
	// @param dDownAverageFlow: 下行平均流量，单位KB/S
	// @param dDownMaxFlow	: 下行最大流量，单位KB/S
	// @param dDownMinFlow	: 下行最小流量，单位KB/S
	virtual void GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow, bool bBg = true) = 0;
#endif
};

extern "C" C3PACKDOWN_API IPackDownManager& GetPackDownManger();
extern "C" C3PACKDOWN_API void RelesePackDownManger();

#endif