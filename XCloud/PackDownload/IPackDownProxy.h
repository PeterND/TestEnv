#pragma once



class IPackDownProxy
{
public:
	virtual void Release(void) = 0;
	virtual bool Init(void) = 0;
	virtual void VersionVertify(void) = 0;
	virtual int GetVertifyStatus(void) = 0;

	virtual bool SendRequest(const char* pszPack) = 0;
	virtual bool ProcessResult(void) = 0;
	virtual const char* GetLastError(void) = 0;
};

enum
{
	DS_VERTIFY_NONE,
	DS_VERTIFY_SUC,
	DS_VERTIFY_ING_VERSION,
	DS_VERTIFY_ING_PACK_INFO,
	DS_VERTIFY_FAIL_VERSION,
	DS_VERTIFY_FAIL_DOWN_PACK_INFO,
	DS_VERTIFY_FAIL_LOAD_PACK_INFO,
};