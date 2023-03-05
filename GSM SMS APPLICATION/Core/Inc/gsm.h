/*
 * gsm.h
 *
 *  Created on: 07-Feb-2022
 *      Author: hariom.intern
 */

#include "main.h"
#ifndef SRC_GSM_H_
#define SRC_GSM_H_

#define MAX_BUFFER_SIZE	512

extern uint8_t gsm_state;


//flag used
typedef struct{
	volatile unsigned start : 1;
	volatile unsigned init : 1;
	volatile unsigned sim_status : 1;



} gsm_flags;

typedef enum{
	FALSE,
	TRUE
} my_bool;

// enum for different gsm states
typedef enum{
	GSM_INIT,
	GSM_NETWORK,
	GSM_DATA_PACKET,
	GSM_SMS_TASK,
	GSM_SLEEP,
	GSM_RESET

}GSM_STATE;



// Structure for GSM Command
typedef struct{
	char gsm_cmd[50];
	char gsm_response[50];
	char response_status[20];
	int timeout;
} hgsm;

extern hgsm SIM_NO ;
extern hgsm GSM_IMEI;
extern hgsm Signal_strength;
extern hgsm SIM_Service_Provider;
extern hgsm Echo_ON;
extern hgsm Echo_OFF;
extern hgsm GSM_IMSI;
extern hgsm R_TIME;
extern hgsm Read_SMS;
extern hgsm Send_SMS;
extern hgsm DEL_ALL_MSG;
extern hgsm SIM_STATUS;
extern hgsm List_UN_MSG;
extern hgsm Check_Storage;

// Structure for GSM
typedef struct{
	unsigned char gsm_rx_data[MAX_BUFFER_SIZE];
	unsigned char gsm_tx_data[MAX_BUFFER_SIZE];
	int count;

	gsm_flags FLAG;

} gsm;

extern gsm gsm_main;

// structure for gsm info
typedef struct{
	unsigned char IMEI[16];
	unsigned char IMSI[16];

} sim_info;

extern sim_info SIM_INFO;

// Structure for Real time clock
typedef struct{
	unsigned char date[9];                 // Current Date
	unsigned char time[12];                // Current Time
}real_time_clock;

extern real_time_clock rtc;

typedef struct{
	unsigned char phone[14];
	unsigned char msg[MAX_BUFFER_SIZE];
	unsigned char date[9];                 // message Date
	unsigned char time[12];				  // message Date
} HSMS;

extern HSMS sms_main;


/* Exported functions prototypes ---------------------------------------------*/
void Send_AT_Command(hgsm* );
void Extract_Date_Time();
char* jump_to_character(char*,char);
void extract_substring(char *dest_buffer,char *ptr1,char *ptr2);
void send_sms(char*);
void extract_sms(char*);
void process_command(char *cmd);
my_bool compare_string(char*,char*);
my_bool check_sim_status();
my_bool verify_response(hgsm*);
void IMEI_Number();
void IMSI_Number();
char* string_concat(char* , char*);
void MX_USART3_UART_Init(void);
/* Exported functions prototypes ---------------------------------------------*/


#endif /* SRC_GSM_H_ */

