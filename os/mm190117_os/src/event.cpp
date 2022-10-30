/*
 * event.cpp
 *
 *  Created on: Jul 26, 2021
 *      Author: OS1
 */
#include "event.h"
#include "kernelEv.h"
extern volatile unsigned int lockFlag;

Event::Event(IVTNo ivtNo){
	myImpl = new KernelEv(ivtNo);
}

void Event::wait(){
	myImpl->wait();
}

void Event::signal(){
	myImpl->signal();
}

Event::~Event(){
	lockFlag = 0;
	delete myImpl;
	lockFlag = 1;
}



