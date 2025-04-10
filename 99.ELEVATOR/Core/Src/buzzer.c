#include "buzzer.h"
#include "extern.h"
#include "def.h"
#include "timer.h"
#include <stdio.h>


/*************************************************************************************************************

	옥타브 및 음계별 주파수표(단위:Hz)

	음계	   0Oct	  1Oct	 2Oct    3Oct	      4Oct	     5Oct	      6Oct	      7Oct	     8Oct	     9Oct
	C	16.3516	32.7032	65.4064	130.8128	261.6256	523.2511	1046.502	2093.005	4186.009	8372.019
	C#	17.3239	34.6478	69.2957	138.5913	277.1826	554.3653	1108.731	2217.461	4434.922
	D	18.3541	36.7081	73.4162	146.8324	293.6648	587.3295	1174.659	2349.318	4698.636
	D#	19.4454	38.8909	77.7817	155.5635	311.127	    622.254	    1244.508	2489.016	4978.032
	E	20.6017	41.2034	82.4069	164.8138	329.6276	659.2551	1318.51	    2637.02	    5274.041
	F	21.8268	43.6535	87.3071	174.6141	349.2282	698.4565	1396.913	2793.826	5587.652
	F#	23.1247	46.2493	92.4986	184.9972	369.9944	739.9888	1479.978	2959.955	5919.911
	G	24.4997	48.9994	97.9989	195.9977	391.9954	783.9909	1567.982	3135.963	6271.927
	G#	25.9565	51.913	103.8262 207.6523	415.3047	830.6094	1661.219	3322.438	6644.875
	A	27.5	55	    110	     220	     440	    880	        1760	    3520	    7040
	A#	29.1352	58.2705	116.5409	233.0819	466.1638	932.3275	1864.655	3729.31	7458.62
	B	30.8677	61.7354	123.4708	246.9417	493.8833	987.7666	1975.533	3951.066	7902.113
	※참고: C=도, D=레, E=미, F=파, G=솔, A=라, B=시


	옥타브 및 음계별 펄스폭표(단위:us)

	Oct  0	      1	     2	     3	     4	     5	     6	     7	     8	     9
	C	61156 	30578 	15289 	7645 	3822 	1911 	956 	478 	239 	119
	C#	57724 	28862 	14431 	7215 	3608 	1804 	902 	451 	225
	D	54484 	27242 	13621 	6810 	3405 	1703 	851 	426 	213
	D#	51426 	25713 	12856 	6428 	3214 	1607 	804 	402 	201
	E	48540 	24270 	12135 	6067 	3034 	1517 	758 	379 	190
	F	45815 	22908 	11454 	5727 	2863 	1432 	716 	358 	179
	F#	43244 	21622 	10811 	5405 	2703 	1351 	676 	338 	169
	G	40817 	20408 	10204 	5102 	2551 	1276 	638 	319 	159
	G#	38526 	19263 	9631 	4816 	2408 	1204 	602 	301 	150
	A	36364 	18182 	9091 	4545 	2273 	1136 	568 	284 	142
	A#	34323 	17161 	8581 	4290 	2145 	1073 	536 	268 	134
	B	32396 	16198 	8099 	4050 	2025 	1012 	506 	253 	127

 *****************************************************************************************************/
