/**
 * @file ui_radarmode_gps.h
 * @brief GPS Status Display for Radar Mode Screen
 * 
 * Updates ui_radarmode UI with real GPS data:
 * - Satellite count
 * - Signal quality status
 * - Signal strength bars
 */

#ifndef UI_RADARMODE_GPS_H
#define UI_RADARMODE_GPS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize GPS UI updates
 */
void UIRadarMode_GPS_Init(void);

/**
 * @brief Update GPS status on radar mode screen
 * Call this periodically (e.g., every 1 second)
 */
void UIRadarMode_GPS_Update(void);

#ifdef __cplusplus
}
#endif

#endif // UI_RADARMODE_GPS_H
