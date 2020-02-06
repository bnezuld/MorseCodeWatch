/*
 * MorseCodeTranslator.c
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#include "MorseCodeTranslator.h"
//#include <string.h>

#define BEEP_TICK_LENGTH 200
#define SPACE_TICK_LENGTH 200

const char MorseCodeTable0[] = {'E','T'};
const char MorseCodeTable1[] = {'I','A','N','M'};
const char MorseCodeTable2[] = {'S','U','R','W','D','K','G','O'};
const char MorseCodeTable3[] = {'H','V','F','-','L','-','P','J','B','X','C','Y','Z','Q'};

#define MAX_MORSECODE 50
uint32_t button[MAX_MORSECODE];// = {102, 91, 115, 86, 122, 861, 2316, 29, 2134, 34, 1869, 2958, 140, 217, 126, 194, 141};
uint32_t buttonCount = 0;

void Translate(uint32_t *morseCode, uint32_t *count)
{
	if(morseCode == 0 && count == 0)
	{
		morseCode = button;
		count = &buttonCount;
	}
	uint32_t tmpCount = 0;
	char c[25];
	uint8_t stringCount = 0;
	uint8_t position = 1;
	uint8_t morseCodeValue = 0;
	uint8_t translateChar;
	while(tmpCount != *count)
	{
		translateChar = 0;
		if(tmpCount % 2 == 0){//a beep
			if(morseCode[tmpCount]/BEEP_TICK_LENGTH >= 3){//dash
				morseCodeValue |= position;
			}
			position = position << 1;
		}else{//a space
			uint32_t i = morseCode[tmpCount]/SPACE_TICK_LENGTH;
			if(i >= 3){//next letter
				translateChar++;
			}
			if (i >= 7){//next word
				translateChar++;
			}
		}
		tmpCount++;
		if(translateChar > 0 || tmpCount == *count){
			//this is the last one so translate the char
			c[stringCount++] = TranslateChar(morseCodeValue, position >> 1);
			morseCodeValue = 0;
			position = 1;
		}
		if(translateChar > 1)
		{
			c[stringCount++] = ' ';
		}
	}
	*count = 0;
}

char TranslateChar(uint8_t val, uint8_t pos)
{
	switch(pos)
	{
	case 1:
		return MorseCodeTable0[val];
	case 2:
		return MorseCodeTable1[val];
	case 4:
		return MorseCodeTable2[val];
	case 8:
		return MorseCodeTable3[val];
	}
	return '-';
}

void ButtonPress(uint32_t timeDiffrence, uint8_t buttonStatus)
{
	if(buttonStatus == 0){// button released
		if(buttonCount != 0){
			button[buttonCount++] = timeDiffrence;
			if(buttonCount >= MAX_MORSECODE)
			{
				buttonCount = 0;
			}
		}
	}else{// button pressed
		if(timeDiffrence > 500 * 10)//a temporary measure to translate the morse code need some way to send the message
		{
			Translate(button,&buttonCount);
		}else
		{
			button[buttonCount++] = timeDiffrence;
			if(buttonCount >= MAX_MORSECODE)
			{
				buttonCount = 0;
			}
		}
	}
}

