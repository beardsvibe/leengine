#pragma once

#include <gb_math.h>
#include <stdint.h>

typedef gbMat4 trns_t;

trns_t tr_persp(float fovy, uint16_t w, uint16_t h, float z_near, float z_far);
trns_t tr_ortho(float left, float right, float bottom, float top, float z_near, float z_far);
trns_t tr_view(float * eye, float * up, float * at);
trns_t tr_identity();
trns_t tr_model_spr(float x, float y, float deg, float sx, float sy, float ox, float oy, float w, float h);
void   tr_debug(trns_t tr);

void tr_set_view_prj(uint8_t viewid, trns_t prj, trns_t view, gbVec2 viewport_pos, gbVec2 viewport_size);
void tr_set_parent_world(trns_t parent_world);
void tr_set_world(trns_t model);

gbVec2 tr_inverted_prj(gbVec2 pos);
