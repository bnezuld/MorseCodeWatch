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
char TranslateChar(int val, int pos);

#endif /* MORSECODETRANSLATOR_H_ */
