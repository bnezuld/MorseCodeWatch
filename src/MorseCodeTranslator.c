/*
 * MorseCodeTranslator.c
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#include "MorseCodeTranslator.h"
//#include <string.h>

const uint32_t BEEP_TICK_LENGTH = 500;
const uint32_t SPACE_TICK_LENGTH = 500;

void Translate(struct node *morseCode, char *word)
{
	//struct node *Head = morseCode;
	uint32_t count = 0;
	//int charSize = 0;
	while(morseCode != 0)
	{
		char tmp;
		if(count % 2 == 0){//a beep
			tmp = (char)((morseCode->data/BEEP_TICK_LENGTH) + 48);
		}else{//a space
			tmp = morseCode->data/SPACE_TICK_LENGTH > 3? '-':' ';
		}
		strncat(word, tmp, 1);

		count++;
		morseCode = morseCode->next;
	}
}

