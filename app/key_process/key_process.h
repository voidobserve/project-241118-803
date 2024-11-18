/****************************************************************************
@FILENAME:	key_process.h
@FUNCTION:	key proc
@AUTHOR:	flege.zhang
@DATE:		2020.05.04
*****************************************************************************/


#ifndef _KEY_PROCESS__H_
#define _KEY_PROCESS__H_


#include "app_cfg.h"
#include "ble_packet_send.h"



/********************************MACRO DEFINE********************************/
typedef struct
{
	uint32_t curr_key;
	uint32_t last_key;

	uint8_t  para[3];
	uint8_t  handle_flag;

	uint32_t pressed_time;
}str_key_info;


void key_process_init(void);
void key_process(uint32_t key_value,uint32_t press_time);

#endif
