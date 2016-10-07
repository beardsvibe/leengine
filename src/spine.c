
#include "spine.h"
#include <stdio.h>
#include <assert.h>
#include <spine/spine.h>
#include <spine/extension.h>

#ifndef SPINE_MESH_VERTEX_COUNT_MAX
#define SPINE_MESH_VERTEX_COUNT_MAX 2048
#endif

#ifdef _MSC_VER
#define alloca _alloca
#endif

void _spAtlasPage_createTexture(spAtlasPage * self, const char * path)
{
	tex_t spr = r_load(path, TEX_FLAGS_NONE
			| (self->magFilter == SP_ATLAS_NEAREST ? TEX_FLAGS_POINT : 0)
			| (self->uWrap == SP_ATLAS_REPEAT && self->vWrap == SP_ATLAS_REPEAT ? TEX_FLAGS_REPEAT : 0)
			);

	tex_t * wut = malloc(sizeof(tex_t)); // TODO fix dat shit
	*wut = spr;
	self->rendererObject = wut;
}

void _spAtlasPage_disposeTexture(spAtlasPage * self)
{
	tex_t * wut = self->rendererObject;
	r_free(*wut);
	free(wut);
}

char * _spUtil_readFile(const char * path, int * length)
{
	return _readFile(path, length); // TODO
}

spine_t sp_load(const char * json_filename, const char * atlas_filename)
{
	spine_t ret = {0};

	const char * atlas_name = atlas_filename;
	const char * json_name = json_filename;
	ret.atlas = spAtlas_createFromFile(atlas_name, 0);

	spSkeletonJson * json = spSkeletonJson_create(ret.atlas);
	json->scale = 1.0f;

	spSkeletonData * skeletonData = spSkeletonJson_readSkeletonDataFile(json, json_name);

	if(!skeletonData)
	{
		fprintf(stderr, "failed to load spine model %s with error %s\n", json_filename, json->error);
		spSkeletonJson_dispose(json);
		spAtlas_dispose(ret.atlas);
		ret.atlas = NULL;
		return ret;
	}
	else
	{
		spSkeletonJson_dispose(json);
	}

	ret.skeleton = spSkeleton_create(skeletonData);
	ret.state = spAnimationState_create(spAnimationStateData_create(skeletonData));

	return ret;
}

void sp_free(spine_t sp)
{
	if(!sp.skeleton)
		return;

	spAnimationStateData_dispose(sp.state->data);
	spAnimationState_dispose(sp.state);
	spSkeletonData_dispose(sp.skeleton->data);
	spSkeleton_dispose(sp.skeleton);
	spAtlas_dispose(sp.atlas);
}

void sp_update(spine_t sp, float dt)
{
	if(!sp.skeleton)
		return;

	float timeScale = 1.0f;

	spSkeleton_update(sp.skeleton, dt);
	spAnimationState_update(sp.state, dt * timeScale);
	spAnimationState_apply(sp.state, sp.skeleton);
	spSkeleton_updateWorldTransform(sp.skeleton);
}

