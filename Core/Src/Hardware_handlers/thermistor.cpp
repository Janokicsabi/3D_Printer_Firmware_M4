/*
 * thermistor.c
 *
 *  Created on: Sep 24, 2023
 *      Author: janok
 */

#include "thermistor.h"
#include "temperature_control.h"

#include <cmath>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#define N_ADC_MAX		(4096)		//[digit]
#define U_VCC			(3.3)		//[V]
//Resistor value of the voltage devider
#define R_VOLT_DEV		(10000)		//[Ohm]
#define ABSOLUTE_ZERO	(-273.15)	//[°C]

#define THERM_PARAM_A	(0.000742985)
#define THERM_PARAM_B	(0.000210407)
#define THERM_PARAM_C	(1.22718e-07)

extern SemaphoreHandle_t temp_adc_sem;

static ADC_HandleTypeDef* hotend_temp_adc;
static ADC_HandleTypeDef* bed_temp_adc;

static uint16_t last_hotend_temp_adc = 0;
static uint16_t last_bed_temp_adc = 0;


void init_thermistor(ADC_HandleTypeDef* hotend_adc, ADC_HandleTypeDef* bed_adc) {
	hotend_temp_adc = hotend_adc;
	bed_temp_adc = bed_adc;
}

void task_temp_read(void* param) {
	while(1) {
		xSemaphoreTake(temp_adc_sem, portMAX_DELAY);
        HAL_ADC_Start(hotend_temp_adc);
        HAL_ADC_Start(bed_temp_adc);
        HAL_ADC_PollForConversion(hotend_temp_adc, 1);
        HAL_ADC_PollForConversion(bed_temp_adc, 1);
        last_hotend_temp_adc = HAL_ADC_GetValue(hotend_temp_adc);
        last_bed_temp_adc = HAL_ADC_GetValue(bed_temp_adc);
        xSemaphoreGive(temp_adc_sem);
		vTaskDelay(100);
	}
}

float adc_to_voltage(uint32_t N_adc) {
	return ((float)N_adc / (float)N_ADC_MAX) * (float)U_VCC;
}

float adc_voltage_to_thermistor_resistance(float U_adc) {
	float num = (U_adc / (float)U_VCC) * (float)R_VOLT_DEV;
	float den = 1 - (U_adc / (float)U_VCC);
	float thermistor_resistance = num / den;
	return thermistor_resistance;
	//return (float)R_VOLT_DEV * ((float)U_VCC / (float)U_adc - (float)U_adc / (float)U_VCC);
}

float thermistor_resistance_to_temperature(float R_therm) {
	float tmp = log(R_therm);
	float tmp_cubed = tmp * tmp * tmp;
	float den = THERM_PARAM_A + THERM_PARAM_B * tmp + THERM_PARAM_C * tmp_cubed;
	return (float)(1.0 / den) + (float)ABSOLUTE_ZERO;
}

float convert_temperature(uint16_t N_adc) {
	float U_adc = adc_to_voltage(N_adc);
	float R_therm = adc_voltage_to_thermistor_resistance(U_adc);
	float temperature = thermistor_resistance_to_temperature(R_therm);
	return temperature;
}

uint16_t get_last_hotend_temp_adc() {
	uint16_t tmp = 0;
	xSemaphoreTake(temp_adc_sem, portMAX_DELAY);
	tmp = last_hotend_temp_adc;
	xSemaphoreGive(temp_adc_sem);
	return tmp;
}

uint16_t get_last_bed_temp_adc() {
	uint16_t tmp = 0;
	xSemaphoreTake(temp_adc_sem, portMAX_DELAY);
	tmp = last_bed_temp_adc;
	xSemaphoreGive(temp_adc_sem);
	return tmp;
}
