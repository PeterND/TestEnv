#ifndef _PACK_RES_DOWN_H_
#define _PACK_RES_DOWN_H_

#ifdef _WIN32
#define PACKRESDOWN_API extern "C" _declspec(dllexport)
#else
#define PACKRESDOWN_API
#define DLL_PUBLIC  __attribute__ ((visibility("default")))
#endif

typedef enum
{
	EnAddr_Empty = 0x0,
	EnAddr_Cdn	 = 0x1,
	EnAddr_Ip	 = 0x2,
	EnAddr_BakIp = 0x3
} EnDownloadAddr;

typedef enum
{
	EnRes_Empty				= 0x0,
	EnRes_Completed			= 0x1,
	EnRes_NotFount			= 0x2,
	EnRes_NetError			= 0x3,
	EnRes_HashError			= 0x4,
	EnRes_CreateFileFail	= 0x5,
	EnRes_FromSourceAddr	= 0x6,
	EnRes_UrlConfigError	= 0x7
} EnPackCompleteStatus;

typedef enum
{
	EnNet_Empty		= 0x0, 
	EnNet_Connect	= 0x1,
	EnNet_UnConnect = 0x2,
	EnNet_Error		= 0x3
}EnNetStatusListener;

// 校验方式
typedef enum
{
	EnVertify_Pack7z_Download_Type	= 0x0,	// 正常更新方式
	EnVertify_Pack7z_BakFile_Type	= 0x1,	// 备份文件方式
	EnVertify_Pack7z_Extend_Type	= 0x2	// 扩展目录方式（IOS端包内）
}EnVertifyType;

// 备份方案错误列表
typedef enum
{
	EnVertify_BakUpdateHashFail		= -102,	// 通过备选方案,文件hash校验失败
	EnVertify_BakPackDecompressFail	= -101,	// 备份文件中pack.7z解压失败
	EnVertify_BakPackFileNoFound	= -100	// 未发现pack.7z备份文件
}EnVertifyBakStatus;

// 下载校验列表
typedef enum
{
	EnVertify_PackVersionError		= -10,  // pack.xml 版本号不同
	EnVertify_SetVersionFail		= -9,	// 设置更新版本号失败
	EnVertify_CreateConfigFail		= -8,	// 创建更新列表文件失败
	EnVertify_GetServerFail			= -7,	// 服务版本号小于等于0
	EnVertify_PathEmptyFail			= -6,	// 资源存放路径为空
	EnVertify_PackDecompressFail	= -5,	// 下载的pack.7z解压失败
	EnVertify_ParsePackFail			= -4,	// 解析pack列表失败
	EnVertify_PackHashFail			= -3,	// pack文件hash校验失败	
 	EnVertify_DownPackFail			= -2,	// 文件下载失败
	EnVertify_Fail					= -1,	// 校验不通过
	EnVertify_UnUpdate				= 0	,	// 无需更新
	EnVertify_Update				= 1 	// 需要更新
}EnVertifyStatus;

// 下载项定义
struct IPackResDownloadItem
{
	// 下载项开始执行
	virtual void OnPackDownloadBegin(const char *pszFileName) = 0;

	// 下载项执行过程中的状态信息
	virtual void OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte) = 0;

	// 下载项完成
	virtual void OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr) = 0;
};

// 下载网络监听
struct IPackResDownloadListen
{
	// 下载项网络的监听 单独监听
	virtual void OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo) = 0;
    
	// @param dDownloadSpeed: 下载速度 KB/S
	// @param llDownSize	: 下载文件的大小
	// @param nDownloadCount: 下载中文件个数
	// @param dDownloadSpeed: 并发线程数
//#ifdef _WIN32
	// 获取正在下载中的下载总速度，下载文件的大小，下载文件个数，下载线程数
    //virtual void GetTaskDownloadInfo(double dDownloadSpeed, long long llDownSize, int nDownloadCount, int nThreadCount) = 0;
//#else
	// 获取正在下载中的下载总速度，下载文件个数，下载线程数
	virtual void GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount) = 0;
//#endif
};

// 校验回调
struct IResVertifyStatus
{
	// 校验进度回调
	// @param nVertifyStatus:	状态详见EnVertifyStatus 
	// @param nProgress:		0 ~ 99: 显示校验进度; 100时反馈校验状态
	// @param nVertifyType:		0 通过下载进行校验， 1 通过本地备份进行校验 (备份校验失败时回调参数为EnVertifyType)
	// @param pszErrInfo:		错误情况的回调提示
	virtual void OnVertifyFile(int nVertifyStatus, int nProgress, int nVertifyType, const char* pszErrInfo) = 0;

	// 校验阶段消息回调
	// @param pszMsg: 校验阶段的消息内容 (包括版本校验时需要删除的文件、删除hash不同的旧文件、版本号异常等内容)
	virtual void OnVertifyProgressMsgs(const char *pszMsg) = 0;

	// 删除文件进度回调
	// @param nCurrentCount: 当前已经删除的数量
	// @param nTotalCount:	  总共要删除的数量
	virtual void OnDeleteFile(int nCurrentCount, int nTotalCount) = 0;
};

// 校验
struct IVertifyInstance
{
	// 初始化配置
	// @param: pszResSavePath:			 下载存放跟目录
	// @param: pszExtendResSavePath:	 IOS端使用指向IOS包内目录
	// @param: pszCdnDomainNameAddr:	 CND域名下载地址			(如 http://nd.cdn.soul.com/updatedat)
	// @param: pszSourceAddrIp:			 数据源IP下载地址			(如 http://192.168.23.231/updatedat) 
	// @param: pszSourceAddrIpBak:		 数据源IP下载地址			(如 http://192.168.23.232/updatedat) 
	// @return:							 返回false配置失败，true 成功
 	virtual bool InitConfig(const char *pszResSavePath, const char *pszExtendResSavePath, const char *pszCdnDomainNameAddr, const char *pszSourceAddrIp, const char *pszSourceAddrIpBak) = 0;

