#ifndef _SUPER_FAST_HASH_H_
#define _SUPER_FAST_HASH_H_

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
	|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
typedef unsigned char uint8_t;
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
	+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

namespace PackSuperHash { 

// file fast hash
unsigned int SuperFastFileHash(const char* pszFileName);

// memory fast hash 
unsigned int SuperFastHash(const char* pszData, int nLength);

}

#endif