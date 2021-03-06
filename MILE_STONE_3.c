 /*	Lab Section: B21
 *  FINAL PROJECT
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  Richard "Dylan" McGee
 */

//#include <avr/limits.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
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

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

void CreateCustomCharacter(unsigned char location, unsigned char *ptr){
	unsigned char i;
	if(location<8){
		LCD_WriteCommand(0x40+(location*8));
		for(i=0;i<8;i++)
		LCD_WriteData(ptr[ i ]);
	}
	
}

//DUE TO EEEPROM THERE IS TROUBLE OUTPUTTING PLAYER_CHAR SO JUSt OUTPUTTING SAME CHAR TO STILL GEt COMPLEXITY
//FIGURE OUT THE EEPROM PROBLEM
unsigned char player_char[8] ={0x00,0x0A,0x0A,0x0A,0x00,0x11,0x0A,0x04};
unsigned char player_custom_loc = 1;//12

unsigned char enemy_char_1 [8] = {0x00,0x1B,0x0A,0x00,0x00,0x0D,0x11,0x00};
unsigned char enemy_char_loc = 4;

unsigned char game_ended_normal = 0;


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
			CreateCustomCharacter(player_custom_loc,player_char);
			CreateCustomCharacter(enemy_char_loc,enemy_char_1);
			break;
	}
	
	switch(menu_state){//actions
		case MENU_START:
			break;
		case MENU_INIT:
			//eeprom_write_byte((uint8_t*)1,0);
			//cursor = 1;
			ADC_init();
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
//LCD_WriteCommand(0x40);


/*
void CreateCustomCharacter(unsigned char *pucPattern,const char cLocation) {
	
	int iLoop=0;
	
	LCD_WriteCommand(0x40+(cLocation*8)); //Send the Address of CGRAM
	for(iLoop=0;iLoop<8;iLoop++){
	LCD_WriteData(pucPattern[iLoop]); //Pass the bytes of pattern on LCD
	//LCD_WriteCommand(pucPattern[iLoop]); //Pass the bytes of pattern on LCD
	}
}
*/



//unsigned char player_char = 'P';
unsigned char button_left = 0;
unsigned char button_right = 0;

enum PLAYER_STATES{PLAYER_START,PLAYER_INIT,WAIT_FOR_PLAYER_BUTTON,BUTTON_PRESS_LEFT,BUTTON_PRESS_RIGHT}player_state;

//INITIALIZING ENEMY_CHAR IN PLAYER INIT MAY CHANGE LATER
unsigned short x = 0;
unsigned short upper_bound =700;

unsigned short lower_bound = 300;
int player_tick(int player_state){
	if(game_on){
	button_left = ~PINA & 0x04;
	button_right = ~PINA & 0x08;
	x = ADC;
	//PORTB = (char)x;
	switch(player_state){
		case PLAYER_START:
			player_state = PLAYER_INIT;
			break;
		case PLAYER_INIT:
			player_state = WAIT_FOR_PLAYER_BUTTON;
			break;
		case WAIT_FOR_PLAYER_BUTTON:
			//LCD_DisplayString(1,"APPLES");
			//if (button_left == 0x04){
			if(x < lower_bound){	
				player_state = BUTTON_PRESS_LEFT;
				//PORTB = 0x01;
				///*
				//if (player_cursor !=1){
				//LCD_ClearScreen();
				//delay_ms(1);
				LCD_Cursor(player_cursor);
				LCD_WriteData(' ');

				player_cursor = 1;
				//CreateCustomCharacter(player_char,player_cursor);
				
				///*
				LCD_Cursor(player_cursor);
				//LCD_WriteData(player_char);
				LCD_WriteData(player_custom_loc);
				
				
				//*/
				/*
				*/
			}		
			//else if (button_right == 0x08){
			else if (x >= upper_bound){
				player_state = BUTTON_PRESS_RIGHT;
				//PORTB = 0x02;
				///*
				//if (player_cursor != 17){
				//LCD_ClearScreen();
				//delay_ms(1);
				LCD_Cursor(player_cursor);
				LCD_WriteData(' ');
				player_cursor = 17;
				
				///*
				LCD_Cursor(player_cursor);
				//LCD_WriteData(player_char);
				LCD_WriteData(player_custom_loc);
				
				//eeprom_write_byte((uint8_t*)4,player_cursor);
				//*/
			}

			else {
				player_state = WAIT_FOR_PLAYER_BUTTON;
				LCD_Cursor(player_cursor);
				//LCD_WriteData(player_char);
				LCD_WriteData(player_custom_loc);
				
			}

			break;
		case BUTTON_PRESS_LEFT:
			//if (button_left == 0x04){
			if (x < lower_bound){
				//PORTB = 0x04;
				player_state = BUTTON_PRESS_LEFT;
			}
			else {
				player_state = WAIT_FOR_PLAYER_BUTTON;
			}
			break;
		case BUTTON_PRESS_RIGHT:
			//if (button_right == 0x08){
			if (x >= upper_bound){
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
			
			if (game_ended_normal == 0){
				player_cursor = eeprom_read_byte((uint8_t*)6);
			}
			if (player_cursor == 17){
				LCD_Cursor(1);
				LCD_WriteData(' ');
			}
			LCD_Cursor(player_cursor);
			//LCD_WriteData(player_char);
			LCD_WriteData(player_custom_loc);
			
			//CreateCustomCharacter(player_char,player_cursor);
			break;
		case WAIT_FOR_PLAYER_BUTTON:
			eeprom_write_byte((uint8_t*)6,player_cursor);
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

unsigned char cursor_index_1 = 0;
unsigned char cursor_values_1 [14] = {16,16,12,10,7,16,13,14,9,15,14,12,8,16};


unsigned char enemy_cursor_1;

//unsigned char enemy_char_1 = '!';
	




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
		//tasks[1].period = 100;
		enemy_cursor_1 = 12;
		cursor_index_1 = 0;
		unsigned char temp = 2;
		if (game_ended_normal == 0){
		enemy_cursor_1 = eeprom_read_byte((uint8_t*)5);
		cursor_index_1 = eeprom_read_byte((uint8_t*)7);				
		}

		for(temp = 2; temp < 12; temp++){
			LCD_Cursor(temp);
			LCD_WriteData(' ');
		}
		break;
	case ENEMY_MOVE_1:
		//LCD_Cursor(enemy_cursor_1);
		//LCD_WriteData(' ');
		
		//if (enemy_cursor_1 <= 0){
		//	enemy_cursor_1 = 16;
		//}
		if (enemy_cursor_1 <=0){
			enemy_cursor_1 = cursor_values_1[cursor_index_1];
			if (cursor_index_1 >= 13){
				cursor_index_1 = 0;
			}
			else{
				cursor_index_1 ++;
			}
			//enemy_cursor_2 = 32;
		}
		//LCD_ClearScreen();
		//delay_ms(1);
		LCD_Cursor(enemy_cursor_1);
		LCD_WriteData(' ');
		//LCD_Cursor(enemy_cursor_1-1);
		//LCD_WriteData(' ');
		enemy_cursor_1 -=1;
		LCD_Cursor(enemy_cursor_1);
		//LCD_WriteData(enemy_char_1);
		LCD_WriteData(enemy_char_loc);
		eeprom_write_byte((uint8_t*)5,enemy_cursor_1);
		eeprom_write_byte((uint8_t*)7,cursor_index_1);
		break;		
	}
	}//if game_one
	return enemy_state_1;

}
unsigned char score = 0;

enum ENEMY_STATES_2{ENEMY_START_2,ENEMY_INIT_2,ENEMY_MOVE_2}enemy_state_2;

unsigned char cursor_index_2 = 0;
unsigned char cursor_values_2 [14] = {32,32,30,32,30,29,30,30,25,30,31,32,27,28};

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
			//tasks[2].period = 100;
			enemy_cursor_2 = 32;

			cursor_index_2 = 0;
			if (game_ended_normal == 0){
				enemy_cursor_2 = eeprom_read_byte((uint8_t*)4);
				cursor_index_2 = eeprom_read_byte((uint8_t*)8);				
			}
			//enemy_cursor_2  = 28;
		break;
		case ENEMY_MOVE_2:
			if (enemy_cursor_2 <=16){
				enemy_cursor_2 = cursor_values_2[cursor_index_2];
				if (cursor_index_2 >= 13){
					cursor_index_2 = 0;
				}
				else{
					cursor_index_2 ++;
				}
				//enemy_cursor_2 = 32;
				score ++;
				PORTB = score;
			}
			LCD_Cursor(enemy_cursor_2);
			LCD_WriteData(' ');
			//LCD_Cursor(enemy_cursor_1-1);
			//LCD_WriteData(' ');
			enemy_cursor_2 -=1;
			LCD_Cursor(enemy_cursor_2);
			//LCD_WriteData(enemy_char_2);
			LCD_WriteData(enemy_char_loc);
			//score = eeprom_read_byte((uint8_t*)3);
			eeprom_write_byte((uint8_t*)4,enemy_cursor_2);
			eeprom_write_byte((uint8_t*)8,cursor_index_2);
		break;
	}
	}//if game_on
	return enemy_state_2;
}

