/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: clock.c,v 1.11 2007/11/12 21:07:39 adamdunkels Exp $
 */


/* LINUX:
 *TimerA: This will esentially tick away at a second a time. If an interrupt occured
 *during the last second then it will be delt with but can be no more specific 
 *as to when it happened
 */



#include "contiki-conf.h"

#include "sys/clock.h"
#include "sys/etimer.h"

#include "interrupts.h"
//#include "semaphore.h"


#include "cgroups_timer.h"

/* 38400 cycles @ 2.4576MHz with divisor 8 ==> 1/8 s */
/* #define INTERVAL (307200ULL / CLOCK_SECOND) */
//#define INTERVAL (4096ULL / CLOCK_SECOND)

//Interval time : 10 ms
#define INTERVAL 1000

#define MAX_TICKS (~((clock_time_t)0) / 2)

struct timeval temp;

static volatile clock_time_t count = 0;

/*---------------------------------------------------------------------------*/
void interruptA(int a) {
    //ENERGEST_ON(ENERGEST_TYPE_IRQ);
    
    /*Schedule the next interrupt, the time is specified in milliseconds*/
    schedule_interrupt(INTERVAL, 'A');

    /*increment the count system ticks counter - milliseconds*/
    count++;
    
    //printk("%d\n", count);

    //printk("n: %d d: %d", count, etimer_next_expiration_time());
    /*If there is an outstanding timer that fired in the last interval*/
    if(etimer_pending() && ( count >=etimer_next_expiration_time() )) {
        etimer_request_poll();
        //printk("request poll\n");
    }
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
void
clock_set(clock_time_t clock, clock_time_t fclock)
{

    printk("\nCLOCK SET\n");
    /*Change to the appropriate timer interrupt*/
    reset_interrupt(fclock + INTERVAL, 'A');
    
    /*reset the system time*/
    count = clock;
}
/*---------------------------------------------------------------------------*/
int
clock_fine_max(void)
{
  return INTERVAL;
}
/*---------------------------------------------------------------------------*/
unsigned short
clock_fine(void)
{
  unsigned short t;
  /* Assign last_tar to local varible that can not be changed by interrupt */
 // t = last_tar;
  /* perform calc based on t, TAR will not be changed during interrupt */
 // return (unsigned short) (TAR - t);

	/*At the moment assuming that the timers are accurate.....*/
	return 0;
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
    /*Set the ball rolling */
    schedule_interrupt(INTERVAL, 'A');
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of 2.83 us.
 */
void
clock_delay(unsigned int i)
{
/*
 *Busy wait
*/	
	/*delay in nanos*/
	uint64_t delay = 2830;
	uint64_t start, current;// = monotonic_clock() + (delay * i);
	struct timespec curr;
	if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
               handle_error("clock_gettime");
	current = curr.tv_sec * 1000000 + curr.tv_nsec;
	start = current + delay * i;
   
    while(current <  start) {
        if (clock_gettime(CLOCK_MONOTONIC, &curr) == -1)
               handle_error("clock_gettime");
	current = curr.tv_sec * 1000000 + curr.tv_nsec;
    }

  //asm("add #-1, r15");
  //asm("jnz $-2");       //JUMP IF NOT ZERO
  /*
   * This means that delay(i) will delay the CPU for CONST + 3x
   * cycles. On a 2.4756 CPU, this means that each i adds 1.22us of
   * delay.
   *
   * do {
   *   --i;
   * } while(i > 0);
   */
}
/*---------------------------------------------------------------------------*/