/*
 === 피에조 부저 제어 방법  ====
  피에조 부저 Resonant Frequency(공진 주파수): 4kHz
STM32에서 주파수를 만들 때 3개의 레지스터를 설정한다.
 PSC(Prescaler), ARR(Peroid), CCRx(Duty)
- Prescaler register(TIMx_PSC) : APB를 통해 공급되느 클럭이 빠를 경우 이 레지스터의 값만큼 분주하여 사용할 수 있다.
- Auto reload register(TIMx_ARR) : TIMx_CNT 레지스터를 초기화 시켜주기 위한 레지스터이다.  
                                   Up Counting에서 TIMx_CNT가 이 값에 도달하면 0으로 초기화되어 다시 카운팅한다. 
- Capture/Compare register(TIMx_CCR) : Capture/Compare 두 개의 모드로 외부 입력 신호에 따라 
  TIMx_CNT의 값을 기록(Capture)하거나 TIMx_CNT와 TIMx_CCR를 비교하여(Compare) 같아지면 인터럽트나 출력 등의 이벤트를 발생시킨다.

 다음과 같이 적용 하면 된다.
 - PSC : Timer Clock / 기준으로 사용할 주파수 - 1
 - ARR : 기준 주파수 / 실제 주파수 - 1
 - CCRx : ARR값 * (적용할 백분율 값 0 ~ 100) / 100   CCR(Counter Capture Register)
예를 들어 동작 클럭이 84Mhz이고 4Khz에 50%비율로 동작하는 PWM을 만들고 싶다면 식은 다음과 같다.
 Prescaler(기준 클럭) : 1.6Mhz을 만든다면 - 84,000,000(타이머 클럭) / 1,600,000(만들 클럭) = 52.5
 실제 레지스터 PSC에 적용시 1을 뺀 51.5값을 적용한다.
 PSC = 52.5-1
 Period : 4khz (실제 주파수) - 1,600,000(기준 클럭) / 4000(실제 주파수) = 400 실제 레지스터 ARR에 적용할땐 1을 뺀 399값을 적용한다.
 ARR = 399; Duty : 50% - 399(ARR) * 50(퍼센트) / 100 = 199
 CCRx = 199
 */


enum notes
{
  C4 = 262, // 도 261.63Hz
  D4 = 294, // 래 293.66Hz
  E4 = 330, // 미 329.63Hz
  F4 = 349, // 파 349.23Hz
  G4 = 392, // 솔 392.00Hz
  A4 = 440, // 라 440.00Hz
  B4 = 494, // 시 493.88Hz
  C5 = 523  // 도 523.25Hz

};


t_buzzer_play_info temp_play_info;


t_buzzer_info buzzer_info;
t_buzzer_play_info buzzer_play_queue[BUZZER_PLAY_QUEUE_SIZE];
volatile int buzzer_play_front;
volatile int buzzer_play_rear;




// 학교종이 떙떙땡
unsigned int school_bell[] =
{
	G4,G4,A4,A4,G4,G4,E4,G4,G4,E4,E4,D4,
	G4,G4,A4,A4,G4,G4,E4,G4,E4,D4,E4,C4
};

// happybirthday to you
unsigned int happy_birthday[] =
{
	 C4,C4,D4,C4,F4,E4,C4,C4,D4,C4,G4,
	 F4,C4,C4,C5,A4,F4,E4,D4,B4,B4,A4,
	 A4,G4,F4
};

 unsigned int duration[] = {1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,1,1,2,2,2,2};

void noTone()
{
     htim3.Instance->CCR1=0;
     HAL_Delay(50);
}

void set_buzzer(int frequency)
{
	int divide_freq = 1600000; // 4KHZ 부저 주파수를 내기 위해 기본 클럭을 분주해서 얻은 주파수

	__HAL_TIM_SET_AUTORELOAD(&htim3, divide_freq / frequency); // PWM
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, divide_freq / frequency / 2);  // Duty를 50%로 설정 한다.
}


void emblance_siren() {


}

void fire_siren()
{
	int frequency=700; // 1.1kHz로 세팅

	static int mode = 1;


	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	for(int i = 0; i < 500; ++i) {
		if(mode) {
			frequency += 35;
		}
		else {
			frequency -= 35;
		}

		if(frequency >= 1500) {
			mode = 0;
		}

		if(frequency <= 700) {
			mode = 1;
		}

		set_buzzer(frequency);
		HAL_Delay(5);
	}


	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}


