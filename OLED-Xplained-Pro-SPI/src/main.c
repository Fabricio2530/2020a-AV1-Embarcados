#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "PIO_FUNCTIONS.h"
#include "PIO_OLED.h"
#include "TC-RTT-RTC.h"

//variaveis globais
volatile char flag_tc_display = 0;
volatile char flag_tc_decrementa = 0;
volatile char start_contagem = 0;
volatile char led_pisca = 0;

char str[300];
char str2[300];

volatile int minutos = 0;
volatile int segundos = 0;

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t seccond;
} calendar;

uint32_t current_hour, current_min, current_sec;
uint32_t current_year, current_month, current_day, current_week;

void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);

	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED_PI3, LED_PI3_IDX_MASK);  
}

void TC3_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC1, 0);

	/** Muda o estado do LED (pisca) **/
	flag_tc_display = 1;
}

void TC6_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC2, 0);

	/** Muda o estado do LED (pisca) **/
	flag_tc_decrementa = 1;
	
}


void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
		flag_rtc_alarm = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}


void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.seccond);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	oled_init();

  // Init OLED
	gfx_mono_ssd1306_init();
  
  // Escreve na tela um circulo e um texto
	
	//CONTAGEM PARA ATUALIZAÇÃO DO DISPLAY
	TC_init(TC1, ID_TC3, 0, 1);
	tc_start(TC1, 0);
	//gfx_mono_draw_string("00:00", 20,15, &sysfont);
	

  /* Insert application code here, after the board has been initialized. */
	while(1) {

		if (flag_tc_display){
			
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
			
			if (current_sec < 10) {
				sprintf(str, "%d:%d:0%d", current_hour, current_min, current_sec);
				} else {
				sprintf(str, "%d:%d:%d", current_hour, current_min, current_sec);
			}
			gfx_mono_draw_string(str, 0,0, &sysfont);
			flag_tc_display = 0;
		}
		
		if (flag_but1 && (start_contagem == 0)) {
			if (minutos < 59) {
				minutos += 1;
			}
			flag_but1 = 0;
			sprintf(str2, "%d:%d  ",minutos, segundos);
			gfx_mono_draw_string(str2, 22,15, &sysfont);
		}
		
		if (flag_but2 && (start_contagem == 0)) {
			if (segundos < 59) {
				segundos += 1;
			}
			flag_but2 = 0;
			sprintf(str2, "%d:%d  ",minutos, segundos);
			gfx_mono_draw_string(str2, 22,15, &sysfont);
		}
		
		if (flag_but3) {
			
			if (start_contagem == 0) {
				start_contagem = 1;
				pio_set_output(LED_PI1, LED_PI1_IDX_MASK, 0, 0, 0);
				TC_init(TC2, ID_TC6, 0, 1);
				tc_start(TC2, 0);
			} else {
				start_contagem = 0;
				pio_set_output(LED_PI1, LED_PI1_IDX_MASK, 1, 0, 0);
				tc_stop(TC2, 0);
				
				if (led_pisca) {
					led_pisca = 0;
					tc_stop(TC0, 1);
					pio_set_output(LED_PI3, LED_PI3_IDX_MASK, 1, 0, 0);
				}
				
			}
			flag_but3 = 0;
		}
		
		if (flag_tc_decrementa) {
			if (minutos == 0) {
				
				if (segundos == 0) {
					tc_stop(TC2, 0);
					//gfx_mono_draw_string("00:00", 22,15, &sysfont);
					TC_init(TC0, ID_TC1, 1, 4);
					tc_start(TC0, 1);
					led_pisca = 1;
				
				} else {
					segundos -= 1;		
				}		
			
			} else {
				if (segundos == 0) {
					segundos = 59;
					minutos -= 1;
				} else {
					segundos -= 1;
				}
			}
			
		sprintf(str2, "%d:%d  ",minutos, segundos);
		gfx_mono_draw_string(str2, 22,15, &sysfont);	
		flag_tc_decrementa = 0;
		
		}	
		
	}
}
