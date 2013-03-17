#ifndef _IP_H_
#define _IP_H_

typedef struct ip_header
{
	uint8_t version_and_header_len;
	uint8_t service_type;
	uint16_t total_len;
	uint16_t indication;
	uint16_t flags_and_fragment_offset;
	uint8_t time_to_live;
	uint8_t protocol;
	uint16_t header_checksum;
	uint32_t source_address;
	uint32_t dest_address;
} ip_header_t;

#define ip_header_length 20

#define ip_version_and_header_len_offset 0
#define ip_service_type_offset 1
#define ip_total_len_offset 2
#define ip_indication_offset 4
#define ip_flags_and_fragment_offset_offset 6
#define ip_time_to_live_offset 8
#define ip_protocol_offset 9
#define ip_header_checksum_offset 10
#define ip_source_address_offset 12
#define ip_dest_address_offset 16
#define ip_data_offset 20

#endif //_IP_H_
