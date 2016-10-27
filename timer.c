/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00	   /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	   /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L	   /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

u16 tick;

int seconds = 0, minutes = 0, hours = 0;

int enable_irq(u16 irq_nr)
{
	printf("enable_irq()\n");	
	lock();
	out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}

int timer_init()
{
	/* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */
	printf("timer_init()\n");
	set_vec(80,int80h);
	set_vec(8,  tinth);
	//tick = seconds = minutes = hours = 0; 
	out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
	out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
	out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
	enable_irq(TIMER_IRQ); 
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
 
void SetRow(int rw);
void SetColumn(int col);
int getRow();
int getCol();
 
int decrement_proc_time(){
	int old_row, old_col, i;
	if (running->inkmode == 1){ //if we were in user mode
		running->time--;
		printf("time = %d for process %d\r", running->time, running->pid);
		if (running->time <= 0){ //if the time has expired
			ClearTimer();
			tswitch(); //switch process
			return 1;
		}
	}else{
		printf("For proc %d, inkmode = %d running_time = %d\n", running->pid, running->inkmode, running->time);
	}		
	return 0;
}
 
int sec_min_hr(){
	seconds++;
	if ((seconds %= 60) == 0){
		minutes++;
		if ((minutes %= 60) == 0){
			hours++;
			//if (hours >= 24) seconds = minutes = hours = 0; 
		}
	}
}

void add_to_queue(TQE **queue, TQE *new_element){

    TQE *start; int inserted_time = new_element->time;
    TQE *prev;

    if (new_element == 0) return;

    //first case -> the queue is null
    if (*queue == 0){
        new_element->next = 0;
        *queue = new_element;
        return;
    }
    //second case -> there is only one element
    if (*queue != 0 && (*queue)->next == 0){
        //check which time is bigger, the bigger one should be last
        if ((*queue)->time > new_element->time){ //queue should be last
            (*queue)->time -= new_element->time;
            new_element->next = *queue;
            (*queue)->next = 0;
            *queue = new_element;
        }else{ //new_element should be last
            new_element->time -= (*queue)->time;
            (*queue)->next = new_element;
            new_element->next = 0; 
        }
        return;
    }

    //third case -> there is more than one element
    for (start = *queue,
         prev = start; start != 0; start = start->next){
        //subtract the current time to the new_element's time

        if ((inserted_time -= start->time) < 0){
                //we have passed the point of insertion
            new_element->time -= prev->time;
            prev->next = new_element;
            new_element->next = start;
            start->time -= new_element->time;
            start->next = 0;
            return;
        }else if (start->next == 0){
            start->next = new_element;
            start->next->time = start->time;
            start->next->next = 0;
            return;
        }
        prev = start;
    }
}

void print_timer_queue(TQE **q){
    
}

TQE* remove_from_queue(TQE **queue){
    TQE *removed  = *queue;
    if (*queue == 0) return 0;
    *queue = (*queue)->next;
    removed->next = 0;
    return removed;
}

int *decrement_time(TQE **queue){
    TQE *current;
    PROC *current_proc;
    if (*queue == 0) return;

    (*queue)->time--;

    //printf("New time until wakeup = %d\n",(*queue)->time );

    for (current = *queue, current_proc = current->proc; current != 0; current = current->next, current_proc = current->proc){
        printf("[%d|%d] -> ", current_proc->pid, current->time);
    }

    printf("NULL\n");

    if ((*queue)->time <= 0){
        return remove_from_queue(queue);
    }

    return 0;
}

int string_to_int(char *s){
    int n = 0;
    while (*s){
        n *= 10;
        n += (*s++) - '0';
    }
    return n;
}

int set_seconds(int sc){
    //seconds in a day = 60 * 60 * 24
    hours   = (int)(sc / 3600);
    sc     -= (hours * 3600);
    minutes = sc / 60;
    sc /= 60;
    seconds = sc; 
}

int timer(){
    char s[64]; 
    int n;
    PROC *prc;
    TQE *removed = 0, *cur = &tqe[running->pid];
    printf("timer()\n");
    cur->proc = running;
    gets(s);
    n = string_to_int(s); 
    cur->time = (int)n;
    lock();

    if (n == 0){
        printf("Invalidating first timer\n");
        removed = remove_from_queue(&tq);
        prc = removed->proc;
        kwakeup(prc->pid);
    }
    else{
        add_to_queue(&tq, cur);
        printf("Starting to sleep for %d seconds\n", cur->time);
        ksleep(running->pid);
    }

    unlock();
}

int thandler()
{
	int old_row, old_col, display_loc, i = 0, removed;
    TQE *finished;
    PROC *finished_proc;
	tick++; 
	tick %= 60;
	out_byte(0x20, 0x20);                // tell 8259 PIC EOI
	if (tick == 0) {
		sec_min_hr();	
		if (part3 == 0){
            display_timer_bottom_right(seconds, 
                        minutes, 
                        hours, 
                        &old_row, 
                        &old_col); 
            SetRow(old_row); 
            SetColumn(old_col);
			decrement_proc_time();
		}else{
			finished = (TQE*)decrement_time(&tq, &removed);
            finished_proc = finished->proc;
            if (finished != 0){ //time has expired on a process
                printf("Time expired, waking up pid = %d\n", finished_proc->pid);
                kwakeup(finished_proc->pid);
            }
		}
	}
}







  





