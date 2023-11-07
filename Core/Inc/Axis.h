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
protected:
	Motor* motor;
	float pos_in_ws_frame;						//The position reladtive to workspace frame, 0 position is at the edge of the printing area
	float displacement_per_microstep;			//Displacement in mm for one microstep

public:
	Axis(Motor* motor, const float full_rotation_displacement);
	virtual ~Axis();

	const uint32_t calculate_step_num(float new_pos);
	const uint8_t calculate_dir(float new_position);
	void control_axis(float move_speed, float new_pos, bool is_feedrate_const);

	virtual float saturate_position(float new_pos);
	virtual Limit_switch* get_limit_switch();

	const float get_axis_pos();
	Motor* get_motor();
	const float get_one_step_displacement();
	const bool is_position_changed(float new_pos);
	void update_position(float new_pos);
};

#endif /* SRC_AXIS_H_ */
