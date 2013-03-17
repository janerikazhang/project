#ifndef __CGROUPS_RADIO_H__
#define __CGROUPS_RADIO_H__


#include "contiki.h"
#include "dev/radio.h"

/*This is for the conversion to ethernet, upd and ip*/
#include "frame.h"
#include "ip.h"
#include "udp.h"
//#include <netfront.h>

#define CGROUPS_RADIO_MAX_PACKET_LEN      127

unsigned char mymac[6];
unsigned char pro0mac[6];
//struct netfront_dev *net_dev;		/*Passed from XEN setup, used to transmitt over Linux*/

/*This is what will hold the recieved message from process0 minus the extra eth, ip and udp headers*/
cgroups_message_t received_message;
cgroups_message_t send_message;

/*This nodes ipaddres, port number and the process0 ip address*/
//int32_t myip;
//int32_t process0ip;
char* dom0ip_str = NULL;
char* myip_str = NULL;
char* port_str = NULL;

uint16_t PORT;

/*Not implemeneted in XenoContiki yet*/
void cgroups_radio_set_chan_pan_addr(unsigned channel, /* 11 - 26 */
				     unsigned pan,
				     unsigned addr,
				     const u8_t *ieee_addr);

/*Not properly implemeneted in XenoContiki yet*/
extern signed char cgroups_radio_last_rssi;
extern u8_t cgroups_radio_last_correlation;

/*This points to the struct that is needed for the xenbus-xmit*/
//struct netfront_dev *cgroups_dev;///need to be fixed

/*A struct that contains all of the commands to be used with the radio
 *  cgroups_radio_init,
    cgroups_radio_prepare,
    cgroups_radio_transmit,
    cgroups_radio_send,
    cgroups_radio_read,
    cgroups_radio_cca, //channel clear assessment to find out if there is a packet in the air or not.
    cgroups_radio_check_receiver,
    cgroups_radio_check_pending,
    cgroups_radio_on,
    cgroups_radio_off,
 */
extern const struct radio_driver cgroups_radio_driver;

/**
 * \param power Between 1 and 31. Not yet uesd in XenoContiki but possibly for power estimation
 */
void cgroups_radio_set_txpower(u8_t power);

/**
 * Interrupt function, called the process event for incomming packets.
 *
 */
int cgroups_radio_interrupt(void);

#endif /*__CGROUPS_RADIO_H__*/
