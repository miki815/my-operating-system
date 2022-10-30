/*
 * kerSem.cpp
 *
 *  Created on: Jul 21, 2021
 *      Author: OS1
 */
#include "kerSem.h"
KernelSem::semNode* KernelSem::semHead = 0;
extern volatile PCB* running;
extern volatile unsigned int lockFlag;


KernelSem::KernelSem(int value) : semVal(value), classicWait(0), sleepWait(0), waitFlag(1){
	semNode* sem = new semNode();
	sem->ksem = this; sem->next = 0;
	if(!semHead) semHead = sem;
	else {
		semNode* cur = semHead;
		while(cur->next) cur = cur->next;
		cur->next = sem;
	}
}

int KernelSem::wait(Time waitTime){
	if(--semVal < 0){
		running->sleep = waitTime;
		Node* pcbWait = new Node();
		pcbWait->pcb = (PCB*) running; pcbWait->next = 0;
		Node* waitHead;
		int modFlag; //classic or sleep
		if(waitTime == 0) waitHead = this->classicWait, modFlag = 0;
		else waitHead = this->sleepWait, modFlag = 1;

		if(waitHead == 0 && modFlag == 0) this->classicWait = pcbWait;
		else if(waitHead == 0 && modFlag == 1) this->sleepWait = pcbWait;
		else {
			Node* tail = waitHead;
			while(tail->next) tail = tail->next;
			tail->next = pcbWait;
		}
		running->status = BLOCKED;
		lockFlag = 1;
		dispatch();
	}
	else waitFlag = 1;
	return waitFlag;
}

void KernelSem::signal(){
	waitFlag = 1;
	if(++semVal <= 0){
		Node* tmp = this->classicWait;
		if(!tmp) tmp = this->sleepWait;
		if(!tmp) return; // probably not possible
		tmp->pcb->status = READY;
		Scheduler::put(tmp->pcb);
		tmp->pcb->sleep = -1;
		if(this->classicWait) this->classicWait = this->classicWait->next;
		else this->sleepWait = this->sleepWait->next;
		tmp->next = 0;
		delete tmp;
	}
}

KernelSem::~KernelSem(){
	semNode* cur = KernelSem::semHead;
	semNode* prev = 0;
	while(cur->ksem != this) {prev = cur; cur = cur->next;}
	if(!prev) KernelSem::semHead = KernelSem::semHead->next;
	else prev->next = cur->next;
	cur->next = 0;
	delete cur;
}

void KernelSem::sleepUpdate(){
	KernelSem::semNode* semHe = KernelSem::semHead;
	int specCase = 0;
	while(semHe) {
		Node* sleepTmp = semHe->ksem->sleepWait;
		Node* sleepPrev = 0;
		while(sleepTmp){
			specCase = 0;
			sleepTmp->pcb->sleep = sleepTmp->pcb->sleep - 1;
			if(sleepTmp->pcb->sleep == 0){
				// finish sleep, remove from sleep list
				semHe->ksem->waitFlag = 0;
				semHe->ksem->semVal += 1;
				sleepTmp->pcb->status = READY;
				Scheduler::put(sleepTmp->pcb);
				if(sleepTmp == semHe->ksem->sleepWait) {
					semHe->ksem->sleepWait = semHe->ksem->sleepWait->next;
					delete sleepTmp;
					sleepTmp = semHe->ksem->sleepWait;
					specCase = 1;
				}
				else{
					sleepPrev->next = sleepTmp->next;
					delete sleepTmp;
					sleepTmp = sleepPrev;
				}
			}
			if(specCase == 0){
				sleepPrev = sleepTmp;
				sleepTmp = sleepTmp->next;
			}
		}
		semHe = semHe->next;
	}
}
