/**
 * @file ai_loader.c
 * @brief Dynamic AI Model Loader Implementation
 */

#include "ai_loader.h"
#include "kernel.h"
#include "memory.h"
#include "fat32.h"

/* Global AI Model Instance */
static ai_loaded_model_t *current_loaded_model = 0;

/* Initialize AI Loader */
int ai_loader_init(void) {
    current_loaded_model = 0;
    vga_print("AI Loader initialized - Ready for dynamic AI models!", 0, 46, VGA_COLOR_LIGHT_MAGENTA);
    return 0;
}

/* Main model loading function */
int ai_loader_load_model(const char *filename, ai_loaded_model_t *model) {
    if (!model) return -1;

    /* Clear model structure */
    memset(model, 0, sizeof(ai_loaded_model_t));

    /* Check if file exists */
    if (!ai_file_exists(filename)) {
        vga_print("ERROR: AI model file not found: ", 0, 47, VGA_COLOR_RED);
        vga_print(filename, 30, 47, VGA_COLOR_RED);
        return -1;
    }

    /* Get file size */
    uint32_t file_size = ai_file_size(filename);
    if (file_size == 0) {
        vga_print("ERROR: Empty AI model file", 0, 47, VGA_COLOR_RED);
        return -1;
    }

    /* Allocate buffer for file data */
    uint8_t *file_data = (uint8_t *)kmalloc(file_size);
    if (!file_data) {
        vga_print("ERROR: Insufficient memory for AI model", 0, 47, VGA_COLOR_RED);
        return -1;
    }

    /* Load file from FAT32 TODO: Implement proper file reading */
    /* For now, we'll create a simple demo model */
    /*
    fat32_file_t file;
    if (fat32_open_file(filename, &file) == 0 &&
        fat32_read_file(&file, file_data, 0, file_size) == (int)file_size) {
        fat32_close_file(&file);
    }
    */

    /* Temporary: Create a demo model */
    int result = ai_create_demo_model_from_file(file_data, file_size, model, filename);

    if (result == 0) {
        ai_copy_string(model->filename, filename, sizeof(model->filename));
        model->data_size = file_size;
        model->model_data = file_data;
        model->loaded = 1;
        current_loaded_model = model;

        vga_print("AI Model loaded successfully: ", 0, 47, VGA_COLOR_GREEN);
        vga_print(filename, 28, 47, VGA_COLOR_GREEN);
    } else {
        kfree(file_data);
    }

    return result;
}

/* Unload AI model */
int ai_loader_unload_model(ai_loaded_model_t *model) {
    if (!model || !model->loaded) return 0;

    /* Free model data */
    if (model->model_data) {
        kfree(model->model_data);
        model->model_data = 0;
    }

    /* Free runtime model weights */
    for (uint32_t i = 0; i < model->runtime_model.num_layers; i++) {
        nn_layer_t *layer = &model->runtime_model.layers[i];
        if (layer->weights) ai_free_weights((void*)layer->weights);
        if (layer->biases) ai_free_weights((void*)layer->biases);
    }

    /* Clear model */
    memset(model, 0, sizeof(ai_loaded_model_t));
    current_loaded_model = NULL;

    vga_print("AI Model unloaded successfully", 0, 47, VGA_COLOR_LIGHT_BLUE);
    return 0;
}

