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

// У�鷽ʽ
typedef enum
{
	EnVertify_Pack7z_Download_Type	= 0x0,	// �������·�ʽ
	EnVertify_Pack7z_BakFile_Type	= 0x1,	// �����ļ���ʽ
	EnVertify_Pack7z_Extend_Type	= 0x2	// ��չĿ¼��ʽ��IOS�˰��ڣ�
}EnVertifyType;

// ���ݷ��������б�
typedef enum
{
	EnVertify_BakUpdateHashFail		= -102,	// ͨ����ѡ����,�ļ�hashУ��ʧ��
	EnVertify_BakPackDecompressFail	= -101,	// �����ļ���pack.7z��ѹʧ��
	EnVertify_BakPackFileNoFound	= -100	// δ����pack.7z�����ļ�
}EnVertifyBakStatus;

// ����У���б�
typedef enum
{
	EnVertify_PackVersionError		= -10,  // pack.xml �汾�Ų�ͬ
	EnVertify_SetVersionFail		= -9,	// ���ø��°汾��ʧ��
	EnVertify_CreateConfigFail		= -8,	// ���������б��ļ�ʧ��
	EnVertify_GetServerFail			= -7,	// ����汾��С�ڵ���0
	EnVertify_PathEmptyFail			= -6,	// ��Դ���·��Ϊ��
	EnVertify_PackDecompressFail	= -5,	// ���ص�pack.7z��ѹʧ��
	EnVertify_ParsePackFail			= -4,	// ����pack�б�ʧ��
	EnVertify_PackHashFail			= -3,	// pack�ļ�hashУ��ʧ��	
 	EnVertify_DownPackFail			= -2,	// �ļ�����ʧ��
	EnVertify_Fail					= -1,	// У�鲻ͨ��
	EnVertify_UnUpdate				= 0	,	// �������
	EnVertify_Update				= 1 	// ��Ҫ����
}EnVertifyStatus;

// �������
struct IPackResDownloadItem
{
	// �����ʼִ��
	virtual void OnPackDownloadBegin(const char *pszFileName) = 0;

	// ������ִ�й����е�״̬��Ϣ
	virtual void OnPackDownloading(const char *pszFileName, int nCurrByte, int nTotalByte) = 0;

	// ���������
	virtual void OnPackDownloadFinish(const char *pszFileName, int nPackVersion, EnPackCompleteStatus enStatus, EnDownloadAddr enAddr) = 0;
};

// �����������
struct IPackResDownloadListen
{
	// ����������ļ��� ��������
	virtual void OnItemStatusListener(const char *pszFileName, EnNetStatusListener enStatus, const char* pszErrInfo) = 0;
    
	// @param dDownloadSpeed: �����ٶ� KB/S
	// @param llDownSize	: �����ļ��Ĵ�С
	// @param nDownloadCount: �������ļ�����
	// @param dDownloadSpeed: �����߳���
//#ifdef _WIN32
	// ��ȡ���������е��������ٶȣ������ļ��Ĵ�С�������ļ������������߳���
    //virtual void GetTaskDownloadInfo(double dDownloadSpeed, long long llDownSize, int nDownloadCount, int nThreadCount) = 0;
//#else
	// ��ȡ���������е��������ٶȣ������ļ������������߳���
	virtual void GetTaskDownloadInfo(double dDownloadSpeed, int nDownloadCount, int nThreadCount) = 0;
//#endif
};

// У��ص�
struct IResVertifyStatus
{
	// У����Ȼص�
	// @param nVertifyStatus:	״̬���EnVertifyStatus 
	// @param nProgress:		0 ~ 99: ��ʾУ�����; 100ʱ����У��״̬
	// @param nVertifyType:		0 ͨ�����ؽ���У�飬 1 ͨ�����ر��ݽ���У�� (����У��ʧ��ʱ�ص�����ΪEnVertifyType)
	// @param pszErrInfo:		��������Ļص���ʾ
	virtual void OnVertifyFile(int nVertifyStatus, int nProgress, int nVertifyType, const char* pszErrInfo) = 0;

	// У��׶���Ϣ�ص�
	// @param pszMsg: У��׶ε���Ϣ���� (�����汾У��ʱ��Ҫɾ�����ļ���ɾ��hash��ͬ�ľ��ļ����汾���쳣������)
	virtual void OnVertifyProgressMsgs(const char *pszMsg) = 0;

	// ɾ���ļ����Ȼص�
	// @param nCurrentCount: ��ǰ�Ѿ�ɾ��������
	// @param nTotalCount:	  �ܹ�Ҫɾ��������
	virtual void OnDeleteFile(int nCurrentCount, int nTotalCount) = 0;
};

// У��
struct IVertifyInstance
{
	// ��ʼ������
	// @param: pszResSavePath:			 ���ش�Ÿ�Ŀ¼
	// @param: pszExtendResSavePath:	 IOS��ʹ��ָ��IOS����Ŀ¼
	// @param: pszCdnDomainNameAddr:	 CND�������ص�ַ			(�� http://nd.cdn.soul.com/updatedat)
	// @param: pszSourceAddrIp:			 ����ԴIP���ص�ַ			(�� http://192.168.23.231/updatedat) 
	// @param: pszSourceAddrIpBak:		 ����ԴIP���ص�ַ			(�� http://192.168.23.232/updatedat) 
	// @return:							 ����false����ʧ�ܣ�true �ɹ�
 	virtual bool InitConfig(const char *pszResSavePath, const char *pszExtendResSavePath, const char *pszCdnDomainNameAddr, const char *pszSourceAddrIp, const char *pszSourceAddrIpBak) = 0;

