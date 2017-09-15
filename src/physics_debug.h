#if 0

#pragma once

#include <chipmunk/chipmunk.h>
#include "render.h"

cpSpaceDebugDrawOptions * p_debug_opt();
void p_debug_flush(float scale);

// needed for chipmunk collision debug
#if 0
void ChipmunkDebugDrawCircle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawSegment(cpVect a, cpVect b, cpSpaceDebugColor color);
void ChipmunkDebugDrawFatSegment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawPolygon(int count, const cpVect * verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor);
void ChipmunkDebugDrawDot(cpFloat size, cpVect pos, cpSpaceDebugColor fillColor);
static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a) {cpSpaceDebugColor color = {r, g, b, a}; return color;}
static inline cpSpaceDebugColor LAColor(float l, float a) {cpSpaceDebugColor color = {l, l, l, a}; return color;}
#endif

#endif
