/**
 * @file ai_loader.h
 * @brief Dynamic AI Model Loader Header
 */

#ifndef AI_LOADER_H
#define AI_LOADER_H

#include <stdint.h>
#include "ai_runtime.h"
#include "fat32.h"

/* AI Model Format Support */
#define AI_FORMAT_ONNX       1
#define AI_FORMAT_TFLITE     2
#define AI_FORMAT_CUSTOM     3
#define AI_FORMAT_COREML     4
#define AI_FORMAT_TENSORRT   5
#define AI_FORMAT_SAFETENSORS 6
#define AI_FORMAT_GGUF       7

/* Maximum supported model parameters */
#define MAX_MODEL_LAYERS     16
#define MAX_WEIGHTS_PER_LAYER 1024
#define MAX_TOTAL_WEIGHTS    8192

/* AI Model Metadata */
typedef struct {
    char name[64];
    uint8_t format;
    uint32_t version;
    uint32_t input_layers;
    uint32_t output_layers;
    uint32_t intermediate_layers;
    uint32_t total_weights;
    uint32_t model_size;
    uint32_t offset_weights;
    uint32_t offset_attributes;
} ai_model_info_t;

/* Loaded AI Model Structure */
typedef struct {
    ai_model_info_t info;
    nn_model_t runtime_model;
    uint8_t *model_data;
    uint32_t data_size;
    uint8_t loaded;
    char filename[256];
} ai_loaded_model_t;

/* ONNX Model Structures (simplified) */
typedef struct __attribute__((packed)) {
    uint32_t magic;           /* ONNX magic: 0x0892A9FF */
    uint32_t length;          /* Protobuf length */
    /* Protobuf data follows */
} onnx_header_t;

/* TensorFlow Lite Structures (simplified) */
typedef struct __attribute__((packed)) {
    uint32_t length;
    uint8_t format_version;
    uint8_t subformat_version;
    uint8_t reserved[2];
    uint32_t offset_table_subgraph;
    uint32_t offset_table_operator_codes;
    uint32_t offset_table_tensor;
    uint32_t offset_table_buffer;
    uint32_t offset_table_metadata;
} tflite_header_t;

/* Function prototypes */
int ai_loader_init(void);
int ai_loader_load_model(const char *filename, ai_loaded_model_t *model);
int ai_loader_unload_model(ai_loaded_model_t *model);

/* Parser implementations */
int ai_parse_onnx(const uint8_t *data, uint32_t size, nn_model_t *model, ai_model_info_t *info);
int ai_parse_tflite(const uint8_t *data, uint32_t size, nn_model_t *model, ai_model_info_t *info);

/* Model conversion utilities */
int ai_convert_onnx_to_runtime(const onnx_header_t *onnx, nn_model_t *runtime, ai_model_info_t *info);
int ai_convert_tflite_to_runtime(const tflite_header_t *tflite, nn_model_t *runtime, ai_model_info_t *info);

/* File helper functions */
int ai_file_exists(const char *filename);
uint32_t ai_file_size(const char *filename);

/* Memory allocation helpers */
void *ai_allocate_weights(uint32_t size);
void ai_free_weights(void *ptr);

/* Utility functions */
uint32_t ai_read_uint32(const uint8_t **buffer);
float ai_read_float(const uint8_t **buffer);
void ai_copy_string(char *dest, const char *src, uint32_t max_len);

#endif
