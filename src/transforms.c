#include "transforms.h"
#include <stdio.h>
#include <bgfx.h>

// all = viewport * proj * view * world
// world = parent_world * model
// model = translation * rotation * scale_origin * scale * negative_origin

static trns_t _tr_muls(trns_t ** muls, size_t count) // small helper to multiply list of matrices
{
	trns_t ret;
	for(size_t i = 0; i < count; ++i)
		if(i)
			ret = tr_mul(ret, *muls[i]);
		else
			ret = *muls[i];
	return ret;
}

trns_t tr_mul(trns_t a, trns_t b) // R = A * B
{
	trns_t r;
	gb_mat4_mul(&r, &a, &b);
	return r;
}

trns_t tr_ortho(float left, float right, float bottom, float top, float z_near, float z_far)
{
	trns_t ret;
	gb_mat4_ortho3d(&ret, left, right, bottom, top, z_near, z_far);
	return ret;
}

trns_t tr_identity()
{
	trns_t ret;
	gb_mat4_identity(&ret);
	return ret;
}

trns_t tr_model_spr(float x, float y,
					float r_deg, float rox, float roy,
					float sx, float sy, float sox, float soy,
					float w, float h, float ox, float oy)
{
	trns_t mpos, mrot, mroto, mroton, mscl, msclo, msclon, mspr, morg;
	gb_mat4_translate(&mpos,	gb_vec3(x, y, 0.0f));
	gb_mat4_from_quat(&mrot,	gb_quat_euler_angles(0.0f, 0.0f, -r_deg * GB_MATH_PI / 180.0f));
	gb_mat4_translate(&mroto,	gb_vec3(rox, roy, 0.0f));
	gb_mat4_translate(&mroton,	gb_vec3(-rox, -roy, 0.0f));
	gb_mat4_scale(&mscl,		gb_vec3(sx, sy, 1.0f));
	gb_mat4_translate(&msclo,	gb_vec3(sox, soy, 0.0f));
	gb_mat4_translate(&msclon,	gb_vec3(-sox, -soy, 0.0f));
	gb_mat4_scale(&mspr,		gb_vec3(w, h, 1.0f));
	gb_mat4_translate(&morg,	gb_vec3(-ox, -oy, 0.0f));

	trns_t * muls[] = {&mpos, &mroto, &mrot, &mroton, &msclo, &mscl, &msclon, &mspr, &morg};
	return _tr_muls(muls, sizeof(muls) / sizeof(muls[0]));
}

void tr_debug(trns_t tr)
{
	// m [j] [i], strange notation, I know
	gbFloat4 * m = gb_float44_m(&tr);
	printf("%2.3f %2.3f %2.3f %2.3f\n", m[0][0], m[1][0], m[2][0], m[3][0]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", m[0][1], m[1][1], m[2][1], m[3][1]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", m[0][2], m[1][2], m[2][2], m[3][2]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", m[0][3], m[1][3], m[2][3], m[3][3]);
}

// -----------------------------------------------------------------------------

struct
{
	// stored as-is
	gbVec2 viewport_pos;
	gbVec2 viewport_size;
	trns_t prj;
	trns_t view;
	trns_t parent_world;
	trns_t model;

	// calculated
	trns_t viewport;
	trns_t world;
	trns_t vpv; // viewport * prj * view
} ctx;

static trns_t _to_bgfx(trns_t t)
{
	// TODO we might need to transpose it, see https://github.com/bkaradzic/bgfx/issues/983
	//gb_mat4_transpose(&t);
	return t;
}

void tr_set_view_prj(uint8_t viewid, trns_t prj, trns_t view, gbVec2 viewport_pos, gbVec2 viewport_size)
{
	ctx.viewport_pos = viewport_pos;
	ctx.viewport_size = viewport_size;
	ctx.prj = prj;
	ctx.view = view;

	// calculate viewport matrix
	trns_t mpos, mscl;
	gb_mat4_translate(&mpos, gb_vec3(viewport_size.x / 2.0f, viewport_size.y / 2.0f, 0.0f));
	gb_mat4_scale(&mscl, gb_vec3(viewport_size.x / 2.0f, -viewport_size.y / 2.0f, 1.0f));
	trns_t * muls[] = {&mpos, &mscl};
	ctx.viewport = _tr_muls(muls, sizeof(muls) / sizeof(muls[0]));

	trns_t * muls2[] = {&ctx.viewport, &ctx.prj, &ctx.view};
	ctx.vpv = _tr_muls(muls2, sizeof(muls2) / sizeof(muls2[0]));

	// set bgfx stuff
	bgfx_set_view_rect(viewid, viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y);
	bgfx_set_view_transform(viewid, _to_bgfx(ctx.view).e, _to_bgfx(ctx.prj).e);
}

void tr_set_parent_world(trns_t parent_world)
{
	ctx.parent_world = parent_world;
}

void tr_set_world(trns_t model)
{
	ctx.model = model;
	ctx.world = tr_mul(ctx.parent_world, ctx.model);
	bgfx_set_transform(_to_bgfx(ctx.world).e, 1);
}

gbVec2 tr_inverted_prj(gbVec2 pos)
{
	// TODO maybe add some caching here?
	trns_t vpvw = tr_mul(ctx.vpv, ctx.world);
	trns_t inv_vpvw;
	gb_mat4_inverse(&inv_vpvw, &vpvw);

	gbVec4 ret;
	gb_mat4_mul_vec4(&ret, &inv_vpvw, gb_vec4(pos.x, pos.y, 0.0f, 1.0f));
	return ret.xy;
}
