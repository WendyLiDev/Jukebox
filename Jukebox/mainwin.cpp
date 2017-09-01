/* Name & E-mail: Wendy Li wli025@ucr.edu
 * Lab Section: 021
 * Assignment: Lab 10
 * Exercise Description: Jukebox, A0 to choose next song, A1 to choose previous song, A2 to play and stop
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <string.h>
/*-------------------------------------------------------------------------*/
#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTD	// port connected to pins 4 and 6 of LCD disp.
#define RS 6			// pin number of uC connected to pin 4 of LCD disp.
#define E 7			// pin number of uC connected to pin 6 of LCD disp.
/*-------------------------------------------------------------------------*/
#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

#define	C0  16.35
#define	Db0	17.32
#define	D0	18.35
#define	Eb0	19.45
#define	E0	20.60
#define	F0	21.83
#define	Gb0	23.12
#define	G0	24.50
#define	Ab0	25.96
#define	LA0	27.50
#define	Bb0	29.14
#define	B0	30.87
#define	C1	32.70
#define	Db1	34.65
#define	D1	36.71
#define	Eb1	38.89
#define	E1	41.20
#define	F1	43.65
#define	Gb1	46.25
#define	G1	49.00
#define	Ab1	51.91
#define	LA1	55.00
#define	Bb1	58.27
#define	B1	61.74
#define	C2	65.41
#define	Db2	69.30
#define	D2	73.42
#define	Eb2	77.78
#define	E2	82.41
#define	F2	87.31
#define	Gb2	92.50
#define	G2	98.00
#define	Ab2	103.83
#define	LA2	110.00
#define	Bb2	116.54
#define	B2	123.47
#define	C3	130.81
#define	Db3	138.59
#define	D3	146.83
#define	Eb3	155.56
#define	E3	164.81
#define	F3	174.61
#define	Gb3	185.00
#define	G3	196.00
#define	Ab3	207.65
#define	LA3	220.00
#define	Bb3	233.08
#define	B3	246.94
#define	C4	261.63
#define	Db4	277.18
#define	D4	293.66
#define	Eb4	311.13
#define	E4	329.63
#define	F4	349.23
#define	Gb4	369.99
#define	G4	392.00
#define	Ab4	415.30
#define	LA4	440.00
#define	Bb4	466.16
#define	B4	493.88
#define	C5	523.25
#define	Db5	554.37
#define	D5	587.33
#define	Eb5	622.25
#define	E5	659.26
#define	F5	698.46
#define	Gb5	739.99
#define	G5	783.99
#define	Ab5	830.61
#define	LA5	880.00
#define	Bb5	932.33
#define	B5	987.77
#define	C6	1046.50
#define	Db6	1108.73
#define	D6	1174.66
#define	Eb6	1244.51
#define	E6	1318.51
#define	F6	1396.91
#define	Gb6	1479.98
#define	G6	1567.98
#define	Ab6	1661.22
#define	LA6	1760.00
#define	Bb6	1864.66
#define	B6	1975.53
#define	C7	2093.00
#define	Db7	2217.46
#define	D7	2349.32
#define	Eb7	2489.02
#define	E7	2637.02
#define	F7	2793.83
#define	Gb7	2959.96
#define	G7	3135.96
#define	Ab7	3322.44
#define	LA7	3520.01
#define	Bb7	3729.31
#define	B7	3951.07
#define	C8	4186.01
#define	Db8	4434.92
#define	D8	4698.64
#define	Eb8	4978.03

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column, const char* string);
void delay_ms(int miliSec);

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}
void LCD_init(void) {

	//wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}
void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); // ClearScreen requires 1.52ms to execute
}
void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}
void LCD_DisplayString( unsigned char column, const char* string) {
	LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}
