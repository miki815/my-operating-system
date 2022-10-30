/*
 * Pcb.h
 *
 *  Created on: Jul 17, 2021
 *      Author: OS1
 */

#ifndef PCB_H_
#define PCB_H_

#include "SCHEDULE.h"
#include "thread.h"
#include "semaphor.h"

enum Status{CREATED, READY, BLOCKED, FINISHED, FAILED};

class Node {
public:
	PCB* pcb;
	Node* next;
};

class PCB{
private:
	static ID IDs;
public:
	unsigned ss, sp, bp;
	ID id;
	unsigned* stack;
	Status status;
	int sleep;
	int isWaiting;
	int childsCnt;
	PCB* parent;
	Semaphore* childSem;
	static void interrupt forkPcb();

	Thread* myThread;
	Time timeSlice;
	StackSize stackSize;

	static Node* PCBhead;
	Node* pcbWait;

	void createStack();

	PCB(StackSize stackSize, Time timeSlice, Thread* thread);
	~PCB();

	static void wrapper();
	static void finish();
	static void forkWait();
};



#endif /* PCB_H_ */
