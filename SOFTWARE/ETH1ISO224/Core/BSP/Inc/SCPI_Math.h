/*
 * SCPI_Math.h
 *
 *  Created on: May 21, 2024
 *      Author: BehrensG
 */

#ifndef BSP_INC_SCPI_MATH_H_
#define BSP_INC_SCPI_MATH_H_

#include "scpi/scpi.h"

scpi_result_t SCPI_MathOffsetEnable(scpi_t * context);
scpi_result_t SCPI_MathOffsetEnableQ(scpi_t * context);
scpi_result_t SCPI_MathOffset(scpi_t * context);
scpi_result_t SCPI_MathOffsetQ(scpi_t * context);


#endif /* BSP_INC_SCPI_MATH_H_ */
