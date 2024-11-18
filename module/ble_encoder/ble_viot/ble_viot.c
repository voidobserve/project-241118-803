/****************************************************************************
@FILENAME:	ble_viot.c
@FUNCTION:	encoder/decoder for ble iot packet
@AUTHOR:	flege.zhang
@DATE:		2020.05.20
@COPYRIGHT(C) ZXW 2020
*****************************************************************************/


#include "ble_viot.h"
#include "string.h"
#include "stdlib.h"


#define BLE_ADV_PDU_MAX_LENGTH										(31)
#define ADV_COMPID													(0xDCDC)

const uint8_t encodetable[] = {0x41,0x92,0x53,0x2A,0xFC,0xAB,0xCE,0x26,0x0D,0x1E,0x99,0x78,0x00,0x22,0x99,0xDE};

uint8_t ble_viot_last_rx_count = 0x0;


uint32_t reflect(uint32_t val, uint8_t bits)
{
	uint32_t reflect_val = 0;

	while (bits > 0) {
		reflect_val = (reflect_val << 1) | (val & 1);
		val >>= 1;
		bits--;
	}

	return reflect_val;
}

uint16_t viot_crc16_ccitt(uint8_t *data,uint8_t len,uint16_t crc_init)
{	
	uint16_t crc16 = crc_init;
	uint8_t i, j;
	
	for(i = 0x0; i < len; i++)
	{
		uint8_t temp = reflect((uint32_t)data[i],8);//refin:reflect input
		crc16 ^= temp << 8;

		for(j = 8; j > 0; j--)
		{
			if(crc16&0x8000)
			{
				crc16 = (crc16<<1)^0x1021;
			}
			else
			{
				crc16 = (crc16<<1);
			}
		}
	}
	
	crc16 =  reflect((uint32_t)crc16,16);//refout:reflect output
	
	return crc16;
}

uint8_t ble_viot_encoder(str_ble_viot_para* opcode_para,uint8_t rand_seed,uint8_t* out_data,uint8_t* out_data_len)
{
	uint8_t ble_payload[BLE_ADV_PDU_MAX_LENGTH] = {0x0};
	uint8_t *p_ble_payload = ble_payload;
	uint8_t tx_rand;
	uint16_t crc;

	//validation check
	if(opcode_para == 0x0)
	{
		return 0xF1;
	}

	if(out_data == 0x0)
	{
		return 0xF2;
	}

	*p_ble_payload++ = 0x02;
	*p_ble_payload++ = 0x01;
	*p_ble_payload++ = 0x02;

	// ad structure 2
	*p_ble_payload++ = 0x1B;
	*p_ble_payload++ = 0x03;
	*p_ble_payload++ = ADV_COMPID & 0xFF;
	*p_ble_payload++ = ADV_COMPID >> 8;
	*p_ble_payload++ = opcode_para->para[0];	 // 按键键值
	*p_ble_payload++ = opcode_para->addr & 0xFF; // 2字节的滚码（32位取低16位）
	*p_ble_payload++ = opcode_para->addr >> 8;
	*p_ble_payload++ = 0xC3;
	*p_ble_payload++ = opcode_para->count;
	*p_ble_payload++ = 0xC3;
	*p_ble_payload++ = 0x16;
	*p_ble_payload++ = 0x17;
	*p_ble_payload++ = 0x18;
	*p_ble_payload++ = 0x19;
	*p_ble_payload++ = 0x1A;
	*p_ble_payload++ = 0x1B;
	*p_ble_payload++ = 0x1C;
	*p_ble_payload++ = 0x1D;
	*p_ble_payload++ = 0x1E;

#if 0
	//generate rand by seed
	srand(rand_seed + 0x55);
	tx_rand = rand();
	
	/*build ble payload*/

	//ad structure 1
	*p_ble_payload++ = 0x02;
	*p_ble_payload++ = 0x01;
	*p_ble_payload++ = 0x02;

	//ad structure 2
	*p_ble_payload++ = 0x1B;
	*p_ble_payload++ = 0x05;

	*p_ble_payload++ = ADV_COMPID&0xFF;
	*p_ble_payload++ = ADV_COMPID>>8;

	*p_ble_payload++ = opcode_para->rfu2[0];
	*p_ble_payload++ = opcode_para->rfu2[1];
	*p_ble_payload++ = opcode_para->rfu2[2];
	*p_ble_payload++ = opcode_para->rfu2[3];
	*p_ble_payload++ = opcode_para->rfu2[4];
	*p_ble_payload++ = opcode_para->relay;
	
	*p_ble_payload++ = opcode_para->type;//13
	*p_ble_payload++ = opcode_para->version;
	*p_ble_payload++ = opcode_para->count;
	*p_ble_payload++ = opcode_para->addr&0xFF;
	*p_ble_payload++ = opcode_para->addr>>8;
	*p_ble_payload++ = opcode_para->addr>>16;
	*p_ble_payload++ = opcode_para->addr>>24;
	*p_ble_payload++ = opcode_para->group_index;
	*p_ble_payload++ = opcode_para->cmd;
	*p_ble_payload++ = opcode_para->para[0];
	*p_ble_payload++ = opcode_para->para[1];
	*p_ble_payload++ = opcode_para->para[2];
	*p_ble_payload++ = opcode_para->rfu[0];
	*p_ble_payload++ = opcode_para->rfu[1];
	*p_ble_payload++ = opcode_para->rfu[2];
	*p_ble_payload++ = tx_rand;

	crc = viot_crc16_ccitt(ble_payload + 13,16,0x5555);
	*p_ble_payload++ = crc;
	*p_ble_payload++ = crc>>8;


	//ENCODE BY RAND,from group_index to rfu,8byts
	for(uint8_t i = 20; i < 28; i++)
	{
		ble_payload[i] ^= tx_rand;
	}

	//ENCODE BY ENCODETABLE,from type to rfu,16byts
	for(uint8_t i = 13; i < 29; i++)
	{
		ble_payload[i] ^= encodetable[i - 13];
	}
	#endif

	//copy the encoded data to out buffer
	*out_data_len = BLE_ADV_PDU_MAX_LENGTH;
	memcpy(out_data,ble_payload,BLE_ADV_PDU_MAX_LENGTH);

	return 0x00;
}

