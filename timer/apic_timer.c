#include <asm/timex.h>
#include "uka_apic_timer.h"
#define SYS_CLOCK 500000000 //(500 MHz)
static struct apic_timer_list test_timer;
unsigned long long timestamp;
static struct egal_daten data1;
void test_timer_handler(unsigned long long exp, unsigned long data)
{
        /* Do here what you think you have to do :? */
        /* e.g., use hard_start_xmit to send a packet */
	printf("hello Rika~~~~");
}
/* ... This is a routine, in which the timer is activated ... */
/* Initialize the apic_timer_list structure */
init_apic_timer(&test_timer);
/* Read the current time (status of the TSC register) */
timestamp = get_cycles();
/* Set the values... */
timer.function = (void*) &test_timer_handler;
timer.expires = timestamp  + (SYS_CLOCK * (2 / 1000000));
timer.data = (unsigned long)  &data1;
/* Register the timer */
add_apic_timer(&timer);
