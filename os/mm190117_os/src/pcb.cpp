/*
 * pcb.cpp
 *
 *  Created on: Jul 17, 2021
 *      Author: OS1
 */

#include "pcb.h"

#include <dos.h>
#include <iostream.h>
#include <string.h>
extern volatile unsigned int lockFlag;
extern volatile PCB* running;

ID PCB::IDs = 0;
Node* PCB::PCBhead = 0;


PCB::PCB(StackSize s, Time t, Thread* th):stackSize(s), timeSlice(t)
, myThread(th), stack(0), status(CREATED), pcbWait(0), sleep(-1)
, childsCnt(0), parent(0), childSem(0), isWaiting(0) {
	id = IDs++;
	// add to all pcbs list
	Node* elem = 0;
	elem = new Node();
	if(elem){
		elem->pcb = this;
		elem->next = 0;
		if(!PCBhead) PCBhead = elem;
		else {
			Node* tail = PCBhead;
			while(tail->next) tail = tail->next;
			tail->next = elem;
		}
	}
	else status = FAILED;
}

void PCB::finish(){
	if(!(running->status == READY || running->status == BLOCKED)) return;
	lockFlag = 0;
	running->status = FINISHED;
	Node* waitHead = running->pcbWait;
	Node* tmp;
	while(waitHead){
		tmp = waitHead;
		waitHead->pcb->status = READY;
		Scheduler::put(waitHead->pcb);
		waitHead = waitHead->next;
		tmp->next = 0;
		delete tmp;
	}
	PCB* parent = running->parent;
	if(parent) {  // fork update
		parent->childsCnt -= 1;
		if(parent->isWaiting) parent->childSem->signal();
	}

	lockFlag = 1;
	dispatch();
}


void PCB::wrapper(){
	running->myThread->run();
	PCB::finish();
}

void PCB::createStack(){
	stackSize /= sizeof(unsigned);
	stack = new unsigned[stackSize];
	stack[stackSize-1] =0x200;
	#ifndef BCC_BLOCK_IGNORE
	stack[stackSize-2] = FP_SEG(PCB::wrapper);
	stack[stackSize-3] = FP_OFF(PCB::wrapper);
	stack[stackSize-12] = 0;
	sp = FP_OFF(stack+stackSize - 12);
	ss = FP_SEG(stack+stackSize - 12);
	bp = sp;
	#endif
}

PCB::~PCB(){
	Node* tmp = PCB::PCBhead;
	Node* prev = 0;
	if(tmp && tmp->pcb == this) {
		PCB::PCBhead = tmp->next;
	} else{
		while(tmp->pcb != this) prev = tmp, tmp = tmp->next;
		prev->next = tmp->next;
	}
	delete tmp;
	delete[] stack;
}


extern PCB* childPcb;
extern int ret;
unsigned* parPtr, *childPtr;
unsigned parOff, childOff;

void PCB::forkWait(){
	running->isWaiting = 1;
	while(running->childsCnt) running->childSem->wait(0);
	running->isWaiting = 0;
}

void interrupt PCB::forkPcb(){
	childPcb->stack = new unsigned[childPcb->stackSize];
	if(!childPcb->stack) {ret = -1; return;}

	if(running->childSem == 0) running->childSem = new Semaphore(0);
	if(running->childSem == 0) {ret = -1; return;}

	childPcb->parent = running->myThread->myPCB; // = running
	running->childsCnt += 1;
	memcpy(childPcb->stack, running->stack, running->stackSize * sizeof(unsigned));
	//#ifndef BCC_BLOCK_IGNORE

#ifndef BCC_BLOCK_IGNORE
	childPcb->ss = FP_SEG(childPcb->stack);
	parOff = FP_OFF(running->stack);
	childOff = FP_OFF(childPcb->stack);
#endif
	childPcb->bp = _BP - parOff + childOff;
	childPcb->sp = childPcb->bp;

	parOff = _BP;
	childOff = childPcb->bp;
	while (1) {
		#ifndef BCC_BLOCK_IGNORE
		parPtr = (unsigned*)MK_FP(_SS, parOff);
		childPtr = (unsigned*)MK_FP(childPcb->ss, childOff);
		#endif
		if (*parPtr == 0) break;
		*childPtr = *parPtr - parOff + childOff;
		parOff = *parPtr;
		childOff = *childPtr;
	}
	*childPtr = 0;

	childPcb->myThread->start();
}


