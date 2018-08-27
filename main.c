/*
 * FINAL_PROJECT.c
 *
 * Created: 8/24/2018 9:14:13 PM
 * Author : Dylan
 */ 
/*	Partner(s) Name & E-mail:
 *	Lab Section: B21
 *	Assignment: Lab # 10 Exercise # 4
 *	Exercise Description: [optional - include for your own benefit]
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
//#include <avr/limits.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include "bit.h"
#include "io.c"

#define tasksSize  6
unsigned long tasksPeriod = 10;

unsigned char game_on = 1;

typedef struct Task {
	int state;						// Task’s current state
	unsigned long period;			// Task period
	unsigned long elapsedTime;	// Time elapsed since last task tick
	int (*TickFct)(int);				// Task tick function
} Task;

Task tasks[tasksSize];
void TimerISR(){
	unsigned char i;
	for (i = 0;i < tasksSize;++i) {
		if (tasks[i].elapsedTime >= tasks[i].period) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriod;
	}
}


// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}


void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}


// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




enum MENU_STATES{MENU_START,MENU_INIT,WAIT_MENU,DISP_CHARACTER,SET_CHARACTER}menu_state;


int menu_tick(int menu_state){
	switch(menu_state){//transitions
		case MENU_START:
			menu_state = MENU_INIT;
			break;
		case MENU_INIT:
			menu_state = WAIT_MENU;
			break;
		case WAIT_MENU:
			menu_state = WAIT_MENU;
			break;
		case DISP_CHARACTER:
			menu_state = WAIT_MENU;
			break;
		case SET_CHARACTER:
			menu_state = WAIT_MENU;
			break;
		default:
			menu_state = MENU_START;
			break;
	}
	
	switch(menu_state){//actions
		case MENU_START:
			break;
		case MENU_INIT:
			//cursor = 1;
			LCD_init();
			LCD_ClearScreen();
			break;
		case WAIT_MENU:
			break;
		case DISP_CHARACTER:
			break;
		case SET_CHARACTER:
			break;		
	}	
	return menu_state;
}


unsigned char player_cursor = 1;
unsigned char player_char = 'P';
unsigned char button_left = 0;
unsigned char button_right = 0;

enum PLAYER_STATES{PLAYER_START,PLAYER_INIT,WAIT_FOR_PLAYER_BUTTON,BUTTON_PRESS_LEFT,BUTTON_PRESS_RIGHT}player_state;

int player_tick(int player_state){
	if(game_on){
	button_left = ~PINA & 0x04;
	button_right = ~PINA & 0x08;

	switch(player_state){
		case PLAYER_START:
			player_state = PLAYER_INIT;
			break;
		case PLAYER_INIT:
			player_state = WAIT_FOR_PLAYER_BUTTON;
			break;
		case WAIT_FOR_PLAYER_BUTTON:
			//LCD_DisplayString(1,"APPLES");
			if (button_left == 0x04){
				player_state = BUTTON_PRESS_LEFT;
				//PORTB = 0x01;
				///*
				//if (player_cursor !=1){
				//LCD_ClearScreen();
				//delay_ms(1);
				LCD_Cursor(player_cursor);
				LCD_WriteData(' ');

				player_cursor = 1;
				LCD_Cursor(player_cursor);
				//LCD_Cursor(1);
				LCD_WriteData(player_char);
				//LCD_Cursor(player_cursor);
				
				//LCD_Cursor(17);
				//LCD_WriteData('-');
				//}
				//*/
			}		
			else if (button_right == 0x08){
				player_state = BUTTON_PRESS_RIGHT;
				//PORTB = 0x02;
				///*
				//if (player_cursor != 17){
				//LCD_ClearScreen();
				//delay_ms(1);
				LCD_Cursor(player_cursor);
				LCD_WriteData(' ');
				player_cursor = 17;
				LCD_Cursor(player_cursor);
				//LCD_Cursor(17);
				LCD_WriteData(player_char);
				//LCD_Cursor(player_cursor);
				//LCD_Cursor(1);
				//LCD_WriteData('-');
				//}
				//*/
			}

			else {
				player_state = WAIT_FOR_PLAYER_BUTTON;
				LCD_Cursor(player_cursor);
				LCD_WriteData(player_char);
			}

			break;
		case BUTTON_PRESS_LEFT:
			if (button_left == 0x04){
				//PORTB = 0x04;
				player_state = BUTTON_PRESS_LEFT;
			}
			else {
				player_state = WAIT_FOR_PLAYER_BUTTON;
			}
			break;
		case BUTTON_PRESS_RIGHT:
			if (button_right == 0x08){
				player_state = BUTTON_PRESS_RIGHT;
				//PORTB = 0x08;
			}
			else{
				player_state = WAIT_FOR_PLAYER_BUTTON;
			}
			break;
		default:
			break;

	}

	switch(player_state){
		case PLAYER_START:
			break;
		case PLAYER_INIT:
			player_cursor = 1;
			LCD_Cursor(player_cursor);
			LCD_WriteData(player_char);
			break;
		case WAIT_FOR_PLAYER_BUTTON:
			break;
		case BUTTON_PRESS_LEFT:
			break;
		case BUTTON_PRESS_RIGHT:
			break;
		default:
			break;

		}
	}//if game_on
		return player_state;
}




