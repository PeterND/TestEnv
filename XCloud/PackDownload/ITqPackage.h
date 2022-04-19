#ifndef __I_TQ_PACKAGE_H__
#define __I_TQ_PACKAGE_H__

#include "C3BaseType.h"

#if defined(_WIN32)
#ifndef TQPACKAGE_API
#ifdef TQPACKAGE_EXPORTS
#define TQPACKAGE_API __declspec(dllexport)
#else
#define TQPACKAGE_API __declspec(dllimport)
#endif
#endif
#else
#define TQPACKAGE_API
#endif

enum TQPRESULT
{
	TQR_OK					=		0, // ����
	TQR_PARAMETER_ERROR		=		1, // ����������
	TQR_BUFFER_OVERFLOW		=		2, // �������
	TQR_FILE_NOTFOUND		=		3, // δ�ҵ��ļ�
	TQR_FILE_OPEN_ERROR		=		4, // ���ļ�ʧ��
	TQR_FILE_READ_ERROR		=		5, // �ļ���ʧ��
	TQR_FILE_WRITE_ERROR	=		6, // �ļ�дʧ��
	TQR_WRITEMUTEX_ERROR	=		7, // ��д��������
	TQR_READMUTEX_ERROR		=		8, // ������������
	TQR_COMPRESS_ERROR		=		9, // ѹ�����ѹ����
	TQR_NOT_MATCHING		=		10, // �����������ļ���ƥ��
	TQR_NOT_OPENFILE		=		11,	// �ļ�δ��
	TQR_REPACK_OK			=		12,	// �������ɹ�
	TQR_REPACK_NOTINIT		=		13, // δ��ʼ��
	TQR_FILENAME_ERROR		=		14,	// �ļ�������
	TQR_NOTINIT				=		15,	// û��ʼ��
	TQR_INVALID_BLOCK		=		16,	// ��Ч�Ŀ�
	TQR_ALLOCMEMORY_ERROR	=		17,	// �����ڴ�ʧ��
};

//�����ļ���дģʽ
enum OpenFileMode
{
	OpenFileMode_Read		=		0,//ֻ��
	OpenFileMode_ReadWrite	=		1,//��д
};

class ITqPackage
{
public:
	// ���ͷŽӿ�
	virtual		DWORD		Release(void)													=	0;

	// ����ļ��Ƿ����
	virtual		bool		CheckFile( const char* pszFileName, DWORD* nFileSize=NULL )		=	0; 

	// ����package...
	virtual		TQPRESULT	Create(const char* pszPackageFileName)							=	0;

	// ��package...
	virtual		TQPRESULT	Open(const char* pszPackageFileName, enum OpenFileMode eMode )	=	0;
	
	// �����ļ���Buffer��uSize����Buffer��С��������ѹ���ļ���С
	virtual		TQPRESULT	Load(const char* pszFileName, BYTE* pBuffer, DWORD& uSize)		=	0;
	// �ر�package...
	virtual		TQPRESULT	Close(void)														=	0;
		
	// ��ӻ����һ���ļ�
	virtual		TQPRESULT   AddFile(const char* pszFileName)								=	0;
	
	// ���������ļ� 
	virtual		TQPRESULT   UpdateIndex(void)												=	0;

	// ɾ��һ���ļ�
	virtual		TQPRESULT	DelFile(const char* pszFileName)								=	0;

	// ������Ƭ
	virtual		TQPRESULT	Repack( DWORD& uItem, char* pFileName )							=	0;

	// ȡ�ļ�����
	virtual		DWORD		GetFileAmount(void)												=	0;
	
	#ifdef TQEDITOR // �����Ʒ����
	virtual		const char*	GetFileName(DWORD uIndex)										=	0;
	#endif	//TQEDITOR
};

extern "C" TQPACKAGE_API ITqPackage* TqPackageCreate();

enum FILETYPE//�ļ�����
{
	FILETYPE_ALL	=	0,	// ����
	FILETYPE_UNPACK,		// ĩ������ļ�
	FILETYPE_PACK,			// ������ļ�
};

enum TQSEEKMODE
{
	TQSEEK_CUR		=	SEEK_CUR,
	TQSEEK_END		=	SEEK_END,
	TQSEEK_BEGIN	=	SEEK_SET,
};
typedef void*	HTQF;		// TqFileHandle

///////////////////////���ݰ����//////////////////////////////////////
// ��һ�����ݰ��ļ�
extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesOpen(const char* pszPackageFile);

// TQPackageClose �ر����ݰ��ļ� ����NULLȫ���ر�
extern "C" void		TQPACKAGE_API		TqPackagesClose(const char* pszPackageFile);
////////////////////////�ļ����////////////////////////////////////////

// ̽��ָ���ļ��Ƿ���ָ�����ļ��б���
extern "C" bool		TQPACKAGE_API		TqFCheck(const char* pszFile, FILETYPE eFileType);

