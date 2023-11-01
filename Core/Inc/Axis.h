/*
 * Axis.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "main.h"

#include <Motor.h>
#include <Limit_switch.h>

#ifndef SRC_AXIS_H_
#define SRC_AXIS_H_

class Axis {
private:
	char axis_name;
	Motor* motor;
	Limit_switch* limit_switch;

	float pos_in_ws_frame;						//The position reladtive to workspace frame, 0 position is at the edge of the printing area
	const float workspace_frame_offset;			//Distance from the limit switch position to the printing area 0 position
	const float axis_endpoint_in_ws_frame;		//The maximum position on the axis

	const float axis_pitch;
	const float pitch_num_in_one_rotation;
	const float displacement_per_microstep;		//Displacement in mm for one microstep

	const uint8_t limit_switch_dir;

public:
	Axis(Motor* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_endpoint,
			const float axis_pitch, const float pitch_num_in_one_rotation, const uint8_t limit_switch_dir);
	virtual ~Axis();

	const uint32_t calculate_step_num(float new_pos);
	const uint8_t calculate_dir(float new_position);
	float saturate_position(float new_pos);
	void control_axis(float move_speed, float new_pos, bool is_feedrate_const);
	void home_axis(float move_speed);

	const char get_axis_name();
	const float get_axis_pos();
	Motor* get_motor();
	const float get_one_step_displacement();
	const bool is_position_changed(float new_pos);
	void update_position(float new_pos);
};

#endif /* SRC_AXIS_H_ */
