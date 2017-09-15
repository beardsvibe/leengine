
#ifdef __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_OS_IOS
		#define STBI_NEON
	#endif
#endif

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define STBI_WRITE_NO_STDIO // don't use this one
//#include <stb_image_write.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
