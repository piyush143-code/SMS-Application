/*
 * gsm.c
 *
 *  Created on: 07-Feb-2022
 *      Author: hariom.intern
 */
#include"gsm.h"
#include "main.h"


hgsm Echo_OFF = {"ATE0","","",450};     	             // to turn the ECHO off
hgsm Echo_ON = {"ATE1","","",450};	                     // to turn the ECHO on
hgsm SIM_NO = {"AT+CICCID","","",450};	                 // to fetch SIM Number
hgsm Signal_strength = {"AT+CSQ","","",450};             // to fetch signal quality
hgsm GSM_IMEI = {"AT+CGSN","","",450};	                     // to fetch IMEI Number
hgsm GSM_IMSI = {"AT+CIMI","","",450};	                     // to fetch IMSI Number
hgsm SIM_Service_Provider = {"AT+CSPN?","","",450}; 	 // to fetch Service Provider Name
hgsm R_TIME = {"AT+CCLK?","","",450};					 // to fetch Real Time
hgsm Send_SMS ={"AT+CMGS=\"+918708167707\"","","",450};
hgsm Read_SMS = {"AT+CMGRD=0","","",450};
hgsm DEL_ALL_MSG = {"AT+CMGD=0,4","","",450};
hgsm SIM_STATUS = {"AT+CPIN?","\r\n+CPIN: READY\r\n\r\nOK\r\n","",450};
hgsm List_UN_MSG = {"AT+CMGL=\"REC UNREAD\"","","",450};
hgsm Check_Storage = {"AT+CPMS?","","",450};


int init_packet_sent = 0;



gsm gsm_main;
real_time_clock rtc;   // real_time_clock structure variable
HSMS sms_main;

sim_info SIM_INFO;
/*

* @name Send_AT_Command()
* @brief Send GSM AT command function to gsm module
* It prepares the command for GSM module through ATCOMMAND structure
* Transmit AT command to the GSM module  through HAL transmit function
* Once the command is send it starts a timer for given timeout
* @param ATCOMMAND *atcommand
* @return void

*/
void Send_AT_Command(hgsm* atcommand)
{
	// empty response buffer
	int i = 0;
	for(i = 0; i < gsm_main.count; i++){
		gsm_main.gsm_rx_data[i] = '\0';
	}

	gsm_main.count = 0;

	// transmit at command data
	i = 0;
	while(atcommand->gsm_cmd[i] != '\0'){
		HAL_UART_Transmit(&huart3,&(atcommand->gsm_cmd[i]),1,100);
		i++;
	}
//	HAL_UART_Transmit(&huart3,atcommand->gsm_cmd,10,200);
	// \r \n in command
	HAL_UART_Transmit(&huart3,"\r\n",2,100);

	//wait for time
	HAL_Delay(atcommand->timeout);

	return;
}

/*
* @name Extract_Date_Time()
* @brief Extract Date and Time from gsm_rx_data (work for both ECHO_ON and ECHO_OFF)
* It traverses the gsm_rx_data and extract the date and time string and
* stores them in real_time_clock structure.
* @param none
* @return void
*/

void Extract_Date_Time()
{

	// Taking ptr to the starting character of date i.e. (1 more than ")
	char *start = jump_to_character(gsm_main.gsm_rx_data,'"');
	start++;

	// jump to end char of date (,)
	char *end = jump_to_character(start,',');

	// Extracting date

	extract_substring(rtc.date,start,end);

	// Taking ptr to the starting character of time i.e. (which is 1 step more from ,)
	start = ++end;

	// jump to end char of time (")
	end = jump_to_character(start,'"');

	// Extracting time
	extract_substring(rtc.time,start,end);

}

/*

* @name jump_to_character()
* @brief It takes pointer to first occurrence of specified character
* @param char *ptr (pointer to buffer), char ch (specified character)
* @return pointer to specified character

*/

char* jump_to_character(char* ptr,char ch){

	while(*ptr != ch)
		ptr++;

	return ptr;
}

/*

* @name extract_substring()
* @brief Extract a substring out of a string given the start (included) and end pointers (excluded)
* @param Pointer to destination buffer, start pointer, end pointer
* @return void

*/

void extract_substring(char *dest_buffer,char *ptr1,char *ptr2){

	while(ptr1 != ptr2)
	{
		(*dest_buffer++) = (*ptr1++);
//		dest_buffer++;
//		ptr1++;
	}
}

/*

* @name send_sms()
* @brief Send SMS to the number provided (here, +91-8708167707)
* @param message buffer
* @return void

*/

void send_sms(char *msg)
{
	Send_AT_Command(&Send_SMS);

	int i=0;

	while(msg[i] != '\0'){
		HAL_UART_Transmit(&huart3,&(msg[i]),1,100);
		i++;
	}

	// \r \n in command
	HAL_UART_Transmit(&huart3,"\r\n",2,100);

	char ctrl_z = 26;
	HAL_UART_Transmit(&huart3,&ctrl_z,1,100);
	HAL_Delay(Send_SMS.timeout);

}

/*

* @name extract_sms()
* @brief Extract the content of the response , for eg., Phone number, Message date, Message time and the msg
* @param rx_data buffer (response received from GSM)
* @return void

*/

void extract_sms(char* buffer)
{
	char *start,*end;
	start = jump_to_character(buffer,',');
	start+=2;

	end = jump_to_character(start,'"');

	extract_substring(sms_main.phone,start,end);

	start = end+6;

	end = jump_to_character(start,',');
	extract_substring(sms_main.date,start,end);

	start = end+1;

	end = jump_to_character(start,'"');
	extract_substring(sms_main.time,start,end);



	int i=0;
	for(i=0;i<512;i++)
		sms_main.msg[i]='\0';

	start = end+3;
	end = jump_to_character(start,'\r');
	extract_substring(sms_main.msg,start,end);

}

