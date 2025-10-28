/**
 * @file sensors.h
 * @brief Generic sensor framework for local AI inference
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

#define MAX_SENSORS 16

/* Sensor types */
typedef enum {
    SENSOR_TYPE_ACCELEROMETER = 0,
    SENSOR_TYPE_GYROSCOPE,
    SENSOR_TYPE_MAGNETOMETER,
    SENSOR_TYPE_AMBIENT_LIGHT,
    SENSOR_TYPE_PROXIMITY,
    SENSOR_TYPE_TOUCH_SCREEN,
    SENSOR_TYPE_BATTERY_LEVEL,
    SENSOR_TYPE_CPU_USAGE,
    SENSOR_TYPE_MEMORY_USAGE,
    SENSOR_TYPE_NETWORK_TX,
    SENSOR_TYPE_NETWORK_RX,
    SENSOR_TYPE_GPS_LOCATION,
    SENSOR_TYPE_TIME_OF_DAY,
    SENSOR_TYPE_USER_ACTIVITY, /* Touch patterns, gestures */
    SENSOR_TYPE_APP_USAGE,
    SENSOR_TYPE_CUSTOM
} sensor_type_t;

/* Sensor data structure */
typedef struct {
    sensor_type_t type;
    uint32_t timestamp;
    float x_value;      /* Primary value */
    float y_value;      /* Secondary value (for 2D/3D sensors) */
    float z_value;      /* Tertiary value */
    uint8_t accuracy;   /* 0-255 accuracy level */
    void *raw_data;     /* Pointer to raw sensor data */
    uint32_t data_size; /* Size of raw data */
} sensor_data_t;

/* Sensor interface functions */
typedef sensor_data_t (*sensor_read_func_t)(void);
typedef int (*sensor_init_func_t)(void);

/* Sensor structure */
typedef struct {
    sensor_type_t type;
    const char *name;
    sensor_init_func_t init_func;
    sensor_read_func_t read_func;
    uint32_t last_timestamp;
    uint8_t active;
} sensor_t;

/* Function prototypes */
void init_sensor_framework();
int register_sensor(sensor_type_t type, const char *name, sensor_init_func_t init_func, sensor_read_func_t read_func);
int enable_sensor(sensor_type_t type);
int disable_sensor(sensor_type_t type);
sensor_data_t read_sensor(sensor_type_t type);
sensor_data_t *read_all_sensors(uint32_t *count);
void update_sensor_timestamp(sensor_type_t type);

#endif
