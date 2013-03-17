#ifndef _CGROUPSMESSAGE_H_
#define _CGROUPSMESSAGE_H_

/* defines a simple structure for passing data between tiny domains */

#define message_size 128 //maximum size of a message between domains (more than enough for a tinyos data packet+headers)
#define cgroups_message_length_size 1

#define max_message_length (cgroups_message_length_size + message_size)

typedef struct cgroups_message
{
	uint8_t length;
	uint8_t payload[message_size];
} cgroups_message_t;

#endif //_CROUPSMESSAGE_H_
