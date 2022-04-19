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
		//����ɰ汾��Դ�ı��ݻ�ԭ
		virtual bool CheckOldVersionRes(const char* pszAppDir, const char* pszCacheDir);
	public:
		virtual bool Init(int nGameType, int nCooperator, int nResVesion, const char* pszAppDir, const char* pszCacheDir,
			const char* pszCndAddr, const char* pszSourceAddr, const char* pszBakAddr, const char* pszFeedbackAddr,PFnSendDataToHttp pfnSend);
		virtual bool ProcessPackVerify(void);
		virtual void Process(void);

		virtual int GetVertifyCode(void);    //��ȡ��֤��.Ϊ��λ���ⷽ��,����֤��ʾ�ڽ�����

		virtual int GetVertifyStatus(int& nSubStatus); // ��ȡ��֤״̬. ����ֵΪ:��״ֵ̬,������IPluginForMY.h�����DS_VERTIFY_XXX.
															 // nSubStatus:��ʾDS_VERTIFY_XXX �����е���״̬��ֵ,�����ڸ��ļ��� DS_SUB_STATUS_XXX
		virtual int GetStatusProgress(void); // ��ȡ״̬���ȣ��ٷ���[0-100]�����Ի�ȡ���ȵ�״̬[DS_SUB_STATUS_DEL_OLD_PACK, DS_SUB_STATUS_UPDATE_VERSION]

		virtual void EnableBgDownload(bool bEnable); //�Ƿ����ú�̨����,Ĭ�������� ini/app_config.xml

		virtual void EnableInitRes(bool bEnable, const char* pszInitResPath); //�Ƿ����ó�ʼ���,��Ҫ��������·��

		virtual bool IsPackDownEnable(void);	//΢�������Ƿ���

		virtual bool IsBgDownloadEnable(void); //��̨�����Ƿ���

		virtual bool IsFgDownloadEnable(void); //ǰ̨�����Ƿ���

		virtual void SetDownloadOrder(int nStrategy = 0);// �ͻ��˸�����Ҫ�������������������

		virtual void SetPackDownCallback(PackDownCallback pFnCallback);//�������ػص�����

		virtual void SetFeedbackInfo(int nServerID,int nAccountID, const char* pszAccountName, const char* pszChannelID, 
			const char* pszDeviceID, const char* pszDeviceName, const char* pszOSVersion, int nFreeSpaceMb);
		//��ȡ������Ϣ
		//nTotalCount:������Ҫ���µİ����ܸ���. nFinishCount:�Ѿ����غõİ��ĸ���
		//uTotalSize:������Ҫ���µ�������Դ�Ĵ�С�ܺ�. uDownSize:�Ѿ�������ɵİ��Ĵ�С�ܺ�.(�������ĵ�λ���� kb)
		//nCurDownSpeed:��ʾ��ǰ�������ٶ�,��λ(kb/s).strCurDownFileName:�ǵ�ǰ�������صİ�������
		virtual bool GetDownLoadInfo(int& nTotalCount, int& nFinishCount, unsigned int& uTotalSize, unsigned int& uDownSize, int& nCurDownSpeed, char* pszCurDownFileName, int& nDownStatus);

		virtual void SendC3CallbackMsg(const char* pszMsg);

		// �����л�(�ƶ�����ʹ��)
		// @return: �����л���� false ʧ�ܣ�true�ɹ�
		virtual bool SwitchNetWorks(void);

		//�������
		virtual bool IsDownloadFinish();

#ifdef _WIN32
		// �������������ݼ��ʱ��
		// @param nUpFlowTime	: �����������ʱ�䣬��λ��
		// @param nDownFlowTime	: �����������ʱ�䣬��λ��
		virtual void SetFlowTime(int nUpFlowTime, int nDownFlowTime, bool bBg = true);

		// ��ȡ������������
		// @param dUpAverageFlow: ����ƽ����������λKB/S
		// @param dUpMaxFlow	: ���������������λKB/S
		// @param dUpMinFlow	: ������С��������λKB/S
		virtual void GetUpFlowInfo(double &dUpAverageFlow, double &dUpMaxFlow, double &dUpMinFlow, bool bBg = true);

		// ��ȡ������������
		// @param dDownAverageFlow: ����ƽ����������λKB/S
		// @param dDownMaxFlow	: ���������������λKB/S
		// @param dDownMinFlow	: ������С��������λKB/S
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
