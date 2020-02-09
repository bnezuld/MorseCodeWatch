/*
 * MorseCodeTranslator.c
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#include "MorseCodeTranslator.h"

/*could do this in a hashtable by using the 'position' to calculate the offset needed*/
const char MorseCodeTable0[] = {'E','T'};
const char MorseCodeTable1[] = {'I','A','N','M'};
const char MorseCodeTable2[] = {'S','U','R','W','D','K','G','O'};
const char MorseCodeTable3[] = {'H','V','F','-','L','-','P','J','B','X','C','Y','Z','Q'};

uint32_t BEEP_TICK_LENGTH = 100;
uint32_t SPACE_TICK_LENGTH = 130;

#define MAX_MORSECODE 100
uint32_t button[MAX_MORSECODE];
uint32_t buttonCount = 0;

char* TranslateSelf()
{
	return Translate(button,&buttonCount);
}

char* Translate(uint32_t *morseCode, uint32_t *count)
{
	uint32_t tmpCount = 0;
	char *c = malloc((*count/2 + 1) * sizeof(char));
	uint8_t stringCount = 0;
	uint8_t position = 1;
	uint8_t morseCodeValue = 0;
	uint8_t translateChar;
	while(tmpCount != *count)
	{
		translateChar = 0;
		if(tmpCount % 2 == 0){//a beep
			if(morseCode[tmpCount]/BEEP_TICK_LENGTH >= 2){//dash(using 2 to get a better range, as 3 units represents a space/dash)
				morseCodeValue |= position;
			}
			position = position << 1;
		}else{//a space
			uint32_t i = morseCode[tmpCount]/SPACE_TICK_LENGTH;
			if(i >= 2){//next letter(using 2 to get a better range, as 3 units represents a space/dash)
				translateChar++;
			}
			if (i >= 7){//next word
				translateChar++;
			}
		}
		tmpCount++;
		if(translateChar > 0 || tmpCount == *count){
			c[stringCount++] = TranslateChar(morseCodeValue, position >> 1);
			morseCodeValue = 0;
			position = 1;
		}
		if(translateChar > 1)
		{
			c[stringCount++] = ' ';
		}
	}
	c[stringCount] = '\0';
	*count = 0;
	return c;
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
//		if(timeDiffrence > 500 * 10)//a temporary measure to translate the morse code need some way to send the message
//		{
//			TranslateSelf();
//		}
//		else
//		{
			button[buttonCount++] = timeDiffrence;
			if(buttonCount >= MAX_MORSECODE)
			{
				buttonCount = 0;
			}
//		}
	}
}

