#include "cgroups_radio_function.h"

/*form a single 32 bit integer from 4 bytes*/
uint32_t createIP(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte, uint8_t fourthByte)
{
	uint32_t ip = 0;
	ip = (ip + fourthByte) << 8;
	ip = (ip + thirdByte) << 8;
	ip = (ip + secondByte) << 8;
	ip = (ip + firstByte);
	return ip;
}
/*----------------------------------------------------------------------------*/
void createMac(uint8_t* dest, uint8_t* src)
{
        int i;
        for(i = 0; i < macLength ; i++)
        {
                dest[i] =  src[i];
        }
}
/*----------------------------------------------------------------------------*/
void copyData(uint8_t* dest, uint32_t dest_start, uint8_t* src, uint32_t len)
{	
        int i ;
        for(i = 0; i < len; i++)
        {
                dest[i+dest_start] = *(src+i);
        }
}
/*----------------------------------------------------------------------------*/
int frameIsForMe(uint8_t* mac)
{
        int i;
        for(i = 0 ; i < macLength; i++)
        {
                if(*(mac+i) != mymac[i])
                {
                        return 0;
                }
        }
        return 1;
}
/*----------------------------------------------------------------------------*/
void writeEthernetHeader(uint8_t* buff, ethernet_header_t* eth_header, uint32_t current_offset)
{
	copyData(buff, current_offset + frame_dest_offset, (uint8_t*)&eth_header->dest, macLength);
	copyData(buff, current_offset + frame_source_offset, (uint8_t*)&eth_header->source, macLength);
	copyData(buff, current_offset + frame_data_len_offset, (uint8_t*)&eth_header->data_len, data_len_size);
}
/*----------------------------------------------------------------------------*/
void writeIPHeader(uint8_t* buff, ip_header_t* ip_header, uint32_t current_offset)
{
	copyData(buff, current_offset + ip_version_and_header_len_offset, (uint8_t*)&ip_header->version_and_header_len, int8_len);
	copyData(buff, current_offset + ip_service_type_offset, (uint8_t*)&ip_header->service_type, int8_len);
	copyData(buff, current_offset + ip_total_len_offset, (uint8_t*)&ip_header->total_len, int16_len);
	copyData(buff, current_offset + ip_indication_offset, (uint8_t*)&ip_header->indication, int16_len);
	copyData(buff, current_offset + ip_flags_and_fragment_offset_offset, (uint8_t*)&ip_header->flags_and_fragment_offset, int16_len);
	copyData(buff, current_offset + ip_time_to_live_offset, (uint8_t*)&ip_header->time_to_live, int8_len);
	copyData(buff, current_offset + ip_protocol_offset, (uint8_t*)&ip_header->protocol, int8_len);
	copyData(buff, current_offset + ip_header_checksum_offset, (uint8_t*)&ip_header->header_checksum, int16_len);
	copyData(buff, current_offset + ip_source_address_offset, (uint8_t*)&ip_header->source_address, int32_len);
	copyData(buff, current_offset + ip_dest_address_offset, (uint8_t*)&ip_header->dest_address, int32_len);
}
/*----------------------------------------------------------------------------*/
void writeUdpHeader(uint8_t* buff, udp_header_t* udp_header, uint32_t current_offset)
{
	copyData(buff, current_offset + udp_source_port_offset, (uint8_t*)&udp_header->source_port, int16_len);
	copyData(buff, current_offset + udp_dest_port_offset, (uint8_t*)&udp_header->dest_port, int16_len);
	copyData(buff, current_offset + udp_total_length_offset, (uint8_t*)&udp_header->total_length, int16_len);
	copyData(buff, current_offset + udp_checksum_offset, (uint8_t*)&udp_header->checksum, int16_len);
}
/*----------------------------------------------------------------------------*/
void writeMessage(uint8_t* buff, xen_message_t* message, uint32_t current_offset)
{
	copyData(buff, current_offset, message->payload, message->length);
}
/*----------------------------------------------------------------------------*/
void writePadding(uint8_t* buff, uint8_t current_offset, uint8_t len)
{
	int i;
	for(i = current_offset; i < current_offset+len; i++)
	{
		buff[i] = 0xFF;
	}
}
/*----------------------------------------------------------------------------*/
void writeChecksum(uint8_t* buff, uint32_t current_offset)
{
	uint32_t checksum = chksum_crc32(buff,current_offset);
	copyData(buff, current_offset, (uint8_t* )&checksum, int32_len);
}
/*----------------------------------------------------------------------------*/
// swaps two bytes of a 16 bit word - used as both ntohs() and htons()
uint16_t swapBytes(uint16_t bytes)
{
	uint16_t newbottom = bytes  << 8;
	uint16_t newtop = bytes >> 8;
	return newtop | newbottom;		
}	
/*----------------------------------------------------------------------------*/
void sendViaEth(ethernet_frame_t* eth)
{
	uint8_t sendBuffer[max_frame_size]; // the byte array that will be sent
	uint8_t current_offset; // keeps track of current offset in the buffer
	uint8_t padding_len; // how much padding is required
	uint8_t message_len; // length of the message being sent
	
	createMac((uint8_t*)&(eth->eth_header.dest),dom0mac);
	createMac((uint8_t*)&(eth->eth_header.source),mymac);
	eth->eth_header.data_len = 0x0008; // 0x0008 means frame's content is IP

	writeEthernetHeader(sendBuffer, &(eth->eth_header), ethernet_header_offset);
	
	writeIPHeader(sendBuffer, &(eth->ip_header), ip_header_offset);
	
	writeUdpHeader(sendBuffer, &(eth->udp_header), udp_header_offset);
	
	writeMessage(sendBuffer, &eth->message, message_offset);

	message_len = eth->message.length;
	current_offset = message_offset+message_len;
	
	if(sum_headers_len + message_len < min_frame_size)
	{
		
		padding_len = min_frame_size - (sum_headers_len + message_len );
		writePadding(sendBuffer, current_offset, padding_len);
		current_offset += padding_len;
	}
	else
	{
		padding_len = 0;
	}
	
	writeChecksum(sendBuffer, current_offset);

	//netfront_xmit(xen_network_device,(unsigned char*) sendBuffer, current_offset+frame_checksum_length);
	netfront_xmit(net_dev,(unsigned char*) sendBuffer, current_offset+frame_checksum_length);//change it to linux version
	
}
/*----------------------------------------------------------------------------*/
void sendViaIP(ethernet_frame_t* eth)
{
	uint8_t temp_header[ip_header_length];
	uint16_t ip_len = ip_header_length + swapBytes(eth->udp_header.total_length); // must swap bytes again
	eth->ip_header.version_and_header_len = 0x45;
	eth->ip_header.service_type = 0;
	eth->ip_header.total_len = swapBytes(ip_len);
	eth->ip_header.indication = 0;
	eth->ip_header.flags_and_fragment_offset = 0;
	eth->ip_header.time_to_live = 255;
	eth->ip_header.protocol = udp_protocol_num;
	eth->ip_header.header_checksum = 0;
	eth->ip_header.source_address = myip;
	eth->ip_header.dest_address = dom0ip;
	
	// generate IP checksum using temporary storage array
	writeIPHeader(temp_header, &eth->ip_header, 0);
	eth->ip_header.header_checksum = cksum((uint16_t*)temp_header, ip_header_length);

	sendViaEth(eth);
}
/*----------------------------------------------------------------------------*/
void sendViaUDP(ethernet_frame_t* eth)
{
	uint16_t udp_len = udp_header_length + eth->message.length;

	eth->udp_header.source_port = swapBytes(PORT);
	eth->udp_header.dest_port = swapBytes(PORT);
	eth->udp_header.total_length = swapBytes(udp_len);
	eth->udp_header.checksum = 0;

	sendViaIP(eth);
}

