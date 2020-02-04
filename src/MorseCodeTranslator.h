/*
 * MorseCodeTranslator.h
 *
 *  Created on: Feb 4, 2020
 *      Author: bnezu
 */

#ifndef MORSECODETRANSLATOR_H_
#define MORSECODETRANSLATOR_H_

#include <stdint.h>
#include "LinkedList.h"

const uint32_t BEEP_TICK_LENGTH = 500;
const uint32_t SPACE_TICK_LENGTH = 500;

void Translate(struct node *morseCode, char *word);

#endif /* MORSECODETRANSLATOR_H_ */
