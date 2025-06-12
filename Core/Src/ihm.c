#include "ihm.h"

#define RX_BUFFER_SIZE 64
#define MAX_SF 12
#define MIN_SF 6
#define MIN_TEMP -20.0f
#define MAX_TEMP 80.0f
#define MIN_HUMIDITY 0.0f
#define MAX_HUMIDITY 1.0f
#define MIN_WINDSPEED 0.0f
#define MAX_WINDSPEED 280.0f
#define MIN_RADIATION 0.5f
#define MAX_RADIATION 40.0f
#define MIN_KC 0
#define MAX_KC 2
#define MIN_KP 0
#define MAX_KP 1
#define MIN_ET0        0.0f
#define MAX_ET0       30.0f
#define MIN_ETC        0.0f
#define MAX_ETC       30.0f
#define MIN_AIR_PRESSURE  10.0f
#define MAX_AIR_PRESSURE 160.0f
#define MIN_HEIGHT        0.0f
#define MAX_HEIGHT     1000.0f
#define MIN_GPS_TIME 00000000
#define MAX_GPS_TIME 23595959
#define MIN_GPS_ALT   -430.0f
#define MAX_GPS_ALT   12000.0f
#define MIN_GPS_LAT -90.0f
#define MAX_GPS_LAT  90.0f
uint8_t date;
uint8_t month;
uint8_t year;
uint8_t hour;
uint8_t minutes;
uint8_t seconds;
uint8_t day;
uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t new_buff[RX_BUFFER_SIZE];
uint8_t rxByte;
uint8_t rxIndex = 0;
uint8_t rx_flag;
uint8_t txBuffer[200];
uint8_t MessageBufferFlag=0;
uint8_t xBytesSent ;
uint8_t received_data[64];
MessageBufferHandle_t  IhmMessageBufferHandle=NULL;
const size_t xIhmMessageBufferSizeBytes = 100;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
uint8_t processing =0;
volatile uint8_t retour=0;
char *tokens[10];
Menu currentMenu=Main_Menu;
Lora LoraValues = {7, 5, 6};
GPS_Data MyGps={545.4,3723.2475,12365500};
SENSORS SensorsValues={27.5f,26.0f,0.5f,0.6f,10.0f,50.5f,1.4f,0.7f,5.8f,7.2f};
uint8_t Sf_New_Value;
uint8_t cmd_buff[180];
uint8_t Cr_New_Value;
uint8_t Bw_New_Value;
uint32_t TimeGps_New_Value;
float AltGps_New_Value;
float LatGps_New_Value;
float SoilTemp_New_Value;
float AirTemp_New_Value;
float RelativeHumidity_New_Value;
float SoilHumidity_New_Value;
float Radiation_New_Value;
float WindSpeed_New_Value;
float Kc_New_Value;
float Kp_New_Value;
float Et0_New_Value;
float Etc_New_Value;
float EtcAdj_New_Value;
float AirPressure_New_Value;
float Height_New_Value ;
uint8_t Old_Default_Sf=7;
uint8_t Old_Default_Bw=5;
uint8_t Old_Default_Cr=6;
float Old_Default_SoilTemp;
float Old_Default_AirTemp;
float Old_Default_RH;
float Old_Default_SH;
float Old_Default_Radiation;
float Old_Default_WS;
float Old_Default_KC;
float Old_Default_KP;
float Old_Default_ET0;
float Old_Default_ETC;
float Old_Default_ETCadj;
float Old_Default_AirPressure;
float Old_Default_Heigh;
float Old_Default_TimeGPS;
float Old_Default_AltGPS;
float Old_Default_LatGPS;

//Useful functions for the code
void UpperCase(char *str){
	while (*str)
	{
		*str=toupper(*str);
		str++;
	}
}

