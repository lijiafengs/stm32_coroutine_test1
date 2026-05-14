#include "stm32f4xx_hal.h"

extern "C" void SysTick_Handler(void) {
  HAL_IncTick();
}

extern "C" void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}
