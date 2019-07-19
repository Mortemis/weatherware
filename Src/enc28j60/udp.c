 #include "udp.h"

//--------------------------------------------------
extern UART_HandleTypeDef huart1;
extern char str1[60];
extern uint8_t net_buf[ENC28J60_MAXFRAME];

extern uint32_t photores_value;
extern uint8_t buzz_flag;
extern uint8_t alarm_flag;
//--------------------------------------------------
uint8_t udp_send(uint8_t *ip_addr, uint16_t port)
{
	uint8_t res=0;
	uint16_t len;
	enc28j60_frame_ptr *frame=(void*) net_buf;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	udp_pkt_ptr *udp_pkt = (void*)(ip_pkt->data);
	udp_pkt->port_dst = be16toword(port);
	udp_pkt->port_src = be16toword(LOCAL_PORT);
	strcpy((char*)udp_pkt->data,"UDP Reply:\r\nHello to UDP Client!!!\r\n");
	len = strlen((char*)udp_pkt->data) + sizeof(udp_pkt_ptr);
	udp_pkt->len = be16toword(len);
	udp_pkt->cs=0;
	udp_pkt->cs=checksum((uint8_t*)udp_pkt-8, len+8, 1);
	memcpy(ip_pkt->ipaddr_src,ip_addr,4);
	ip_pkt->prt = IP_UDP;
	ip_pkt->id = 0;
	ip_pkt->ts = 0;
	ip_pkt->verlen = 0x45;
	frame->type=ETH_IP;
	ip_send(frame,len+sizeof(ip_pkt_ptr));
 return res;
}
//--------------------------------------------------
uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	udp_pkt_ptr *udp_pkt = (void*)(ip_pkt->data);

	process_command((char*)udp_pkt->data);
	check_parity();
	udp_reply(frame, len);

	return res;
}
//--------------------------------------------------
uint8_t udp_reply(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	uint16_t port;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	udp_pkt_ptr *udp_pkt = (void*)(ip_pkt->data);
	port = udp_pkt->port_dst;
	udp_pkt->port_dst = udp_pkt->port_src;
	udp_pkt->port_src = port;

	strcpy((char*)udp_pkt->data, str1);
	len = strlen((char*)udp_pkt->data) + sizeof(udp_pkt_ptr);
	udp_pkt->len = be16toword(len);

	udp_pkt->cs=0;
	udp_pkt->cs=checksum((uint8_t*)udp_pkt-8, len+8, 1);
	memcpy(frame->addr_dest,frame->addr_src,6);
	ip_send(frame,len+sizeof(ip_pkt_ptr));

	return res;
}

//BUG: if length is even, controller hard faults
void check_parity() {
	uint16_t l = strlen(str1);
	if (l % 2 == 1)
		strcat(str1, " \r\n");
	else
		strcat(str1, "\r\n");
}
//Process UDP command
void process_command(char cmd[]) {
	if (strcmp(cmd, "buzz\n") == 0) {
		if (alarm_flag) {
			strcpy(str1, "busy");
		} else {
			buzz_flag = 1;
			strcpy(str1, "ok");
		}
	} else if (strcmp(cmd, "sens\n") == 0) {
		sprintf(str1, "light:[%lu%%]", 100 - photores_value * 100 / 4096);
	} else if (strcmp(cmd, "alarm\n") == 0) {
		alarm_flag = 1;
		strcpy(str1, "ok");
	} else if (strcmp(cmd, "alarmoff\n") == 0) {
		alarm_flag = 0;
		strcpy(str1, "ok");
	} else {
		strcpy(str1, "error");
	}
}