/*

* @name process_command()
* @brief Perform instructions based on the user's command
* @param command received from the user
* @return void

*/

void process_command(char *cmd)
{
	if(cmd==NULL)
		return;

	// if cmd == blue0, reset blue led and red led

	if(compare_string(cmd,"blue0")){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_RESET);
		send_sms("Ok Sir,blue turned off");

	}

	// if cmd == blue1, set blue led and reset red led

	else if(compare_string(cmd,"blue1")){
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_RESET);
		send_sms("Ok Sir,blue turned ON");

	}

	// otherwise, reset blue led and set red led

	else{
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);
		send_sms("Sir please verify ur cmd");
	}
}

/*

* @name compare_string()
* @brief compare two strings
* @param starting pointer of the two strings
* @return TRUE if both strings are equal else FALSE

*/

my_bool compare_string(char* str1,char* str2)
{

	int i = 0;

	while(str1[i] !='\0' && str2[i]!='\0')
	{
		if(str1[i] != str2[i])
			return FALSE;

		i++;
	}

	if(str1[i]!='\0' || str2[i]!='\0')
		return FALSE;

	return TRUE;

}

/*

* @name IMEI_Number()
* @brief extract IMEI Number and store it into SIM_INFO.IMEI
* @param void
* @return void

*/

void IMEI_Number()
{
	Send_AT_Command(&GSM_IMEI);
	char *start = jump_to_character(gsm_main.gsm_rx_data,'\n');
	start++;
	char *end = jump_to_character(start,'\r');
	extract_substring(SIM_INFO.IMEI,start,end);

}

/*

* @name IMSI_Number()
* @brief extract IMSI Number and store it into SIM_INFO.IMSI
* @param void
* @return void

*/

void IMSI_Number()
{
	Send_AT_Command(&GSM_IMSI);
	char *start = jump_to_character(gsm_main.gsm_rx_data,'\n');
	start++;
	char *end = jump_to_character(start,'\r');
	extract_substring(SIM_INFO.IMSI,start,end);

}

/*

* @name check_sim_status()
* @brief check sim connectivity atmost 3 times
* @param void
* @return TRUE if sim connected else FALSE

*/

my_bool check_sim_status()
{

		int retry = 3;

		while(retry--)
		{
		    Send_AT_Command(&SIM_STATUS);

		    if(verify_response(&SIM_STATUS))
		    	return TRUE;

		}

		return FALSE;

}

/*

* @name gsm_task()
* @brief Perform various operations based on current gsm states
* @param void
* @return void

*/


uint8_t gsm_state;
void gsm_task()
{
	switch(gsm_state)
	{
	    // First state of GSM TASK, i.e., GSM Initialization
		case GSM_INIT:
		{
			if(gsm_main.FLAG.start == TRUE && gsm_main.FLAG.init == FALSE)
			{
				MX_USART3_UART_Init();
				if(HAL_UART_Receive_IT(&huart3,&rx_buffer,1) == HAL_OK)
				{
					gsm_main.FLAG.init = TRUE;
				}
			}

			Send_AT_Command(&Echo_OFF);
			Send_AT_Command(&DEL_ALL_MSG);

			if(gsm_main.FLAG.init == TRUE)
			{
				// if sim is connected , extract sim information
				if(check_sim_status())
				{
					IMEI_Number();
					IMSI_Number();
					gsm_main.FLAG.sim_status = TRUE;
					gsm_state = GSM_DATA_PACKET;

				}

				// else Blink Red LED
				else{
					while(1)
					{
						HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
						HAL_Delay(200);
					}

				}
			}
		}
		break;

		// Check Signal Strength
		case GSM_NETWORK:
		{

			Send_AT_Command(&Signal_strength);
			gsm_state = GSM_DATA_PACKET;


		}
		break;

		// Send Initialization Packet
		case GSM_DATA_PACKET:
		{
			{
				send_sms(string_concat("INIT Connected with : #dsg",SIM_INFO.IMEI+11));
				gsm_state = GSM_SMS_TASK;

			}
		}
		break;

		/*

		 * In Read State , checking for commands from user,
		 * if there is any unread message , read the message
		 * extract the message content and
		 * process the command

		*/
		case GSM_SMS_TASK:
		{
			Send_AT_Command(&List_UN_MSG);

			if(!compare_string(gsm_main.gsm_rx_data,"\r\nOK\r\n")){
				Send_AT_Command(&Read_SMS);
				extract_sms(gsm_main.gsm_rx_data);
				process_command(sms_main.msg);
			}

		}
		break;

		case GSM_SLEEP:
		{
			{

			}
		}
		break;

		case GSM_RESET:
		{

		}
		break;
	}

}

/*

* @name verify_response()
* @brief matches instantaneous response with original response of the given command.
* @param ATCOMMAND
* @return TRUE if response matches with original response else FALSE

*/

my_bool verify_response(hgsm* atcommand)
{
	return compare_string(atcommand->gsm_response,gsm_main.gsm_rx_data);
}

/*

* @name string_concat()
* @brief concatenates second string with first
* @param Starting pointers to two strings
* @return starting pointer to the concatenated string

*/

char* string_concat(char* str1,char *str2)
{
	char *ans = (char*)calloc(sizeof(char),1);
	int i=0;

	while(*str1 != '\0')
	{
		ans[i] = *str1;
		i++;
		ans = realloc(ans,sizeof(char)*(i+1));
		str1++;
	}

	while(*str2 != '\0')
	{
		ans[i] = *str2;
		i++;
		ans = realloc(ans,sizeof(char)*(i+1));
		str2++;
	}
	ans[i]='\0';

	return ans;
}


void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}
