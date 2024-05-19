/*
 * SCPI_Measure.c
 *
 *  Created on: Apr 7, 2024
 *      Author: grzegorz
 */


#include "cmsis_os.h"

#include "SCPI_Measure.h"
#include "BSP.h"
#include "ADC.h"
#include "GPIO.h"
#include "Utility.h"

extern float measurements[];
extern bsp_t bsp;
extern scpi_choice_def_t scpi_boolean_select[];
extern SemaphoreHandle_t MeasMutex;

#define MAX_SAMPLES_IN_PACKAGE	1000

static scpi_result_t SCPI_ResultASCII(scpi_t * context, float* measurements, uint32_t sample_count)
{
	char* str;

	str = UTIL_FloatArrayToASCII(measurements, sample_count);
	SCPI_ResultCharacters(context, str, strlen(str));

	return SCPI_RES_OK;
}

static scpi_result_t SCPI_ResultREAL(scpi_t * context, float* measurements, uint32_t sample_count)
{
	char* str;
	uint32_t size = bsp.adc.sample_count * sizeof(float);

	str = UTIL_FloatArrayToREAL(measurements, sample_count);
	SCPI_ResultArbitraryBlock(context, str, size);
	return SCPI_RES_OK;
}

scpi_result_t SCPI_MeasureQ(scpi_t * context)
{

	if(pdTRUE == xSemaphoreTake(MeasMutex,  pdMS_TO_TICKS(20000)))
	{
		if(ADC_Measurement(bsp.adc.sample_count))
		{

			if(FORMAT_DATA_ASCII == bsp.format.data)
			{
				SCPI_ResultASCII(context, measurements, bsp.adc.sample_count);

			}
			else
			{
				SCPI_ResultREAL(context, measurements, bsp.adc.sample_count);
			}

			xSemaphoreGive(MeasMutex);
			return SCPI_RES_OK;
		}
		else
		{
			xSemaphoreGive(MeasMutex);
			SCPI_ErrorPush(context, SCPI_ERROR_SYSTEM_ERROR);
			return SCPI_RES_ERR;
		}

	}
	else
	{
		return SCPI_RES_ERR;
	}

}

scpi_result_t SCPI_FetchQ(scpi_t * context)
{
	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		if(FORMAT_DATA_ASCII == bsp.format.data)
		{
			SCPI_ResultASCII(context, measurements, bsp.adc.sample_count);
		}
		else
		{
			SCPI_ResultREAL(context, measurements, bsp.adc.sample_count);
		}

		xSemaphoreGive(MeasMutex);
		return SCPI_RES_OK;
	}
	else
	{
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;

}

scpi_result_t SCPI_Initiate(scpi_t * context)
{
	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		if(!ADC_Measurement(bsp.adc.sample_count))
		{
			xSemaphoreGive(MeasMutex);
			SCPI_ErrorPush(context, SCPI_ERROR_SYSTEM_ERROR);
			return SCPI_RES_ERR;
		}

		xSemaphoreGive(MeasMutex);
		return SCPI_RES_OK;
	}
	else
	{
		return SCPI_RES_ERR;
	}
}


static float MEAS_Average(uint32_t sample_count)
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

scpi_result_t SCPI_NullOffsetEnable(scpi_t * context)
{
	int32_t value;
	if (!SCPI_ParamChoice(context, scpi_boolean_select, &value, TRUE))
	{
		return SCPI_RES_ERR;
	}

	bsp.adc.offset.enable = (bool)value;

	return SCPI_RES_OK;
}

scpi_result_t SCPI_NullOffset(scpi_t * context)
{
	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		HAL_Delay(10);
		GPIO_DG419(true);
		HAL_Delay(200);

		if(ADC_Sample(ADC_DEF_SIZE))
		{
			ADC_SignalConditioningZeroOffset(bsp.adc.gain.value, ADC_DEF_SIZE);
			bsp.adc.offset.zero[bsp.adc.gain.index] = -1.0f * MEAS_Average(ADC_DEF_SIZE);
		}
		else
		{
			xSemaphoreGive(MeasMutex);
			SCPI_ErrorPush(context, SCPI_ERROR_SYSTEM_ERROR);
			return SCPI_RES_ERR;

			HAL_Delay(1);
			GPIO_DG419(false);
			HAL_Delay(100);
		}

		HAL_Delay(1);
		GPIO_DG419(false);
		HAL_Delay(100);

		xSemaphoreGive(MeasMutex);

		return SCPI_RES_OK;
	}
	else
	{
		return SCPI_RES_ERR;
	}
}

scpi_result_t SCPI_NullOffsetQ(scpi_t * context)
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

	float offset =  bsp.adc.offset.zero[index];
	SCPI_ResultFloat(context, offset);
	return SCPI_RES_OK;
}

scpi_result_t SCPI_DataDataQ(scpi_t * context)
{

	uint32_t index = 0;
	uint32_t count = 1;

	if(!SCPI_ParamUInt32(context, &index, TRUE))
	{
		return SCPI_RES_ERR;
	}

	if(!SCPI_ParamUInt32(context, &count, TRUE))
	{
		return SCPI_RES_ERR;
	}

	if (index > bsp.adc.sample_count)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_RANGE);
		return SCPI_RES_ERR;
	}

	if (count > bsp.adc.sample_count)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_RANGE);
		return SCPI_RES_ERR;
	}

	if ((index + count) > bsp.adc.sample_count)
	{
		SCPI_ErrorPush(context, SCPI_ERROR_INVALID_RANGE);
		return SCPI_RES_ERR;
	}

	if(pdTRUE == xSemaphoreTake(MeasMutex, pdMS_TO_TICKS(20000)))
	{
		if(FORMAT_DATA_ASCII == bsp.format.data)
		{
			SCPI_ResultASCII(context, &measurements[index], count);
		}
		else
		{
			SCPI_ResultREAL(context, &measurements[index], count);
		}

		xSemaphoreGive(MeasMutex);
		return SCPI_RES_OK;
	}
	else
	{
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}
