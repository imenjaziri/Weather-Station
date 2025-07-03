/*
 * mygps.h
 *
 *  Created on: Jun 12, 2025
 *      Author: ThinkPad
 */

#ifndef MYGPS_H_
#define MYGPS_H_
#include "ihm.h"
#include "gps.h"
#include "rtc.h"
void Sentence_parse(char* str);
void RTC_SetFromGPS(GPS_Data *gps_rtc);

#endif /* MYGPS_H_ */
