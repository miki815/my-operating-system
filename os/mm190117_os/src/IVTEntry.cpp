/*
 * IVTEntry.cpp
 *
 *  Created on: Jul 26, 2021
 *      Author: OS1
 */
#include "IVTEntry.h"
#include <dos.h>
#include <iostream.h>
IVTEntry* IVTEntry::IVT[256] = {0};
extern volatile unsigned int lockFlag;

IVTEntry::IVTEntry(IVTNo ivtN, pInterrupt newR) : ivt(ivtN), kEv(0){
	lockFlag = 0;
	if(ivt >= 0 && ivt < 256) {
	IVT[ivt] = this;
	#ifndef BCC_BLOCK_IGNORE
	oldRoutine = getvect(ivt);
	setvect(ivt, newR);
	#endif
	}
	lockFlag = 1;
}

IVTEntry::~IVTEntry(){
	lockFlag = 0;
	#ifndef BCC_BLOCK_IGNORE
	setvect(ivt, oldRoutine);
	#endif
	lockFlag = 1;
}


void IVTEntry::signal(){
	lockFlag = 0;
	kEv->signal();
	lockFlag = 1;
}
