#ifndef __TIM_H_
#define __TIM_H_

#define TIM_PERIOD 5555 /* for 10KHz PWM frequency. See : http://stm32f4-discovery.com/2014/05/stm32f4-stm32f429-discovery-pwm-tutorial/ */
#define INI_PULSE_LENGTH 1388 /* (((TIM_Period + 1) * DutyCycle / 100 - 1  ) Start with a 25% duty cycle*/

extern TIM_HandleTypeDef htim2;

void TIM_Init(void);

#endif
