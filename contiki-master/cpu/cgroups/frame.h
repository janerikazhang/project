#ifndef _FRAME_H_
#define _FRAME_H_

#include "ip.h"
#include "udp.h"
#include "cgroupsmessage.h"

#define min_frame_size 64

#define int8_len 1
#define int16_len 2
#define int32_len 4

#define ethernet_header_length 14
#define frame_checksum_length int32_len
#define macLength 6
#define data_len_size int16_len

#define ethernet_header_offset 0
#define ip_header_offset ethernet_header_length
#define udp_header_offset (ip_header_offset + ip_header_length)
#define message_offset (udp_header_offset + udp_header_length)

#define max_frame_size (ethernet_header_length + ip_header_length + udp_header_length + max_message_length + frame_checksum_length)

#define frame_dest_offset 0 
#define frame_source_offset 6
#define frame_data_len_offset 12
#define frame_data_offset 14

#define sum_headers_len ((ethernet_header_length+ip_header_length+udp_header_length))

typedef struct ethernet_header
{
	uint8_t dest[6];
	uint8_t source[6];
	uint16_t data_len;
} ethernet_header_t;

typedef struct ethernet_frame
{
	ethernet_header_t eth_header;
	ip_header_t ip_header;
	udp_header_t udp_header;
	cgroups_message_t message;
	uint32_t checksum;
} ethernet_frame_t;

#endif //_FRAME_H_
