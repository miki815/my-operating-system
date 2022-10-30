/*
 * thread.cpp
 *
 *  Created on: Jul 17, 2021
 *      Author: OS1
 */


#include "thread.h"

#include "SCHEDULE.h"
#include <iostream.h>

#include "pcb.h"
extern volatile PCB* running;
extern volatile unsigned int lockFlag;
extern void interrupt timer();
extern int context_switch_on_demand;

Thread::Thread(StackSize stackSize, Time timeSlice) : myPCB(0){
	lockFlag = 0;
	if(stackSize < 128) stackSize = 128;
	if(stackSize > 65536) stackSize = 65536;
	myPCB = new PCB(stackSize, timeSlice, this);
	lockFlag = 1;
}

Thread::~Thread(){
	lockFlag = 0;
	delete myPCB;
	lockFlag = 1;
}

void Thread::start(){
	lockFlag = 0;
	if(myPCB->parent == 0) myPCB->createStack();
	myPCB->status = READY;
    Scheduler::put(myPCB);
	lockFlag = 1;
}

ID Thread::getId(){
	return myPCB->id;
}

ID Thread::getRunningId(){
	return running->id;
}

Thread* Thread::getThreadById(ID id){
	lockFlag = 0;
	Node* head = PCB::PCBhead;
	while(head){
		if(head->pcb->id == id && head->pcb->status != FINISHED)
		{lockFlag = 1; return head->pcb->myThread;}
		head = head->next;
	}
	lockFlag = 1;
	return 0;
}



void dispatch(){
	asm cli;
	context_switch_on_demand = 1;
	lockFlag = 1;
	asm int 8h;
	asm sti;
}


void Thread::waitToComplete(){
	lockFlag = 0;
	Node* waitHead, *tmp;
	ID id = this->getId();
	if(id == running->id){lockFlag = 1; return;}
	for(tmp = running->pcbWait;tmp;tmp=tmp->next){
		if(tmp->pcb->id == id) {lockFlag = 1; return;} // deadlock
	}
	if(myPCB->status == FINISHED || myPCB->status == CREATED) {
		lockFlag = 1;
		return;
	}
	waitHead = myPCB->pcbWait;
	Node* el = new Node();
	el->pcb = (PCB*) running; el->next = 0;
	if(!waitHead) myPCB->pcbWait = el;
	else{
		Node* tail = waitHead;
		while(tail->next) tail = tail->next;
		tail->next = el;
	}
	running->status = BLOCKED;
	lockFlag = 1;
	dispatch();
}


// fork
volatile PCB* childPcb;
volatile int ret = 0;
ID Thread::fork(){
	lockFlag = 0;
	ret = 0;
	Thread* childT = 0;
	childT = running->myThread->clone();
	if(childT == 0 || childT->myPCB == 0 || childT->myPCB->status == FAILED)
		{lockFlag = 1; return -1;}
	childPcb = childT->myPCB;
	ID childId = childT->myPCB->id;
	PCB::forkPcb();
	if(ret != 0) {lockFlag = 1; return -1;}

	lockFlag = 1;
	return running->id == childId ? 0 : childId;
}

void Thread::exit(){
	lockFlag = 0;
	PCB::finish();
	lockFlag = 1;
}

void Thread::waitForForkChildren(){
	lockFlag = 0;
	PCB::forkWait();
	lockFlag = 1;
}

Thread* Thread::clone() const{
	return new Thread(myPCB->stackSize, myPCB->timeSlice);
}
