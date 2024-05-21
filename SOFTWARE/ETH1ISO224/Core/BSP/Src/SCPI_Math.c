/*
 * SCPI_Math.c
 *
 *  Created on: May 21, 2024
 *      Author: BehrensG
 */

#include "cmsis_os.h"

#include "SCPI_Math.h"
#include "BSP.h"
#include "ADC.h"

extern float measurements[];
extern SemaphoreHandle_t MeasMutex;
extern scpi_choice_def_t scpi_boolean_select[];
extern bsp_t bsp;


scpi_result_t SCPI_MathOffsetEnable(scpi_t * context)
{
	int32_t value;
	if (!SCPI_ParamChoice(context, scpi_boolean_select, &value, TRUE))
	{
		return SCPI_RES_ERR;
	}

	bsp.adc.math_offset.enable = (bool)value;

	return SCPI_RES_OK;
}

scpi_result_t SCPI_MathOffsetEnableQ(scpi_t * context)
{

	SCPI_ResultBool(context, bsp.adc.math_offset.enable);

	return SCPI_RES_OK;
}


static float MATH_Average(uint32_t sample_count)
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

scpi_result_t SCPI_MathOffset(scpi_t * context)
{
	uint32_t sample_size = ADC_DEF_SIZE;
	uint8_t gain = bsp.adc.gain.value;
	bool null_offset_state = bsp.adc.math_offset.enable;

	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		HAL_Delay(10);
		GPIO_DG419(true);
		HAL_Delay(200);

	 bsp.adc.math_offset.enable = false;

		if(ADC_Measurement(sample_size))
		{

			bsp.adc.math_offset.zero[bsp.adc.gain.index] = -1.0f * MATH_Average(sample_size);
			bsp.adc.math_offset.enable = null_offset_state;
		}
		else
		{
			xSemaphoreGive(MeasMutex);
			SCPI_ErrorPush(context, SCPI_ERROR_SYSTEM_ERROR);
			bsp.adc.math_offset.enable = null_offset_state;


			HAL_Delay(1);
			GPIO_DG419(false);
			HAL_Delay(100);

			return SCPI_RES_ERR;

		}

		HAL_Delay(1);
		GPIO_DG419(false);
		HAL_Delay(100);
		bsp.adc.math_offset.enable = null_offset_state;
		xSemaphoreGive(MeasMutex);

		return SCPI_RES_OK;
	}
	else
	{
		return SCPI_RES_ERR;
	}
}


scpi_result_t SCPI_MathOffsetQ(scpi_t * context)
{
	uint32_t gain;
	uint8_t index;
	if(!SCPI_ParamUInt32(context, &gain, FALSE))
	{
		index = bsp.adc.gain.index;
	}
	else
	{
		(ADC_CheckGain(gain)) ? (index = ADC_GainIndex(gain)) :  (index = bsp.adc.gain.index);

	}

	float offset =  bsp.adc.math_offset.zero[index];
	SCPI_ResultFloat(context, offset);
	return SCPI_RES_OK;
}
