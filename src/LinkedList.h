/*
 * LinkedList.h
 *
 *  Created on: Jan 10, 2020
 *      Author: bnezu
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdint.h>

struct node
{
    // Any data type can be stored in this node
	uint32_t  data;

    struct node *next;
};

struct node* initializeNode(uint32_t data);

void addNode(uint32_t data, struct node* head);

void freeList(struct node* head);

#endif /* LINKEDLIST_H_ */
