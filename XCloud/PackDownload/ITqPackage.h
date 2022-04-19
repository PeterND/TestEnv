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
	TQR_OK					=		0, // 正常
	TQR_PARAMETER_ERROR		=		1, // 参数检查错误
	TQR_BUFFER_OVERFLOW		=		2, // 缓冲溢出
	TQR_FILE_NOTFOUND		=		3, // 未找到文件
	TQR_FILE_OPEN_ERROR		=		4, // 打开文件失败
	TQR_FILE_READ_ERROR		=		5, // 文件读失败
	TQR_FILE_WRITE_ERROR	=		6, // 文件写失败
	TQR_WRITEMUTEX_ERROR	=		7, // 包写操作互斥
	TQR_READMUTEX_ERROR		=		8, // 包读操作互斥
	TQR_COMPRESS_ERROR		=		9, // 压缩或解压错误
	TQR_NOT_MATCHING		=		10, // 数据与索引文件不匹配
	TQR_NOT_OPENFILE		=		11,	// 文件未打开
	TQR_REPACK_OK			=		12,	// 重整包成功
	TQR_REPACK_NOTINIT		=		13, // 未初始化
	TQR_FILENAME_ERROR		=		14,	// 文件名错误
	TQR_NOTINIT				=		15,	// 没初始化
	TQR_INVALID_BLOCK		=		16,	// 无效的块
	TQR_ALLOCMEMORY_ERROR	=		17,	// 分配内存失败
};

//数据文件读写模式
enum OpenFileMode
{
	OpenFileMode_Read		=		0,//只读
	OpenFileMode_ReadWrite	=		1,//读写
};

class ITqPackage
{
public:
	// 自释放接口
	virtual		DWORD		Release(void)													=	0;

	// 检查文件是否存在
	virtual		bool		CheckFile( const char* pszFileName, DWORD* nFileSize=NULL )		=	0; 

	// 创建package...
	virtual		TQPRESULT	Create(const char* pszPackageFileName)							=	0;

	// 打开package...
	virtual		TQPRESULT	Open(const char* pszPackageFileName, enum OpenFileMode eMode )	=	0;
	
	// 载入文件到Buffer，uSize传入Buffer大小，传出解压后文件大小
	virtual		TQPRESULT	Load(const char* pszFileName, BYTE* pBuffer, DWORD& uSize)		=	0;
	// 关闭package...
	virtual		TQPRESULT	Close(void)														=	0;
		
	// 添加或更新一个文件
	virtual		TQPRESULT   AddFile(const char* pszFileName)								=	0;
	
	// 更新索引文件 
	virtual		TQPRESULT   UpdateIndex(void)												=	0;

	// 删除一个文件
	virtual		TQPRESULT	DelFile(const char* pszFileName)								=	0;

	// 整理碎片
	virtual		TQPRESULT	Repack( DWORD& uItem, char* pFileName )							=	0;

	// 取文件数量
	virtual		DWORD		GetFileAmount(void)												=	0;
	
	#ifdef TQEDITOR // 不随产品发布
	virtual		const char*	GetFileName(DWORD uIndex)										=	0;
	#endif	//TQEDITOR
};

extern "C" TQPACKAGE_API ITqPackage* TqPackageCreate();

enum FILETYPE//文件类型
{
	FILETYPE_ALL	=	0,	// 所有
	FILETYPE_UNPACK,		// 末打包的文件
	FILETYPE_PACK,			// 打包的文件
};

enum TQSEEKMODE
{
	TQSEEK_CUR		=	SEEK_CUR,
	TQSEEK_END		=	SEEK_END,
	TQSEEK_BEGIN	=	SEEK_SET,
};
typedef void*	HTQF;		// TqFileHandle

///////////////////////数据包相关//////////////////////////////////////
// 打开一个数据包文件
extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesOpen(const char* pszPackageFile);

// TQPackageClose 关闭数据包文件 输入NULL全部关闭
extern "C" void		TQPACKAGE_API		TqPackagesClose(const char* pszPackageFile);
////////////////////////文件相关////////////////////////////////////////