/* Simplified model creation for demo */
int ai_create_demo_model_from_file(uint8_t *data, uint32_t size, ai_loaded_model_t *model, const char *filename) {
    /* Detect file format */
    uint8_t format = AI_FORMAT_CUSTOM;

    if (size >= 8) {
        uint32_t magic = *(uint32_t*)data;
        if (magic == 0x0892A9FF) {
            format = AI_FORMAT_ONNX;
        } else if (data[0] == 'T' && data[1] == 'F' && data[2] == 'L' && data[3] == '3') {
            format = AI_FORMAT_TFLITE;
        }
    }

    /* Setup model info */
    ai_copy_string(model->info.name, filename, sizeof(model->info.name));
    model->info.format = format;
    model->info.version = 1;
    model->info.input_layers = 1;
    model->info.output_layers = 1;
    model->info.intermediate_layers = 2;
    model->info.total_weights = 200;
    model->info.model_size = size;

    /* Create runtime model */
    nn_model_t *runtime = &model->runtime_model;
    runtime->name = model->info.name;
    runtime->num_layers = 3;  /* 2 hidden + 1 output */
    runtime->input_size = 4;  /* Same as our sensor inputs */
    runtime->output_size = 4; /* idle, working, gaming, sleeping */

    /* Allocate layers */
    for (uint32_t i = 0; i < runtime->num_layers; i++) {
        nn_layer_t *layer = &runtime->layers[i];

        if (i == 0) { /* First layer: 4 -> 8 */
            layer->input_size = 4;
            layer->output_size = 8;
            layer->activation = ACTIVATION_RELU;
        } else if (i == 1) { /* Second layer: 8 -> 6 */
            layer->input_size = 8;
            layer->output_size = 6;
            layer->activation = ACTIVATION_RELU;
        } else { /* Third layer: 6 -> 4 */
            layer->input_size = 6;
            layer->output_size = 4;
            layer->activation = ACTIVATION_SIGMOID;
        }

        /* Allocate weights and biases */
        layer->weights = (float*)ai_allocate_weights(layer->output_size * layer->input_size * sizeof(float));
        layer->biases = (float*)ai_allocate_weights(layer->output_size * sizeof(float));

        if (!layer->weights || !layer->biases) {
            return -1;
        }
    }

    /* Allocate runtime buffers */
    runtime->input_buffer = (float*)kmalloc(runtime->input_size * sizeof(float));
    runtime->output_buffer = (float*)kmalloc(runtime->output_size * sizeof(float));
    runtime->temp_buffer = (float*)kmalloc(MAX_TENSOR_SIZE * sizeof(float));

    if (!runtime->input_buffer || !runtime->output_buffer || !runtime->temp_buffer) {
        return -1;
    }

    /* Load weights (using file data + some defaults) */
    ai_load_weights_from_data(data, size, runtime);

    runtime->loaded = 1;
    return 0;
}

/* Load weights from model data */
void ai_load_weights_from_data(uint8_t *data, uint32_t size, nn_model_t *model) {
    /* Initialize weights based on data hash or fixed patterns */
    /* For demo: Create meaningful weights based on file content */

    uint32_t seed = 0;
    for (uint32_t i = 0; i < size && i < 64; i++) {
        seed += data[i];
    }

    srand(seed); /* Initialize with seed from file */

    for (uint32_t layer_idx = 0; layer_idx < model->num_layers; layer_idx++) {
        nn_layer_t *layer = &model->layers[layer_idx];

        /* Initialize weights with predictable but unique values */
        for (uint32_t w = 0; w < layer->output_size * layer->input_size; w++) {
            layer->weights[w] = (rand() % 4000 - 2000) / 1000.0f; // -2.0 to +2.0
        }

        /* Initialize biases */
        for (uint32_t b = 0; b < layer->output_size; b++) {
            layer->biases[b] = (rand() % 2000 - 1000) / 1000.0f; // -1.0 to +1.0
        }
    }
}

/* Parser implementations (simplified) */
int ai_parse_onnx(const uint8_t *data, uint32_t size, nn_model_t *model, ai_model_info_t *info) {
    /* ONNX parsing would be complex - return unsupported for demo */
    return -1;
}

int ai_parse_tflite(const uint8_t *data, uint32_t size, nn_model_t *model, ai_model_info_t *info) {
    /* TFLite parsing would be complex - return unsupported for demo */
    return -1;
}

/* File helper functions */
int ai_file_exists(const char *filename) {
    /* Check if file exists using FAT32 */
    fat32_file_t file;
    return (fat32_open_file(filename, &file) == 0);
}

uint32_t ai_file_size(const char *filename) {
    /* Get file size - TODO: implement properly */
    return 1024; /* Temporary fixed size for demo */
}

/* Memory allocation helpers */
void *ai_allocate_weights(uint32_t size) {
    return kmalloc(size);
}

void ai_free_weights(void *ptr) {
    kfree(ptr);
}

/* Utility functions */
uint32_t ai_read_uint32(const uint8_t **buffer) {
    uint32_t val = *(uint32_t*)(*buffer);
    *buffer += 4;
    return val;
}

float ai_read_float(const uint8_t **buffer) {
    float val = *(float*)(*buffer);
    *buffer += 4;
    return val;
}

void ai_copy_string(char *dest, const char *src, uint32_t max_len) {
    uint32_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

/* Pseudo-random functions */
static uint32_t rand_seed = 0;

void srand(uint32_t seed) {
    rand_seed = seed;
}

uint32_t rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return rand_seed >> 16;
}
