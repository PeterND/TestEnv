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

//���ػص�����
typedef int(*PackDownCallback)(int nStatus);

//�ռ���Ϣ���ͺ���
typedef void (*PFnSendDataToHttp)(const char* pszAddr, const char* pszMsg);

enum
{
	DS_SUB_STATUS_NONE,

	//////////////////////////////////////////////////////////////////////////
	// DS_VERTIFY_ING_VERSION ��״̬
	DS_SUB_STATUS_DEL_OLD_PACK,		// ɾ���ɰ�
	DS_SUB_STATUS_CHECK_VERSION,	// ���汾��
	DS_SUB_STATUS_UPDATE_VERSION,	// ����΢�˰汾
	// DS_VERTIFY_ING_PACK_INFO ��״̬
	DS_SUB_STATUS_DOWN_PACK_INFO_BEGIN,		// ��ʼ����pack_info��
	DS_SUB_STATUS_DOWN_PACK_INFO_END,	// ����pack_info�����
	DS_SUB_STATUS_LOAD_PACK_INFO_END,	// ����pack_info�����
	//DS_VERTIFY_FAIL_VERSION ��״̬
	DS_SUB_STATUS_VERTIFY_FAIL_LOCAL_ERROR,  //��֤ʱ���ش���,���Գ���������Ϸ
	DS_SUB_STATUS_VERTIFY_FAIL_SERVER_ERROR, //��֤ʱ����������
	//////////////////////////////////////////////////////////////////////////
};

//�ļ�����ʧ����
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
//΢�˻ص����ͻ����߼��ĺ���:
//���� nStatus ��΢���ڲ����룬�ص������ͻ���
//����ֵ �Ǹ��ͻ���ʹ�ã�����ͻ������߼�

class IPackDownManager
{
public:
	//����ɰ汾��Դ�ı��ݻ�ԭ
	virtual bool CheckOldVersionRes(const char* pszAppDir, const char* pszCacheDir) = 0;
public:
	virtual bool Init(int nGameType, int nCooperator, int nResVesion, const char* pszAppDir, const char* pszCacheDir,
		const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr, PFnSendDataToHttp pfnSend) = 0;
	virtual bool ProcessPackVerify(void) = 0;
	virtual void Process(void) = 0;

	virtual int GetVertifyCode(void) = 0;   //��ȡ��֤��.Ϊ��λ���ⷽ��,����֤��ʾ�ڽ�����

	virtual int GetVertifyStatus(int& nSubStatus) = 0; // ��ȡ��֤״̬. ����ֵΪ:��״ֵ̬,������IPluginForMY.h�����DS_VERTIFY_XXX.
	// nSubStatus:��ʾDS_VERTIFY_XXX �����е���״̬��ֵ,�����ڸ��ļ��� DS_SUB_STATUS_XXX
	virtual int GetStatusProgress(void) = 0; // ��ȡ״̬���ȣ��ٷ���[0-100]�����Ի�ȡ���ȵ�״̬[DS_SUB_STATUS_DEL_OLD_PACK, DS_SUB_STATUS_UPDATE_VERSION]

	virtual void EnableBgDownload(bool bEnable) = 0; //�Ƿ����ú�̨����,Ĭ�������� ini/app_config.xml

	virtual void EnableInitRes(bool bEnable, const char* pszInitResPath) = 0; //�Ƿ����ó�ʼ���,��Ҫ��������·��

	virtual bool IsPackDownEnable(void) = 0;	//΢�������Ƿ���

	virtual bool IsBgDownloadEnable(void) = 0; //��̨�����Ƿ���

	virtual bool IsFgDownloadEnable(void) = 0; //ǰ̨�����Ƿ���

	virtual void SetDownloadOrder(int nStrategy = 0) = 0;// �ͻ��˸�����Ҫ�������������������

	virtual void SetPackDownCallback(PackDownCallback pFnCallback) = 0;//�������ػص�����

	//��¼�ɹ�֮������
	virtual void SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
								 const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion,int nFreeSpaceMb) = 0;

	//��ȡ������Ϣ
	//nTotalCount:������Ҫ���µİ����ܸ���. nFinishCount:�Ѿ����غõİ��ĸ���
	//uTotalSize:������Ҫ���µ�������Դ�Ĵ�С�ܺ�. uDownSize:�Ѿ�������ɵİ��Ĵ�С�ܺ�.(�������ĵ�λ���� kb)
	//nCurDownSpeed:��ʾ��ǰ�������ٶ�,��λ(kb/s).strCurDownFileName:�ǵ�ǰ�������صİ�������
	virtual bool GetDownLoadInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, int& nCurDownSpeed, char* pszCurDownFileName, int& nDownStatus) = 0;

	virtual void SendC3CallbackMsg(const char* pszMsg) = 0;

	// �����л�(�ƶ�����ʹ��)
	// @return: �����л���� false ʧ�ܣ�true�ɹ�
	virtual bool SwitchNetWorks(void) = 0;

	//�������
	virtual bool IsDownloadFinish() = 0;

#ifdef _WIN32
	// �������������ݼ��ʱ��
	// @param nUpFlowTime	: �����������ʱ�䣬��λ��
	// @param nDownFlowTime	: �����������ʱ�䣬��λ��
	virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime, bool bBg = true) = 0;

	// ��ȡ������������
	// @param dUpAverageFlow: ����ƽ����������λKB/S
	// @param dUpMaxFlow	: ���������������λKB/S
	// @param dUpMinFlow	: ������С��������λKB/S
	virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow, bool bBg = true) = 0;

	// ��ȡ������������
	// @param dDownAverageFlow: ����ƽ����������λKB/S
	// @param dDownMaxFlow	: ���������������λKB/S
	// @param dDownMinFlow	: ������С��������λKB/S
	virtual void GetDownFlowInfo(double &dDownAverageFlow, double &dDownMaxFlow, double &dDownMinFlow, bool bBg = true) = 0;
#endif
};

extern "C" C3PACKDOWN_API IPackDownManager& GetPackDownManger();
extern "C" C3PACKDOWN_API void RelesePackDownManger();

#endif