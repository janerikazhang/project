#ifndef __UDP_CHECK__
#define __UDP_CHECK__

#include "contiki-conf.h"
/*UDP CHECKSUM CALCULATION
(see also short Description of Internet Checksum)

To calculate UDP checksum a "pseudo header" is added to the UDP header. This includes:

IP Source Address	4 bytes
IP Destination Address	4 bytes
Protocol		2 bytes
UDP Length  		2 bytes

The checksum is calculated over all the octets of the pseudo header, UDP header and data. 
If the data contains an odd number of octets a pad, zero octet is added to the end of data. 
The pseudo header and the pad are not transmitted with the packet. 

In the example code, 
u16 buff[] is an array containing all the octets in the UDP header and data.
u16 len_udp is the length (number of octets) of the UDP header and data.
BOOL padding is 1 if data has an even number of octets and 0 for an odd number. 
u16 src_addr[4] and u16 dest_addr[4] are the IP source and destination address octets

*/

/*
		This is based on the code that was taken from  : http://www.netfor2.com/udpsum.htm
*/
/*
**************************************************************************
Function: udp_sum_calc()
Description: Calculate UDP checksum
***************************************************************************
*/


uint16_t udp_sum_calc(uint16_t len_udp, uint16_t src_addr[],uint16_t dest_addr[], int padding, uint16_t buff[])
{
	uint16_t prot_udp=17;
	uint16_t padd=0;
	uint16_t word16;
	uint32_t sum;	
	int i;
	
	printk("CHECK SUM!!!\n");
	// Find out if the length of data is even or odd number. If odd,
	// add a padding byte = 0 at the end of packet
	if (padding&1==1){
		padd=1;
		buff[len_udp]=0;
	}
	
	//initialize sum to zero
	sum=0;
	printk("CHECK SUM!!!\n");
	// make 16 bit words out of every two adjacent 8 bit words and 
	// calculate the sum of all 16 bit words
	for (i=0;i<len_udp+padd;i=i+2){
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (unsigned long)word16;
	}	
	// add the UDP pseudo header which contains the IP source and destinationn addresses
	for (i=0;i<4;i=i+2){
printk("LOOP %d, 1\n", i);
		word16 =((src_addr[i]<<8)&0xFF00)+(src_addr[i+1]&0xFF);
printk("LOOP %d,2\n", i);
		sum=sum+word16;	

		printk("LOOP %d,3\n", i);
	}
	printk("CHECK SUM!!!\n");
	for (i=0;i<4;i=i+2){
		word16 =((dest_addr[i]<<8)&0xFF00)+(dest_addr[i+1]&0xFF);
		sum=sum+word16; 	
	}
	// the protocol number and the length of the UDP packet
	sum = sum + prot_udp + len_udp;
	printk("CHECK SUM!!!\n");
	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
    	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);
		
	printk("CHECK SUM!!!\n");
	// Take the one's complement of sum
	sum = ~sum;

return ((uint16_t) sum);
}
#endif 
