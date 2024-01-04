/*
 * G_code_reader.h
 *
 *  Created on: 2023. okt. 16.
 *      Author: janok
 */

#ifndef G_CODE_READER_H_
#define G_CODE_READER_H_

#include <string>
using namespace std;

#include "sd_card.h"
#include "task_creator.h"
#include "Stepper.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"


void task_g_code_interpreter(void* sd_card);


#endif /* SRC_G_CODE_COMMAND_H_ */
