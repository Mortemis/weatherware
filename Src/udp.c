 #include "udp.h"

//--------------------------------------------------
extern UART_HandleTypeDef huart1;
extern char str1[60];
extern uint8_t net_buf[ENC28J60_MAXFRAME];
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
	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r\n",
	 frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
	 frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
	 frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
	 frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);

	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	sprintf(str1,"%d.%d.%d.%d-%d.%d.%d.%d udp request\r\n",
	 ip_pkt->ipaddr_src[0],ip_pkt->ipaddr_src[1],ip_pkt->ipaddr_src[2],ip_pkt->ipaddr_src[3],
	 ip_pkt->ipaddr_dst[0],ip_pkt->ipaddr_dst[1],ip_pkt->ipaddr_dst[2],ip_pkt->ipaddr_dst[3]);
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);

	udp_pkt_ptr *udp_pkt = (void*)(ip_pkt->data);
	sprintf(str1,"%u-%u\r\n", be16toword(udp_pkt->port_src),be16toword(udp_pkt->port_dst));
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	HAL_UART_Transmit(&huart1,udp_pkt->data,len-sizeof(udp_pkt_ptr),0x1000);
	HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);

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

	strcpy((char*)udp_pkt->data,"UDP Reply:\r\nHello from UDP Server to UDP Client!!!\r\n");
	len = strlen((char*)udp_pkt->data) + sizeof(udp_pkt_ptr);
	udp_pkt->len = be16toword(len);

	udp_pkt->cs=0;
	udp_pkt->cs=checksum((uint8_t*)udp_pkt-8, len+8, 1);
	memcpy(frame->addr_dest,frame->addr_src,6);
	ip_send(frame,len+sizeof(ip_pkt_ptr));

	return res;
}


