
/*TimerB:
 *This will schedule an interrupt at a specific time in the future, 
 *This has a higher precision than TimerA as it is in millisecond granularity
*/
#include <sys/timerfd.h>
#include "sys/rtimer.h"

#include "interrupts.h"
#include "cgroups_timer.h"

#include "clock.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printk(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


static uint64_t last_time = 0 ;
/*---------------------------------------------------------------------------*/
/*Called when a timer interrupt for timerB fires*/
void 
interruptB(void) {
  //ENERGEST_ON(ENERGEST_TYPE_IRQ);

    rtimer_run_next();
  
  //ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /*This does nothing as the rtimer has no needed state to be set up*/ 
}
/*---------------------------------------------------------------------------*/
/*Schedule a TimerB event 't' amount in the future*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
	schedule_interrupt(t , 'B');
  
	/*Get the time for the last scheduled interrupt*/
	struct timespec curr;
	if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
               ;//handle error
	last_time = curr.tv_sec * 1000000 + curr.tv_nsec;
	//last_time = monotonic_clock();
}
/*----------------------------------------------------------------------------*/
/*
 *This will return the amount of milliseconds past for the current second.
 *i.e. 7600 gives 600
 */
rtimer_clock_t 
rtimer_get_time(void)
{
    //uint64_t current = monotonic_clock();
    struct timespec curr;
    if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
           ;//handle error
    uint64_t current = curr.tv_sec * 1000000 + curr.tv_nsec;


    /*Return the differnce between the current time and last schedule in milliseconds*/

    //printk("The size : %lld\n",((current - last_time)%1000) );
    //printk("OTHER : %d\n", (rtimer_clock_t)((current - last_time)%1000));
    
    return (rtimer_clock_t)((current - last_time)/1000000);
}
