/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
  DP_CODEC.C :

  Designed by THE on SEP 16, 2019
/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include 		<common.h>
#include		"dp.h"
#include		"dp_flow.h"
#include		"dp_sock.h"
#include		"ofpd.h"
#include		"ofp_codec.h"
#include        "ofp_fsm.h"
#include        "ofp_dbg.h"
#include 		"ofp_sock.h"
#include 		"ofp_asyn.h"
#include 		"ofp_ctrl2sw.h"
#include 		"ofp_oxm.h"
#include 		<unistd.h>

STATUS parse_tcp(struct ethhdr *eth_hdr, struct iphdr *ip_hdr, uint16_t port_id, uint32_t *flow_index);
STATUS parse_udp(struct ethhdr *eth_hdr, struct iphdr *ip_hdr, uint16_t port_id, uint32_t *flow_index);
STATUS parse_ip(struct ethhdr *eth_hdr, uint16_t port_id, uint32_t *flow_index);

extern flow_t flow[256];

extern STATUS find_flow(pkt_info_t pkt_info, uint32_t *flow_index);
extern STATUS apply_flow(U8 *mu, U16 mulen, uint32_t flow_index, dp_io_fds_t *dp_io_fds_head);
extern void dp_drv_xmit(U8 *mu, U16 mulen, uint16_t port_id, dp_io_fds_t *dp_io_fds_head);

/*============================ DECODE ===============================*/

/*****************************************************
 * DP_decode_frame
 * 
 * input : pArg - mail.param
 * output: imsg, event
 * return: session ccb
 *****************************************************/
STATUS DP_decode_frame(tOFP_MBX *mail, dp_io_fds_t *dp_io_fds_head, uint32_t *buffer_id)
{
	U16	mulen;
	U8	*mu;
	tDP_MSG *msg;
	struct ethhdr *eth_hdr;
	uint32_t flow_index;
	int ret;
	
	if (mail->len > MSG_LEN) {
	    DBG_OFP(DBGLVL1, 0, "error! too large frame(%d)\n", mail->len);
	    return ERROR;
	}
	
	msg = (tDP_MSG *)(mail->refp);
	mu = (U8 *)(msg->buffer);
	mulen = (mail->len) - (sizeof(int) + sizeof(uint16_t));
	//PRINT_MESSAGE(mu,mulen);

	eth_hdr = (struct ethhdr *)mu;
	if (eth_hdr->h_proto == htons(ETH_P_IP)) {
		if (parse_ip(eth_hdr, msg->port_no, &flow_index) == FALSE)
			return ERROR;
	}
	else if (eth_hdr->h_proto == htons(ETH_P_ARP)) {
		pkt_info_t pkt_info;
		memset(&pkt_info, 0, sizeof(pkt_info));

		memcpy(&pkt_info, eth_hdr, sizeof(struct ethhdr));
		pkt_info.port_id = msg->port_no;
		//TODO: save L2 payload

		if (find_flow(pkt_info, &flow_index) == FALSE)
			return ERROR;
	}
	else {
		return ERROR;
	}
	//TODO: send pkt from rule
	//puts("flow found.");
	if ((ret = apply_flow(mu, mulen, flow_index, dp_io_fds_head)) == ERROR)
		return ERROR;
	else if (ret == FALSE) {
		*buffer_id = flow[flow_index].buffer_id; // only when pkt needs to be sent to controller will return false;
		return FALSE;
	}
	else
		return TRUE;
}

STATUS parse_ip(struct ethhdr *eth_hdr, uint16_t port_id, uint32_t *flow_index)
{
	pkt_info_t pkt_info;

	struct iphdr *ip_hdr = (struct iphdr *)(eth_hdr + 1);

	ip_hdr = (struct iphdr *)(eth_hdr + 1);
	if (ip_hdr->version != 4)
		return FALSE;
	switch (ip_hdr->protocol) {
	case IPPROTO_ICMP:
		memcpy(&pkt_info, eth_hdr, sizeof(struct ethhdr));
		pkt_info.ip_proto = ip_hdr->protocol;
		pkt_info.ip_dst = ip_hdr->daddr;
		pkt_info.ip_src = ip_hdr->saddr;
		pkt_info.port_id = port_id;

		if (find_flow(pkt_info, flow_index) == FALSE) {
			return FALSE;
		}
		return TRUE;
	/*case IPPROTO_TCP:
		if (parse_tcp(eth_hdr, ip_hdr, port_id, flow_index) == FALSE)
			return FALSE;
		return TRUE;*/
	case IPPROTO_UDP:
		if (parse_udp(eth_hdr, ip_hdr, port_id, flow_index) == FALSE)
			return FALSE;
		return TRUE;
	default:
		return FALSE;
	}
}