void siren(int repeat)
{
	int frequency=1111; // 1.1kHz로 세팅

	set_buzzer(frequency);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	for (int i=0; i < repeat; i++)
	{
		for (int j=0; j < 100; j++)
		{
			frequency += 10;  // add 10Hz
			set_buzzer(frequency);
			HAL_Delay(20);
		}
		for (int j=0; j < 100; j++)
		{
			frequency -= 10;  // sub 10Hz
			set_buzzer(frequency);
			HAL_Delay(20);
		}
	    noTone();
	}
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void rrr(void)
{
	 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	for (int i=0; i < 20; i++)
	{
		set_buzzer(880); // 880Hz로 세팅
		HAL_Delay(100);
	    noTone();
	    HAL_Delay(20);
	}
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void beep(int repeat)
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

    for (int i=0; i < repeat; i++)
    {
    	set_buzzer(2000);   // 2KHZ
    	HAL_Delay(500);
    	// beep stop
    	noTone();
       	HAL_Delay(200);
    }
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}


void buzzer_main()
{
   int divide_freq = 1600000; // 4KHZ 부저 주파수를 내기 위해 기본 클럭을 분주해서 얻은 주파수

  while (1)
  {
#if 1
	  open_Buzzer();
	  HAL_Delay(1000);
	  close_buzzer();
	  //fire_siren();
	  HAL_Delay(1000);

#endif

#if 0

	// 학교 종이 땡땡땡
    for (int i=0; i < 24; i++)
    {
		__HAL_TIM_SET_AUTORELOAD(&htim3, divide_freq / school_bell[i]);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, divide_freq / school_bell[i] / 2);  // Duty를 50%로 설정 한다.
		HAL_Delay(500);
		noTone();  /* note 소리 내고 50ms 끊어주기 */
    }

    /* 음악 끝나고 3초 후 시작*/
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1) ;
    HAL_Delay(3000);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) ;

    // happy birthday to you
    for (int i=0; i < 25; i++)
    {
		__HAL_TIM_SET_AUTORELOAD(&htim3, divide_freq / happy_birthday[i]);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, divide_freq / happy_birthday[i] / 2);
		HAL_Delay(300*duration[i]);
		noTone();
    }

    /* 음악 끝나고 3초 후 시작 */
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1) ;
    HAL_Delay(3000);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) ;
#endif
  }
}

#if 0
//------------------- happy birthday START ---------------
uint32_t Frequency = 0;
uint32_t CLOCK = 16000000;   // 4KHZ 부저 주파수를 내기 위해 기본 클럭을 분주해서 얻은 주파수
int i;

/*                       Hap  py  Birth Day  to  you,  Hap py   birth day  to
                         C4   C4   D4   C4   F4   E4   C4   C4   D4   C4   G4 */
unsigned int notes[] = { 262, 262, 294, 262, 349, 330, 262, 262, 294, 262, 392,

/*                       you, Hap py  Birth Day  dear  xxxx      Hap  py   birth
                         F4   C4   C4   C5   A4   F4   E4   D4   B4b  B4b  A4 */
                         349, 262, 262, 523, 440, 349, 330, 294, 466, 466, 440,

/*                       day  to  you
                         F4   G4   F4   */
                         349, 392, 349
                        };

enum note1
{
  C4 = 262, // 도(261.63Hz)
  D4 = 294, // 래(293.66Hz)
  E4 = 330, // 미(329.63Hz)
  F4 = 349, // 파(349.23Hz)
  G4 = 392, // 솔(392.00Hz)
  A4 = 440, // 라(440.00Hz)
  B4 = 494, // 시(493.88Hz)
  C5 = 523  // 도(523.25Hz)
};

/* 학교종
 * 솔 솔 라 라 솔 솔 미 솔 솔 미 미 래
 * 솔 솔 라 라 솔 솔 미 솔 미 래 미 도*/
enum note1 A[] = {G4,G4,A4,A4,G4,G4,E4,G4,G4,E4,E4,D4,
                  G4,G4,A4,A4,G4,G4,E4,G4,E4,D4,E4,C4};


unsigned int duration[] = {1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,1,1,2,2,2,2};


void noTone()
{
    htim3.Instance->CCR1=0;
    HAL_Delay(50);
}

void playSong()
{

    for (int i = 0 ; i < 24; i++)
    {
		Frequency = CLOCK/A[i];
		htim3.Instance->ARR=Frequency;
		htim3.Instance->CCR1=Frequency/2;  // OK
		HAL_Delay(400*duration[i]);
		noTone();
    }

    HAL_Delay(5000);

    for (i = 0; i <25; i++)
    {
		Frequency = CLOCK/notes[i];
		htim3.Instance->ARR=Frequency;
		htim3.Instance->CCR1=Frequency/2;  // OK
		HAL_Delay(400*duration[i]);
		noTone();
    }


}
//--------- happy birthday  END ---------------
#endif


