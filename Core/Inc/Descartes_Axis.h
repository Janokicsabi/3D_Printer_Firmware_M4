/*
 * Descartes_Axis.h
 *
 *  Created on: Nov 6, 2023
 *      Author: janok
 */

#ifndef DESCARTES_AXIS_H_
#define DESCARTES_AXIS_H_

#include <Axis.h>

class Descartes_Axis: public Axis {
private:
	Limit_switch* limit_switch;
	const uint8_t limit_switch_dir;
	const float workspace_frame_offset;			//Distance from the limit switch position to the printing area 0 position
	const float axis_length;					//The distance from motor possible endpoint to other in millimeter

public:
	Descartes_Axis(Motor* motor, Limit_switch* limit_switch, const float workspace_frame_offset, const float axis_endpoint,
			const float full_rotation_displacement, const uint8_t limit_switch_dir);
	virtual ~Descartes_Axis();

	float saturate_position(float new_pos);
	Limit_switch* get_limit_switch();
	void home_axis(float move_speed);
};

#endif /* DESCARTES_AXIS_H_ */