void Start_IHM_Task(void const * argument)
{
	/* USER CODE BEGIN Start_IHM_Task */
	MainMenu();
	IhmMessageBufferHandle = xMessageBufferCreate(xIhmMessageBufferSizeBytes);
	if( IhmMessageBufferHandle != NULL )
	{
	}
	else
	{
		HAL_UART_Transmit(&huart2,(uint8_t*)"Error in MessageBuffer Creation\r\n", 34, 100);
	}

	/* Infinite loop */
	for(;;)
	{
		xMessageBufferReceive( IhmMessageBufferHandle, received_data, sizeof(received_data), portMAX_DELAY);
		memset(new_buff,0,sizeof(new_buff));
		UpperCase((char*)received_data);
		tokenization((char*)received_data);
		ParseCommand();
		memset(received_data,0,sizeof(received_data));
		osDelay(1);
	}
	/* USER CODE END Start_IHM_Task */
}

// Fonction pour afficher le menu principal complet


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	  if (huart==&huart2)
	{if (!processing)
	{ if ((rxByte == '\r')||(rxByte=='\n'))
	{   if (rxIndex!=0)
	{
		processing=1;
		rxBuffer[rxIndex] = '\0';
		memcpy(new_buff,rxBuffer,rxIndex);
		xBytesSent=xMessageBufferSendFromISR(IhmMessageBufferHandle,new_buff,strlen((char*)new_buff),&xHigherPriorityTaskWoken);
		//The number of bytes actually written to the message buffer.  If the
		// * message buffer didn't have enough free space for the message to be stored
		// * then 0 is returned, otherwise xDataLengthBytes is returned.
		if( xBytesSent != strlen((char*)new_buff))
		{
			HAL_UART_Transmit(&huart2, (const uint8_t *)"Message sent !=buffer data\r\n",26,100);
		}
		memset(rxBuffer,0,sizeof(rxBuffer));
		rxIndex = 0;}
	}

	else { if (rxIndex > 0){

		if (rxByte == '\b') {
			rxBuffer[rxIndex]=' ';
			rxIndex=rxIndex-1;
			rxBuffer[rxIndex]=' ';
			retour=rxIndex-1;
			HAL_UART_Transmit(huart, (uint8_t *)" \b", 2, 100);

			//	for (uint8_t i=0;i< strlen((char*)rxBuffer);i++)
			//{rxBuffer[i]=rxBuffer[retour++];
			//if (i==retour)
			//rxBuffer[i]='\0';}
		}
	}
	if (rxIndex < RX_BUFFER_SIZE-1) {

		rxBuffer[rxIndex++] = rxByte;
	}
	else {
		rxIndex=0;
	}
	}
	}
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}
}

