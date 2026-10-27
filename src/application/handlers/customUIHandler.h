#ifndef CUSTOM_UI_HANDLER_H
#define CUSTOM_UI_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Data types that can be displayed
typedef enum {
    DATA_TYPE_MAP = 0,
    DATA_TYPE_BATTERY,
    DATA_TYPE_RPM,
    DATA_TYPE_SPEED,
    DATA_TYPE_ENGINE_TEMP,
    DATA_TYPE_TPS,
    DATA_TYPE_AFR,
    DATA_TYPE_IGNITION,
    DATA_TYPE_INJ_DUTY,
    DATA_TYPE_TINJ,
    DATA_TYPE_OIL_TEMP,
    DATA_TYPE_STF,
    DATA_TYPE_LTF,
    DATA_TYPE_IAT,
    DATA_TYPE_COUNT
} DataType_t;

// Container configuration
typedef struct {
    uint8_t container_id;      // 1-9 for Container1-9
    uint8_t position;          // Display position 1-9
    DataType_t data_type;
    bool enabled;
} ContainerConfig_t;

// Lifecycle
void CustomUIHandler_Init(void);
void CustomUIHandler_Update(void);
void CustomUIHandler_Destroy(void);
lv_obj_t* CustomUIHandler_GetScreen(void);

// Navigation
void CustomUIHandler_NavUp(void);
void CustomUIHandler_NavDown(void);
void CustomUIHandler_NavLeft(void);
void CustomUIHandler_NavRight(void);
void CustomUIHandler_Select(void);
void CustomUIHandler_Exit(void);

// Configuration management
void CustomUIHandler_SaveConfig(void);
void CustomUIHandler_LoadConfig(void);
ContainerConfig_t* CustomUIHandler_GetContainerConfig(uint8_t container_id);
void CustomUIHandler_SetContainerDataType(uint8_t container_id, DataType_t data_type);
const char* CustomUIHandler_GetDataTypeName(DataType_t data_type);

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_UI_HANDLER_H
