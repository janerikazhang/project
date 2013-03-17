/*
 *The Xen Rtimer hardware emmulation
 *
 * Edited by Rika
 * Date: Feb 16, 2013
 *The init and scehdule code can be found in sys/rtimer.h
*/

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__


/*#include "sys/rtimer.h"*/
#include "contiki-conf.h"

rtimer_clock_t rtimer_get_time(void);

#define RTIMER_ARCH_SECOND 1000        //time in milliseconds

#define rtimer_arch_now() rtimer_get_time()

#endif /* __RTIMER_ARCH_H__ */