// 探测指定文件是否在指定的文件列表中
extern "C" bool		TQPACKAGE_API		TqFCheck(const char* pszFile, FILETYPE eFileType);

// 加载某个文件, 返回缓冲区以及缓冲区大小; 如果数据包文件中没有找到这个文件, 则返回false
extern "C" TQPRESULT	TQPACKAGE_API		TqFDump(const char* pszFile, void*& pBuf, DWORD& uSize);

// 清空DUMP缓冲区
extern "C" TQPRESULT	TQPACKAGE_API	 	TqFUndump(void* pBuffer);

// 定位数据包中的某个文件	 
extern "C" TQPRESULT	TQPACKAGE_API		TqFOpen(const char* pszFile, HTQF& hTqf);

//关闭
extern "C" TQPRESULT	TQPACKAGE_API		TqFClose(HTQF hTqf );

// 从文件读入一个数据块, 返回读入的数据大小(buf由外部分配)(兼容fread)
extern "C" int			TQPACKAGE_API		TQFRead(void* pBuf, DWORD uSize, DWORD uCount, HTQF hTqf);

// 定位文件偏移
extern "C" bool		TQPACKAGE_API		TqFSeek(HTQF hTqf, int uOffset, TQSEEKMODE eSeek);

// 取得读写指针位置
extern "C" DWORD		TQPACKAGE_API		TqFTell(HTQF hTqf);

// 探测文件是否到文件尾
extern "C" bool		TQPACKAGE_API		TqFIsEof(HTQF hTqf);

//取文件长度
extern "C" DWORD		TQPACKAGE_API		TqFGetLen(HTQF hTqf);

//取版本号
extern "C" DWORD		TQPACKAGE_API		TqFGetVersion(void);

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesInit(const char* pszAppFolderPath, const char* pszUpdateFolderPath);

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesShutdown();

extern "C" TQPRESULT	TQPACKAGE_API		TqPackagesSetPriority(const char* pszPackFile, DWORD nPriority);

//////////////////////////////////////////////////////////////////////////
// 文件读取接口，线程安全
//////////////////////////////////////////////////////////////////////////
class IDataFileLoader
{
public:
	virtual bool CheckFile(const char* pszFile, DWORD* pSize = NULL) = 0;
	virtual bool LoadFile(const char* pszFile, void* pBuf, DWORD dwSize) = 0; // Buf内存由调用者分配和回收,大小要足够存放要加载文件的大小
};

// 不从Tqp包中加载只从文件加载器中加载打包文件
extern "C" TQPACKAGE_API bool TqPackagesOpenWithFileLoader(const char* pszPackageFile, IDataFileLoader* pLoader);
//////////////////////////////////////////////////////////////////////////
// 数据包对象(小包)
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
		PACK_STATE_FAIL,	 // >=FAIL的都是失败状态
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
	virtual void ClosePack(void) = 0; // 关闭文件句柄,清除数据,包路径和状态信息不会被清除. [线程不安全]

	virtual void SetPackState(int nState) = 0;
	virtual int GetPackState(void) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// 在资源包加载完成后以下接口才是线程安全的
public:
	virtual int GetFileAmount(void) = 0;
	virtual const char* GetFileName(int nIndex) = 0;
	virtual IDataFileLoader* GetFileLoader(void) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 打包相关接口[线程不安全][在调用打包接口时不能多线程调用文件加载接口]
	// pszFile为包内的文件名; pszFullPath为未打包的文件在硬盘上的绝对路径
public:
	virtual bool AddFile(const char* pszFile, const char* pszFullPath) = 0;
	virtual bool DelFile(const char* pszFile) = 0;
	virtual bool SavePack(const char* pszRootPath) = 0;
};

TQPACKAGE_API IDataPack* DataPackCreate(void);
//////////////////////////////////////////////////////////////////////////

#endif//__I_TQ_PACKAGE_H__