STATUS parse_tcp(struct ethhdr *eth_hdr, struct iphdr *ip_hdr, uint16_t port_id, uint32_t *flow_index)
{
	pkt_info_t pkt_info;
	struct tcphdr *tcp_hdr = (struct tcphdr *)(ip_hdr + 1);

	memcpy(&pkt_info, eth_hdr, sizeof(struct ethhdr));
	pkt_info.ip_proto = ip_hdr->protocol;
	pkt_info.ip_dst = ip_hdr->daddr;
	pkt_info.ip_src = ip_hdr->saddr;
	pkt_info.dst_port = tcp_hdr->dest;
	pkt_info.src_port = tcp_hdr->source;
	pkt_info.port_id = port_id;

	if (find_flow(pkt_info, flow_index) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

STATUS parse_udp(struct ethhdr *eth_hdr, struct iphdr *ip_hdr, uint16_t port_id, uint32_t *flow_index)
{
	pkt_info_t pkt_info;
	struct udphdr *udp_hdr = (struct udphdr *)(ip_hdr + 1);

	memcpy(&pkt_info, eth_hdr, sizeof(struct ethhdr));
	pkt_info.ip_proto = ip_hdr->protocol;
	pkt_info.ip_dst = ip_hdr->daddr;
	pkt_info.ip_src = ip_hdr->saddr;
	pkt_info.dst_port = udp_hdr->dest;
	pkt_info.src_port = udp_hdr->source;
	pkt_info.port_id = port_id;

	if (find_flow(pkt_info, flow_index) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

STATUS pkt_out_process(packet_out_info_t packet_out_info, dp_io_fds_t *dp_io_fds_head)
{
	U8 *mu = packet_out_info.ofpbuf;
	U16 mulen = packet_out_info.msg_len - sizeof(packet_out_info_t);

	for(int i = 0;; i++) {
		if (i >= 20) {
			puts("reach max number action field in pkt_out");
			return FALSE;
		}
		printf("aciton type = %u\n", packet_out_info.action_info[i].type);
		switch (packet_out_info.action_info[i].type) {
		case PORT:
			dp_drv_xmit(mu, mulen, packet_out_info.action_info[i].port_id, dp_io_fds_head);
			return TRUE;
		case DST_MAC:
			memcpy(mu, packet_out_info.action_info[i].dst_mac, ETH_ALEN);
			break;
		case SRC_MAC:
			memcpy(mu+ETH_ALEN, packet_out_info.action_info[i].src_mac, ETH_ALEN);
			break;
		case ETHER_TYPE:
			((struct ethhdr *)mu)->h_proto = htons(packet_out_info.action_info[i].ether_type);
			break;
		case DST_IP:
			((struct iphdr *)(((struct ethhdr *)mu) + 1))->daddr = htonl(packet_out_info.action_info[i].ip_dst);
			break;
		case SRC_IP:
			((struct iphdr *)(((struct ethhdr *)mu) + 1))->saddr = htonl(packet_out_info.action_info[i].ip_src);
			break;
		case IP_PROTO:
			((struct iphdr *)(((struct ethhdr *)mu) + 1))->protocol = packet_out_info.action_info[i].ip_proto;
			break;
		case DST_PORT:
			*(uint16_t *)(((struct iphdr *)(((struct ethhdr *)mu) + 1)) + 1) = htons(packet_out_info.action_info[i].dst_port);
			break;
		case SRC_PORT:
			*(((uint16_t *)(((struct iphdr *)(((struct ethhdr *)mu) + 1)) + 1)) + 1) = htons(packet_out_info.action_info[i].src_port);	
			break;
		default:
			break;
		}
	}
	return FALSE;
}