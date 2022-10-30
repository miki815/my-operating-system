/*
 * semaphor.cpp
 *
 *  Created on: Jul 21, 2021
 *      Author: OS1
 */
#include "semaphor.h"
#include "kerSem.h"
extern volatile unsigned int lockFlag;

Semaphore::Semaphore(int init){
	lockFlag = 0;
	if(init < 0) init = 0;
	myImpl = new KernelSem(init);
	lockFlag = 1;
}

void Semaphore::signal(){
	lockFlag = 0;
	myImpl->signal();
	lockFlag = 1;
}

int Semaphore::wait(Time maxTimeToWait){
	lockFlag = 0;
	int ret = myImpl->wait(maxTimeToWait);
	lockFlag = 1;
	return ret;
}

int Semaphore::val() const{
	return myImpl->semVal;
}

Semaphore::~Semaphore(){
	lockFlag = 0;
	delete this->myImpl;
	lockFlag = 1;
}
