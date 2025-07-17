/*
 * mygps.h
 */

#ifndef MYGPS_H_
#define MYGPS_H_
#include "ihm.h"
#include "gps.h"
#include "rtc.h"
typedef struct {
		float alt_gps;
		float lat_gps ;
		float long_gps;
}GPS_IHM;
typedef struct {
	int hours;
	int minutes;
	int seconds;
	int day;
	int month;
	int year;
	int offset;
}GPS_RTC;
extern GPS_IHM gps_to_ihm;
extern GPS_RTC gps_to_rtc;
void Sentence_parse(char* str);
void RTC_SetFromGPS(GPS_RTC *gps_rtc);
void GPS_GetFromRTC(GPS_RTC *gps_rtc);


#endif /* MYGPS_H_ */
