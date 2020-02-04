/*
 * LinkedList.c
 *
 *  Created on: Jan 10, 2020
 *      Author: bnezu
 */

#include "LinkedList.h"

struct node* initializeNode(uint32_t data)
{
	struct node* headNode = (struct node*) malloc(sizeof(struct node));

	headNode->data = data;
	headNode->next = 0;

	return headNode;
};


void addNode(uint32_t data, struct node* head)
{
	struct node* newNode = initializeNode(data);

	head->next = newNode;
}

void freeList(struct node* head)
{
   struct node* tmp;

   while (head != 0)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }

}

