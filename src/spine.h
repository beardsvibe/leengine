#pragma once

#include "render.h"

typedef struct spAtlas spAtlas;
typedef struct spSkeleton spSkeleton;
typedef struct spAnimationState spAnimationState;

typedef struct
{
	spAtlas * atlas;
	spSkeleton * skeleton;
	spAnimationState * state;
} spine_t;

spine_t sp_load(const char * json_filename, const char * atlas_filename);
void sp_free(spine_t sp);
void sp_update(spine_t sp, float dt);
void sp_render_ex(spine_t sp, float x, float y, float deg, float sx, float sy);
void sp_render(spine_t sp, float x, float y, float deg);
void sp_set(spine_t sp, const char * anim_name, bool loop);
