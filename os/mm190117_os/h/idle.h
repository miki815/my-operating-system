/*
 * idle.h
 *
 *  Created on: Jul 22, 2021
 *      Author: OS1
 */

#ifndef IDLE_H_
#define IDLE_H_

#include <iostream.h>
#include "thread.h"
#include "pcb.h"
extern volatile unsigned int lockFlag;


class Idle : public Thread{
public:
	PCB* pcbI;
	Idle() : Thread(128, 1){pcbI = myPCB;}

protected:
	void run(){
		while(1);
	}
};


#endif /* IDLE_H_ */
