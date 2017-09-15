#define ENTRYPOINT_CTX
#include "entrypoint.h"
#include "filesystem.h"

#if defined(__APPLE__) && TARGET_OS_IOS

#include <UIKit/UIKit.h>

FILE * fsopen(const char * filename, const char * mode)
{
	char buf[1024 * 2] = {0};
	_fs_path(filename, buf, sizeof(buf));
	return fopen(buf, mode);
}

void _fs_path(const char * filename, char * buf, size_t size)
{
	NSString * fn = [NSString stringWithUTF8String:filename];
	NSString * fn_path = [fn stringByDeletingLastPathComponent];
	NSString * fn_name = [fn lastPathComponent];
	NSString * fn_root = [[NSBundle mainBundle] pathForResource:fn_name ofType:nil inDirectory:fn_path];
	strlcpy(buf, [fn_root UTF8String], size);
}

FILE * fsopen_gamesave(const char * filename, const char * mode)
{
	NSArray * lib_paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	if(!lib_paths || [lib_paths count] == 0)
		return NULL;

	char buf[1024 * 2] = {0};
	snprintf(buf, sizeof(buf), "%s/%s", [[lib_paths objectAtIndex:0] UTF8String], filename);
	return fopen(buf, mode);
}

#endif
