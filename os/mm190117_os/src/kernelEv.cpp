/*
 * kernelEv.cpp
 *
 *  Created on: Jul 26, 2021
 *      Author: OS1
 */
#include "kernelEv.h"
extern volatile PCB* running;
extern volatile unsigned int lockFlag;

KernelEv::KernelEv(IVTNo ivtn) : sem(0), ivtN(ivtn){
	lockFlag = 0;
	owner = (PCB*) running;
	if(ivtn >= 0 && ivtn < 256) IVTEntry::IVT[ivtn]->kEv = this;
	lockFlag = 1;
}

KernelEv::~KernelEv(){
	lockFlag = 0;
	if(ivtN >= 0 && ivtN < 256) IVTEntry::IVT[ivtN] = 0;
	lockFlag = 1;
}

void KernelEv::wait(){
	lockFlag = 0;
	if((PCB*) running != owner) {
		lockFlag = 1;
		return;
	}
	if(--sem < 0) {
		owner->status = BLOCKED;
		dispatch();
	}
	lockFlag = 1;
}

void KernelEv::signal(){
	lockFlag = 0;
	if(++sem <= 0){
		owner->status = READY;
		Scheduler::put(owner);
	}
	else sem = 1;
	lockFlag = 1;
}



