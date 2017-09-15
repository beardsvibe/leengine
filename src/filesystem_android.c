
#if defined(__ANDROID__)

#define ENTRYPOINT_CTX
#include "entrypoint.h"

#include "filesystem.h"
#include "portable.h"

#include <android/asset_manager_jni.h>

static int asset_read(void * asset, char * buf, int size) {return asset ? AAsset_read((AAsset*)asset, buf, size) : 0;}
static fpos_t asset_seek(void * asset, fpos_t pos, int dir) {return asset ? AAsset_seek((AAsset*)asset, pos, dir) : 0;}
static int asset_close(void * asset) {if(asset) {AAsset_close((AAsset*)asset); return 0;} return -1;}

FILE * fsopen(const char * filename, const char * mode)
{
	// TODO remove this hack
	if(memcmp(filename, "res/", 4) == 0)
		filename = filename + 4;
	else
		return NULL;

	if(!ep_ctx()->app || !ep_ctx()->app->activity || !ep_ctx()->app->activity->assetManager)
		return NULL;

	AAsset * asset = AAssetManager_open(ep_ctx()->app->activity->assetManager, filename, AASSET_MODE_UNKNOWN);
	if(asset)
		return funopen(asset, asset_read, NULL, asset_seek, asset_close);
	else
		return NULL;
}

void _fs_path(const char * filename, char * buf, size_t size)
{
	// the way to load fmod files on android
	// TODO remove this hack
	if(memcmp(filename, "res/", 4) == 0)
		filename = filename + 4;
	snprintf(buf, size, "file:///android_asset/%s", filename);
}

FILE * fsopen_gamesave(const char * filename, const char * mode)
{
	return NULL;
}

#endif
