#include "arp.h"
//--------------------------------------------------
extern UART_HandleTypeDef huart1;
extern uint8_t ipaddr[4];
extern uint8_t macaddr[6];
extern char str1[60];
extern uint32_t clock_cnt;//счетчик секунд
extern uint8_t net_buf[ENC28J60_MAXFRAME];
uint8_t macbroadcast[6]=MAC_BROADCAST;
uint8_t macnull[6]=MAC_NULL;
arp_record_ptr arp_rec[5];
uint8_t current_arp_index=0;
//--------------------------------------------------
uint8_t arp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	arp_msg_ptr *msg=(void*)(frame->data);
	if(len>sizeof(arp_msg_ptr))
	{
		if((msg->net_tp==ARP_ETH)&&(msg->proto_tp==ARP_IP))
		{
			if(!memcmp(msg->ipaddr_dst,ipaddr,4))
			{
				if(msg->op==ARP_REQUEST)
				{

//					sprintf(str1,"\r\nrequest\r\nmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//						msg->macaddr_src[0],msg->macaddr_src[1],msg->macaddr_src[2],
//						msg->macaddr_src[3],msg->macaddr_src[4],msg->macaddr_src[5]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"ip_src %d.%d.%d.%d\r\n",
//						msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"mac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//						msg->macaddr_dst[0],msg->macaddr_dst[1],msg->macaddr_dst[2],
//						msg->macaddr_dst[3],msg->macaddr_dst[4],msg->macaddr_dst[5]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"ip_dst %d.%d.%d.%d\r\n",
//						msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
					res=1;
				}
				else if(msg->op==ARP_REPLY)
				{
//					sprintf(str1,"\r\nreply\r\nmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//					msg->macaddr_src[0],msg->macaddr_src[1],msg->macaddr_src[2],
//					msg->macaddr_src[3],msg->macaddr_src[4],msg->macaddr_src[5]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"ip_src %d.%d.%d.%d\r\n",
//					msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"mac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//					msg->macaddr_dst[0],msg->macaddr_dst[1],msg->macaddr_dst[2],
//					msg->macaddr_dst[3],msg->macaddr_dst[4],msg->macaddr_dst[5]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//					sprintf(str1,"ip_dst %d.%d.%d.%d\r\n",
//					msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
//					HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
					res=2;
				} 
			}
		}
	}
	return res;
}
//-----------------------------------------------
void arp_send(enc28j60_frame_ptr *frame)
{
	arp_msg_ptr *msg=(void*)(frame->data);
	msg->op = ARP_REPLY;
	memcpy(msg->macaddr_dst,msg->macaddr_src,6);
	memcpy(msg->macaddr_src,macaddr,6);
	memcpy(msg->ipaddr_dst,msg->ipaddr_src,4);
	memcpy(msg->ipaddr_src,ipaddr,4);
	eth_send(frame,sizeof(arp_msg_ptr));

	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X(%d.%d.%d.%d)-",
		frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
		msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X(%d.%d.%d.%d) arp request\r\n",
		frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
	    msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
}
//-----------------------------------------------
uint8_t arp_request(uint8_t *ip_addr)
{
	uint8_t i;
	//проверим, может такой адрес уже есть в таблице ARP, а задодно и удалим оттуда просроченные записи
	for(i=0;i<5;i++)
  {
		//≈сли записи уже более 12 часов, то удалим еЄ
		if((clock_cnt-arp_rec[i].sec)>43200)
		{
			memset(arp_rec+(sizeof(arp_record_ptr)*i),0,sizeof(arp_record_ptr));
		}
		if(!memcmp(arp_rec[i].ipaddr,ip_addr,4))
		{
			//смотрим ARP-таблицу
			for(i=0;i<5;i++)
			{
				sprintf(str1,"%d.%d.%d.%d - %02X:%02X:%02X:%02X:%02X:%02X - %lu\r\n",
				arp_rec[i].ipaddr[0],arp_rec[i].ipaddr[1],arp_rec[i].ipaddr[2],arp_rec[i].ipaddr[3],
				arp_rec[i].macaddr[0],arp_rec[i].macaddr[1],arp_rec[i].macaddr[2],
				arp_rec[i].macaddr[3],arp_rec[i].macaddr[4],arp_rec[i].macaddr[5],
				(unsigned long)arp_rec[i].sec);
				HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
			}
			return 0;
		}
	}
	enc28j60_frame_ptr *frame=(void*)net_buf;
	arp_msg_ptr *msg=(void*)(frame->data);
	msg->net_tp = ARP_ETH;
	msg->proto_tp = ARP_IP;
	msg->macaddr_len = 6;
	msg->ipaddr_len = 4;
	msg->op = ARP_REQUEST;
	memcpy(msg->macaddr_src,macaddr,6);
	memcpy(msg->ipaddr_src,ipaddr,4);	
	memcpy(msg->macaddr_dst,macnull,6);
	memcpy(msg->ipaddr_dst,ip_addr,4);
	memcpy(frame->addr_dest,macbroadcast,6);
	memcpy(frame->addr_src,macaddr,6);
	frame->type = ETH_ARP;
	enc28j60_packetSend((void*)frame,sizeof(arp_msg_ptr) + sizeof(enc28j60_frame_ptr));
	return 1;
}
//--------------------------------------------------
void arp_table_fill(enc28j60_frame_ptr *frame)
{
	uint8_t i;
	arp_msg_ptr *msg=(void*)(frame->data);
	memcpy(arp_rec[current_arp_index].ipaddr,msg->ipaddr_src,4);
	memcpy(arp_rec[current_arp_index].macaddr,msg->macaddr_src,6);
	arp_rec[current_arp_index].sec = clock_cnt;
	if(current_arp_index<4) current_arp_index++;
	else current_arp_index=0;
	for(i=0;i<5;i++)
  {
    sprintf(str1,"%d.%d.%d.%d - %02X:%02X:%02X:%02X:%02X:%02X - %lu\r\n",
      arp_rec[i].ipaddr[0],arp_rec[i].ipaddr[1],arp_rec[i].ipaddr[2],arp_rec[i].ipaddr[3],
      arp_rec[i].macaddr[0],arp_rec[i].macaddr[1],arp_rec[i].macaddr[2],
      arp_rec[i].macaddr[3],arp_rec[i].macaddr[4],arp_rec[i].macaddr[5],
      (unsigned long)arp_rec[i].sec);
    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  } 
}
//--------------------------------------------------
