#include "Utility_pack.h"
#import <Foundation/Foundation.h>


namespace c3pack_down
{

	bool MyDeleteFile(const char *pszDirectory)
	{
		if (pszDirectory)
		{
			NSString* strDirPath =[NSString stringWithFormat:@"%s",pszDirectory];
			NSFileManager* filemanager = [NSFileManager defaultManager];
			return [filemanager removeItemAtPath:strDirPath error:nil];
		}
		return false;
	}

}
