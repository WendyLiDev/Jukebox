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
#include <string.h>
#include "io.c"


//const double noteFreq[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25}; // array of notes to play
//static char currFreqIndex = 0;
//static bool sound;

static unsigned char currSong = 0;
const string StartMessage = "Select Song";
static bool songDone = 0;

const double *arrayOfArrays[];

const double HappyBirthday[] = {392.00, 392.00, 440.00, 392.00, 261.63, 493.88, 0,
                                    392.00, 392.00,  440.00, 392.00, 293.66, 261.63, 0,
                                    392.00, 392.00, 392.00, 329.63, 261.63};
const double YankeeDoodle[] = {};
const string songList[] = {"Happy Birthday", "Yankee Doodle" };
arrayOfArrays[0] = HappyBirthday;
arrayOfArrays[1] = YankeeDoodle;
unsigned char GetBit(unsigned char x, unsigned char k){
	return ((x & (0x01 << k)) != 0);
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
            LCD_init();
            //Display "Select Song"
            LCD_DisplayString(1, "Select Song");
            break;
        case select_next:
            if(currSong != (songList.size()-1)){
                ++currSong;
            }
            LCD_DisplayString(1, songList[currSong]); //Update LCD song display
            break;
        case select_prev:
            if(currSong != 0){
                --currSong;
            }
            LCD_DisplayString(1, songList[currSong]); //Update LCD song display
            break;
        default:
			break;
	}
}

int main(void)
{
	selectSong_State = select_init;
    playSong_State = play_init;
    while(1)
    {
		selectSong();
        playSong();
    }
    return 0;
}