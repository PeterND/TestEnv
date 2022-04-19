#include "SuperFastHash.h"
#include <stdio.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

// static const block
static const int INCREMENTAL_READ_BLOCK = 1024;

// to do file hash ptr
unsigned int SuperFastHashFilePtr(FILE* pFile);

// file memory incremental hash
unsigned int SuperFastHashIncremental(const void* pszData, int nLength, unsigned int uLastHash);


// memory fast hash 
unsigned int PackSuperHash::SuperFastHash(const char* pszData, int nLen)
{
	int nRemaining = nLen;
	unsigned int uLastHash = nLen;
	int nOffset = 0;

	while (nRemaining >= INCREMENTAL_READ_BLOCK)
	{
		uLastHash = SuperFastHashIncremental(pszData + nOffset, INCREMENTAL_READ_BLOCK, uLastHash);
		nRemaining -= INCREMENTAL_READ_BLOCK;
		nOffset += INCREMENTAL_READ_BLOCK;
	}

	if (nRemaining > 0)
	{
		uLastHash = SuperFastHashIncremental(pszData + nOffset, nRemaining, uLastHash);
	}

	return uLastHash;
}

// to do file fast hash
unsigned int PackSuperHash::SuperFastFileHash(const char* pszFileName)
{
	FILE* pFile = NULL;

#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) && (!defined WINCE)
	errno_t err = fopen_s(&pFile, pszFileName, "rb");
	if (err)
	{
		return NULL;
	}
#else
	pFile = fopen(pszFileName, "rb");
#endif
	if (pFile == NULL)
	{
		return 0;
	}

	unsigned int nHash = SuperFastHashFilePtr(pFile);
	fclose(pFile);
	pFile = NULL;

	return nHash;
}

// to do file hash file ptr
unsigned int SuperFastHashFilePtr(FILE* pFile)
{
	fseek(pFile, 0, SEEK_END);
	int nLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	int nRemaining = nLen;
	unsigned int uLastHash = nLen;
	char szReadBlock[INCREMENTAL_READ_BLOCK];

	while (nRemaining >= (int)sizeof(szReadBlock))
	{
		memset(szReadBlock, 0, sizeof(szReadBlock));
		fread(szReadBlock, sizeof(szReadBlock), 1, pFile);
		uLastHash = SuperFastHashIncremental(szReadBlock, (int)sizeof(szReadBlock), uLastHash);
		nRemaining -= (int)sizeof(szReadBlock);
	}

	if (nRemaining > 0)
	{
		memset(szReadBlock, 0, nRemaining);
		fread(szReadBlock, nRemaining, 1, pFile);
		uLastHash = SuperFastHashIncremental(szReadBlock, nRemaining, uLastHash);
	}

	return uLastHash;
}

// file memory incremental hash
unsigned int SuperFastHashIncremental(const void* pszDataKey, int nLen, unsigned int uLastHash)
{
	int nRem = 0;
	uint32_t uTmp = 0;
	uint32_t uHash = (uint32_t)uLastHash;
	const unsigned char * pszData = (const unsigned char *)pszDataKey;
	if ((nLen <= 0) || (pszData == NULL))
	{
		return 0;
	}

	nRem = nLen & 3;
	nLen >>= 2;

	for (; nLen > 0; nLen--)
	{
		uHash += get16bits(pszData);
		uTmp = (get16bits(pszData + 2) << 11) ^ uHash;
		uHash = (uHash << 16) ^ uTmp;
		pszData += 2 * sizeof(uint16_t);
		uHash += uHash >> 11;
	}

	switch (nRem)
	{
	case 3:
		uHash += get16bits(pszData);
		uHash ^= uHash << 16;
		uHash ^= pszData[sizeof(uint16_t)] << 18;
		uHash += uHash >> 11;
		break;

	case 2:
		uHash += get16bits(pszData);
		uHash ^= uHash << 11;
		uHash += uHash >> 17;
		break;

	case 1:
		uHash += *pszData;
		uHash ^= uHash << 10;
		uHash += uHash >> 1;
		break;
	}

	uHash ^= uHash << 3;
	uHash += uHash >> 5;
	uHash ^= uHash << 4;
	uHash += uHash >> 17;
	uHash ^= uHash << 25;
	uHash += uHash >> 6;

	return (unsigned int)uHash;
}