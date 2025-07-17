/*
 * SatellitePredict.c
 *
 *  Created on: Jun 23, 2025
 *      Author: ThinkPad
 */
#include "SatellitePredict.h"
#include "math.h"
#define ONE_SEC_IN_DAYS   (1.0/86400.0)
#define SCAN_STEP_SEC     10
#define SCAN_STEP_DAYS    (SCAN_STEP_SEC/86400.0)
#define MIN_EL_DEG        10.0
#define MIN_EL_RAD        (MIN_EL_DEG * M_PI/180.0)
char buff[600];
time_t timestamp;
time_t first_los_utc=0;
#ifdef SATELLITE
//#ifdef SATELLITE
void Start_SatellitePrediction_Task(void const * argument)
{
	/* USER CODE BEGIN Start_SatellitePrediction_Task */
	const char *tle_line_1 = "1 25544U 98067A   25189.21335687  .00007259  00000-0  13303-3 0  9996";
	const char *tle_line_2 = "2 25544  51.6336 202.8559 0002499 346.6817  13.4106 15.50431524518421";

	// Create orbit object
	predict_orbital_elements_t *iss = predict_parse_tle(tle_line_1, tle_line_2);
	if (!iss) {
		HAL_UART_Transmit(&huart2,(const uint8_t *)"Failed to initialize orbit from tle!", strlen("Failed to initialize orbit from tle!"),100);
		exit(1);
	}

	// Create observer object
	//predict_observer_t *obs = predict_create_observer("My Ground Station", MyGps.lat_gps*M_PI/180.0, MyGps.long_gps*M_PI/180.0, MyGps.alt_gps);
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
	list_next_passes(obs, iss, 10, julian_time);

	for(;;)
	{
		time_t now=rtc_to_time_t();
		if (now>first_los_utc)
		{
			predict_julian_date_t  jd       = predict_to_julian(now);
			list_next_passes(obs, iss, 10, jd);
		}

		// Add if actual timestamp is less than the last pass then execute again  list next passes
		osDelay(10000);

		/* USER CODE END Start_SatellitePrediction_Task */
	}
}
time_t rtc_to_time_t()
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	time_t raw;
	// 2) Fill struct tm with the *raw* RTC time (assumed local)
	struct tm timeinfo = { 0 };
	timeinfo.tm_hour =sTime.Hours;
	timeinfo.tm_min  =  sTime.Minutes;
	timeinfo.tm_sec  = sTime.Seconds ;
	timeinfo.tm_year = sDate.Year + 100;
	timeinfo.tm_mon  = sDate.Month - 1;
	timeinfo.tm_mday = sDate.Date;
	timeinfo.tm_isdst = 0;

	// 3) Get a “local” time_t
	raw = mktime(&timeinfo);
	raw -= ((MyGps.offset/100) * 3600)+((MyGps.offset%100)*60);

	// 5) Now raw is the true UTC time_t
	timestamp = raw;
	return timestamp;}


time_t list_next_passes(predict_observer_t *obs,predict_orbital_elements_t *sat,
		uint8_t count,predict_julian_date_t start_time_julian)
{
	double elevation_step;
	predict_julian_date_t t = start_time_julian;
	for(int i = 0; i < count; i++) {
		// Next AOS prediction
		struct predict_observation aos =predict_next_aos(obs, sat, t);
		// 2) rough horizon-set to bound the pass
		struct predict_observation los0 =predict_next_los(obs, sat,aos.time + ONE_SEC_IN_DAYS);

		// 3) scan between AOS and LOS0 in 10 s steps for true peak
		double best_el = -1e9, best_t = aos.time;
		for(elevation_step=aos.time;elevation_step <= los0.time;elevation_step += SCAN_STEP_DAYS)
		{
			struct predict_position pos;
			predict_orbit(sat, &pos, elevation_step);

			struct predict_observation obsr;
			predict_observe_orbit(obs, &pos, &obsr);

			if (obsr.elevation > best_el) {
				best_el = obsr.elevation;
				best_t  = elevation_step;
			}
		}

		// 4) skip if peak < 10°
		if (best_el < MIN_EL_RAD) {
			t = los0.time + ONE_SEC_IN_DAYS;
			i--;    // don’t count it
			continue;
		}

		// 5) now find the *real* LOS after that peak
		struct predict_observation los =
				predict_next_los(obs, sat,
						best_t + ONE_SEC_IN_DAYS);

		// 6) convert & print
		time_t aos_t = predict_from_julian(aos.time);
		time_t max_t = predict_from_julian(best_t);
		time_t los_t = predict_from_julian(los.time);
		if (i == 0)
		{first_los_utc = los_t;}

		int    dur    = (int)(los_t - aos_t);
		struct tm aos_tm, max_tm, los_tm;
		{
			struct tm *tmp = gmtime(&aos_t);
			if (tmp) memcpy(&aos_tm, tmp, sizeof(struct tm));
		}
		{
			struct tm *tmp = gmtime(&max_t);
			if (tmp) memcpy(&max_tm, tmp, sizeof(struct tm));
		}
		{
			struct tm *tmp = gmtime(&los_t);
			if (tmp) memcpy(&los_tm, tmp, sizeof(struct tm));
		}

		snprintf(buff, sizeof(buff),
				"-------------------- NEXT ISS PASS --------------------\r\n"
				"AOS     : %04d-%02d-%02d %02d:%02d:%02d UTC\r\n"
				"LOS     : %02d:%02d:%02d UTC\r\n"
				"Durée   : %02d:%02d min\r\n"
				"Max El  : %.1f° at %02d:%02d:%02d UTC\r\n"
				"Az AOS  : %.1f°\r\n"
				"Az LOS  : %.1f°\r\n"
				"--------------------------------------------------------\r\n\r\n",
				// date & time from aos_tm:
				aos_tm.tm_year + 1900, aos_tm.tm_mon + 1, aos_tm.tm_mday,
				aos_tm.tm_hour,      aos_tm.tm_min,      aos_tm.tm_sec,
				los_tm.tm_hour,      los_tm.tm_min,      los_tm.tm_sec,
				dur/60, dur%60, best_el * 180.0/M_PI,
				max_tm.tm_hour,max_tm.tm_min,max_tm.tm_sec,
				aos.azimuth * 180.0/M_PI,
				los.azimuth * 180.0/M_PI
		);
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,strlen(buff),HAL_MAX_DELAY);

		// 7) advance to after this LOS
		t = los.time + ONE_SEC_IN_DAYS;
	}

	return first_los_utc;
}
#endif