// ����ĳ���ļ�, ���ػ������Լ���������С; ������ݰ��ļ���û���ҵ�����ļ�, �򷵻�false
extern "C" TQPRESULT	TQPACKAGE_API		TqFDump(const char* pszFile, void*& pBuf, DWORD& uSize);

// ���DUMP������
extern "C" TQPRESULT	TQPACKAGE_API	 	TqFUndump(void* pBuffer);

// ��λ���ݰ��е�ĳ���ļ�	 
extern "C" TQPRESULT	TQPACKAGE_API		TqFOpen(const char* pszFile, HTQF& hTqf);

//�ر�
extern "C" TQPRESULT	TQPACKAGE_API		TqFClose(HTQF hTqf );

// ���ļ�����һ�����ݿ�, ���ض�������ݴ�С(buf���ⲿ����)(����fread)
extern "C" int			TQPACKAGE_API		TQFRead(void* pBuf, DWORD uSize, DWORD uCount, HTQF hTqf);

// ��λ�ļ�ƫ��
extern "C" bool		TQPACKAGE_API		TqFSeek(HTQF hTqf, int uOffset, TQSEEKMODE eSeek);

// ȡ�ö�дָ��λ��
extern "C" DWORD		TQPACKAGE_API		TqFTell(HTQF hTqf);

// ̽���ļ��Ƿ��ļ�β
extern "C" bool		TQPACKAGE_API		TqFIsEof(HTQF hTqf);

//ȡ�ļ�����
extern "C" DWORD		TQPACKAGE_API		TqFGetLen(HTQF hTqf);

//ȡ�汾��
extern "C" DWORD		TQPACKAGE_API		TqFGetVersion(void);

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesInit(const char* pszAppFolderPath, const char* pszUpdateFolderPath);

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesShutdown();

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesSetPriority(const char* pszPackFile, DWORD nPriority);

//////////////////////////////////////////////////////////////////////////
// �ļ���ȡ�ӿڣ��̰߳�ȫ
//////////////////////////////////////////////////////////////////////////
class IDataFileLoader
{
public:
	virtual bool CheckFile(const char* pszFile, DWORD* pSize = NULL) = 0;
	virtual bool LoadFile(const char* pszFile, void* pBuf, DWORD dwSize) = 0; // Buf�ڴ��ɵ����߷���ͻ���,��СҪ�㹻���Ҫ�����ļ��Ĵ�С
};

// ����Tqp���м���ֻ���ļ��������м��ش���ļ�
extern "C" TQPACKAGE_API bool TqPackagesOpenWithFileLoader(const char* pszPackageFile, IDataFileLoader* pLoader);
//////////////////////////////////////////////////////////////////////////
// ���ݰ�����(С��)
//////////////////////////////////////////////////////////////////////////
class IDataPack
{
public:
	enum PACK_STATE
	{
		PACK_STATE_NONE,
		PACK_STATE_DOWN_ING,
		PACK_STATE_LOAD_ING,
		PACK_STATE_READY,
		PACK_STATE_FAIL,	 // >=FAIL�Ķ���ʧ��״̬
		PACK_STATE_FAIL_DOWN,
		PACK_STATE_FAIL_LOAD,
	};
	static bool IsLoadOver(int nState) { return nState >= PACK_STATE_READY; }
	static bool IsPackFail(int nState) { return nState >= PACK_STATE_FAIL; }
	static bool IsPackReady(int nState) { return nState == PACK_STATE_READY; }

public:
	virtual void Release(void) = 0;
	virtual const char* GetPackFile(void) = 0;
	virtual void SetPackFile(const char* pszFile) = 0;
	virtual bool LoadPack(const char* pszRootPath) = 0;
	virtual void ClosePack(void) = 0; // �ر��ļ����,�������,��·����״̬��Ϣ���ᱻ���. [�̲߳���ȫ]

	virtual void SetPackState(int nState) = 0;
	virtual int GetPackState(void) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// ����Դ��������ɺ����½ӿڲ����̰߳�ȫ��
public:
	virtual int GetFileAmount(void) = 0;
	virtual const char* GetFileName(int nIndex) = 0;
	virtual IDataFileLoader* GetFileLoader(void) = 0;

	//////////////////////////////////////////////////////////////////////////
	// �����ؽӿ�[�̲߳���ȫ][�ڵ��ô���ӿ�ʱ���ܶ��̵߳����ļ����ؽӿ�]
	// pszFileΪ���ڵ��ļ���; pszFullPathΪδ������ļ���Ӳ���ϵľ���·��
public:
	virtual bool AddFile(const char* pszFile, const char* pszFullPath) = 0;
	virtual bool DelFile(const char* pszFile) = 0;
	virtual bool SavePack(const char* pszRootPath) = 0;
};

TQPACKAGE_API IDataPack* DataPackCreate(void);
//////////////////////////////////////////////////////////////////////////

#endif//__I_TQ_PACKAGE_H__
