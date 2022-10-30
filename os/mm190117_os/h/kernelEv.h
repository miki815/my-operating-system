/*
 * kernelEv.h
 *
 *  Created on: Jul 25, 2021
 *      Author: OS1
 */

#ifndef KERNELEV_H_
#define KERNELEV_H_

#include "IVTEntry.h"
#include "pcb.h"
typedef unsigned char IVTNo;

class KernelEv{
private:
	PCB* owner;
	int sem;
	IVTNo ivtN;
public:
	KernelEv(IVTNo ivtn);
	~KernelEv();
	void wait();
	void signal();
};



#endif /* KERNELEV_H_ */
