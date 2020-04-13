/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
   DP.H
 
 *\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#ifndef _DP_H_
#define _DP_H_

#define ETH_MTU					1514
#define JUMBO_MTU				9000

/*-----------------------------------------
 * msg from dp sock
 *----------------------------------------*/
typedef struct {
	uint16_t  		port_no;
	int 			sockfd;
	char          	buffer[ETH_MTU];
}tDP_MSG;

/*-----------------------------------------
 * msg from dp to ofp
 *----------------------------------------*/
typedef struct {
	int   			id;
	char          	buffer[ETH_MTU];
}tDP2OFP_MSG;

#endif