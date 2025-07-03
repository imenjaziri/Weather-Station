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
time_t rtc_to_time_t();
void list_next_passes(predict_observer_t *obs, predict_orbital_elements_t *sat, int count,predict_julian_date_t start_time);
#endif /* INC_SATELLITEPREDICT_H_ */
