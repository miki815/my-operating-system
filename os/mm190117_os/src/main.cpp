#include <iostream.h>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include "SCHEDULE.h"
#include "pcb.h"
#include "thread.h"
#include "semaphor.h"
#include "kerSem.h"
#include "idle.h"
#include "event.h"
#include "IVTEntry.h"
#include "user.h"

#define lock asm cli
#define unlock asm sti

volatile PCB* running; 
volatile Idle* idle;
volatile unsigned int lockFlag = 1;
volatile int testFlag = 0;
unsigned oldTimerOFF, oldTimerSEG;
void interrupt timer();
volatile int cntr = 20;
volatile int context_switch_on_demand = 0;
extern void tick();
extern int userMain(int argc, char** argv);


void inic(){
	asm{
		cli
		push es
		push ax

		mov ax,0
		mov es,ax // es = 0

		// pamti staru rutinu
		mov ax, word ptr es:0022h
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax

		// postavlja novu rutinu
		mov word ptr es:0022h, seg timer
		mov word ptr es:0020h, offset timer

		// postavlja staru rutinu na int 60h
		mov ax, oldTimerSEG
		mov word ptr es:0182h, ax
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}

void restore(){
	asm {
		cli
		push es
		push ax

		mov ax,0
		mov es,ax

		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}


void interrupt timer(){
	if (!context_switch_on_demand) {
		cntr--;
		tick();
		KernelSem::sleepUpdate();
	}

	if ((cntr == 0 || context_switch_on_demand) && lockFlag) {
		context_switch_on_demand = 0;
		running->sp = _SP;
		running->ss = _SS;
		running->bp = _BP;

		if(running->status == READY  && (PCB*)running != idle->pcbI) Scheduler::put((PCB*)running);
		running = Scheduler::get();
		if(running == 0) running = idle->pcbI;

		_SP = running->sp;
		_SS = running->ss;
		_BP = running->bp;

		cntr = running->timeSlice;

		}
	else if((cntr == 0 || context_switch_on_demand) && !lockFlag)
		context_switch_on_demand = 1;

	if(!context_switch_on_demand) asm int 60h;
}


class MainThread : public Thread{
private:
	int argc;
	char** argv;
public:
	MainThread(int argc, char** argv) : Thread(), argc(argc), argv(argv){}
	~MainThread(){
		waitToComplete();
	}
private:
	virtual void run(){
	    userMain(argc, argv);
	}
};


int main(int argc, char** argv){
	inic();
	idle = new Idle();
	running = new PCB(0, 0, 0);
	MainThread* mainT = new MainThread(argc, argv);
	mainT->start();
	idle->start();
	delete mainT;
	delete idle;
	restore();
	return 0;
}
