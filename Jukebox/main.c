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
//#include "io.c"

volatile unsigned char TimerFlag = 0; //TimerISR() sets this to 1, we need to clear to 0

unsigned long _avr_timer_M = 1; //start count from here to 0, default 1 ms
unsigned long _avr_timer_cntcurr = 0; // current internal count of 1 ms ticks

static unsigned char currSong = 0;
const char* StartMessage = "Select Song";
static bool songDone = 0;

const double *songData[2];

const double HappyBirthday[] = {    392.00, 392.00, 440.00, 392.00, 523.25, 493.88, 493.88, 0,
                                    392.00, 392.00, 440.00, 392.00, 587.33, 587.33, 523.25, 523.25 0,
                                    392.00, 392.00, 392.00, 659.25, 659.25, 523.25, 493.88, 440.00, 440.00, 0,
                                    698.46, 698.46, 659.25, 523.25, 587.33, 587.33, 523.25, 0
                                };

const double YankeeDoodle[] = {};
const char* songList[] = {"Happy Birthday", "Yankee Doodle" };
songData[0] = HappyBirthday;
songData[1] = YankeeDoodle;
unsigned char GetBit(unsigned char x, unsigned char k){
	return ((x & (0x01 << k)) != 0);
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

enum playSongs{play_init, play_wait, play_press, play_waitrel, play_play, play_stop, play_stop_wait_rel}playSong_State;

void playSong(){
    switch(playSong_State){
        case play_init:
            playSong_State = play_wait;
            break;
        case play_wait:
            if(GetBit(~PINA, 2)){
                playSong_State = play_press;
            }
            else{
                playSong_State = play_wait;
            }
            break;
        case play_press:
            if(GetBit(~PINA, 2)){
                playSong_State = play_waitrel;
            }
            else{
                playSong_State = play_play;
            }
            break;
        case play_waitrel:
            if(GetBit(~PINA, 2)){
                playSong_State = play_waitrel;
            }
            else{
                playSong_State = play_play;
            }
            break;
        case play_play:
            if(GetBit(~PINA, 2)){
                playSong_State = play_stop;
            }
            else if(!GetBit(~PINA, 2) && !songDone){
                playSong_State = play_play;
            }
            else{
                playSong_State = play_wait;
            }
            break;
        case play_stop:
            if(GetBit(~PINA, 2)){
                playSong_State = play_stop_wait_rel;
            }
            else{
                playSong_State = play_wait;
            }
            break;
        case play_stop_wait_rel:
            if(GetBit(~PINA, 2)){
                playSong_State = play_stop_wait_rel;
            }
            else{
                playSong_State = play_wait;
            }
            break;
    }
    switch(playSong_State){
        case play_init:
            songDone = 0;
            break;
        case play_wait:
            songDone = 0;
            break;
        case play_press:
            break;
        case play_waitrel:
            break;
        case play_play:
            for(int i = 0; i < songData[currSong].size()-1; ++i){
                set_PWM(songData[currSong].at(i));
            }
            songDone = 1;
            break;
        case play_stop:
            break;
        case play_stop_wait_rel:
            break;
    }
}

enum selectSongs{select_init, select_wait, select_next, select_waitrel1, select_prev, select_waitrel2}selectSong_State;

void selectSong(){
	switch(adjust_State){
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
	
	switch(adjust_State){
		case select_init:
            currSong = 0;
            //LCD_init();
            //LCD_DisplayString(1, "Select Song"); //Display "Select Song"
            break;
        case select_next:
            if(currSong != (songList.size()-1)){
                ++currSong;
            }
            //LCD_DisplayString(1, songList[currSong]); //Update LCD song display
            break;
        case select_prev:
            if(currSong != 0){
                --currSong;
            }
            //LCD_DisplayString(1, songList[currSong]); //Update LCD song display
            break;
        default:
			break;
	}
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	TimerSet(37.5);
	TimerOn();
    
	selectSong_State = select_init;
    playSong_State = play_init;
    while(1)
    {
		selectSong();
        playSong();
        
        while(!TimerFlag);
        TimerFlag = 0;
        //continue;
    }
    return 0;
}