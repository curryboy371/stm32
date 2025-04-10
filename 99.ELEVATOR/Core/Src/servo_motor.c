#include "servo_motor.h"
#include "extern.h"
#include "def.h"
#include <stdio.h>

// todo remove
volatile int TIM10_servo_motor_counter;


/*
  System clock: 84MHZ (84,000,000HZ)
  TIM3 50KHZ 입력 : 84000000/1680 ==> 50,000HZ(50KHZ)
  T=1/f = 1/50000 = 0.00002sec (20us)
  20ms : 0.00002 x 1000개
  2ms(180도) : 0.00002 x 100개
  1.5ms(90도) : 0.00002 x 75개
  1ms(0도): 0.00002 x 50개
 */
void servo_motor_main(void)
{

	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);

#if 1
	while(1) {



		for(int i = 0; i < 10; ++i) {

			printf("180\n");
			__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,130); // 180
			//__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,100); // 180
			HAL_Delay(100);
		}




	    printf("90\n");

	    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,80);  // 90
		//__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,75);  // 90
		HAL_Delay(1000);


	    printf("0\n");

	    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,40);  // 0
		//__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,50);  // 0
		HAL_Delay(1000);


	}

    HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_2);


#else
	static uint8_t servo_state=0;

	if (TIM10_servo_motor_counter >= 1000) // 1sec
	{
		TIM10_servo_motor_counter=0;
		// 180 --> 90 --> 0
		switch(servo_state)
		{
		case 0:   // 180
			__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,100);
			break;
		case 1:   // 90
			__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,75);
			break;
		case 2:   // 0
			__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2,50);
			break;
		}
		servo_state = (servo_state + 1) % 3;
//		servo_state++;
//		servo_state %= 3;
	}

#endif



}

// GPIO_PIN_1 : 발생시 서보모터 0도 --> 180도 이동 (3초유지) --> 서보 모터0도
// 3초유지는 HAL Delay를 쓰지 말고 TIM10_servo_motor_counter변수 활용
void servo_motor_control(void)
{
	static uint8_t servo_state=0;


}

