#ifndef __NODE_ID_H__
#define __NODE_ID_H__

/* Paul Harvey
 * UNIQUE NODE ID
 */

/*Loads a nodes id from the xen store*/
void node_id_restore(void);

/*Not yet implemented*/
void node_id_burn(unsigned short node_id);

/*This nodes unique id*/
extern unsigned char node_id;

#endif /* __NODE_ID_H__ */
