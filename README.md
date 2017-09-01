# Jukebox
Play from a selection of songs through a speaker. The song title will be displayed on the LCD screen. The LED's connected will light up and dance with the song.

## Controls:
- To select a song, use buttons connected to A0 and A1 to choose a song. A0 selects the next song while A1 will select the previous song.
- To play your selected song, press the button connected to A2. To stop the song, press the button connected to A2 again.
- To speed up the song, press A3. Note that there is a maximum speed to which a song can be set.

## Technologies and components: 
- AVR Studio 6
- ATmega1284
- Speaker
- 4x Push Button
- 4x 330 Ohm Resistor
- 4x 5mm LED
- LCD Screen
- 10k potentiometer 
- Power adapter

## Video Demonstration: 
https://www.youtube.com/watch?v=5l4ot1A8iqE

## How I wired my board:
In the folder labeled "images", there are images of my board setup.
- Buttons are connected to PA0 through PA3
- LEDs are connected to PD0 through PD3
- LCD is connected to Port C
- Speaker is connected to PB6

## Source Code: 
https://github.com/wendingoli/Jukebox/blob/master/Jukebox/main.c <br />
The source code that controls the entire program.

## Used outer sources: 
http://discuss.littlebits.cc/t/playing-a-melody-using-a-button-a-counter/25035 <br />
I used this code's note definitions for my program.