	// 初始化校验回调，调用优先于校验更新接口
	// @param:		pVertify 校验回调
	// @note:		通过pVertify回调校验结果
	// @return:		false 初始化失败, ture 成功
	virtual bool InitResVertifyCallback(IResVertifyStatus *pVertify) = 0;

	// 版本校验完后，获取当前更新的版本号
	// @return: 返回最新版本号，-1 获取失败
	virtual int GetServerVersion(void) = 0;

	// 设置当前服务器版本号
	virtual void SetServerVersion(int nServerVersion) = 0;

	// 获取差异列表文件路径
	// @return: 返回差异列表文件的路径
	virtual const char *GetVertifyFilePath(void) = 0;

	// 获取差异文件列表的路径
	// @return: 返回校验后删除文件列表的路径
	virtual const char *GetVertifyDelFilePath(void) = 0;

	// 删除差异文件
	virtual void DeleteDifferFile(void) = 0;

	// 停止删除差异文件
	virtual void StopDeleteDifferFile(void) = 0;

	// 校验更新
	// @pararm: bAutoUpdate 默认为false, 设置为true则表明接口会强制触发下载pack.7z重新执行校验流程
	virtual void VertifyPackRes(bool bAutoUpdate) = 0;

	// 校验本地*.pack文件的hash和pack.xml中的hash值是否相同
	// @param: pszFileName: 如c3pack/pack_info_xxx.pack
	// @param: nType: 0表示无结果, 1表示内存中文件匹配成功, 2表示IOS中包内文件匹配成功
	// @return: 返回true值相同, 返回false值不同(pack文件不存返回false)
	virtual bool PackFileHashCompare(const char *pszFileName, int &nType) = 0;

	// 校验事件回调
	virtual void EventProcess(void) = 0;
};

// 下载任务
struct IPackDownloadTask
{
	// 开始更新任务
	// @param pPackResItem:   下载进度
	// @param pPackResListen: 下载网络监听
	// @param nThreadCount:   指定下载线程数
	// @return: 返回启动任务的状态, 0 失败, 1 成功
	virtual bool StartTask(IPackResDownloadItem *pPackResItem, IPackResDownloadListen *pPackResListen, int nThreadCount) = 0;

	// 下载事件回调
	virtual void EventProcess(void) = 0;

	// 未完成的下载项数
	// @return: 返回未完成的下载数
	virtual int UnFinishDownloadItemCount(void) = 0;

	// 添加下载项
	// @param bPrior: 下载项是否优先  
	// @param pszFileName: 需要下载的文件名
	// @return: 返回添加的状态
	virtual bool PushDownloadItem(bool bPrior, const char *pszFileName) = 0;

	// 下载项是否结束
	// @return: 下载结束的文件名
	// @Note: 每次返回结果
	virtual const char *PopDownloadItem(EnPackCompleteStatus &enStatus) = 0;

	// 获取文件的大小
	// @param pszFileName: 文件名  
	// @param uFileHash: 文件hash
	virtual unsigned long GetFileSize(const char *pszFileName, unsigned int uFileHash) = 0;

#ifdef _WIN32
	// 设置上下行数据监控时间
	// @param nUpFlowTime	: 上行流量监控时间，单位秒
	// @param nDownFlowTime	: 下行流量监控时间，单位秒
	virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime) = 0;

	// 获取下载上行数据
	// @param dUpAverageFlow: 上行平均流量，单位KB/S
	// @param dUpMaxFlow	: 上行最大流量，单位KB/S
	// @param dUpMinFlow	: 上行最小流量，单位KB/S
	virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow) = 0;

	// 获取下载下行数据
	// @param dDownAverageFlow: 下行平均流量，单位KB/S
	// @param dDownMaxFlow	  : 下行最大流量，单位KB/S
	// @param dDownMinFlow	  : 下行最小流量，单位KB/S
	virtual void GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow) = 0;
#endif
};

// 下载操作
struct IDownloadManage
{	
	// 校验实例
	// @return: 返回校验的单例
	virtual IVertifyInstance *GetVertifyInstance(void) = 0;

	// 创建任务
	// @return: 返回下载任务的实例
	virtual IPackDownloadTask *CreateTask(void) = 0;

	// 网络切换(移动端下使用)
	// @return: 返回切换结果 false 失败，true成功
	virtual bool SwitchNetWorks(void) = 0;

	// 退出下载线程, 并任务销毁, 统一调用该接口
	// @Note: CreateTask与DestroyTask配对使用。
	virtual void DestoryTask(void) = 0;

	// 清理内存，最后调用
	virtual void Release(void) = 0;
};

// 创建实例
#ifdef _WIN32
extern "C" PACKRESDOWN_API IDownloadManage* CreatePackResDown();
#else
extern "C" DLL_PUBLIC PACKRESDOWN_API IDownloadManage* CreatePackResDown();
#endif

// 获取文件的哈希值
// @param pszFileName: 文件名  
// @return: 返回哈希值，文件不存在值为0
#ifdef _WIN32
extern "C" PACKRESDOWN_API unsigned int GetCurFileHash(const char* pszFileName);
#else
extern "C" DLL_PUBLIC PACKRESDOWN_API unsigned int GetCurFileHash(const char* pszFileName);
#endif

#endif
