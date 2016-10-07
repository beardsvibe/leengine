#include "transforms.h"
#include <stdio.h>
#include <bgfx.h>

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

trns_t tr_model(float * pos, float * rot, float * scl)
{
	trns_t ret, rett, mpos, mrot, mscl;
	gbVec3 vpos = {pos[0], pos[1], pos[2]};
	gbQuat qrot = gb_quat_euler_angles(rot[0] * GB_MATH_PI / 180.0f, rot[1] * GB_MATH_PI / 180.0f, rot[2] * GB_MATH_PI / 180.0f);
	gbVec3 vscl = {scl[0], scl[1], scl[2]};
	gb_mat4_translate(&mpos, vpos);
	gb_mat4_from_quat(&mrot, qrot);
	gb_mat4_scale(&mscl, vscl);

	gb_mat4_mul(&rett, &mpos, &mrot);
	gb_mat4_mul(&ret, &rett, &mscl);
	//gb_mat4_transpose(&mpos);
	return ret;
}

trns_t tr_world(trns_t parent_world, trns_t model)
{
	trns_t ret;
	gb_mat4_mul(&ret, &parent_world, &model);
	return ret;
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

void tr_set_mats(uint8_t viewid, trns_t prj, trns_t view)
{
	bgfx_set_view_transform(viewid, view.e, prj.e);
}

void tr_set_world(trns_t world)
{
	bgfx_set_transform(world.e, 1);
}
