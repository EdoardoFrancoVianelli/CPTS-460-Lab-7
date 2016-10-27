#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
TQE *tq, tqe[NPROC];

int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun\0", "Mercury\0", "Venus\0", "Earth\0",  "Mars\0", "Jupiter\0", "Saturn\0", "Uranus\0", "Neptune\0" };

#include "int.c"
#include "timer.c"
#include "vid.c"

int init()
{
	PROC *p;
	TQE  *q;
	int i;

	printf("init ....");

	for (i=0; i<NPROC; i++){   // initialize all procs
		p = &proc[i];
		q = &tqe[i];

		//q->proc = p;
		//q->next = &tqe[i+1];
		//q->time = -1;
		//q->action = 0;

		p->pid = i;
		p->status = FREE;
		p->priority = 0;  
		strcpy(proc[i].name, pname[i]);
		p->inkmode = 1;   
		p->next = &proc[i+1];
		p->time = 5;
	}
	freeList = &proc[0];      // all procs are in freeList
	proc[NPROC-1].next = 0;
	tqe[NPROC-1].next = 0;
	readyQueue = sleepList = tq = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList);
	p->status = RUNNING;
	p->ppid   = 0;
	p->parent = p;
	running = p;
	nproc = 1;
	printf("done\n");
} 

int scheduler()
{
	int old_row, old_col;
	if (running->status == RUNNING){
		running->status = READY;
		enqueue(&readyQueue, running);
	}
	running = dequeue(&readyQueue);
	running->status = RUNNING;
	if (part3 == 0){
		running->time   = 5; //set the time slice to 5s
	}
	goUmode();
}

int set_vec(vector, addr) u16 vector, addr;
{
	u16 location,cs;
	location = vector << 2;
	put_word(addr,  0,   location);
	put_word(0x1000,0, location+2);
}
        
int enable_interrupts(){ }
            
main()
{
	int i;
	vid_init();
	printf("MTX starts in main()\n");
	lock();	
	
	init();      // initialize and create P0 as running

	set_vec(80,int80h);
	set_vec(8,  tinth);
	
	kfork("/bin/u1");     // P0 kfork() P1	
	kfork("/bin/u1");     // P0 kfork() P2
	kfork("/bin/u1"); // P0 kfork() P3
	kfork("/bin/u1"); // P0 kfork() P4	
		
	printf("About to tswitch in kmode\n");
	
	timer_init(); tswitch();
}




















