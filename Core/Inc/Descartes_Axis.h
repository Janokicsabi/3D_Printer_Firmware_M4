/*
 * Descartes_Axis.h
 *
 *  Created on: Nov 6, 2023
 *      Author: janok
 */

#ifndef DESCARTES_AXIS_H_
#define DESCARTES_AXIS_H_

#include <Axis.h>

typedef enum {
	LIMIT_SWITCH_NULL_POS,			//The limit switch is at the 0 end of the axis
	LIMIT_SWITCH_END_POS			//The limit switch is at the end of the axis
}Limit_switch_pos;

class Descartes_Axis: public Axis {
private:
	Limit_switch* limit_switch;
	const uint8_t limit_switch_dir;
	Limit_switch_pos limit_switch_pos;
	const float workspace_frame_offset;			//Distance from the limit switch position to the printing area 0 position
	const float axis_length;					//The distance from motor possible endpoint to other in millimeter
	float home_pos_in_ws_frame;

public:
	Descartes_Axis(Stepper* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_length,
			const float full_rotation_displacement, const uint8_t limit_switch_dir, Limit_switch_pos limit_switch_pos, float max_acc, float max_speed, float max_jerk);
	virtual ~Descartes_Axis();

	void set_home_position();
	float saturate_position(float new_pos);
	uint8_t get_limit_switch_dir();
	Limit_switch* get_limit_switch();
	bool can_motor_move(uint8_t move_dir);
};

#endif /* DESCARTES_AXIS_H_ */
