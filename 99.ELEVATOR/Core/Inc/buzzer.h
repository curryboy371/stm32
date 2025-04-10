#include "main.h"



typedef struct {

	uint8_t is_playing;		// 재생 중인지 아닌지

}t_buzzer_info;


typedef struct {

	int frequency;
	uint16_t play_delay;

}t_buzzer_play_info;

#define BUZZER_PLAY_QUEUE_SIZE 24

void beep(int repeat);
void set_buzzer(int frequency);
void buzzer_main();
void PowerOnBuzzer();
void rrr(void);

void buzzer_init();

void buzzer_update();

void buzzer_welcom_play();
void buzzer_on_button();
void buzzer_off_button();

uint8_t buzzer_is_full_queue();
uint8_t buzzer_is_empty_queue();
t_buzzer_play_info buzzer_pop_queue();
void buzzer_push_queue(const t_buzzer_play_info* play_info);
void buzzer_push_queue_multi(const t_buzzer_play_info play_infos[], int size);

