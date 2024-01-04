/*
 * Descartes_Axis.cpp
 *
 *  Created on: Nov 6, 2023
 *      Author: janok
 */

#include "Descartes_Axis.h"

Descartes_Axis::Descartes_Axis(Stepper* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_length,
		const float full_rotation_displacement, const uint8_t limit_switch_dir, Limit_switch_pos limit_switch_pos, float max_acc, float max_speed, float max_jerk) :
		Axis(motor, full_rotation_displacement, max_acc, max_speed, max_jerk), limit_switch_dir {limit_switch_dir},
		workspace_frame_offset {workspace_frame_offset}, axis_length {axis_length}, limit_switch {limit_switch}, limit_switch_pos{limit_switch_pos} {
			if (this->limit_switch_pos == LIMIT_SWITCH_NULL_POS) {
				this->home_pos_in_ws_frame = 0.0f - workspace_frame_offset;

			} else {
				this->home_pos_in_ws_frame = axis_length - workspace_frame_offset;
			}
			this->pos_in_ws_frame = home_pos_in_ws_frame;
}

Descartes_Axis::~Descartes_Axis() {

}

float Descartes_Axis::saturate_position(float new_pos) {
	uint8_t dir = calculate_dir(new_pos);
	if (dir != limit_switch_dir) {
		if (new_pos > (axis_length - workspace_frame_offset)) {
			new_pos = (axis_length - workspace_frame_offset);
		} else if (new_pos < (0.0 - workspace_frame_offset)) {
			new_pos = (0.0 - workspace_frame_offset);
		}
	}
	return new_pos;
}

void Descartes_Axis::set_home_position() {
	this->pos_in_ws_frame = this->home_pos_in_ws_frame;
}

Limit_switch* Descartes_Axis::get_limit_switch() {
	return this->limit_switch;
}

uint8_t Descartes_Axis::get_limit_switch_dir() {
	return this->limit_switch_dir;
}

bool Descartes_Axis::can_motor_move(uint8_t move_dir) {
	if (this->limit_switch->is_switch_pressed() && move_dir == this->limit_switch_dir) {
		return false;
	}
	return true;
}