//tableau de liste des commandes
CMD cmd_list[]={
		{"LORA",(char*)":TO ACCESS LORA MENU WRITE LORA",LoraMenu,Main_Menu},
		{"SENSORS",(char*)":TO ACCESS SENSORS MENU WRITE SENSORS",SensorsMenu,Main_Menu},
		{"GPS",(char*)":TO ACCESS GPS MENU WRITE GPS",GPSMenu,Main_Menu},
		{"SYSCONF",(char*)":TO ACCESS SYSTEM CONFIGURATION WRITE SYSCONF",SysConfigMenu,Main_Menu},
		//Lora Menu Command List
		{"SETSF",(char*)":TO SET SF VALUE WRITE SETSF 7\r\n[Possible Values :{6,7,8,9,10,11,12}]",SetSF_f,Lora_Menu},
		{"GETSF",(char*)":TO GET SF VALUE WRITE GETSF",GetSF_f,Lora_Menu},
		{"SETCR",(char*)":TO SET CR VALUE WRITE SETCR VALUE\r\n[Value for each CR :{4/5 -> 1 ; 4/6 -> 2 ; 4/7 -> 3 ; 4/8 -> 4}] ",SetCR_f,Lora_Menu},
		{"GETCR",(char*)":TO GET CR WRITE GETCR",GetCR_f,Lora_Menu},
		{"SETBW",(char*)":TO SET BANDWIDTH VALUE WRITE SETBW 4 \r\n[Values for BW :{125Khz->4 ; 250Khz->5 ; 500Khz->6}] ",SetBW_f,Lora_Menu},
		{"GETBW",(char*)":TO GET BANDWIDTH VALUE WRITE GETBW\r\nNote : you must respect the Values provided ",GetBW_f,Lora_Menu},
		//Sensors Menu Command List
		{"GETST",(char*)":TO GET SOIL TEMPERATURE VALUE WRITE GETST",GetSoilTemp_f,Sensors_Menu},
		{"GETAT",(char*)":TO GET AIR TEMPERATURE VALUE WRITE GETAT",GetAirTemp_f,Sensors_Menu},
		{"GETAP",(char*)":TO GET AIR PRESSURE VALUE WRITE GETAP",GetAirPressure_f,Sensors_Menu},
		{"GETRH",(char*)":TO GET RELATIVE HUMIDITY VALUE WRITE GETRH",GetRelativeHumidity_f,Sensors_Menu},
		{"GETSH",(char*)":TO GET SOIL HUMIDITY VALUE WRITE GETSH",GetSoilHumidity_f,Sensors_Menu},
		{"GETWS",(char*)":TO GET WIND SPEED VALUE WRITE GETWS",GetWindSpeed_f,Sensors_Menu},
		{"SETH",(char*)":TO SET THE HEIGH VALUE IN METERS WRITE SETH 2\r\n",SetHeigh_f,Sensors_Menu},
		{"GETH",(char*)":TO GET THE HEIGH VALUE IN METERS WRITE GETH \r\n",GetHeigh_f,Sensors_Menu},
		{"SETR",(char*)":TO SET RADIATION VALUE WRITE SETR 7.3\r\nPossible Values [0,40]",SetRadiation_f,Sensors_Menu},
		{"GETR",(char*)":TO GET RADIATION VALUE WRITE GETR",GetRadiation_f,Sensors_Menu},
		{"SETKC",(char*)":TO SET Kc VALUE WRITE SETKC 1.12",SetKc_f,Sensors_Menu},
		{"GETKC",(char*)":TO GET Kc VALUE WRITE GETKC",GetKc_f,Sensors_Menu},
		{"SETKP",(char*)":TO SET Kp VALUE WRITE SETKP 0.7\r\nPossible Values [0,1]",SetKp_f,Sensors_Menu},
		{"GETKP",(char*)":TO GET Kp VALUE WRITE GETKP",GetKp_f,Sensors_Menu},
		{"SETET0",(char*)":TO SET ET0 VALUE WRITE SETET0 8.4",SetET0_f,Sensors_Menu},
		{"SETET0",(char*)":TO GET ET0 VALUE WRITE GETET0",GetET0_f,Sensors_Menu},
		{"SETETC",(char*)":TO SET ETC=Kc*ET0 VALUE WRITE SETETC 6.8",SetETC_f,Sensors_Menu},
		{"GETETC",(char*)":TO GET ETC VALUE WRITE GETETC",GetETC_f,Sensors_Menu},
		{"SETETCADJ",(char*)":TO SET ETc(adj)=Kc*Kp*ET0 VALUE WRITE SETETCADJ\r\nNote: ETc(adj) must be < than ETc",SetETCadj_f,Sensors_Menu},
		{"GETETCADJ",(char*)":TO GET ET0 VALUE WRITE GETET0\r\nPossible values [0,30]",GetETCadj_f,Sensors_Menu},
		//GPS Menu Command List
		{"SETALT",(char*)"TO SET ALTITUDE VALUE WRITE SETALT\r\nPossible Values : [-430.0,12000.0]",SetAltGPS_f,GPS_Menu},
		{"GETALT",(char*)":TO GET ALTITUDE VALUE WRITE GETALT",GetAltGPS_f,GPS_Menu},
		{"SETLAT",(char*)":TO SET LATITUDE VALUE WRITE SETLAT\r\nPossible Values : [-90.0,90.0]",SetLatGPS_f,GPS_Menu},
		{"GETLAT",(char*)":TO GET LATITUDE VALUE WRITE GETLAT",GetLatGPS_f,GPS_Menu},
		{"SETUTC",(char*)":TO SET TIME VALUE WRITE SETUTC 12361500\r\n 12361500 is equivalent to 12H36 minutes and 15 seconds",SetTimeGPS_f,GPS_Menu},
		{"GETUTC",(char*)":TO GET TIME VALUE WRITE GETUTC",GetTimeGPS_f,GPS_Menu},
		//SystemConfig Menu
		{"SAVE",(char*)":TO SAVE MODIFIED PARAMETERS PERMANENTLY WRITE SAVE",Save_f,SysConfig_Menu},
		{"RESTORE",(char*)":TO RESTORE OLD PARAMETERS WRITE RESTORE",Restore_f,SysConfig_Menu},

};
void MainMenu(void) {
	// Afficher tout le menu une seule fois
	currentMenu=Main_Menu;
	char* weekday_str[] = {
			"Invalid",      // index 0 (not used)
			"Monday",       // 1
			"Tuesday",      // 2
			"Wednesday",    // 3
			"Thursday",     // 4
			"Friday",       // 5
			"Saturday",     // 6
			"Sunday"        // 7
	};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	if (Reset_Flag==1)
	{

		day=sDate.WeekDay;
		date=sDate.Date;
		month=sDate.Month;
		year=sDate.Year;
		hour=sTime.Hours;
		minutes=sTime.Minutes;
		seconds=sTime.Seconds;
		Reset_Flag=0;
	}

	sprintf((char*)txBuffer,"\033[1;30;107m----------------Main Menu---------------\033[0m\n \r\nDate : %s , %02d/%02d/%02d        Session opened at: %02d:%02d:%02d\r\n",
			weekday_str[day],date,month, 2000+year,hour,minutes, seconds);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Main_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);

}
void tokenization(char *str) //function to tokenize input string
{
	tokens[0]=strtok(str," ");
	for (uint8_t i=1; i<10;i++)
	{   tokens[i]=strtok(NULL," ");
	if (tokens[i]==NULL)
		break;
	}
}
uint8_t cl_elements=sizeof(cmd_list)/sizeof(cmd_list[0]);
void ParseCommand() {
	uint8_t c=0;
	uint8_t correspond=0;
	uint8_t true=0;
	uint8_t goback=0;
	if (strcmp(tokens[0],"..")==0)
	{MainMenu();
	goback=1;}
	while (c<cl_elements)
	{if (strcmp(tokens[0], cmd_list[c].Name)== 0)
	{ true=1;
	if (currentMenu==cmd_list[c].MenuIndex)
	{cmd_list[c].handler(tokens[1]);
	correspond=1;}
	else HAL_UART_Transmit(&huart2, (uint8_t*)"Wrong Menu\r\n",strlen("Wrong Menu\r\n"),100);
	break;
	}
	c++;
	}
	if (true==0 && correspond==0 && goback==0)
		HAL_UART_Transmit(&huart2, (uint8_t*)"COMMAND ERROR\r\n",16,100);

	processing=0;
}

