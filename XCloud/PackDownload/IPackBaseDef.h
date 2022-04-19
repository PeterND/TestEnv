#ifndef _C3_IPACK_BASE_DEF_H_
#define _C3_IPACK_BASE_DEF_H_



namespace c3pack_down
{
#if ((defined _WIN32) || (defined WIN32))
	typedef __int64			  int64;
#else
	typedef long long			int64;
	typedef unsigned int   DWORD;
#endif
}

using namespace c3pack_down;
#endif