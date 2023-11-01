/*
 * Instruction_control.h
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#ifndef SRC_INSTRUCTION_CONTROL_H_
#define SRC_INSTRUCTION_CONTROL_H_


#define INSTRUCTION_DEPTH	100

class Instruction_control {
public:
	Instruction_control();
	virtual ~Instruction_control();

	void update_instruction_parameters();
};

#endif /* SRC_INSTRUCTION_CONTROL_H_ */
