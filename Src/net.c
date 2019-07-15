#include "net.h"
//-----------------------------------------------
extern UART_HandleTypeDef huart1;
//-----------------------------------------------
uint8_t net_buf[ENC28J60_MAXFRAME];
extern uint8_t macaddr[6];
uint8_t ipaddr[4]=IP_ADDR;
uint32_t clock_cnt=0;//seconds counter
char str1[60]={0};
extern char str[20];
USART_prop_ptr usartprop;
//-----------------------------------------------
void net_ini(void)
{
	usartprop.usart_buf[0]=0;
	usartprop.usart_cnt=0;
	usartprop.is_ip=0;
	HAL_UART_Transmit(&huart1,(uint8_t*)"123456\r\n",8,0x1000);				
	enc28j60_ini();
}
//-----------------------------------------------
uint16_t checksum(uint8_t *ptr, uint16_t len, uint8_t type)
{
	uint32_t sum=0;
	 if(type==1)
	{
		sum+=IP_UDP;
		sum+=len-8;
	}
	while(len>0)
	{
		sum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
		ptr+=2;
		len-=2;
	}
	if(len) sum+=((uint32_t)*ptr)<<8;
	while(sum>>16) sum=(uint16_t)sum+(sum>>16);
	return ~be16toword((uint16_t)sum);
}
//-----------------------------------------------
void eth_send(enc28j60_frame_ptr *frame, uint16_t len)
{
	memcpy(frame->addr_dest,frame->addr_src,6);
	memcpy(frame->addr_src,macaddr,6);
	enc28j60_packetSend((void*)frame,len+sizeof(enc28j60_frame_ptr));
}
//-----------------------------------------------
uint8_t ip_send(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	//Заполним заголовок пакета IP
	ip_pkt->len=be16toword(len);
	ip_pkt->fl_frg_of=0;
	ip_pkt->ttl=128;
	ip_pkt->cs=0;
	memcpy(ip_pkt->ipaddr_dst,ip_pkt->ipaddr_src,4);
	memcpy(ip_pkt->ipaddr_src,ipaddr,4);
	ip_pkt->cs=checksum((void*)ip_pkt,sizeof(ip_pkt_ptr),0);
	//отправим фрейм
	eth_send(frame,len);
	return res;
}
//-----------------------------------------------
uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	icmp_pkt_ptr *icmp_pkt = (void*)(ip_pkt->data);
	if((len>=sizeof(icmp_pkt_ptr))&&(icmp_pkt->msg_tp==ICMP_REQ))
	{
//		sprintf(str1,"icmp request\r\n");
//		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		icmp_pkt->msg_tp=ICMP_REPLY;
		icmp_pkt->cs=0;
		icmp_pkt->cs=checksum((void*)icmp_pkt,len,0);
		ip_send(frame,len+sizeof(ip_pkt_ptr));

		sprintf(str1,"%d.%d.%d.%d-%d.%d.%d.%d icmp request\r\n",
		    ip_pkt->ipaddr_dst[0],ip_pkt->ipaddr_dst[1],ip_pkt->ipaddr_dst[2],ip_pkt->ipaddr_dst[3],
		    ip_pkt->ipaddr_src[0],ip_pkt->ipaddr_src[1],ip_pkt->ipaddr_src[2],ip_pkt->ipaddr_src[3]);
		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	}
	return res;
}
//-----------------------------------------------
uint8_t ip_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res = 0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	if((ip_pkt->verlen==0x45)&&(!memcmp(ip_pkt->ipaddr_dst,ipaddr,4)))
	{
		//длина данных
		len = be16toword(ip_pkt->len) - sizeof(ip_pkt_ptr);
		if(ip_pkt->prt==IP_ICMP)
		{
			icmp_read(frame,len);
		}
		else if(ip_pkt->prt==IP_TCP)
		{
		}
		else if(ip_pkt->prt==IP_UDP)
		{
			udp_read(frame,len);
		}
	}
	return res;
}
//-----------------------------------------------
void ip_extract(char* ip_str, uint8_t len, uint8_t* ipextp)
{
	uint8_t offset = 0;
  uint8_t i;
  char ss2[5] = {0};
  char *ss1;
  int ch = '.';
	for(i=0;i<3;i++)
	{
		ss1 = strchr(ip_str,ch);
		offset = ss1-ip_str+1;
		strncpy(ss2,ip_str,offset);
		ss2[offset]=0;
		ipextp[i] = atoi(ss2);
		ip_str+=offset;
		len-=offset;
	}
	strncpy(ss2,ip_str,len);
	ss2[len]=0;
	ipextp[3] = atoi(ss2);
}
//-----------------------------------------------
void eth_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	if(len>sizeof(enc28j60_frame_ptr))
	{
		if(frame->type==ETH_ARP)
		{
//			sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; arp\r\n",
//				frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
//				frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
//				frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
//				frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
//			HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
			res = arp_read(frame,len-sizeof(enc28j60_frame_ptr));
			if(res==1)
			{
				arp_send(frame);
			}
			else if(res==2)
			{
				arp_table_fill(frame);
			}
		}
		if(frame->type==ETH_IP)
		{
//			sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r\n",
//				frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
//				frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
//				frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
//				frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
//			HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
			ip_read(frame,len-sizeof(ip_pkt_ptr));
		} else {
		  sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; %04X",
		  frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
		  frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
		  len, be16toword(frame->type));
		  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		  HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
		}
	}
}
//-----------------------------------------------
void net_poll(void)
{
	uint16_t len;
	enc28j60_frame_ptr *frame=(void*)net_buf;
	while((len=enc28j60_packetReceive(net_buf,sizeof(net_buf)))>0)
	{
		eth_read(frame,len);
	}
}
//-----------------------------------------------
void UART1_RxCpltCallback(void)
{
	uint8_t b;
	b=str[0];
	//если вдруг случайно превысим длину буфера
	if(usartprop.usart_cnt>20)
	{
		usartprop.usart_cnt=0;
	}
	else if(b == 'a')
	{
		usartprop.is_ip=1;//статус отправки ARP-запроса
		net_cmd();
	}
	else
	{
		usartprop.usart_buf[usartprop.usart_cnt] = b;
		usartprop.usart_cnt++;
	}
	HAL_UART_Receive_IT(&huart1, (uint8_t*)str,1);
}
//-----------------------------------------------
void TIM_PeriodElapsedCallback(void)
{
  //считаем секунды и записываем их в clock_cnt
  clock_cnt++;
}
//-----------------------------------------------
void net_cmd(void)
{
 uint8_t ip[4]={0};
 if(usartprop.is_ip==1)//статус отправки ARP-запроса
 {
   HAL_UART_Transmit(&huart1,usartprop.usart_buf,usartprop.usart_cnt,0x1000);
   HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
   ip_extract((char*)usartprop.usart_buf,usartprop.usart_cnt,ip);
   arp_request(ip);
   usartprop.is_ip = 0;
   usartprop.usart_cnt=0;
 }

}

//-----------------------------------------------
