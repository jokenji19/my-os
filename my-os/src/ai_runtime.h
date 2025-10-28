/**
 * @file ai_runtime.h
 * @brief AI Runtime Engine for executing ML models
 */

#ifndef AI_RUNTIME_H
#define AI_RUNTIME_H

#include <stdint.h>
#include "sensors.h"

/* Maximum tensor dimensions for on-device ML */
#define MAX_TENSOR_SIZE 1024
#define MAX_LAYERS 16
#define MAX_WEIGHTS 4096

/* Types of neural network layers */
typedef enum {
    LAYER_TYPE_NONE = 0,
    LAYER_TYPE_DENSE,
    LAYER_TYPE_FLATTEN,
    LAYER_TYPE_ACTIVATION
} layer_type_t;

/* Activation functions */
typedef enum {
    ACTIVATION_LINEAR,
    ACTIVATION_RELU,
    ACTIVATION_SIGMOID,
    ACTIVATION_TANH
} activation_func_t;

/* AI decision types */
typedef enum {
    AI_DECISION_NONE = 0,
    AI_DECISION_IDLE,
    AI_DECISION_AWAKE,
    AI_DECISION_WORKING,
    AI_DECISION_GAMING,
    AI_DECISION_SLEEPING,
    AI_DECISION_ANOMALY_DETECTED,
    AI_DECISION_CONTEXT_SWITCH,
    AI_DECISION_LEARN_PATTERN
} ai_decision_t;

/* Neural network layer */
typedef struct {
    layer_type_t type;
    uint32_t input_size;
    uint32_t output_size;
    activation_func_t activation;
    float *weights;      /* Weight matrix (output_size x input_size) */
    float *biases;       /* Bias vector (output_size) */
} nn_layer_t;

/* Neural network model */
typedef struct {
    const char *name;
    uint32_t num_layers;
    uint32_t input_size;
    uint32_t output_size;
    nn_layer_t layers[MAX_LAYERS];
    float *input_buffer;
    float *output_buffer;
    float *temp_buffer;
    uint8_t loaded;      /* Whether model is loaded */
    uint32_t last_inference_time;
} nn_model_t;

/* Ai context structure containing sensor state */
typedef struct {
    sensor_data_t accelerometer;
    sensor_data_t cpu_usage;
    sensor_data_t memory_usage;
    sensor_data_t user_activity;
    sensor_data_t time_of_day;
    uint32_t system_uptime;
} ai_context_t;

/* Function prototypes */
void init_ai_runtime();
int load_demo_model(nn_model_t *model);
ai_decision_t run_ai_inference(nn_model_t *model, ai_context_t *context);
void preprocess_sensor_data(ai_context_t *context, float *input_tensor, uint32_t *tensor_size);

/* Neural network operations */
float sigmoid(float x);
float relu(float x);
int matrix_multiply(const float *a, const float *b, float *c, uint32_t rows_a, uint32_t cols_a, uint32_t cols_b);
void apply_activation(float *tensor, uint32_t size, activation_func_t activation);

/* Model loader (temporary - hardcoded models) */
int load_context_awareness_model(nn_model_t *model);

#endif