enum ENEMY_STATES_1{ENEMY_START_1,ENEMY_INIT_1,ENEMY_MOVE_1}enemy_state_1;


unsigned char enemy_char_1 = '!';

unsigned char enemy_cursor_1;

int enemy_tick_1(int enemy_state_1){
	if (game_on){
	switch(enemy_state_1){
	case ENEMY_START_1:
		enemy_state_1 = ENEMY_INIT_1;
		break;
	case ENEMY_INIT_1:
		enemy_state_1 = ENEMY_MOVE_1;
		break;
	case ENEMY_MOVE_1:
		enemy_state_1 = ENEMY_MOVE_1;
		break;
	}

	switch(enemy_state_1){
	case ENEMY_START_1:
		break;
	case ENEMY_INIT_1:
		enemy_cursor_1 = 10;
		break;
	case ENEMY_MOVE_1:
		//LCD_Cursor(enemy_cursor_1);
		//LCD_WriteData(' ');
		
		if (enemy_cursor_1 <= 0){
			enemy_cursor_1 = 16;
		}
		//LCD_ClearScreen();
		//delay_ms(1);
		LCD_Cursor(enemy_cursor_1);
		LCD_WriteData(' ');
		//LCD_Cursor(enemy_cursor_1-1);
		//LCD_WriteData(' ');
		enemy_cursor_1 -=1;
		LCD_Cursor(enemy_cursor_1);
		LCD_WriteData(enemy_char_1);
		
		break;		
	}
	}//if game_one
	return enemy_state_1;

}

enum ENEMY_STATES_2{ENEMY_START_2,ENEMY_INIT_2,ENEMY_MOVE_2}enemy_state_2;


unsigned char enemy_char_2 = '&';
unsigned char enemy_cursor_2;

int enemy_tick_2(int enemy_state_2){
	if (game_on){
	switch(enemy_state_2){
		case ENEMY_START_2:
		enemy_state_2 = ENEMY_INIT_2;
		break;
		case ENEMY_INIT_2:
		enemy_state_2 = ENEMY_MOVE_2;
		break;
		case ENEMY_MOVE_2:
		enemy_state_2 = ENEMY_MOVE_2;
		break;
	}

	switch(enemy_state_2){
		case ENEMY_START_2:
		break;
		case ENEMY_INIT_2:
		enemy_cursor_2 = 32;
		//enemy_cursor_2  = 28;
		break;
		case ENEMY_MOVE_2:
		if (enemy_cursor_2 <=16){
			enemy_cursor_2 = 32;
		}
		LCD_Cursor(enemy_cursor_2);
		LCD_WriteData(' ');
		//LCD_Cursor(enemy_cursor_1-1);
		//LCD_WriteData(' ');
		enemy_cursor_2 -=1;
		LCD_Cursor(enemy_cursor_2);
		LCD_WriteData(enemy_char_2);
		break;
	}
	}//if game_on
	return enemy_state_2;
}

