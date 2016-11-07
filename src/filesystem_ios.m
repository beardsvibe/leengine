#define ENTRYPOINT_CTX
#include "entrypoint.h"
#include "filesystem.h"

#if defined(__APPLE__) && TARGET_OS_IOS

#include <UIKit/UIKit.h>

FILE * fsopen(const char * filename, const char * mode)
{
	NSString * fn = [NSString stringWithUTF8String:filename];
	NSString * fn_path = [fn stringByDeletingLastPathComponent];
	NSString * fn_name = [fn lastPathComponent];
	NSString * fn_root = [[NSBundle mainBundle] pathForResource:fn_name ofType:nil inDirectory:fn_path];
	return fopen([fn_root UTF8String], mode);
}

#endif