void sp_render_ex(spine_t sp, float x, float y, float deg, float sx, float sy)
{
	if(!sp.skeleton)
		return;

	float pos[3] = {x, y, 0.0f};
	float rot[3] = {0.0f, 0.0f, -deg};
	float scl[3] = {sx, sy, 1.0f};
	trns_t model = tr_model(pos, rot, scl);

	for (int i = 0; i < sp.skeleton->slotsCount; ++i) {
		spSlot * slot = sp.skeleton->drawOrder[i];
		spAttachment * attachment = slot->attachment;
		if(!attachment)
			continue;

		r_color_t color = r_to_color(sp.skeleton->r * slot->r * 255, sp.skeleton->g * slot->g * 255, sp.skeleton->b * slot->b * 255, sp.skeleton->a * slot->a * 255);

		uint64_t blend =
				 (slot->data->blendMode == SP_BLEND_MODE_ADDITIVE) ? BGFX_STATE_BLEND_ADD :
				((slot->data->blendMode == SP_BLEND_MODE_MULTIPLY) ? BGFX_STATE_BLEND_MULTIPLY : BGFX_STATE_BLEND_ALPHA);

		// TODO optimize rendering
		if(attachment->type == SP_ATTACHMENT_REGION)
		{
			float worldVertices[8];
			spRegionAttachment * regionAttachment = (spRegionAttachment*)attachment;
			tex_t * texture = (tex_t*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;
			spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVertices);

			bgfx_transient_vertex_buffer_t vt;
			bgfx_alloc_transient_vertex_buffer(&vt, 4, r_decl());
			vrtx_t * vert = (vrtx_t*)vt.data;
			#define _VERT(_i) \
			{ \
				vert[_i - 1].x = worldVertices[SP_VERTEX_X ## _i]; \
				vert[_i - 1].y = worldVertices[SP_VERTEX_Y ## _i]; \
				vert[_i - 1].z = 0.0f; \
				vert[_i - 1].u = regionAttachment->uvs[SP_VERTEX_X ## _i]; \
				vert[_i - 1].v = regionAttachment->uvs[SP_VERTEX_Y ## _i]; \
				vert[_i - 1].color = color; \
			}
			_VERT(1);
			_VERT(2);
			_VERT(3);
			_VERT(4);
			#undef _VERT

			bgfx_transient_index_buffer_t it;
			bgfx_alloc_transient_index_buffer(&it, 6);
			uint16_t * id = (uint16_t*)it.data;
			id[0] = 0; id[1] = 1; id[2] = 2;
			id[3] = 0; id[4] = 2; id[5] = 3;

			tr_set_world(model);
			if(texture)
				bgfx_set_texture(0, r_u_tex(), texture->tex, -1);
			bgfx_set_transient_index_buffer(&it, 0, -1);
			bgfx_set_transient_vertex_buffer(&vt, 0, -1);
			bgfx_set_state(BGFX_STATE_DEFAULT_2D | blend, 0);
			bgfx_submit(0, r_prog(), 0, false);

		}
		else if(attachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* mesh = (spMeshAttachment*)attachment;
			if(mesh->super.worldVerticesLength > SPINE_MESH_VERTEX_COUNT_MAX)
			{
				fprintf(stderr, "too many verteces in the spine model: %i\n", mesh->super.worldVerticesLength);
				continue;
			}
			float * worldVertices = alloca(sizeof(float) * SPINE_MESH_VERTEX_COUNT_MAX);
			tex_t * texture = (tex_t*)((spAtlasRegion*)mesh->rendererObject)->page->rendererObject;
			spMeshAttachment_computeWorldVertices(mesh, slot, worldVertices);

			assert(mesh->super.worldVerticesLength % 2 == 0);

			bgfx_transient_vertex_buffer_t vt;
			bgfx_alloc_transient_vertex_buffer(&vt, mesh->super.worldVerticesLength / 2, r_decl());
			vrtx_t * vert = (vrtx_t*)vt.data;
			#define _VERT(_i) \
			{ \
				vert[_i].x = worldVertices[(_i) * 2 + 0]; \
				vert[_i].y = worldVertices[(_i) * 2 + 1]; \
				vert[_i].z = 0.0f; \
				vert[_i].u = mesh->uvs[(_i) * 2 + 0]; \
				vert[_i].v = mesh->uvs[(_i) * 2 + 1]; \
				vert[_i].color = color; \
			}
			for(size_t i = 0; i < mesh->super.worldVerticesLength / 2; ++i)
			{
				_VERT(i);
			}
			#undef _VERT

			bgfx_transient_index_buffer_t it;
			bgfx_alloc_transient_index_buffer(&it, mesh->trianglesCount);
			uint16_t * id = (uint16_t*)it.data;
			for(size_t i = 0; i < mesh->trianglesCount; ++i)
				id[i] = mesh->triangles[i];

			tr_set_world(model);
			if(texture)
				bgfx_set_texture(0, r_u_tex(), texture->tex, -1);
			bgfx_set_transient_index_buffer(&it, 0, -1);
			bgfx_set_transient_vertex_buffer(&vt, 0, -1);
			bgfx_set_state(BGFX_STATE_DEFAULT_2D | blend, 0);
			bgfx_submit(0, r_prog(), 0, false);
		}
	}
}

void sp_render(spine_t sp, float x, float y, float deg)
{
	sp_render_ex(sp, x, y, deg, 1.0f, 1.0f);
}

void sp_set(spine_t sp, const char * anim_name, bool loop)
{
	if(!sp.skeleton)
		return;

	spAnimation * animation = spSkeletonData_findAnimation(sp.state->data->skeletonData, anim_name);
	if(!animation)
		return;

	spAnimationState_setAnimation(sp.state, 0, animation, loop);
}
