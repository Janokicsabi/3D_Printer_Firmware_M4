/*
 * Axis.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: janok
 */

#include "main.h"

#include <Stepper.h>
#include <Limit_switch.h>

#ifndef SRC_AXIS_H_
#define SRC_AXIS_H_

class Axis {
protected:
	Stepper* motor;
	float pos_in_ws_frame;						//The position reladtive to workspace frame, 0 position is at the edge of the printing area
	float displacement_per_microstep;			//Displacement in mm for one microstep
	float max_acc;								//Maximum acceleration in [mm/s^2]
	float max_speed;							//Max speed in [mm/min]
	float max_jerk;								//Max instantenous speed change [mm/min]

public:
	Axis(Stepper* motor, const float full_rotation_displacement, float max_acc, float max_speed, float max_jerk);
	virtual ~Axis();

	const uint32_t calculate_step_num(float new_pos);
	const uint8_t calculate_dir(float new_position);
	void control_axis_pwm(float move_speed, float new_pos, bool is_feedrate_const);

	float time_to_reach_speed_max_accel(float desired_speed, float distance_to_move);
	float acceleration_for_time_and_distance(float acc_time, float distance_to_move);
	float convert_feedrate_to_mm_p_s(float feedrate_mm_p_min);

	virtual float saturate_position(float new_pos);
	virtual Limit_switch* get_limit_switch();

	const float get_axis_pos();
	Stepper* get_motor();
	const float get_one_step_displacement();
	float get_max_accel();
	float get_max_speed();
	float get_max_jerk();
	const bool is_position_changed(float new_pos);
	void update_position(float new_pos);
	virtual void set_home_position();
};

#endif /* SRC_AXIS_H_ */
