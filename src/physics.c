
#include "physics.h"
#include "physics_debug.h"
#include <chipmunk/chipmunk.h>
#include <chipmunk/cpHastySpace.h>
#include <chipmunk/chipmunk_unsafe.h>

static void _shape_free_wrap(cpSpace * space, cpShape * shape, void * unused) {cpSpaceRemoveShape(space, shape); cpShapeFree(shape);}
static void _shape_free(cpShape * shape, cpSpace * space) {cpSpaceAddPostStepCallback(space, (cpPostStepFunc)_shape_free_wrap, shape, NULL);}
static void _body_shape_free(cpBody * body, cpShape * shape, cpSpace * space) {cpSpaceAddPostStepCallback(space, (cpPostStepFunc)_shape_free_wrap, shape, NULL);}
static void _constraint_free_wrap(cpSpace * space, cpConstraint * constraint, void * unused) {cpSpaceRemoveConstraint(space, constraint); cpConstraintFree(constraint);}
static void _constraint_free(cpConstraint * constraint, cpSpace * space) {cpSpaceAddPostStepCallback(space, (cpPostStepFunc)_constraint_free_wrap, constraint, NULL);}
static void _body_free_wrap(cpSpace * space, cpBody * body, void * unused) {cpSpaceRemoveBody(space, body); cpBodyFree(body);}
static void _body_free(cpBody * body, cpSpace * space) {cpSpaceAddPostStepCallback(space, (cpPostStepFunc)_body_free_wrap, body, NULL);}

static struct
{
	cpSpace * space;
	float scale;
} ctx;

void _p_init()
{
	ctx.space = cpSpaceNew();
	p_set_scale(1.0f);
	//cpSpaceSetIterations(ctx.space, 30);
}

void _p_deinit()
{
	cpSpaceEachShape(ctx.space, _shape_free, ctx.space);
	cpSpaceEachConstraint(ctx.space, _constraint_free, ctx.space);
	cpSpaceEachBody(ctx.space, _body_free, ctx.space);
	cpSpaceFree(ctx.space);
}

void _p_update(double dt)
{
	uint8_t k = 4;
	for(size_t i = 0; i < k; ++i)
		cpSpaceStep(ctx.space, dt / (float)k);
}

void _p_debug_render()
{
	cpSpaceDebugDraw(ctx.space, p_debug_opt());
	p_debug_flush(ctx.scale);
}

cpSpace * p_space() {return ctx.space;}

void p_set_scale(float scale)
{
	ctx.scale = scale;
	cpSpaceSetGravity(ctx.space, cpv(0, -980.0f / ctx.scale));
	cpSpaceSetSleepTimeThreshold(ctx.space, 1.0f);
	cpSpaceSetCollisionSlop(ctx.space, 0.05f / ctx.scale);
}

float p_scale() {return ctx.scale;}

void p_remove_body(cpBody * body)
{
	cpBodyEachShape(body, _body_shape_free, ctx.space);
	_body_free(body, ctx.space);
}
