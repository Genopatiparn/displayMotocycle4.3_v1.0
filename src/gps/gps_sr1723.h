/**
 * @file gps_sr1723.h
 * @brief SR1723 GPS Module Driver
 * 
 * NMEA 0183 protocol parser for SR1723 GPS module
 * Based on existing GPS_Driver but optimized for SR1723
 */

#ifndef GPS_SR1723_H
#define GPS_SR1723_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum satellites to track (for signal strength bars)
#define GPS_MAX_SATELLITES 99

// Satellite information structure
typedef struct {
    uint8_t prn;        // Satellite PRN number
    uint8_t elevation;  // Elevation in degrees (0-90)
    uint16_t azimuth;   // Azimuth in degrees (0-359)
    uint8_t snr;        // Signal-to-Noise Ratio (0-99 dB, 0=not tracking)
    uint8_t constellation; // 0=unknown, 1=GPS, 2=GLONASS, 3=Galileo, 4=BeiDou, 5=QZSS, 6=SBAS
} gps_satellite_t;

// GPS Debug Information Structure
typedef struct {
    char lat[24];           // Latitude string (e.g., "13.746891")
    char lon[24];           // Longitude string (e.g., "100.534191")
    char num_sats[8];       // Number of satellites in use
    char speed_kmh[16];     // Speed in km/h
    char altitude_m[16];    // Altitude in meters
    char utc_time[16];      // UTC time (HH:MM:SS)
    char utc_date[16];      // UTC date (DD/MM/YY)
    char total_sats[8];     // Total satellites visible
    uint8_t fix_quality;    // GGA fix quality
    uint32_t horizontal_accuracy_mm; // UBX NAV-PVT hAcc
} gps_debug_info_t;

typedef struct {
    double latitude_deg;
    double longitude_deg;
    float speed_kmh;
    uint32_t horizontal_accuracy_mm;
    uint32_t updated_at_ms;
    bool valid;
} gps_fix_t;

/**
 * @brief Initialize GPS module (Pure UBX configuration)
 * @return true if initialization successful, false otherwise
 */
bool GPS_Init(void);

/**
 * @brief Process incoming GPS data (call frequently in loop)
 */
void GPS_ProcessIncoming(void);

/**
 * @brief Get GPS debug information
 * @param out Output structure
 */
void GPS_GetDebugInfo(gps_debug_info_t *out);

/** @brief Get one coherent GPS fix snapshot, including position and accuracy. */
void GPS_GetFix(gps_fix_t *out);

/**
 * @brief Check if GPS has valid fix
 * @return true if GPS fix is valid
 */
bool GPS_HasFix(void);

/**
 * @brief Get speed in km/h
 * @return Speed value
 */
float GPS_GetSpeed(void);

/**
 * @brief Get latitude as float
 * @return Latitude in decimal degrees
 */
float GPS_GetLatitude(void);

/**
 * @brief Get longitude as float
 * @return Longitude in decimal degrees
 */
float GPS_GetLongitude(void);

/**
 * @brief Get number of satellites
 * @return Satellite count
 */
uint8_t GPS_GetSatellites(void);

/**
 * @brief Get total visible satellites
 * @return Total visible satellite count
 */
uint8_t GPS_GetVisibleSatellites(void);

/**
 * @brief Get satellite information for signal strength display
 * @param satellites Output array (must have space for GPS_MAX_SATELLITES)
 * @return Number of satellites with valid data
 */
uint8_t GPS_GetSatelliteInfo(gps_satellite_t *satellites);

/**
 * @brief Get GPS signal quality status string
 * @return "Excellent", "Good", "Moderate", "Fair", or "Bad"
 */
const char* GPS_GetQualityStatus(void);

#ifdef __cplusplus
}
#endif

#endif // GPS_SR1723_H
