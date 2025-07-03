/*
 * SatellitePredict.c
 *
 *  Created on: Jun 23, 2025
 *      Author: ThinkPad
 */
#include "SatellitePredict.h"
#include "math.h"
char buff[600];
time_t timestamp;
#define ONE_SEC_IN_DAYS (1.0/86400.0)
void Start_SatellitePrediction_Task(void const * argument)
{
	/* USER CODE BEGIN Start_SatellitePrediction_Task */
	const char *tle_line_1 = "1 25544U 98067A   25183.17152840  .00005361  00000-0  10070-3 0  9993";
	const char *tle_line_2 = "2 25544  51.6357 232.8024 0002162 321.8485 134.2034 15.50326686517486";

	// Create orbit object
	predict_orbital_elements_t *iss = predict_parse_tle(tle_line_1, tle_line_2);
	if (!iss) {
		HAL_UART_Transmit(&huart2,(const uint8_t *)"Failed to initialize orbit from tle!", strlen("Failed to initialize orbit from tle!"),100);
		exit(1);
	}

	// Create observer object
	predict_observer_t *obs = predict_create_observer("My Ground Station", 36.84*M_PI/180.0, 10.9*M_PI/180.0, 60.3);
	if (!obs) {
		HAL_UART_Transmit(&huart2,(const uint8_t *)"Failed to initialize observer!", strlen("Failed to initialize observer!"),100);
	}
	rtc_to_time_t();
	predict_julian_date_t julian_time = predict_to_julian(timestamp);
	struct predict_position iss_orbit;
	predict_orbit(iss, &iss_orbit, julian_time);
	sprintf(buff,"ISS: lat=%f, lon=%f, alt=%f\n", iss_orbit.latitude*180.0/M_PI, iss_orbit.longitude*180.0/M_PI, iss_orbit.altitude);
	HAL_UART_Transmit(&huart2,(const uint8_t *)buff, strlen(buff),100);

	// Observe ISS
	struct predict_observation iss_obs;
	predict_observe_orbit(obs, &iss_orbit, &iss_obs);
	sprintf(buff,"ISS: azi=%f (rate: %f), ele=%f (rate: %f)\n", iss_obs.azimuth*180.0/M_PI, iss_obs.azimuth_rate*180.0/M_PI, iss_obs.elevation*180.0/M_PI, iss_obs.elevation_rate*180.0/M_PI);
	HAL_UART_Transmit(&huart2,(const uint8_t *)buff, strlen(buff),100);
	// Predict ISS
	list_next_passes(obs, iss, 7, julian_time);
	for(;;)
	{
// Add if actual timestamp is less than the last pass then execute again  list next passes
		osDelay(10000);

		/* USER CODE END Start_SatellitePrediction_Task */
	}
}
time_t rtc_to_time_t()
{/*HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);*/
struct tm timeinfo;

timeinfo.tm_year = 25 + 100;      // RTC donne année depuis 2000, tm_year depuis 1900
timeinfo.tm_mon  = 6;       // tm_mon : 0-11, RTC : 1-12
timeinfo.tm_mday = 3;

timeinfo.tm_hour = 10;
timeinfo.tm_min  = 48;
timeinfo.tm_sec  = 30;

timeinfo.tm_isdst = 0;
timestamp = mktime(&timeinfo);
return timestamp;}

void list_next_passes(predict_observer_t *obs, predict_orbital_elements_t *sat, int count, predict_julian_date_t start_time) {
	predict_julian_date_t t = start_time;

	for (int i = 0; i < count; i++) {
		struct predict_observation aos = predict_next_aos(obs, sat, t);
		struct predict_observation max_el = predict_at_max_elevation(obs, sat, aos.time+(60.0/86400.0)  );
		struct predict_observation los = predict_next_los(obs, sat, max_el.time+(60.0/86400.0)  );


		float elevation_deg = max_el.elevation * 180.0 / M_PI;

		time_t aos_t = predict_from_julian(aos.time);
		time_t los_t = predict_from_julian(los.time);
		int duration = (int)(los_t - aos_t);

		struct tm aos_tm_storage, los_tm_storage;
		struct tm *aos_tm = gmtime(&aos_t);
		if (aos_tm) memcpy(&aos_tm_storage, aos_tm, sizeof(struct tm));
		struct tm *los_tm = gmtime(&los_t);
		if (los_tm) memcpy(&los_tm_storage, los_tm, sizeof(struct tm));

		sprintf(buff,
		    "-------------------- NEXT ISS PASS --------------------\r\n"
		    "AOS     : %04d-%02d-%02d %02d:%02d:%02d UTC\r\n"
		    "LOS     : %02d:%02d:%02d UTC\r\n"
		    "Durée   : %02d:%02d min\r\n"
		    "Max El  : %.1f°\r\n"
		    "Az AOS  : %.1f°\r\n"
		    "Az LOS  : %.1f°\r\n"
		    "--------------------------------------------------------\r\n\r\n",
		    aos_tm_storage.tm_year + 1900, aos_tm_storage.tm_mon + 1, aos_tm_storage.tm_mday,
		    aos_tm_storage.tm_hour, aos_tm_storage.tm_min, aos_tm_storage.tm_sec,
		    los_tm_storage.tm_hour, los_tm_storage.tm_min, los_tm_storage.tm_sec,
		    duration / 60, duration % 60,
		    elevation_deg,
		    aos.azimuth * 180.0 / M_PI,
		    los.azimuth * 180.0 / M_PI);

		HAL_UART_Transmit(&huart2, (uint8_t *)buff, strlen(buff), 1000);
		t = los.time + (60.0/86400.0)  ;
	}
}