	// ��ʼ��У��ص�������������У����½ӿ�
	// @param:		pVertify У��ص�
	// @note:		ͨ��pVertify�ص�У����
	// @return:		false ��ʼ��ʧ��, ture �ɹ�
	virtual bool InitResVertifyCallback(IResVertifyStatus *pVertify) = 0;

	// �汾У����󣬻�ȡ��ǰ���µİ汾��
	// @return: �������°汾�ţ�-1 ��ȡʧ��
	virtual int GetServerVersion(void) = 0;

	// ���õ�ǰ�������汾��
	virtual void SetServerVersion(int nServerVersion) = 0;

	// ��ȡ�����б��ļ�·��
	// @return: ���ز����б��ļ���·��
	virtual const char *GetVertifyFilePath(void) = 0;

	// ��ȡ�����ļ��б��·��
	// @return: ����У���ɾ���ļ��б��·��
	virtual const char *GetVertifyDelFilePath(void) = 0;

	// ɾ�������ļ�
	virtual void DeleteDifferFile(void) = 0;

	// ֹͣɾ�������ļ�
	virtual void StopDeleteDifferFile(void) = 0;

	// У�����
	// @pararm: bAutoUpdate Ĭ��Ϊfalse, ����Ϊtrue������ӿڻ�ǿ�ƴ�������pack.7z����ִ��У������
	virtual void VertifyPackRes(bool bAutoUpdate) = 0;

	// У�鱾��*.pack�ļ���hash��pack.xml�е�hashֵ�Ƿ���ͬ
	// @param: pszFileName: ��c3pack/pack_info_xxx.pack
	// @param: nType: 0��ʾ�޽��, 1��ʾ�ڴ����ļ�ƥ��ɹ�, 2��ʾIOS�а����ļ�ƥ��ɹ�
	// @return: ����trueֵ��ͬ, ����falseֵ��ͬ(pack�ļ����淵��false)
	virtual bool PackFileHashCompare(const char *pszFileName, int &nType) = 0;

	// У���¼��ص�
	virtual void EventProcess(void) = 0;
};

// ��������
struct IPackDownloadTask
{
	// ��ʼ��������
	// @param pPackResItem:   ���ؽ���
	// @param pPackResListen: �����������
	// @param nThreadCount:   ָ�������߳���
	// @return: �������������״̬, 0 ʧ��, 1 �ɹ�
	virtual bool StartTask(IPackResDownloadItem *pPackResItem, IPackResDownloadListen *pPackResListen, int nThreadCount) = 0;

	// �����¼��ص�
	virtual void EventProcess(void) = 0;

	// δ��ɵ���������
	// @return: ����δ��ɵ�������
	virtual int UnFinishDownloadItemCount(void) = 0;

	// ���������
	// @param bPrior: �������Ƿ�����  
	// @param pszFileName: ��Ҫ���ص��ļ���
	// @return: ������ӵ�״̬
	virtual bool PushDownloadItem(bool bPrior, const char *pszFileName) = 0;

	// �������Ƿ����
	// @return: ���ؽ������ļ���
	// @Note: ÿ�η��ؽ��
	virtual const char *PopDownloadItem(EnPackCompleteStatus &enStatus) = 0;

	// ��ȡ�ļ��Ĵ�С
	// @param pszFileName: �ļ���  
	// @param uFileHash: �ļ�hash
	virtual unsigned long GetFileSize(const char *pszFileName, unsigned int uFileHash) = 0;

#ifdef _WIN32
	// �������������ݼ��ʱ��
	// @param nUpFlowTime	: �����������ʱ�䣬��λ��
	// @param nDownFlowTime	: �����������ʱ�䣬��λ��
	virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime) = 0;

	// ��ȡ������������
	// @param dUpAverageFlow: ����ƽ����������λKB/S
	// @param dUpMaxFlow	: ���������������λKB/S
	// @param dUpMinFlow	: ������С��������λKB/S
	virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow) = 0;

	// ��ȡ������������
	// @param dDownAverageFlow: ����ƽ����������λKB/S
	// @param dDownMaxFlow	  : ���������������λKB/S
	// @param dDownMinFlow	  : ������С��������λKB/S
	virtual void GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow) = 0;
#endif
};

// ���ز���
struct IDownloadManage
{	
	// У��ʵ��
	// @return: ����У��ĵ���
	virtual IVertifyInstance *GetVertifyInstance(void) = 0;

	// ��������
	// @return: �������������ʵ��
	virtual IPackDownloadTask *CreateTask(void) = 0;

	// �����л�(�ƶ�����ʹ��)
	// @return: �����л���� false ʧ�ܣ�true�ɹ�
	virtual bool SwitchNetWorks(void) = 0;

	// �˳������߳�, ����������, ͳһ���øýӿ�
	// @Note: CreateTask��DestroyTask���ʹ�á�
	virtual void DestoryTask(void) = 0;

	// �����ڴ棬������
	virtual void Release(void) = 0;
};

// ����ʵ��
#ifdef _WIN32
extern "C" PACKRESDOWN_API IDownloadManage* CreatePackResDown();
#else
extern "C" DLL_PUBLIC PACKRESDOWN_API IDownloadManage* CreatePackResDown();
#endif

// ��ȡ�ļ��Ĺ�ϣֵ
// @param pszFileName: �ļ���  
// @return: ���ع�ϣֵ���ļ�������ֵΪ0
#ifdef _WIN32
extern "C" PACKRESDOWN_API unsigned int GetCurFileHash(const char* pszFileName);
#else
extern "C" DLL_PUBLIC PACKRESDOWN_API unsigned int GetCurFileHash(const char* pszFileName);
#endif

#endif
