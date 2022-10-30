/*
 * IVTEntry.h
 *
 *  Created on: Jul 26, 2021
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_
#include "kernelEv.h"
typedef unsigned char IVTNo;
typedef void interrupt(*pInterrupt)(...);

class KernelEv;

class IVTEntry{
public:
	IVTEntry(IVTNo ivt, pInterrupt newR);
	~IVTEntry();
	KernelEv* kEv;
	static IVTEntry* IVT[256];
	void signal();
	pInterrupt oldRoutine;
private:
	IVTNo ivt;
};

#define PREPAREENTRY(ivt, oldFlag)\
	void interrupt inter##ivt(...);\
	IVTEntry newE##ivt(ivt, inter##ivt);\
	void interrupt inter##ivt(...){\
		newE##ivt.signal();\
		if(oldFlag)\
			newE##ivt.oldRoutine();\
	}


#endif /* IVTENTRY_H_ */