/***** Receiving functionality *****/

//copy from source to dest, starting at src_start bytes into the source
void extractData(uint8_t* dest, uint32_t src_start, uint8_t* src, uint32_t len)
{	
	int i ;
	for(i = 0; i < len; i++)
	{
		dest[i] = *(src+i+src_start);
	}
}

// fills in fields of the provided ethernet_header_t from the buffer, starting at the offset
void readEthernetHeader(uint8_t* buff, ethernet_header_t* eth_header, uint32_t current_offset)
{
	extractData((uint8_t*)&eth_header->dest, current_offset + frame_dest_offset, buff, macLength);
	extractData((uint8_t*)&eth_header->source, current_offset + frame_source_offset, buff, macLength);
	extractData((uint8_t*)&eth_header->data_len, current_offset + frame_data_len_offset, buff, int8_len);
}

// fills in fields of the provided ip_header from the buffer, starting at the offset
void readIPHeader(uint8_t* buff, ip_header_t* ip_header, uint32_t current_offset)
{
	extractData((uint8_t*)&ip_header->version_and_header_len, current_offset + ip_version_and_header_len_offset, buff, int8_len);
	extractData((uint8_t*)&ip_header->service_type, current_offset + ip_service_type_offset, buff, int8_len);
	extractData((uint8_t*)&ip_header->total_len, current_offset + ip_total_len_offset, buff, int16_len);
	ip_header->total_len = swapBytes(ip_header->total_len);
	extractData((uint8_t*)&ip_header->indication, current_offset + ip_indication_offset, buff, int16_len);
	extractData((uint8_t*)&ip_header->flags_and_fragment_offset, current_offset + ip_flags_and_fragment_offset_offset, buff, int16_len);
	extractData((uint8_t*)&ip_header->time_to_live, current_offset + ip_time_to_live_offset, buff, int8_len);
	extractData((uint8_t*)&ip_header->protocol, current_offset + ip_protocol_offset, buff, int8_len);
	extractData((uint8_t*)&ip_header->header_checksum, current_offset + ip_header_checksum_offset, buff, int16_len);
	extractData((uint8_t*)&ip_header->source_address, current_offset + ip_source_address_offset, buff, int32_len);
	extractData((uint8_t*)&ip_header->dest_address, current_offset + ip_dest_address_offset, buff, int32_len);
}

// fills in fields of the provided udp_header from the buffer, starting at the offset
void readUdpHeader(uint8_t* buff, udp_header_t* udp_header, uint32_t current_offset)
{
	extractData((uint8_t*)&udp_header->source_port, current_offset + udp_source_port_offset, buff, int16_len);
	udp_header->source_port = swapBytes(udp_header->source_port);
	extractData((uint8_t*)&udp_header->dest_port, current_offset + udp_dest_port_offset, buff, int16_len);
	udp_header->dest_port = swapBytes(udp_header->dest_port);
	extractData((uint8_t*)&udp_header->total_length, current_offset + udp_total_length_offset, buff, int16_len);
	udp_header->total_length = swapBytes(udp_header->total_length);
	extractData((uint8_t*)&udp_header->checksum, current_offset + udp_checksum_offset, buff, int16_len);
}

// fills in fields of the provided message from the buffer, starting at the offset
void readMessage(uint8_t* buff, uint32_t current_offset, cgroups_message_t* message, uint8_t length)
{
	message->length = length;
	extractData(message->payload, current_offset, buff, length);
}
