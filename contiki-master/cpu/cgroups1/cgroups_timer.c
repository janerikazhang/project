#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
//#include <stdint.h>        /* Definition of uint64_t */

#include "cgroups_timer.h"
#include "prioQ.h"
#include "interrupts.h"
//#include "semaphore.h" change it to cgroups's lock
//#include "node-id.h"

//#define TimerEvent struct TimerEvent
#define printHeap //

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printk(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*Handle the errors of the fdtimer*/
#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)


/*The mutex to go around the event queue- the minios version*/
//DECLARE_MUTEX(lock_event);
/*The mutex to go around the event queue -- the linux version*/
pthread_mutex_t lock_event;

/*A general lock for around any scheduling*/
//DECLARE_MUTEX(schedule_lock);
//pthread_mutex_t schedule_lock;

/*For the busy wait condition variable*/
//DECLARE_MUTEX(rdy_lock);
pthread_mutex_t rdy_lock;

/*The timer priority queue*/
static Queue Q;

/*Specifies the current sceduled timer request*/
static TimerEvent outstanding  = {0, 'A'};

/*Used to guard against errors during initilisation stages*/
static int set = 0;

static int place = 0;

static int ready = 1;

/*Basically a condition variable*/
static int asleep = 1;

void init_timer()
{

    /*initialise the event queue structure*/
    place = q_create();

    PRINTF("CGROUPS TIMER INIT : place = %d\n", place);
    
    /*Initialise the two mutex*/
    lock_event = PTHREAD_MUTEX_INITIALIZER;
    rdy_lock = PTHREAD_MUTEX_INITIALIZER;

    set  = 0; 
}

/*This is called from the timers to add an interrupt to be sent to process
@param interval: 	this is the time in the future for the interrupt to fire in milliseconds
@param timer:		this is the timer that requested the scheduling
*/
void schedule_interrupt(uint64_t interval, char timer)
{    
	/*The lock for scheduling*/
	//down(&schedule_lock);
	struct timespec curr;
	struct itimerspec new_value;
	int fd;
	uint64_t exp;

	/*Need the current time so as to dynamically calculate timer distance*/
	//uint64_t current_time = monotonic_clock();
	if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
               handle_error("clock_gettime");

  	PRINTF("Linux sched TIMER %c\n", timer);
	
	
	/*Create an event to add into the event priority queue
         * System time is the time since the system booted
         */
	TimerEvent t;

	/*The interval should be a multiple of INTERVAL OR RTIMER so
	  multiply by 1,000,000 to get into nano seconds
	*/
	//t.due = (interval * 1000000) + current_time;
	t.due = ((interval + curr.tv_sec) * 1000000) + curr.tv_nsec;
	t.timer = timer;
/*
	printk("*****************************\n");
	printk("Interval 	 :  %lld\n", interval);
	printk("System time 	 :  %lld\n", current_time);
	printk("Due time         :  %lld\n", t.due);
  	printk("The timer        :  %c\n", t.timer);
  	printk("Outstanding timer:  %c\n", outstanding.timer);
  	printk("Outstanding due  :  %lld\n", outstanding.due);
	printk("*****************************\n");
*/	
	if(!set)
	{
		outstanding = t;	
		set = 1;
		PRINTF("INTIAL\n");
		//up(&schedule_lock);
		return;
	}
	/*ATOMIC - so as to esure that the timer que isn't interrupted*/
        //down(&lock_event);
	pthread_mutex_lock( &lock_event);
	
	if(t.due >= outstanding.due)
	{
        	/*Add the event into the schduling queue*/
        	q_add(&place, (void *)&t);
		PRINTF("NORMAL ADDITION\n");
		
	}
	else
	{
		PRINTF("PREMPTIVE ADDITION\n");
		q_add(&place, (void *)&outstanding);	
		
		outstanding = t;
		
		//HYPERVISOR_set_timer_op(t.due);	change it to cgroups hypercall
		new_value.it_value.tv_sec = curr.tv_sec + interval;
           	new_value.it_value.tv_nsec = curr.tv_nsec;
		new_value.it_interval.tv_sec = 0;
		new_value.it_interval.tv_nsec = 0;
		fd = timerfd_create(CLOCK_REALTIME, 0);
		if (fd == -1)
               		handle_error("timerfd_create");

           	if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
               		handle_error("timerfd_settime");

               	if (read(fd, &exp, sizeof(uint64_t))!= sizeof(uint64_t))
                   	handle_error("read");
	}

	/*End atomic */	
       //up(&lock_event);
	pthread_mutex_unlock( &lock_event );
}

void reset_interrupt(uint64_t interval, char timer)
{
    struct itimerspec new_value;
    int fd;
    uint64_t exp;
    /*This will reset the outstanding timer request of timer A. This is essentially resetting the clock*/
    if(outstanding.timer == 'A')
    {
        /*
         *Schedule the new interrupt and then call replace the outstanding timer to replace the current timer (ignore the current one)
         *This means that the interruptA won't be called for the current at the wrong time
         */
        schedule_interrupt(interval, timer);

   	/*Lift the next event to be scheduled*/
        TimerEvent temp;

        /*ATOMIC*/
        //down(&lock_event);          //this will also disable the local interrupts
	pthread_mutex_lock( &lock_event );

        
        q_remove(&place, (void *)&temp);
	printk("LINUX: Rescheduled a new timer %lld, %c\n", temp.due, temp.timer);

        /*reset the current to hold the interrupt to be scheduled, convert millis into nano*/
        outstanding.due   = temp.due;					//dist in future
        outstanding.timer = temp.timer;
        
        /*Schedule a timer interupt with xen*/
        //HYPERVISOR_set_timer_op(temp.due);///////////////change it to cgroups hypercall
	new_value.it_value.tv_sec = temp.due / 1000000;
        new_value.it_value.tv_nsec = temp.due % 1000000;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;
	fd = timerfd_create(CLOCK_REALTIME, 0);
	if (fd == -1)
               	handle_error("timerfd_create");

        if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
              	handle_error("timerfd_settime");
	if (read(fd, &exp, sizeof(uint64_t))!= sizeof(uint64_t))
                handle_error("read");
        
        /*End Atomic*/	
        //up(&lock_event);
	pthread_mutex_unlock( &lock_event );
    }
    else
    {
        //down(&lock_event);
	pthread_mutex_lock( &lock_event );
        q_delete_first_timer(Q, 'A');
        //up(&lock_event);
	pthread_mutex_unlock( &lock_event );
        schedule_interrupt(interval, timer);
    }
}

int is_asleep(void)
{
	int temp;
	//down(&rdy_lock);
	temp = asleep;
	if(asleep == 0)	
		asleep = 1;
	//up(&rdy_lock);
	return temp;
}


/*****DEBUG*******/
//#ifdef DEBUG

static double mean = 0.0;
static double mean2 = 0.0;
static double delta = 0.0;
static unsigned long number = 0;
static unsigned long temp = 0;
static double x = 0;
static double granularity = 1000;

//#endif
/********DEBUG*******/

/*This is called by the interrupt handler upon a return from the irq
	it was fire an interrupt for the apropriate timer and 
	it will also schedule the next timer with xen*/
void cgroups_timer()
{
    //printk("here\n");
    //if the timers have not yet been set
    if(!set )
    {
	printk("no timer set\n");
	return;
    }
	
	struct itimerspec new_value;
	struct timespec curr;
    	int max_exp, fd;
	uint64_t exp;
	  uint8_t mini_interrupt_flags;
	  local_irq_save(mini_interrupt_flags);
	  local_irq_disable();

  	//get the current time in nanoseconds 
  	//uint64_t current_time = monotonic_clock();
	if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
               handle_error("clock_gettime");
	uint64_t current_time = curr.tv_sec * 1000000 + curr.tv_nsec;

    	if(ready)
	{
   		while(current_time >= outstanding.due)
		{ 

			ready = 0;

			if(outstanding.timer == 'A')
	    	   	{
#if 0
				//Take measurements for timer interrupts
				
				delta 	 = x - mean;
				//printk("THIS is the number %d\n", number);
				x = (double) ((current_time - outstanding.due)/granularity);
	 
				number++;
				mean 	+= delta/number;
    				mean2	+= delta * (x - mean);
    		
		
				if(((number%10) == 0) && node_id == 1000 )
				{
					uint64_t highM  = (uint64_t)((unsigned long)mean)  ;
					unsigned long  lowM   = (unsigned long) 1000 * (mean -  (double)highM);		

					uint64_t  highM2 = (uint64_t)((unsigned long)mean2) ; 
					unsigned long  lowM2 = (unsigned long) 1000 * (mean2 - (double) highM2);
			
					//printk("Timer Measurements:\n");
					printk("Mean  	: %llu.%03lu\n", highM,  lowM);
					printk("Mean2 	: %llu.%03lu\n", highM2, lowM2);
					//printk("Itterations	: %ld\n", number);
					//printk("System time 	 :  %lld\n", current_time);
					//printk("Due	 	 :  %lld\n", outstanding.due);
					//printk("diff	 	 :  %lld\n", current_time - outstanding.due);
					//printk("System time 	 :  %lld\n", current_time / granularity);

/*
					int q= 0;
					for(q; q < 100000 ; q++)
					{
						printk("DEAD\n");
					}
*/					
					//do_exit();
				}
#endif
				interruptA(mini_interrupt_flags);
				PRINTF("***********Timer A\n");
                        }
			else if(outstanding.timer == 'B')
                        {
				interruptB();
				PRINTF("***********Timer B\n");
                        }
			else
			{
				PRINTF("No Timer val set!\n\n");
				local_irq_restore(mini_interrupt_flags);
				//up(&schedule_lock);
				ready = 1;
				return;
			}


			//Lift the next event to be scheduled
			TimerEvent temp;
	
			//printk("Current pos %d\n", place);
			//ATOMIC
			//down(&lock_event);          //this will also disable the local interrupts
			pthread_mutex_lock( &lock_event );
		
			q_remove(&place, (void *)&temp);
	
			PRINTF("\nHave removed timer %lld, %c\n", temp.due, temp.timer);
	
			//reset the current to hold the interrupt to be scheduled
			outstanding.due   = temp.due;          //absolute time in future
			outstanding.timer = temp.timer;
	
			//if the current timer is not due
			if(current_time < outstanding.due)
			{
				//uint64_t tmp = (uint64_t)((((unsigned long)mean)/2) + (((unsigned long)mean)/3));
				//HYPERVISOR_set_timer_op(temp.due /*- ((uint64_t)((((unsigned long)mean)/100)*80))*/);
				new_value.it_value.tv_sec = temp.due / 1000000;
        			new_value.it_value.tv_nsec = temp.due % 1000000;
				new_value.it_interval.tv_sec = 0;
				new_value.it_interval.tv_nsec = 0;
				
				fd = timerfd_create(CLOCK_REALTIME, 0);
				if (fd == -1)
               				handle_error("timerfd_create");

        			if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1)
              				handle_error("timerfd_settime");
				if (read(fd, &exp, sizeof(uint64_t))!= sizeof(uint64_t))
                			handle_error("read");
			}

	//		End Atomic
			//up(&lock_event);
			pthread_mutex_unlock( &lock_event );
			//printHeap(&place);
		}/*Loop while the oustanding is due to fire*/
	}
	ready = 1;
	
	local_irq_restore(mini_interrupt_flags);
}

/*Returns 1 if "a" is of a higher priority than "b"*/
int lessThan(struct event a, struct event b)
{
	return a.due < b.due;
}
