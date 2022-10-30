/*
 * Thread.h
 *
 *  Created on: Jul 17, 2021
 *      Author: OS1
 */

#ifndef THREAD_H_
#define THREAD_H_

typedef unsigned long StackSize;
const StackSize defaultStackSize = 4096;
typedef unsigned int Time; // time, x 55ms
const Time defaultTimeSlice = 2; // default = 2*55ms
typedef int ID;

class PCB; // Kernel's implementation of a user's thread

class Thread {
public:
	void start();
	void waitToComplete();
	virtual ~Thread();

	ID getId();
	static ID getRunningId();
	static Thread * getThreadById(ID id);

	// fork
	static ID fork();
	static void exit();
	static void waitForForkChildren();
	virtual Thread* clone() const;

protected:
	friend class PCB;
	friend class Idle; //upgrade of implementation
	Thread(StackSize stackSize = defaultStackSize, Time timeSlice =
   defaultTimeSlice);
public:
	virtual void run() {}

private:
	PCB* myPCB;
};

void dispatch();

#endif /* THREAD_H_ */