void close_buzzer()
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	set_buzzer(1000); // 1kHz로 세팅
	HAL_Delay(70);
	set_buzzer(2000); // 2kHz로 세팅
	HAL_Delay(70);
	set_buzzer(3000); // 3kHz로 세팅
	HAL_Delay(70);
	set_buzzer(4000); // 4kHz로 세팅
	HAL_Delay(70);

	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void open_Buzzer()
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	set_buzzer(261); // 261Hz로 세팅
	HAL_Delay(70);
	set_buzzer(329); // 329Hz로 세팅
	HAL_Delay(70);
	set_buzzer(392); // 392Hz로 세팅
	HAL_Delay(70);
	set_buzzer(554); // 554Hz로 세팅
	HAL_Delay(70);

	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void buzzer_init() {

	buzzer_play_rear = 0;
	buzzer_play_front = 0;

	buzzer_info.is_playing = FALSE;

}

void buzzer_update() {

	if(TIM11_1ms_delay_counter[MODULE_BUZZER] > 0) {
		return;
	}

	if(buzzer_is_empty_queue()) {

		if(buzzer_info.is_playing == TRUE) {
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
			buzzer_info.is_playing = FALSE;
		}
		return;
	}

	t_buzzer_play_info play_info = buzzer_pop_queue();

	if(buzzer_info.is_playing == FALSE) {
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	}

	set_buzzer(play_info.frequency);
	TIM11_1ms_delay_counter[MODULE_BUZZER] = play_info.play_delay;
	buzzer_info.is_playing = TRUE;
}

void buzzer_welcom_play() {

	int size = 4;

	const static t_buzzer_play_info on_button[4] = {
			{C4, 500},
			{E4, 500},
			{G4, 500},
			{E4, 500}
	};

	buzzer_push_queue_multi(on_button, size);



}

void buzzer_on_button() {

	int size = 4;
	const static t_buzzer_play_info on_button[4] = {
			{267, 70},
			{329, 70},
			{392, 70},
			{554, 70}
	};


	buzzer_push_queue_multi(on_button, size);

}

void buzzer_off_button() {

	int size = 4;
	const static t_buzzer_play_info off_button[8] = {
			{1000, 70},
			{2000, 70},
			{3000, 70},
			{4000, 70},
	};

	buzzer_push_queue_multi(off_button, size);

}

uint8_t buzzer_is_full_queue() {

	return (buzzer_play_rear + 1) % BUZZER_PLAY_QUEUE_SIZE == buzzer_play_front;

}

uint8_t buzzer_is_empty_queue() {

	return (buzzer_play_front == buzzer_play_rear);

}

t_buzzer_play_info buzzer_pop_queue() {

	if(buzzer_is_empty_queue() == TRUE) {
		return temp_play_info;
	}


	int pop_idx = buzzer_play_front;

	buzzer_play_front = (buzzer_play_front + 1) % BUZZER_PLAY_QUEUE_SIZE;

	return buzzer_play_queue[pop_idx];
}

void buzzer_push_queue(const t_buzzer_play_info* play_info) {

	if(!play_info) {
		return;
	}

	if(buzzer_is_full_queue() == TRUE) {
		printf("buzzer queue full \r\n");
		return;
	}

	buzzer_play_queue[buzzer_play_rear] = *play_info;

	buzzer_play_rear = (buzzer_play_rear + 1) % BUZZER_PLAY_QUEUE_SIZE;


}

void buzzer_push_queue_multi(const t_buzzer_play_info play_infos[], int size) {

	int remain_size = (BUZZER_PLAY_QUEUE_SIZE + buzzer_play_front - buzzer_play_rear - 1) % BUZZER_PLAY_QUEUE_SIZE;

	if(remain_size < size) {
		printf("buzzer not enough size \r\n");
		return;
	}

	for(int i = 0; i < size; ++i) {

		buzzer_push_queue(&play_infos[i]);
	}

}

