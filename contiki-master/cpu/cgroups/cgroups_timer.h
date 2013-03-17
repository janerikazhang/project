#ifndef __CGROUPS_TIMER_H__
#define __CGROUPS_TIMER_H__

#include "prioQ.h"
#include "contiki-conf.h"
#include "contiki.h"
#include "sys/process.h"

/*This holds who the event is for and it's absolute firing time*/
typedef struct event{
	uint64_t due;
	char timer;
	int fd;
} TimerEvent;

/*Specifies the current sceduled timer request*/
static TimerEvent outstanding  = {0, 'A', -1};

void init_timer(void);

/*This is called from the timers to add an interrupt to be sent to xen
 * @param interval: 	this is the time in the future for the interrupt to fire in micro seconds
 * @param timer:	this is the timer that requested the scheduling
 */
void schedule_interrupt(uint64_t interval, char timer);

/*This is called when the timerA needs to reset its clock timer
 *  @param interval: 	this is the time in the future for the interrupt to fire in micro seconds
 *  @param timer:	this is the timer that requested the scheduling
 */
void reset_interrupt(uint64_t interval, char timer);

/* This is called by the interrupt handler upon a return from the irq
 * it will fire an interrupt for the apropriate timer and it will also 
 * schedule the next outstanding timer with xen
 */


/* Synchronized method to determin if the mote should be allowed to run*/
int is_asleep(void);

/*Called by the each process*/
void cgroups_timer();

/*Returns 1 if "a" is of a higher priority than "b"*/
int lessThan(TimerEvent a, TimerEvent b);

#endif
