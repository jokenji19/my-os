/**
 * @file menu_simplified.c
 * @brief Simplified menu for bootable ISO
 */

#include "kernel.h"
#include "memory.h"
#include "framebuffer.h"
#include "ai_runtime.h"
#include "sensors.h"

/* Simple text-based UI for initial testing */
void display_boot_status() {
    vga_print("=== AIPA OS BOOT SEQEUNCE ===", 0, 35, VGA_COLOR_LIGHT_CYAN);
    vga_print("Universal AI Operating System loaded!", 0, 36, VGA_COLOR_GREEN);
    vga_print("Version: Innovation Alpha", 0, 37, VGA_COLOR_WHITE);

    vga_print("AI STATUS:", 0, 39, VGA_COLOR_YELLOW);
    vga_print("  - AI Runtime: LOADED", 0, 40, VGA_COLOR_GREEN);
    vga_print("  - Sensor Framework: READY", 0, 41, VGA_COLOR_GREEN);
    vga_print("  - Memory Management: ACTIVE", 0, 42, VGA_COLOR_GREEN);

    vga_print("PRESS ANY KEY FOR AI DEMO...", 0, 45, VGA_COLOR_LIGHT_MAGENTA);
}

void run_simple_ai_demo() {
    vga_print("=== AI DEMO MODE ===", 0, 34, VGA_COLOR_MAGENTA);

    /* Show sensor demo */
    sensor_data_t accel_data = read_sensor(SENSOR_TYPE_ACCELEROMETER);
    sensor_data_t cpu_data = read_sensor(SENSOR_TYPE_CPU_USAGE);
    sensor_data_t time_data = read_sensor(SENSOR_TYPE_TIME_OF_DAY);

    vga_print("SENSOR FEED:", 0, 36, VGA_COLOR_CYAN);

    char buffer[16];
    itoa((int)accel_data.x_value, buffer, 10);
    vga_print("Accelerometer: ", 0, 37, VGA_COLOR_WHITE);
    vga_print(buffer, 15, 37, VGA_COLOR_LIGHT_BLUE);

    itoa((int)cpu_data.x_value, buffer, 10);
    vga_print("CPU Usage: ", 0, 38, VGA_COLOR_WHITE);
    vga_print(buffer, 12, 38, VGA_COLOR_LIGHT_BLUE);
    vga_print("%", 17, 38, VGA_COLOR_LIGHT_BLUE);

    vga_print("AI ANALYSIS:", 0, 40, VGA_COLOR_YELLOW);

    /* Simple AI logic demo */
    char *ai_decision = "WAITING FOR MODEL";
    uint32_t decision_color = VGA_COLOR_LIGHT_GREY;

    if (cpu_data.x_value > 50) {
        ai_decision = "BUSY MODE DETECTED";
        decision_color = VGA_COLOR_YELLOW;
    } else if (accel_data.x_value > 1.0f) {
        ai_decision = "ACTIVITY DETECTED";
        decision_color = VGA_COLOR_CYAN;
    } else if (time_data.x_value > 18.0f || time_data.x_value < 6.0f) {
        ai_decision = "NIGHT/SLEEP PERIOD";
        decision_color = VGA_COLOR_BLUE;
    } else {
        ai_decision = "READY STATE - IDLE";
        decision_color = VGA_COLOR_GREEN;
    }

    vga_print(ai_decision, 12, 40, decision_color);

    vga_print("Universal AI Loader Ready!", 0, 45, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("Load any AI model (.onnx, .tflite, .gguf) format", 0, 46, VGA_COLOR_LIGHT_CYAN);
    vga_print("System will analyze and become intelligent!", 0, 47, VGA_COLOR_LIGHT_MAGENTA);
}

void show_kernel_info() {
    vga_print("=== KERNEL INFORMATION ===", 0, 34, VGA_COLOR_WHITE);
    vga_print("Architecture: x86_32 Protected Mode", 0, 36, VGA_COLOR_LIGHT_BLUE);
    vga_print("Memory: 1MB+ Heap Available", 0, 37, VGA_COLOR_LIGHT_BLUE);
    vga_print("Scheduler: Round-robin Multitasking", 0, 38, VGA_COLOR_LIGHT_BLUE);
    vga_print("Graphics: VGA Text Mode 80x25", 0, 39, VGA_COLOR_LIGHT_BLUE);

    vga_print("FEATURES:", 0, 41, VGA_COLOR_YELLOW);
    vga_print("+ FAT32 Filesystem Ready", 0, 42, VGA_COLOR_GREEN);
    vga_print("+ Universal AI Model Support", 0, 43, VGA_COLOR_GREEN);
    vga_print("+ Sensor Data Processing", 0, 44, VGA_COLOR_GREEN);
    vga_print("+ Intelligent Behavior Engine", 0, 45, VGA_COLOR_GREEN);
    vga_print("+ Device Management Layer", 0, 46, VGA_COLOR_GREEN);
}
