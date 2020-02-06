/*
 * MorseCodeTranslator.h
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#ifndef MORSECODETRANSLATOR_H_
#define MORSECODETRANSLATOR_H_

#include "LinkedList.h"

void Translate(uint32_t *morseCode, uint32_t *count);
char TranslateChar(uint8_t val, uint8_t pos);

void ButtonPress(uint8_t ButtonStatus, uint8_t timeDiffrence);

#endif /* MORSECODETRANSLATOR_H_ */