/*//////////////////////////////////////////////LORA MENU\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void LoraMenu(char* arg){

	currentMenu=Lora_Menu;
	sprintf((char*)txBuffer,"\033[1;38;2;25;25;112;107m--------------- LORA Menu ---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Lora_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
}
void SetSF_f(char* arg){
	uint8_t success = 0;

	if (tokens[1] != NULL && strlen(tokens[1]) < 3) {
		int sf_new_value = atoi(tokens[1]);

		if (sf_new_value >= MIN_SF && sf_new_value <= MAX_SF) {
			sprintf((char*)cmd_buff, "SF VALUE SET TO %d SUCCESSFULLY\r\n", sf_new_value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			success = 1;
		}
	}

	if (success==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));  // always clear at the end
}
/*void SaveSF_f(char* arg){
switch (sf_new_value)
{
case 5:
	LoraValues.sf_l=0x05;
case 6:
	LoraValues.sf_l=0x06;
case 7:
	LoraValues.sf_l=0x07;
case 8:
	LoraValues.sf_l=0x08;
case 9:
	LoraValues.sf_l=0x09;
case 10:
	LoraValues.sf_l=0x0A;
case 11:
	LoraValues.sf_l=0x0B;
case 12:
	LoraValues.sf_l=0x0C;
}

}*/
void GetSF_f(char* arg)
{
	sprintf((char*)cmd_buff,"SF VALUE IS %d \r\nNote: DEFAULT VALUE IS %d",Sf_New_Value,LoraValues.sf_l);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetCR_f(char* arg)
{   uint8_t cr_flag=0;
char* CR_Values[]={"1","2","3","4"};
for (uint8_t cr=0 ; cr<sizeof(CR_Values) / sizeof(CR_Values[0]);cr++)
{if (tokens[1]!=NULL &&
		strcmp(tokens[1],CR_Values[cr])==0)
{Cr_New_Value=atoi(tokens[1]);
cr_flag=1;
sprintf((char*)cmd_buff, "CR VALUE SET TO %d SUCCESSFULLY\r\n", Cr_New_Value);
HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
memset(cmd_buff,0,sizeof(cmd_buff));
break;
}
}

if (cr_flag==0)
	HAL_UART_Transmit(&huart2,(const uint8_t*)"WRONG CR VALUE\r\n",strlen("WRONG CR VALUE\r\n"), 100);
}

void GetCR_f(char* arg)
{
	sprintf((char*)cmd_buff,"CR VALUE IS %d \r\nNote:DEFAULTVALUE IS : %d\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Cr_New_Value,LoraValues.cr_l);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetBW_f(char* arg)
{
	uint8_t bw_flag=0;
	char* BW_Values[]={"4","5","6"};
	for (uint8_t bw=0 ; bw<sizeof(BW_Values) / sizeof(BW_Values[0]);bw++)
	{if (tokens[1]!=NULL &&
			strcmp(tokens[1],BW_Values[bw])==0)
	{Bw_New_Value=atoi(tokens[1]);
	bw_flag=1;
	sprintf((char*)cmd_buff, "BW VALUE SET TO %d SUCCESSFULLY\r\n", Bw_New_Value);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
	break;
	}
	}

	if (bw_flag==0)
		HAL_UART_Transmit(&huart2,(const uint8_t*)"WRONG BW VALUE\r\n",strlen("WRONG CR VALUE\r\n"), 100);
}
void GetBW_f(char* arg)
{
	sprintf((char*)cmd_buff,"BW VALUE IS : %d \r\nNote:DEFAULT VALUE IS : %d\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Bw_New_Value,LoraValues.bw_l);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}


/*//////////////////////////////////////////////GPS MENU\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void GPSMenu(char* arg){
	currentMenu=GPS_Menu;
	sprintf((char*)txBuffer,"\033[1;34;107m-----------------GPS Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==GPS_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
}

void SetAltGPS_f(char* arg){
	uint8_t gpsalt_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 12) {
		AltGps_New_Value = atoi(tokens[1]);
		if (TimeGps_New_Value >MIN_GPS_ALT && TimeGps_New_Value <MAX_GPS_ALT){
			sprintf((char*)cmd_buff, "GPS ALTITUDE VALUE SET TO %.2f SUCCESSFULLY\r\n", AltGps_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			gpsalt_flag = 1;
		}


		if (gpsalt_flag==0) {
			HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
		}

		memset(cmd_buff, 0, sizeof(cmd_buff));

	}
}
void GetAltGPS_f(char* arg){
	sprintf((char*)cmd_buff,"GPS ALTITUDE VALUE IS : %.2f \r\nGPS ALTITUDE  DEFAULT VALUE IS : %.2f\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",AltGps_New_Value,MyGps.alt_gps);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetLatGPS_f(char* arg){
	uint8_t gpslat_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 12) {
		LatGps_New_Value = atoi(tokens[1]);
		if (LatGps_New_Value >MIN_GPS_LAT && TimeGps_New_Value <MAX_GPS_LAT){
			sprintf((char*)cmd_buff, "GPS LATITUDE VALUE SET TO %.2f SUCCESSFULLY\r\n", LatGps_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			gpslat_flag = 1;
		}


		if (gpslat_flag==0) {
			HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
		}

		memset(cmd_buff, 0, sizeof(cmd_buff));

	}
}
void GetLatGPS_f(char* arg){
	sprintf((char*)cmd_buff,"GPS LATITUDE VALUE IS : %.2f \r\nGPS LATITUDE  DEFAULT VALUE IS : %.2f\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",LatGps_New_Value,MyGps.lat_gps);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetTimeGPS_f(char* arg){
	uint8_t gpstime_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 12) {
		TimeGps_New_Value = atol(tokens[1]);
		if (TimeGps_New_Value >MIN_GPS_TIME && TimeGps_New_Value <MAX_GPS_TIME){
			sprintf((char*)cmd_buff, "GPS TIME VALUE SET TO %lu SUCCESSFULLY\r\n", TimeGps_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			gpstime_flag = 1;
		}


		if (gpstime_flag==0) {
			HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
		}

		memset(cmd_buff, 0, sizeof(cmd_buff));

	}
}
void GetTimeGPS_f(char* arg){
	sprintf((char*)cmd_buff,"GPS TIME VALUE IS : %lu \r\nGPS TIME  DEFAULT VALUE IS : %lu\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",TimeGps_New_Value,MyGps.time_gps);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

/*////////////////////////////////////////////// SENSORS MENU\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void SensorsMenu(char* arg){
	currentMenu=Sensors_Menu;
	sprintf((char*)txBuffer,"\033[1;32;107m--------------Sensors Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Sensors_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
}

void GetSoilTemp_f(char* arg){
	sprintf((char*)cmd_buff,"SOIL TEMPERATURE VALUE IS : %.2f \r\n",SensorsValues.SoilTemp_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void GetAirTemp_f(char* arg){
	sprintf((char*)cmd_buff,"AIR TEMPERATURE VALUE IS : %.2f°C",SensorsValues.AirTemp_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void GetRelativeHumidity_f(char* arg){
	sprintf((char*)cmd_buff,"RELATIVE HUMIDITY VALUE IS : %.2f \r\n",SensorsValues.RelativeHumidity_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void GetSoilHumidity_f(char* arg){
	sprintf((char*)cmd_buff,"SOIL HUMIDITY VALUE IS : %.2f",SensorsValues.SoilHumidity_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void GetWindSpeed_f(char* arg){
	sprintf((char*)cmd_buff,"WIND SPEED VALUE IS : %.2f IN Km/h\r",SensorsValues.WindSpeed_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetRadiation_f(char* arg){
	uint8_t radiation_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Radiation_New_Value = atof(tokens[1]);

		if (Radiation_New_Value >= MIN_RADIATION && Radiation_New_Value <= MAX_RADIATION) {
			sprintf((char*)cmd_buff, "RADIATION VALUE SET TO %.2f SUCCESSFULLY\r\n", Radiation_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			radiation_flag = 1;
		}
	}

	if (radiation_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));
}
void GetRadiation_f(char* arg){
	sprintf((char*)cmd_buff,"RADIATION VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Radiation_New_Value,SensorsValues.Radiation_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetKc_f(char* arg){
	uint8_t kc_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Kc_New_Value = atof(tokens[1]);

		if (RelativeHumidity_New_Value >= MIN_KC && Sf_New_Value <= MAX_KP) {
			sprintf((char*)cmd_buff, "Kc VALUE SET TO %.2f SUCCESSFULLY\r\n", Kc_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			kc_flag = 1;
		}
	}

	if (kc_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));
}
void GetKc_f(char* arg){
	sprintf((char*)cmd_buff,"KC VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Kc_New_Value,SensorsValues.Kc);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetKp_f(char* arg){
	uint8_t kp_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Kp_New_Value = atof(tokens[1]);

		if (Kp_New_Value >= MIN_KP && Sf_New_Value <= MAX_KP) {
			sprintf((char*)cmd_buff, "Kp VALUE SET TO %.2f SUCCESSFULLY\r\n", Kp_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			kp_flag = 1;
		}
	}

	if (kp_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));
}
void GetKp_f(char* arg){
	sprintf((char*)cmd_buff,"KP VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Kp_New_Value,SensorsValues.Kp);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetET0_f(char* arg){
	uint8_t et0_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Et0_New_Value = atof(tokens[1]);

		if (Et0_New_Value >= MIN_ET0 && Et0_New_Value <= MAX_ET0) {
			sprintf((char*)cmd_buff, "ET0 VALUE SET TO %.2f SUCCESSFULLY\r\n", Et0_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			et0_flag = 1;
		}
	}

	if (et0_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));
}
void GetET0_f(char* arg){
	sprintf((char*)cmd_buff,"ET0 VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Et0_New_Value,SensorsValues.ET0);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void SetETC_f(char* arg){
	uint8_t etc_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Etc_New_Value = atof(tokens[1]);
		if (Etc_New_Value >= MIN_ETC && Etc_New_Value <= MAX_ETC) {
			sprintf((char*)cmd_buff, "ETC VALUE SET TO %.2f SUCCESSFULLY\r\n", Etc_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			etc_flag = 1;
		}
	}
	if (etc_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}
	memset(cmd_buff, 0, sizeof(cmd_buff));
}

void GetETC_f(char* arg){
	sprintf((char*)cmd_buff,"ETC VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Etc_New_Value,SensorsValues.ETc);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void SetETCadj_f(char* arg){
	uint8_t etcadj_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		EtcAdj_New_Value = atof(tokens[1]);
		if (EtcAdj_New_Value >= 0 && EtcAdj_New_Value <= Old_Default_ETC) {
			sprintf((char*)cmd_buff, "ETC ADJ VALUE SET TO %.2f SUCCESSFULLY\r\n", EtcAdj_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			etcadj_flag = 1;
		}
	}
	if (etcadj_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}
	memset(cmd_buff, 0, sizeof(cmd_buff));
}

void GetETCadj_f(char* arg){
	sprintf((char*)cmd_buff,"ETC ADJ VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",EtcAdj_New_Value,SensorsValues.ETcAdj);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void SetAirPressure_f(char* arg){
	uint8_t airp_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 6) {
		AirPressure_New_Value = atof(tokens[1]);
		if (AirPressure_New_Value >= 80.0f && AirPressure_New_Value <= 110.0f) { // typical pressure range
			sprintf((char*)cmd_buff, "AIR PRESSURE VALUE SET TO %.2f SUCCESSFULLY\r\n", AirPressure_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			airp_flag = 1;
		}
	}
	if (airp_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}
	memset(cmd_buff, 0, sizeof(cmd_buff));
}

void GetAirPressure_f(char* arg){
	sprintf((char*)cmd_buff,"AIR PRESSURE VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",AirPressure_New_Value,SensorsValues.AirPressure_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void SetHeigh_f(char* arg){
	uint8_t height_flag = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		Height_New_Value = atof(tokens[1]);
		if (Height_New_Value >= 0.0f && Height_New_Value <= 5000.0f) {
			sprintf((char*)cmd_buff, "HEIGHT VALUE SET TO %.2f SUCCESSFULLY\r\n", Height_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			height_flag = 1;
		}
	}
	if (height_flag==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}
	memset(cmd_buff, 0, sizeof(cmd_buff));
}

void GetHeigh_f(char* arg){
	sprintf((char*)cmd_buff,"HEIGHT VALUE IS : %.2f\r\nNote:DEFAULT VALUE IS : %.2f \r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",Height_New_Value,SensorsValues.AirPressure_s);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}

void SysConfigMenu(char* arg){
	currentMenu=SysConfig_Menu;
	sprintf((char*)txBuffer,"------------------ SYSTEM CONFIGURATION MENU -----------------\r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==SysConfig_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}

}
void Save_f(char* arg){
	//Saving Lora Values
	Old_Default_Sf=LoraValues.sf_l;
	LoraValues.sf_l=Sf_New_Value;
	sprintf((char*)txBuffer,"The default SF Value is now %d\r\n",LoraValues.sf_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	Old_Default_Bw=LoraValues.bw_l;
	LoraValues.bw_l=Bw_New_Value;
	sprintf((char*)txBuffer,"The default Bandwidth Value is now %d\r\n",LoraValues.bw_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	Old_Default_Cr=LoraValues.cr_l;
	LoraValues.cr_l=Cr_New_Value;
	sprintf((char*)txBuffer,"The default CR buffer is now %d\r\n",LoraValues.cr_l);

	//Saving GPS Values
	Old_Default_AltGPS=MyGps.alt_gps ;
	sprintf((char*)cmd_buff, "The default GPS ALTITUDE is now %.2f\r\n",MyGps.alt_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	Old_Default_LatGPS=MyGps.lat_gps ;
	sprintf((char*)cmd_buff, "The default GPS LATITUDE is now %.2f\r\n",MyGps.lat_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	Old_Default_TimeGPS=MyGps.time_gps ;
	sprintf((char*)cmd_buff, "The default GPS TIME is now %lu\r\n",MyGps.time_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	//Saving Sensors Values

	Old_Default_Radiation = SensorsValues.Radiation_s;
	SensorsValues.Radiation_s = Radiation_New_Value;
	sprintf((char*)txBuffer, "The default Radiation is now %.2f\r\n", SensorsValues.Radiation_s);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_KC = SensorsValues.Kc;
	SensorsValues.Kc = Kc_New_Value;
	sprintf((char*)txBuffer, "The default Kc value is now %.2f\r\n", SensorsValues.Kc);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_KP = SensorsValues.Kp;
	SensorsValues.Kp = Kp_New_Value;
	sprintf((char*)txBuffer, "The default Kp value is now %.2f\r\n", SensorsValues.Kp);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_ET0 = SensorsValues.ET0;
	SensorsValues.ET0 = Et0_New_Value;
	sprintf((char*)txBuffer, "The default ET0 value is now %.2f\r\n", SensorsValues.ET0);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_ETC = SensorsValues.ETc;
	SensorsValues.ETc = Etc_New_Value;
	sprintf((char*)txBuffer, "The default ETc value is now %.2f\r\n", SensorsValues.ETc);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_ETCadj = SensorsValues.ETcAdj;
	SensorsValues.ETcAdj = EtcAdj_New_Value;
	sprintf((char*)txBuffer, "The default ETcAdj value is now %.2f\r\n", SensorsValues.ETcAdj);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	Old_Default_Heigh = Height_New_Value;
	sprintf((char*)txBuffer, "The default Height is now %.2f\r\n", Old_Default_Heigh);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

}
void Restore_f(char* arg){
	//Restoring Lora Values
	LoraValues.sf_l=Old_Default_Sf;
	sprintf((char*)txBuffer,"SF value restored to %d\r\n",LoraValues.sf_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	LoraValues.bw_l=Old_Default_Bw;
	sprintf((char*)txBuffer,"BW value restored to %d\r\n",LoraValues.bw_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	LoraValues.cr_l=Old_Default_Cr;
	sprintf((char*)txBuffer,"CR value restored to %d\r\n",LoraValues.cr_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	//Restoring GPS Values
	MyGps.alt_gps = Old_Default_AltGPS;
	sprintf((char*)cmd_buff, "GPS ALTITUDE restored to %.2f\r\n",MyGps.alt_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	MyGps.lat_gps = Old_Default_LatGPS;
	sprintf((char*)cmd_buff, "GPS LATITUDE restored to %.2f\r\n",MyGps.lat_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	MyGps.time_gps = Old_Default_TimeGPS;
	sprintf((char*)cmd_buff, "GPS TIME restored to %lu\r\n",MyGps.time_gps);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
	//Restoring Sensors Values

	SensorsValues.Radiation_s = Old_Default_Radiation;
	sprintf((char*)cmd_buff, "Radiation restored to %.2f\r\n", SensorsValues.Radiation_s);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	SensorsValues.Kc = Old_Default_KC;
	sprintf((char*)cmd_buff, "Kc restored to %.2f\r\n", SensorsValues.Kc);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	SensorsValues.Kp = Old_Default_KP;
	sprintf((char*)cmd_buff, "Kp restored to %.2f\r\n", SensorsValues.Kp);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	SensorsValues.ET0 = Old_Default_ET0;
	sprintf((char*)cmd_buff, "ET0 restored to %.2f\r\n", SensorsValues.ET0);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	SensorsValues.ETc = Old_Default_ETC;
	sprintf((char*)cmd_buff, "ETC restored to %.2f\r\n", SensorsValues.ETc);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	SensorsValues.ETcAdj = Old_Default_ETCadj;
	sprintf((char*)cmd_buff, "ETC Adjusted restored to %.2f\r\n", SensorsValues.ETcAdj);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

	Height_New_Value = Old_Default_Heigh;
	sprintf((char*)cmd_buff, "Height restored to %.2f\r\n", Height_New_Value);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);

}