uint8_t is_ble_viot_pdu(uint8_t* ble_payload,uint8_t payload_len)
{
	//check length
	if(payload_len != BLE_ADV_PDU_MAX_LENGTH)
	{
		return 0xF0;
	}

	//check compid
	if(((ble_payload[6]<<8)|ble_payload[5]) != ADV_COMPID)
	{
		return 0xF1;
	}
	return 0x00;
}

uint8_t ble_viot_decoder(uint8_t* ble_payload,uint8_t payload_len,str_ble_viot_para* opcode_para)
{
	uint8_t tx_rand;
	uint16_t cal_crc,crc;
	//DECODE BY ENCODETABLE,from type to rfu,16byts
	for(uint8_t i = 13; i < 29; i++)
	{
		ble_payload[i] ^= encodetable[i - 13];
	}

	tx_rand = ble_payload[28];

	//DECODE BY RAND,from group_index to rfu,8byts
	for(uint8_t i = 20; i < 28; i++)
	{
		ble_payload[i] ^= tx_rand;
	}
	//check count
	if(ble_viot_last_rx_count == ble_payload[15])
	{
		return 0x01;
	}

	//check crc
	//check crc
	cal_crc = viot_crc16_ccitt(ble_payload + 13,16,0x5555);
	crc = (ble_payload[30]<<8)|ble_payload[29];

	if(cal_crc != crc)
	{
		return 0x02;
	}

	ble_viot_last_rx_count = ble_payload[15];
	
	opcode_para->rfu[0] 		= ble_payload[7];
	opcode_para->rfu[1] 		= ble_payload[8];
	opcode_para->rfu[2] 		= ble_payload[9];
	opcode_para->rfu[3] 		= ble_payload[10];
	opcode_para->rfu[4] 		= ble_payload[11];
	opcode_para->relay 			= ble_payload[12];
	
	opcode_para->type 			= ble_payload[13];
	opcode_para->version 		= ble_payload[14];
	opcode_para->count 			= ble_payload[15];
	opcode_para->addr 			= (ble_payload[19]<<24)|(ble_payload[18]<<16)|(ble_payload[17]<<8)|ble_payload[16];
	opcode_para->group_index 	= ble_payload[20];
	opcode_para->cmd 			= ble_payload[21];
	opcode_para->para[0] 		= ble_payload[22];
	opcode_para->para[1] 		= ble_payload[23];
	opcode_para->para[2] 		= ble_payload[24];
	opcode_para->rfu[0] 		= ble_payload[25];
	opcode_para->rfu[1] 		= ble_payload[26];
	opcode_para->rfu[2] 		= ble_payload[27];

	return 0x0;
}
