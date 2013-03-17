#ifndef _UDP_H_
#define _UDP_H_

typedef struct udp_header
{
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t total_length;
	uint16_t checksum;
} udp_header_t;

#define udp_header_length 8
#define udp_protocol_num 17

#define udp_source_port_offset 0
#define udp_dest_port_offset 2
#define udp_total_length_offset 4
#define udp_checksum_offset 6
#define udp_data_offset 8
#endif //_UDP_H_
