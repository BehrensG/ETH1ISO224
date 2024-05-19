/*
 * SCPI_Calculate.c
 *
 *  Created on: Apr 25, 2024
 *      Author: grzegorz
 */

#include "cmsis_os.h"

#include "SCPI_Calculate.h"
#include "BSP.h"

extern bsp_t bsp;

extern float measurements[];
extern SemaphoreHandle_t MeasMutex;

static float CALC_Average(uint32_t sample_count)
{
	float average = 0.0f;
	float tmp = 0.0f;

	for (uint32_t i = 0; i < sample_count ; i++)
	{
		tmp += measurements[i];
	}

	average = (float)(tmp/sample_count);

	return average;
}


scpi_result_t SCPI_CalculateAverageQ(scpi_t * context)
{

	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		SCPI_ResultFloat(context, CALC_Average(bsp.adc.sample_count));
		xSemaphoreGive(MeasMutex);
	}
	else
	{
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}
