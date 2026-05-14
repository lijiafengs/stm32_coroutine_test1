/**
* @brief Implements STM32 HAL interrupt and MSP hooks required by the firmware.
* @author codex
* @date 2026-05-14
* @version 1.0
*/

#include "stm32f4xx_hal.h"

/**
* @brief Handles the SysTick interrupt and advances the HAL tick counter.
* @param[in] None No input parameters.
* @return None.
*/
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
* @brief Initializes the minimal HAL MSP clock enables used by this project.
* @param[in] None No input parameters.
* @return None.
*/
extern "C" void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}