void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}
void delay_ms(int miliSec) {//for 8 Mhz crystal
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

volatile unsigned char TimerFlag = 0; //TimerISR() sets this to 1, we need to clear to 0

unsigned long _avr_timer_M = 1; //start count from here to 0, default 1 ms
unsigned long _avr_timer_cntcurr = 0; // current internal count of 1 ms ticks

unsigned char GetBit(unsigned char x, unsigned char k){
	return ((x & (0x01 << k)) != 0);
}
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b){
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
void TimerOn(){
    //Initialize and start the timer
	//AVR timer/counter controller register TCCR1
	/*
     bit3 = 0: CTC mode(clear timer on compare)
     bit2-bit1-bit0 = 011: pre-scaler/64
     00001011: 0x0B
     So 8 MHz clock or 8000000 / 64 = 125000 ticks/s
     So TCNT1 register will count at 125000 ticks/s
     */
	TCCR1B = 0x0B;
    
	//AVR output compare register OCR1A
	/*
     Timer interrupt will be generated when TCNT1 == OCR1A
     Want a 1 ms tick; .001 s * 125000 ticks/s = 125
     So when TCNT1 register equals 125, 1 ms has passed
     So we compare to 125
     */
	OCR1A = 125;
    
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A: enable compare match interrupt
    
	//init avr counter
	TCNT1 = 0;
    
	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR called every _avr_timer_cntcurr milliseconds
    
	//enable global interrupts
	SREG |= 0x80;
}
void TimerOff(){
    //Stop the timer
	TCCR1B = 0x00; //timer off bc 0
}
void TimerISR(){
    //Auto-call when the timer ticks, with the contents filled by the user ONLY with an instruction that sets TimerFlag = 1
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
    //Interrupt
	_avr_timer_cntcurr--; //count down to 0
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR(); //call ISR that user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M){
    //Set the timer to tick every M ms
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void set_PWM(double frequency) {
    // 0.954 hz is lowest frequency possible with this function,
    // based on settings in PWM_on()
    // Passing in 0 as the frequency will stop the speaker from generating sound
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
        
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
        
		// prevents OCR3A from underflowing, using prescaler 64
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
        
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
        
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}
void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

static bool songDone = 0;
static bool req = 0;// used in waitPress to tell playSong when to begin playing song
static char currSong = -1;
static unsigned char currNote = 0;
const char numSongs = 4;
const char* songList[] = { "Happy Birthday", "Yankee Doodle", "Jingle Bells", "Twinkle Twinkle" };
char* disp; // For displaying what is playing 
const double songData[4][150] = { { C4, 0, C4, 0, D4, D4, 0, C4, C4, 0, F4, F4, 0, E4, E4, E4, E4, 0,
									C4, 0, C4, 0, D4, D4, 0, C4, C4, 0, G4, G4, 0, F4, F4, F4, F4, 0,
									C4, 0, C4, 0, C5, C5, 0, LA4, LA4, 0, F4, F4, 0, E4, E4, 0, F4 ,F4, F4, F4, 1 }, 
								 {	G4, 0, G4, 0, LA4, 0, B4, 0, G4, 0, B4, 0, LA4, LA4, 0,
									G4, 0, G4, 0, LA4, 0, B4, 0, G4, G4, 0, Gb4, Gb4, 0,
									G4, 0, G4, 0, LA4, 0, B4, 0, C5, 0, B4, 0, LA4, 0, G4, 0, Gb4, 0, D4, 0, E4, 0, Gb4, 0, G4, G4, 0, G4, G4, 1 }, 
							     {	E4, 0, E4, 0, E4, E4, 0, E4, 0, E4, 0, E4, E4, 0, E4, 0, G4, 0, C4, 0, D4, 0, E4, E4, 0,
								    F4, 0, F4, 0, F4, 0, F4, 0, F4, 0, E4, 0, E4, 0, E4, 0, E4, 0, D4, 0, D4, 0 ,E4, 0, D4, D4, 0, G4, G4, 1 },
								 {	C4, 0, C4, 0, G4, 0, G4, 0, LA4, 0, LA4, 0, G4, G4, 0,
									F4, 0, F4, 0, E4, 0, E4, 0, D4, 0, D4, 0, C4, C4, 0,
									G4, 0 ,G4, 0, F4, 0, F4, 0, E4, 0, E4, 0, D4, D4, 0,
									G4, 0 ,G4, 0, F4, 0, F4, 0, E4, 0, E4, 0, D4, D4, 0,
									C4, 0, C4, 0, G4, 0, G4, 0, LA4, 0, LA4, 0, G4, G4, 0,
									F4, 0, F4, 0, E4, 0, E4, 0, D4, 0, D4, 0, C4, C4, 1}
							   }; //songDone will be denoted with a frequency note of 1 at the end of each song
								
						
enum waitPressStates{wait_init, wait_wait, wait_press, wait_rel}waitPress_State;
void waitPress(){
	switch(waitPress_State){
		case wait_init:
			waitPress_State = wait_wait;
			break;
		case wait_wait:
			if(!GetBit(~PINA, 2)){
				waitPress_State = wait_wait;
			}
			else if(GetBit(~PINA, 2)){
				waitPress_State = wait_press;
			}
			break;
		case wait_press:
			if(!GetBit(~PINA, 2)){
				waitPress_State = wait_wait;
			}
			else if(GetBit(~PINA, 2)){
				waitPress_State = wait_rel;
			}
			break;
		case wait_rel: 
			if(!GetBit(~PINA, 2)){
				waitPress_State = wait_wait;
			}
			else if(GetBit(~PINA, 2)){
				waitPress_State = wait_rel;
			}
			break;
	}
	switch(waitPress_State){
		case wait_init:
			req = 0;
			break;
		case wait_wait:
			req = 0;
			break;
		case wait_press:
			req = 1;
			break;
		case wait_rel:
			//req = 0;
			break;
	}
}

enum playSongs{play_init, play_wait, play_play, play_stop}playSong_State;
void playSong(){
    switch(playSong_State){
	    case play_init:
			playSong_State = play_wait;
			break;
	    case play_wait:
			if(!req){
				playSong_State = play_wait;
			}
			else if(req){
				playSong_State = play_play;
			}
			break;
	    case play_play:
			if(req){ // if req is up again, stop song
				playSong_State = play_stop;
			}
			else if(!songDone && !req){ // continue to play song
				playSong_State = play_play;
			}
			else if(songDone){ // if song is finished playing, return to wait state
				playSong_State = play_wait;
			}
			break;
	    case play_stop:
			playSong_State = play_wait;
			break;
    }
	switch(playSong_State){
		case play_init:
			songDone = 0;
			currNote = 0;
			PWM_on();
			break;
		case play_wait:
			PORTD = SetBit(~PORTD, 0, 0);
			PORTD = SetBit(~PORTD, 1, 0);
			PORTD = SetBit(~PORTD, 2, 0);
			PORTD = SetBit(~PORTD, 3, 0);
			songDone = 0;
			set_PWM(0);
			break;
		case play_play:
			LCD_DisplayString(1, songList[currSong]);
			if(songData[currSong][currNote] == 1){
				songDone = 1;
			}
			if(!songDone){
				set_PWM(songData[currSong][currNote]);
				if(songData[currSong][currNote] == C4 || songData[currSong][currNote] == D4){
					PORTD = SetBit(~PORTD, 0, 1);
					PORTD = SetBit(~PORTD, 1, 0);
					PORTD = SetBit(~PORTD, 2, 0);
					PORTD = SetBit(~PORTD, 3, 0);
				}
				else if(songData[currSong][currNote] == E4 || songData[currSong][currNote] == F4){
					PORTD = SetBit(~PORTD, 0, 0);
					PORTD = SetBit(~PORTD, 1, 1);
					PORTD = SetBit(~PORTD, 2, 0);
					PORTD = SetBit(~PORTD, 3, 0);
				}
				else if(songData[currSong][currNote] == G4 || songData[currSong][currNote] == LA4){
					PORTD = SetBit(~PORTD, 0, 0);
					PORTD = SetBit(~PORTD, 1, 0);
					PORTD = SetBit(~PORTD, 2, 1);
					PORTD = SetBit(~PORTD, 3, 0);
				}
				else if(songData[currSong][currNote] == B4 || songData[currSong][currNote] == C5){
					PORTD = SetBit(~PORTD, 0, 0);
					PORTD = SetBit(~PORTD, 1, 0);
					PORTD = SetBit(~PORTD, 2, 0);
					PORTD = SetBit(~PORTD, 3, 1);
				}
				++currNote;
			}
			break;
		case play_stop:
			set_PWM(0);
			currNote = 0;
			break;
	}
	
}

enum selectSongs{select_init, select_wait, select_next, select_waitrel1, select_prev, select_waitrel2}selectSong_State;

void selectSong(){
	switch(selectSong_State){
        case select_init:
            selectSong_State = select_wait;
            break;
        case select_wait:
            if(GetBit(~PINA, 0) && !GetBit(~PINA, 1)){
                selectSong_State = select_next;
            }
            else if(!GetBit(~PINA, 0) && GetBit(~PINA, 1)){
                selectSong_State = select_prev;
            }
            else{
                selectSong_State = select_wait;
            }
            break;
        case select_next:
            if(GetBit(~PINA, 0) && !GetBit(~PINA, 1)){
                selectSong_State = select_waitrel1;
            }
            else{
                selectSong_State = select_wait;
            }
            break;
        case select_waitrel1:
            if(GetBit(~PINA, 0) && !GetBit(~PINA, 1)){
                selectSong_State = select_waitrel1;
            }
            else{
                selectSong_State = select_wait;
            }
            break;
        case select_prev:
            if(!GetBit(~PINA, 0) && GetBit(~PINA, 1)){
                selectSong_State = select_waitrel2;
            }
            else{
                selectSong_State = select_wait;
            }
            break;
        case select_waitrel2:
            if(!GetBit(~PINA, 0) && GetBit(~PINA, 1)){
                selectSong_State = select_waitrel2;
            }
            else{
                selectSong_State = select_wait;
            }
            break;
        default:
            selectSong_State = select_wait;
			break;
	}
	switch(selectSong_State){
		case select_init:
            currSong = -1;
            //Display "Select Song"
            LCD_DisplayString(1, "Select Song");
            break;
        case select_next:
            if(currSong != numSongs-1 ){ 
                ++currSong;
            }
			currNote = 0;
			LCD_DisplayString(1, songList[currSong]);
			
            break;
        case select_prev:
            if(currSong != 0){
                --currSong;
            }
			currNote = 0;
			LCD_DisplayString(1, songList[currSong]);
            break;
        default:
			break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	TimerSet(100);
	TimerOn();
	
	waitPress_State = wait_init;
	selectSong_State = select_init;
    playSong_State = play_init;
	
	
	LCD_init();
	LCD_DisplayString(1, "Select Song");
	
	PWM_on();
    while(1)
    {
		waitPress();
		selectSong();
        playSong();
		
		while(!TimerFlag);
		TimerFlag = 0;
		continue;
    }
    return 0;
}