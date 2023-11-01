/*
 * thermistor.h
 *
 *  Created on: Sep 24, 2023
 *      Author: janok
 */

#ifndef INC_THERMISTOR_H_
#define INC_THERMISTOR_H_

#include "main.h"


void init_thermistor(ADC_HandleTypeDef* hotend_adc, ADC_HandleTypeDef* bed_adc);
void task_temp_read(void* param);
uint16_t read_temperature_adc(ADC_HandleTypeDef* adc);
float adc_to_voltage(uint32_t N_adc);
float adc_voltage_to_thermistor_resistance(float U_adc);
float thermistor_resistance_to_temperature(float R_therm);
float convert_temperature(uint16_t N_adc);
void read_therm_adc_callback(ADC_HandleTypeDef* adc);
uint16_t get_last_hotend_temp_adc();
uint16_t get_last_bed_temp_adc();

#endif /* INC_THERMISTOR_H_ */
