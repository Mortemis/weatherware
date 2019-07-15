#ifndef ARP_H_
#define ARP_H_
//--------------------------------------------------
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc28j60.h"
#include "net.h"
//--------------------------------------------------
uint8_t arp_read(enc28j60_frame_ptr *frame, uint16_t len);
void arp_send(enc28j60_frame_ptr *frame);
uint8_t arp_request(uint8_t *ip_addr);
void arp_table_fill(enc28j60_frame_ptr *frame);
//--------------------------------------------------
typedef struct arp_record{
  uint8_t ipaddr[4];
  uint8_t macaddr[6];
  uint32_t sec; //какое было количество секунд в переменной clock_cnt, когда была сделана запись
} arp_record_ptr;
//-------------------------------------------------- 
#endif /* ARP_H_ */
