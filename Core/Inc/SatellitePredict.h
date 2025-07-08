/*
 * SatellitePredict.h
 *
 *  Created on: Jun 23, 2025
 *      Author: ThinkPad
 */

#ifndef INC_SATELLITEPREDICT_H_
#define INC_SATELLITEPREDICT_H_
#include "ihm.h"
#include <predict/predict.h>
extern char buff[600];
time_t rtc_to_time_t();
time_t list_next_passes(predict_observer_t *obs, predict_orbital_elements_t *sat, uint8_t count,predict_julian_date_t start_time_julian);
#endif /* INC_SATELLITEPREDICT_H_ */