//unsigned char score = 0;
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
			
			if (game_ended_normal == 0){
				score = eeprom_read_byte((uint8_t*)3);	
			}
			
			PORTB = score;
		break;
		case SCORE:
			//timer ++;
			//if (timer >= 20){
			
			//if (enemy_cursor_2 == 16){
			//	if (score <15){
			//		score +=1;
			//	}
				
				//if (score == 7){
				//	tasks[1].period -=5;
				//	tasks[2].period -=5;
				//}	
				PORTB = score;
					
				eeprom_write_byte((uint8_t*)3,score);
				//score = eeprom_read_byte((uint8_t)3);
				//if (tasks[1].period)
				//tasks[1].period -=5;
				//tasks[2].period -=5;
				//timer = 0;
			//}
		
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
				LCD_DisplayString(1,"GAME OVER!");
				game_on = 0;			
				detect_state = COLLISION_DETECTED_OR_WON;
			}
			else if (player_cursor == enemy_cursor_2){
				LCD_DisplayString(1,"GAME OVER!");
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
			tasks[y].state = SCORE_START;
			y++;
			tasks[y].state = ENEMY_START_1;
			y++;
			tasks[y].state = ENEMY_START_2;
			y++;
			tasks[y].state = PLAYER_START;
			game_on = 1;

			game_ended_normal = 1;
			break; 
	}
	return detect_state;
}




int main(void)
{
	//LCD_init();
	
	DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
	//DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	
	DDRA = 0x06; PORTA = 0xF9;

	DDRD = 0xFF; PORTD = 0x00;

	unsigned char i = 0;
	tasks[i].state = -1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &menu_tick;
	i++;
	tasks[i].state = SCORE_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &score_tick;
	i++;
	tasks[i].state = ENEMY_START_1;
	tasks[i].period = 150;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &enemy_tick_1;
	i++;
	tasks[i].state = ENEMY_START_2;
	tasks[i].period = 150;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = & enemy_tick_2;
	i++;
	tasks[i].state = PLAYER_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &player_tick;
	i++;
	tasks[i].state = DETECT_START;
	tasks[i].period = 1;
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
	//LCD_DisplayString(1,"helloWORLD");
	while(1) {
	//x = ADC;
	//PORTB = (char)x;
	}	
}
