/*
 * MorseCodeTranslator.c
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#include "MorseCodeTranslator.h"
//#include <string.h>

const uint32_t BEEP_TICK_LENGTH = 400;
const uint32_t SPACE_TICK_LENGTH = 400;

const char MorseCodeTable0[] = {'E','T'};
const char MorseCodeTable1[] = {'I','A','N','M'};
const char MorseCodeTable2[] = {'S','U','R','W','D','K','G','O'};
const char MorseCodeTable3[] = {'H','V','F','-','L','-','P','J','B','X','C','Y','Z','Q'};

void Translate(uint32_t *morseCode, uint32_t *count)
{
	uint32_t tmpCount = 0;
	char c[300];
	uint8_t stringCount = 0;
	uint8_t position = 1;
	uint8_t morseCodeValue = 0;
	while(tmpCount != *count)
	{
		if(tmpCount % 2 == 0){//a beep
			if(morseCode[tmpCount]/BEEP_TICK_LENGTH > 3){//dash
				morseCodeValue += position;
			}
			position = position << 1;
		}else{//a space
			uint32_t i = morseCode[tmpCount]/SPACE_TICK_LENGTH;
			if(i > 3){//next letter
				c[stringCount++] = TranslateChar(morseCodeValue, position >> 1);
				morseCodeValue = 0;
				position = 1;
			}else if (i > 7){//next word

			}
		}
		tmpCount++;
	}
	*count = 0;
}

char TranslateChar(int val, int pos)
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

