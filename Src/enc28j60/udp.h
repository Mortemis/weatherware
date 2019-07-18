#ifndef UDP_H_
#define UDP_H_

//--------------------------------------------------
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc28j60.h"
#include "net.h"
#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "adc.h"
//--------------------------------------------------
#define LOCAL_PORT 2000

typedef struct udp_pkt {
  uint16_t port_src; //порт отправителя
  uint16_t port_dst; //порт получателя
  uint16_t len; //длина
  uint16_t cs; //контрольная сумма заголовка
  uint8_t data[]; //данные
} udp_pkt_ptr;
//--------------------------------------------------
uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t udp_reply(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t udp_send(uint8_t *ip_addr, uint16_t port);
void process_command(char *cmd);
void check_parity(void);
//--------------------------------------------------
#endif /* UDP_H_ */
