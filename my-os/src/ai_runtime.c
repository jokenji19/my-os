/**
 * @file ai_runtime.c
 * @brief Implementation of AI inference engine for on-device ML
 */

#include "ai_runtime.h"
#include "kernel.h"
#include "memory.h"

uint32_t get_tick_count();
static float exp(float x);
static float sqrt(float x);
static void memcpy(void *dest, const void *src, uint32_t n);

/* Global AI model (demo version) */
static nn_model_t *active_model = 0;

/* Sample weights for demo "context awareness" model */
/* This is a very simple 3-layer neural network trained to recognize user context */
/* Input: [accelerometer_magnitude, time_of_day_hour, cpu_usage, touch_pressure] */
/* Output: [idle, working, gaming, sleeping] probability distribution */

/* Layer 1 (4 inputs -> 8 neurons) */
static float layer1_weights[] = {
    0.2, 0.5, 0.1, 0.8,   /* Neuron 1 */
    0.3, 0.4, 0.2, 0.6,   /* Neuron 2 */
    0.1, 0.7, 0.5, 0.3,   /* Neuron 3 */
    0.6, 0.2, 0.8, 0.1,   /* Neuron 4 */
    0.4, 0.3, 0.6, 0.5,   /* Neuron 5 */
    0.7, 0.8, 0.2, 0.4,   /* Neuron 6 */
    0.5, 0.1, 0.9, 0.7,   /* Neuron 7 */
    0.8, 0.6, 0.3, 0.2    /* Neuron 8 */
};
static float layer1_biases[] = {0.1, 0.2, 0.1, 0.3, 0.2, 0.4, 0.3, 0.1};

/* Layer 2 (8 inputs -> 6 neurons) */
static float layer2_weights[] = {
    0.4, 0.6, 0.2, 0.8, 0.5, 0.3, 0.7, 0.1,   /* Neuron 1 */
    0.3, 0.7, 0.4, 0.2, 0.9, 0.6, 0.1, 0.8,   /* Neuron 2 */
    0.6, 0.2, 0.8, 0.4, 0.3, 0.7, 0.5, 0.9,   /* Neuron 3 */
    0.1, 0.9, 0.3, 0.7, 0.4, 0.8, 0.2, 0.6,   /* Neuron 4 */
    0.8, 0.4, 0.6, 0.1, 0.9, 0.2, 0.7, 0.3,   /* Neuron 5 */
    0.7, 0.3, 0.9, 0.5, 0.1, 0.8, 0.4, 0.6    /* Neuron 6 */
};
static float layer2_biases[] = {0.2, 0.1, 0.3, 0.4, 0.2, 0.1};

/* Layer 3 (6 inputs -> 4 outputs) */
static float layer3_weights[] = {
    0.6, 0.8, 0.3, 0.9, 0.4, 0.7,   /* Idle */
    0.7, 0.2, 0.9, 0.5, 0.8, 0.1,   /* Working */
    0.3, 0.9, 0.4, 0.7, 0.2, 0.8,   /* Gaming */
    0.8, 0.1, 0.6, 0.2, 0.9, 0.3    /* Sleeping */
};
static float layer3_biases[] = {0.1, 0.3, 0.2, 0.4};

/* Initialize AI runtime */
void init_ai_runtime() {
    active_model = kmalloc(sizeof(nn_model_t));
    if (!active_model) {
        vga_print("ERRORE: Allocazione AI model fallita!", 0, 34, VGA_COLOR_RED);
        return;
    }

    active_model->loaded = 0;
    load_context_awareness_model(active_model);

    vga_print("AI Runtime inizializzato - Pronto per modelli ML!", 0, 18, VGA_COLOR_LIGHT_MAGENTA);
}

/* Load demo context awareness model */
int load_context_awareness_model(nn_model_t *model) {
    model->name = "Context Awareness Demo";
    model->num_layers = 3;
    model->input_size = 4;  /* accel_mag, time_hour, cpu_usage, touch_pressure */
    model->output_size = 4; /* idle, working, gaming, sleeping */

    /* Allocate buffers */
    model->input_buffer = kmalloc(model->input_size * sizeof(float));
    model->output_buffer = kmalloc(model->output_size * sizeof(float));
    model->temp_buffer = kmalloc(MAX_TENSOR_SIZE * sizeof(float));

    if (!model->input_buffer || !model->output_buffer || !model->temp_buffer) {
        vga_print("ERRORE: Allocazione buffer AI fallita!", 0, 35, VGA_COLOR_RED);
        return -1;
    }

    /* Configure layer 1: Dense 4->8 with ReLU */
    model->layers[0].type = LAYER_TYPE_DENSE;
    model->layers[0].input_size = 4;
    model->layers[0].output_size = 8;
    model->layers[0].activation = ACTIVATION_RELU;
    model->layers[0].weights = layer1_weights;
    model->layers[0].biases = layer1_biases;

    /* Configure layer 2: Dense 8->6 with ReLU */
    model->layers[1].type = LAYER_TYPE_DENSE;
    model->layers[1].input_size = 8;
    model->layers[1].output_size = 6;
    model->layers[1].activation = ACTIVATION_RELU;
    model->layers[1].weights = layer2_weights;
    model->layers[1].biases = layer2_biases;

    /* Configure layer 3: Dense 6->4 with Sigmoid (soft voting) */
    model->layers[2].type = LAYER_TYPE_DENSE;
    model->layers[2].input_size = 6;
    model->layers[2].output_size = 4;
    model->layers[2].activation = ACTIVATION_SIGMOID;
    model->layers[2].weights = layer3_weights;
    model->layers[2].biases = layer3_biases;

    model->loaded = 1;
    model->last_inference_time = get_tick_count();

    return 0;
}

