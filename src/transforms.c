#include "transforms.h"
#include <stdio.h>
#include <bgfx.h>

// small helper to multiply list of matrices
static trns_t _tr_muls(trns_t ** muls, size_t count)
{
	trns_t a = tr_identity(), b;
	trns_t * ret = &a, * temp = &b;
	for(size_t i = 0; i < count; ++i)
	{
		gb_mat4_mul(temp, ret, muls[i]);
		trns_t * t = ret;
		ret = temp;
		temp = t;
	}
	return *ret;
}

trns_t tr_persp(float fovy, uint16_t w, uint16_t h, float z_near, float z_far)
{
	trns_t ret;
	gb_mat4_perspective(&ret, fovy, (float)w/ (float)h, z_near, z_far);
	return ret;
}

trns_t tr_ortho(float left, float right, float bottom, float top, float z_near, float z_far)
{
	trns_t ret;
	gb_mat4_ortho3d(&ret, left, right, bottom, top, z_near, z_far);
	return ret;
}

trns_t tr_view(float * eye, float * up, float * at)
{
	trns_t ret;
	gbVec3 veye = {eye[0], eye[1], eye[2]};
	gbVec3 vctr = {at[0], at[1], at[2]};
	gbVec3 vup  = {up[0], up[1], up[2]};
	gb_mat4_look_at(&ret, veye, vctr, vup);
	return ret;
}

trns_t tr_identity()
{
	trns_t ret;
	gb_mat4_identity(&ret);
	return ret;
}

trns_t tr_model_spr(float x, float y, float deg, float sx, float sy, float ox, float oy, float w, float h)
{
	trns_t mpos, mrot, mscl, morg, morgn, mspr;
	gb_mat4_translate(&mpos, gb_vec3(x, y, 0.0f));
	gb_mat4_from_quat(&mrot, gb_quat_euler_angles(0.0f, 0.0f, -deg * GB_MATH_PI / 180.0f));
	gb_mat4_scale(&mscl, gb_vec3(sx, sy, 1.0f));
	gb_mat4_translate(&morg, gb_vec3(-ox, -oy, 0.0f));
	gb_mat4_translate(&morgn, gb_vec3(ox, oy, 0.0f));
	gb_mat4_scale(&mspr, gb_vec3(w, h, 1.0f));

	trns_t * muls[] = {&mpos, &morgn, &mrot, &mscl, &morg, &mspr};
	return _tr_muls(muls, sizeof(muls) / sizeof(muls[0]));
}

void tr_debug(trns_t tr)
{
	// 0 4 8 c
	// 1 5 9 d
	// 2 6 a e
	// 3 7 b f
	printf("%2.3f %2.3f %2.3f %2.3f\n", tr.e[0x0], tr.e[0x4], tr.e[0x8], tr.e[0xc]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", tr.e[0x1], tr.e[0x5], tr.e[0x9], tr.e[0xd]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", tr.e[0x2], tr.e[0x6], tr.e[0xa], tr.e[0xe]);
	printf("%2.3f %2.3f %2.3f %2.3f\n", tr.e[0x3], tr.e[0x7], tr.e[0xb], tr.e[0xf]);
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
	bgfx_set_view_transform(viewid, view.e, prj.e);
}

void tr_set_parent_world(trns_t parent_world)
{
	ctx.parent_world = parent_world;
}

void tr_set_world(trns_t model)
{
	ctx.model = model;
	gb_mat4_mul(&ctx.world, &ctx.parent_world, &ctx.model);
	bgfx_set_transform(ctx.world.e, 1);
}