unsigned char score = 0;
unsigned char timer = 0;
enum SCORE_STATES{SCORE_START,SCORE_INIT,SCORE,}score_state;

int score_tick(int score_state){
	if(game_on){
	switch(score_state){
		case SCORE_START:
		score_state = SCORE_INIT;
		break;
		case SCORE_INIT:
		score_state = SCORE;
		break;
		case SCORE:
		//if (score >= 15){
			//score_state = WON_GAME;

			//LCD_ClearScreen(1,"YOU WON !!!")
			//game_on = 0;
		//}
		//else{
			score_state = SCORE;
			
		//}
		break;
		
	}
	switch(score_state){
		case SCORE_START:
		break;
		case SCORE_INIT:
		score = 0;
		timer = 0;
		PORTB = score;
		break;
		case SCORE:
		timer ++;
		if (timer >= 20){
			if (score <15){
				score +=1;
				PORTB = score;
			}
			
			timer = 0;
		}
		
		break;
	}
	}//if game_on
	return score_state;
}

unsigned char restart_button;
unsigned char y = 0;
enum DETECTION_STATES{DETECT_START,DETECT_INIT,DETECT,COLLISION_DETECTED_OR_WON,RESET_STATES}detect_state;

int detection_tick(int detect_state){
	restart_button = ~PINA & 0x10;

	switch(detect_state){
		case DETECT_START:
			detect_state = DETECT_INIT;
			break;
		case DETECT_INIT:
			detect_state = DETECT;
			break;
		case DETECT:
			if (restart_button == 0x10){
				detect_state = RESET_STATES;
			}
			else if (player_cursor == enemy_cursor_1){
				LCD_DisplayString(1,"GAME_OVER");
				game_on = 0;			
				detect_state = COLLISION_DETECTED_OR_WON;
			}
			else if (player_cursor == enemy_cursor_2){
				LCD_DisplayString(1,"GAME_OVER");
				game_on = 0;
				
				detect_state = COLLISION_DETECTED_OR_WON;
			}
			else if (score >=15){
				LCD_DisplayString(1,"YOU WON!!");
				game_on = 0;
				
			}
			else{
				detect_state = DETECT;
			}
			break;
		case COLLISION_DETECTED_OR_WON:
			if (restart_button == 0x10){
				detect_state = RESET_STATES;
			}
			else{
				detect_state = COLLISION_DETECTED_OR_WON;
			}
			break;
		case RESET_STATES:
			detect_state = DETECT;
			break;

	}	
	switch(detect_state){
		case DETECT_START:
			break;
		case DETECT_INIT:
			restart_button = 0;
			break;
		case DETECT:
			break;
		case COLLISION_DETECTED_OR_WON:
			//LCD_ClearScreen();
			break;
		case RESET_STATES:
			y = 0;
			tasks[y].state = MENU_START;
			y++;
			tasks[y].state = ENEMY_START_1;
			y++;
			tasks[y].state = ENEMY_START_2;
			y++;
			tasks[y].state = PLAYER_START;
			y++;
			tasks[y].state = SCORE_START;
			game_on = 1;
			break; 
	}
	return detect_state;
}




int main(void)
{
	//LCD_init();
	
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	//DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	
	DDRA = 0x03; PORTA = 0xFC;

	DDRD = 0xFF; PORTD = 0x00;

	unsigned char i = 0;
	tasks[i].state = MENU_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &menu_tick;
	i++;
	tasks[i].state = ENEMY_START_1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &enemy_tick_1;
	i++;
	tasks[i].state = ENEMY_START_2;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = & enemy_tick_2;
	i++;
	tasks[i].state = PLAYER_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &player_tick;
	i++;
	tasks[i].state = SCORE_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &score_tick;
		
	i++;
	tasks[i].state = DETECT_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &detection_tick;

	
	/*
	i++;
	tasks[i].state = 
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = & 
	*/

	TimerSet(tasksPeriod);
	TimerOn();
	
	//LCD_init();
	//LCD_Cursor(8);
	//LCD_WriteData(player_char);
	
	while(1) {
	//LCD_Cursor(2);
	//LCD_WriteData('P');
	//PORTB = 0x02;
///*
	//PORTB = 0x03;
	}	
}