/* Activation functions */
float sigmoid(float x) {
    if (x > 10) return 1.0;
    if (x < -10) return 0.0;
    return 1.0 / (1.0 + exp(-x));  /* Approximation */
}

float relu(float x) {
    return x > 0 ? x : 0;
}

/* Simple matrix multiplication (for fixed point arithmetic) */
int matrix_multiply(const float *a, const float *b, float *c,
                   uint32_t rows_a, uint32_t cols_a, uint32_t cols_b) {
    for (uint32_t i = 0; i < rows_a; i++) {
        for (uint32_t j = 0; j < cols_b; j++) {
            c[i * cols_b + j] = 0;
            for (uint32_t k = 0; k < cols_a; k++) {
                c[i * cols_b + j] += a[i * cols_a + k] * b[k * cols_b + j];
            }
        }
    }
    return 0;
}

/* Apply activation to tensor */
void apply_activation(float *tensor, uint32_t size, activation_func_t activation) {
    for (uint32_t i = 0; i < size; i++) {
        switch (activation) {
            case ACTIVATION_LINEAR:
                break;
            case ACTIVATION_RELU:
                tensor[i] = relu(tensor[i]);
                break;
            case ACTIVATION_SIGMOID:
                tensor[i] = sigmoid(tensor[i]);
                break;
            case ACTIVATION_TANH:
                if (tensor[i] > 0) {
                    tensor[i] = 1 - 2 / (exp(2 * tensor[i]) + 1);
                } else {
                    tensor[i] = -1 + 2 / (exp(-2 * tensor[i]) + 1);
                }
                break;
        }
    }
}

/* Preprocess sensor data into input tensor */
void preprocess_sensor_data(ai_context_t *context, float *input_tensor, uint32_t *tensor_size) {
    /* Normalize and extract features:
     * [0] accelerometer magnitude (0.0-1.0)
     * [1] time of day hours (0.0-23.0) normalized to 0-1
     * [2] cpu usage percentage (0.0-1.0)
     * [3] touch pressure (0.0-1.0)
     */

    /* Accelerometer magnitude */
    float accel_mag = sqrt(context->accelerometer.x_value * context->accelerometer.x_value +
                           context->accelerometer.y_value * context->accelerometer.y_value +
                           context->accelerometer.z_value * context->accelerometer.z_value);
    input_tensor[0] = accel_mag / 20.0; /* Normalize ~9.8m/s² base */

    /* Time of day (normalized to 0-1) */
    input_tensor[1] = context->time_of_day.x_value / 24.0;

    /* CPU usage */
    input_tensor[2] = context->cpu_usage.x_value / 100.0;

    /* User activity (touch pressure) */
    input_tensor[3] = context->user_activity.y_value / 100.0;

    *tensor_size = 4;
}

/* Run AI inference */
ai_decision_t run_ai_inference(nn_model_t *model, ai_context_t *context) {
    if (!model || !model->loaded) {
        return AI_DECISION_NONE;
    }

    /* Preprocess sensor data */
    uint32_t input_size;
    preprocess_sensor_data(context, model->input_buffer, &input_size);

    /* Forward pass through network */
    float *current_input = model->input_buffer;

    for (uint32_t layer_idx = 0; layer_idx < model->num_layers; layer_idx++) {
        const nn_layer_t *layer = &model->layers[layer_idx];

        if (layer->type == LAYER_TYPE_DENSE) {
            /* Dense layer: matrix multiplication + bias + activation */

            /* Matrix multiply: weights(current_input) + bias -> temp_buffer */
            matrix_multiply(layer->weights, current_input, model->temp_buffer,
                          layer->output_size, layer->input_size, 1);

            /* Add bias */
            for (uint32_t i = 0; i < layer->output_size; i++) {
                model->temp_buffer[i] += layer->biases[i];
            }

            /* Apply activation */
            apply_activation(model->temp_buffer, layer->output_size, layer->activation);

            /* Swap buffers */
            current_input = model->temp_buffer;
            model->temp_buffer = (current_input == model->input_buffer) ? model->output_buffer : model->input_buffer;
        }
    }

    /* Copy final output */
    memcpy(model->output_buffer, current_input, model->output_size * sizeof(float));

    /* Find decision based on highest probability */
    ai_decision_t decision = AI_DECISION_NONE;
    float max_prob = -1.0;
    uint32_t max_idx = 0;

    for (uint32_t i = 0; i < model->output_size; i++) {
        if (model->output_buffer[i] > max_prob) {
            max_prob = model->output_buffer[i];
            max_idx = i;
        }
    }

    /* Map output indices to decisions */
    switch (max_idx) {
        case 0: decision = AI_DECISION_IDLE; break;
        case 1: decision = AI_DECISION_WORKING; break;
        case 2: decision = AI_DECISION_GAMING; break;
        case 3: decision = AI_DECISION_SLEEPING; break;
    }

    model->last_inference_time = get_tick_count();
    return decision;
}

/* Load demo model (wrapper) */
int load_demo_model(nn_model_t *model) {
    return load_context_awareness_model(model);
}

/* Simple exponential function approximation for sigmoid */
static float exp(float x) {
    /* Very rough approximation: e^x ≈ 1 + x + x²/2 */
    if (x > 2.0) return 7.0;   /* Cap at reasonable value */
    if (x < -2.0) return 0.1;  /* Cap at reasonable value */

    float result = 1.0 + x + (x * x) / 2.0;
    return result;
}

/* Simple sqrt approximation */
static float sqrt(float x) {
    if (x <= 0) return 0;
    float result = 1.0;
    for (int i = 0; i < 10; i++) {
        result = (result + x / result) / 2.0;
    }
    return result;
}

/* Memory copy (since we're freestanding) */
static void memcpy(void *dest, const void *src, uint32_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
}
