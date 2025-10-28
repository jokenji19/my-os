/**
 * @file sensors.c
 * @brief Implementation of sensor framework for local AI
 */

#include "sensors.h"
#include "kernel.h"

uint32_t get_tick_count();
static int rand();

/* Global sensor array */
static sensor_t sensors[MAX_SENSORS];
static int sensor_count = 0;

/* Sample sensor implementations - simulated for now */
static sensor_data_t read_accelerometer() {
    sensor_data_t data;
    data.type = SENSOR_TYPE_ACCELEROMETER;
    data.timestamp = get_tick_count() * 10; /* Convert to ms */
    data.x_value = 0.0 + (rand() % 1000) / 500.0 - 1.0; /* -1.0 to +1.0 */
    data.y_value = 9.8 + (rand() % 200) / 100.0 - 1.0;  /* ~9.8 m/s² */
    data.z_value = 0.0 + (rand() % 1000) / 500.0 - 1.0;
    data.accuracy = 95;
    data.raw_data = 0;
    data.data_size = 0;
    return data;
}

static sensor_data_t read_cpu_usage() {
    sensor_data_t data;
    data.type = SENSOR_TYPE_CPU_USAGE;
    data.timestamp = get_tick_count() * 10;
    data.x_value = (rand() % 100) + (rand() % 100) / 100.0; /* 0-99% */
    data.y_value = ((rand() % sensor_count) / (float)sensor_count) * 100.0; /* Kernel load */
    data.z_value = 0.0;
    data.accuracy = 100;
    data.raw_data = 0;
    data.data_size = 0;
    return data;
}

static sensor_data_t read_memory_usage() {
    sensor_data_t data;
    data.type = SENSOR_TYPE_MEMORY_USAGE;
    data.timestamp = get_tick_count() * 10;
    data.x_value = (rand() % 80) + 20.0; /* 20-99% used */
    data.y_value = 1048576 - (data.x_value * 10485.76); /* Free bytes simulation */
    data.z_value = 1 << (rand() % 24); /* Fragmentation metric */
    data.accuracy = 90;
    data.raw_data = 0;
    data.data_size = 0;
    return data;
}

static sensor_data_t read_time_of_day() {
    sensor_data_t data;
    data.type = SENSOR_TYPE_TIME_OF_DAY;
    data.timestamp = get_tick_count() * 10;
    /* Simulate current time as HH.MM */
    data.x_value = 12 + (rand() % 24) / 10.0; /* Hours */
    data.y_value = (rand() % 60); /* Minutes */
    data.z_value = data.timestamp / (24 * 60 * 60 * 1000); /* Day number */
    data.accuracy = 100;
    data.raw_data = 0;
    data.data_size = 0;
    return data;
}

static sensor_data_t read_user_activity() {
    sensor_data_t data;
    data.type = SENSOR_TYPE_USER_ACTIVITY;
    data.timestamp = get_tick_count() * 10;
    data.x_value = rand() % 4; /* Activity level 0-3 (idle -> high) */
    data.y_value = rand() % 100; /* Touch pressure simulation */
    data.z_value = rand() % 360; /* Touch angle */
    data.accuracy = 85;
    data.raw_data = 0;
    data.data_size = 0;
    return data;
}

/* Simple random number generator */
static int rand() {
    static unsigned int seed = 12345;
    seed = (seed * 1103515245 + 12345) % (1U << 31);
    return seed;
}

/* Initialize sensor framework */
void init_sensor_framework() {
    /* Initialize sensor array */
    int i;
    for (i = 0; i < MAX_SENSORS; i++) {
        sensors[i].type = SENSOR_TYPE_CUSTOM;
        sensors[i].name = "";
        sensors[i].init_func = 0;
        sensors[i].read_func = 0;
        sensors[i].last_timestamp = 0;
        sensors[i].active = 0;
    }
    sensor_count = 0;

    /* Register built-in sensors */
    register_sensor(SENSOR_TYPE_ACCELEROMETER, "accelerometer", 0, read_accelerometer);
    register_sensor(SENSOR_TYPE_CPU_USAGE, "cpu_usage", 0, read_cpu_usage);
    register_sensor(SENSOR_TYPE_MEMORY_USAGE, "memory_usage", 0, read_memory_usage);
    register_sensor(SENSOR_TYPE_TIME_OF_DAY, "time_of_day", 0, read_time_of_day);
    register_sensor(SENSOR_TYPE_USER_ACTIVITY, "user_activity", 0, read_user_activity);

    vga_print("Sensor framework inizializzato con 5 sensori AI!", 0, 18, VGA_COLOR_MAGENTA);
}

/* Register a sensor */
int register_sensor(sensor_type_t type, const char *name,
                   sensor_init_func_t init_func, sensor_read_func_t read_func) {
    if (sensor_count >= MAX_SENSORS) return -1;

    sensors[sensor_count].type = type;
    sensors[sensor_count].name = name;
    sensors[sensor_count].init_func = init_func;
    sensors[sensor_count].read_func = read_func;
    sensors[sensor_count].last_timestamp = 0;
    sensors[sensor_count].active = (read_func != 0) ? 1 : 0; /* Auto-enable if has read func */

    sensor_count++;
    return 0;
}

/* Enable a sensor */
int enable_sensor(sensor_type_t type) {
    int i;
    for (i = 0; i < sensor_count; i++) {
        if (sensors[i].type == type) {
            sensors[i].active = 1;
            if (sensors[i].init_func) {
                sensors[i].init_func();
            }
            return 0;
        }
    }
    return -1; /* Sensor not found */
}

/* Disable a sensor */
int disable_sensor(sensor_type_t type) {
    int i;
    for (i = 0; i < sensor_count; i++) {
        if (sensors[i].type == type) {
            sensors[i].active = 0;
            return 0;
        }
    }
    return -1; /* Sensor not found */
}

/* Read sensor data */
sensor_data_t read_sensor(sensor_type_t type) {
    sensor_data_t empty_data = {SENSOR_TYPE_CUSTOM, 0, 0.0, 0.0, 0.0, 0, 0, 0};

    int i;
    for (i = 0; i < sensor_count; i++) {
        if (sensors[i].type == type && sensors[i].active && sensors[i].read_func) {
            sensor_data_t data = sensors[i].read_func();
            sensors[i].last_timestamp = data.timestamp;
            return data;
        }
    }
    return empty_data;
}

/* Read all active sensors */
sensor_data_t *read_all_sensors(uint32_t *count) {
    static sensor_data_t sensor_buffer[MAX_SENSORS];

    int active_count = 0;
    int i;
    for (i = 0; i < sensor_count; i++) {
        if (sensors[i].active && sensors[i].read_func) {
            sensor_buffer[active_count] = sensors[i].read_func();
            sensors[i].last_timestamp = sensor_buffer[active_count].timestamp;
            active_count++;
        }
    }

    *count = active_count;
    return sensor_buffer;
}

/* Update sensor timestamp (used by sensors that push data) */
void update_sensor_timestamp(sensor_type_t type) {
    /* Could implement push-based sensor model here */
}
