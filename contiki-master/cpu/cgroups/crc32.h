#ifndef __CRC32_H__
#define __CRC32_H__

#include "contiki-conf.h"		

/* chksum_crc() -- to a given block, this one calculates the
 *				crc32-checksum until the length is
 *				reached. the crc32-checksum will be
 *				the result.
 */
uint32_t chksum_crc32 (unsigned char *data_block, unsigned int length);

/* chksum_crc32gentab() --      to a global crc_tab[256], this one will
 *				calculate the crcTable for crc32-checksums.
 *				it is generated to the polynom [..]
 */

void chksum_crc32gentab(void);

#endif