// wait for https://github.com/gingerBill/gb/issues/9
bool gb_mat4_inverse(gbMat4 * out, gbMat4 * in)
{
	#define _M(__i, __j) in->col[__j].e[__i]
	#define _MO(__i, __j) out->col[__j].e[__i]
	#define _M3(a0, a1, a2, a3, a4, a5, a6, a7, a8) \
				( a0 * (a4 * a8 - a5 * a7) \
				- a1 * (a3 * a8 - a5 * a6) \
				+ a2 * (a3 * a7 - a4 * a6) ) * d

	float ka = _M(2, 2) * _M(3, 3) - _M(2, 3) * _M(3, 2);
	float kb = _M(2, 1) * _M(3, 3) - _M(2, 3) * _M(3, 1);
	float kc = _M(2, 1) * _M(3, 2) - _M(2, 2) * _M(3, 1);
	float kd = _M(2, 0) * _M(3, 3) - _M(2, 3) * _M(3, 0);
	float ke = _M(2, 0) * _M(3, 2) - _M(2, 2) * _M(3, 0);
	float kf = _M(2, 0) * _M(3, 1) - _M(2, 1) * _M(3, 0);

	float d = _M(0, 0) * (ka * _M(1, 1) - kb * _M(1, 2) + kc * _M(1, 3))
			- _M(0, 1) * (ka * _M(1, 0) - kd * _M(1, 2) + ke * _M(1, 3))
			+ _M(0, 2) * (kb * _M(1, 0) - kd * _M(1, 1) + kf * _M(1, 3))
			- _M(0, 3) * (kc * _M(1, 0) - ke * _M(1, 1) + kf * _M(1, 2));

	if(fabs(d) <= 0.0000000001f) // find a good epsilon
	{
		gb_mat4_identity(out);
		return false;
	}

	d = 1.0f / d;

	_MO(0, 0) =  _M3(_M(1, 1), _M(1, 2), _M(1, 3), _M(2, 1), _M(2, 2), _M(2, 3), _M(3, 1), _M(3, 2), _M(3, 3));
	_MO(0, 1) = -_M3(_M(0, 1), _M(0, 2), _M(0, 3), _M(2, 1), _M(2, 2), _M(2, 3), _M(3, 1), _M(3, 2), _M(3, 3));
	_MO(0, 2) =  _M3(_M(0, 1), _M(0, 2), _M(0, 3), _M(1, 1), _M(1, 2), _M(1, 3), _M(3, 1), _M(3, 2), _M(3, 3));
	_MO(0, 3) = -_M3(_M(0, 1), _M(0, 2), _M(0, 3), _M(1, 1), _M(1, 2), _M(1, 3), _M(2, 1), _M(2, 2), _M(2, 3));
	_MO(1, 0) = -_M3(_M(1, 0), _M(1, 2), _M(1, 3), _M(2, 0), _M(2, 2), _M(2, 3), _M(3, 0), _M(3, 2), _M(3, 3));
	_MO(1, 1) =  _M3(_M(0, 0), _M(0, 2), _M(0, 3), _M(2, 0), _M(2, 2), _M(2, 3), _M(3, 0), _M(3, 2), _M(3, 3));
	_MO(1, 2) = -_M3(_M(0, 0), _M(0, 2), _M(0, 3), _M(1, 0), _M(1, 2), _M(1, 3), _M(3, 0), _M(3, 2), _M(3, 3));
	_MO(1, 3) =  _M3(_M(0, 0), _M(0, 2), _M(0, 3), _M(1, 0), _M(1, 2), _M(1, 3), _M(2, 0), _M(2, 2), _M(2, 3));
	_MO(2, 0) =  _M3(_M(1, 0), _M(1, 1), _M(1, 3), _M(2, 0), _M(2, 1), _M(2, 3), _M(3, 0), _M(3, 1), _M(3, 3));
	_MO(2, 1) = -_M3(_M(0, 0), _M(0, 1), _M(0, 3), _M(2, 0), _M(2, 1), _M(2, 3), _M(3, 0), _M(3, 1), _M(3, 3));
	_MO(2, 2) =  _M3(_M(0, 0), _M(0, 1), _M(0, 3), _M(1, 0), _M(1, 1), _M(1, 3), _M(3, 0), _M(3, 1), _M(3, 3));
	_MO(2, 3) = -_M3(_M(0, 0), _M(0, 1), _M(0, 3), _M(1, 0), _M(1, 1), _M(1, 3), _M(2, 0), _M(2, 1), _M(2, 3));
	_MO(3, 0) = -_M3(_M(1, 0), _M(1, 1), _M(1, 2), _M(2, 0), _M(2, 1), _M(2, 2), _M(3, 0), _M(3, 1), _M(3, 2));
	_MO(3, 1) =  _M3(_M(0, 0), _M(0, 1), _M(0, 2), _M(2, 0), _M(2, 1), _M(2, 2), _M(3, 0), _M(3, 1), _M(3, 2));
	_MO(3, 2) = -_M3(_M(0, 0), _M(0, 1), _M(0, 2), _M(1, 0), _M(1, 1), _M(1, 2), _M(3, 0), _M(3, 1), _M(3, 2));
	_MO(3, 3) =  _M3(_M(0, 0), _M(0, 1), _M(0, 2), _M(1, 0), _M(1, 1), _M(1, 2), _M(2, 0), _M(2, 1), _M(2, 2));

	#undef _M3
	#undef _M
	#undef _MO

	return true;
}

// wait for https://github.com/gingerBill/gb/issues/11
void gb_float44_mul_vec4_fixed(gbVec4 *out, float m[4][4], gbVec4 v)
{
	out->x = m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w;
	out->y = m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w;
	out->z = m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w;
	out->w = m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w;
}
void gb_mat4_mul_vec4_fixed(gbVec4 *out, gbMat4 *mi, gbVec4 in)
{
	gb_float44_mul_vec4_fixed(out, gb_float44_m(mi), in);
}

gbVec2 tr_inverted_prj(gbVec2 pos)
{
	// TODO maybe add some caching here?
	trns_t * muls[] = {&ctx.vpv, &ctx.world};
	trns_t mvp = _tr_muls(muls, sizeof(muls) / sizeof(muls[0]));
	trns_t inv;
	gb_mat4_inverse(&inv, &mvp);

	gbVec4 ret;
	gb_mat4_mul_vec4_fixed(&ret, &inv, gb_vec4(pos.x, pos.y, 0.0f, 1.0f));
	return ret.xy;
}
