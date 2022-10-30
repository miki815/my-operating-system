/*
 * kerSem.h
 *
 *  Created on: Jul 21, 2021
 *      Author: OS1
 */

#ifndef KERSEM_H_
#define KERSEM_H_
#include "pcb.h"
typedef unsigned int Time;

class KernelSem{
public:
	struct semNode {
		KernelSem* ksem;
		semNode* next;
	};

	KernelSem(int value);
	~KernelSem();
	static semNode* semHead;
	int wait(Time waitTime);
	void signal();
	static void sleepUpdate();
	Node* classicWait; // maxTime = 0
	Node* sleepWait;  // maxTime > 0
	int semVal;
	int waitFlag;


};




#endif /* KERSEM_H_ */
