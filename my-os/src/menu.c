/**
 * @file menu.c
 * @brief Implementation of interactive menu system
 */

#include "menu.h"
#include "kernel.h"
#include "memory.h"
#include "framebuffer.h"
#include "ai_runtime.h"
#include "sensors.h"
#include "fat32.h"
#include "ai_loader.h"

#include <string.h>

/* Forward declarations */
uint32_t get_tick_count();
char *strcpy(char *dest, const char *src);

/* Maximum items per menu */
#define MAX_MENU_ITEMS 10

/* Current active menu */
static menu_t *active_menu = 0;

/* Initialize menu system */
void init_menu_system() {
    active_menu = 0;
}

/* Create a new menu */
menu_t *create_menu(const char *title, int x, int y) {
    menu_t *menu = kmalloc(sizeof(menu_t));
    if (!menu) return 0;

    menu->title = title;
    menu->items = kmalloc(sizeof(menu_item_t) * MAX_MENU_ITEMS);
    if (!menu->items) {
        kfree(menu);
        return 0;
    }

    menu->num_items = 0;
    menu->selected_index = 0;
    menu->x_offset = x;
    menu->y_offset = y;

    return menu;
}

/* Add item to menu */
int add_menu_item(menu_t *menu, menu_item_type_t type, const char *label,
                  int px, int py, int w, int h, uint32_t color_normal, uint32_t color_highlight, void (*callback)(void)) {
    if (!menu || menu->num_items >= MAX_MENU_ITEMS) return -1;

    menu_item_t *item = &menu->items[menu->num_items];
    item->type = type;
    item->label = label;
    item->selected = 0;
    item->x = px;
    item->y = py;
    item->width = w;
    item->height = h;
    item->color_normal = color_normal;
    item->color_highlight = color_highlight;
    item->callback = callback;

    menu->num_items++;
    return 0;
}

/* Render a menu item */
static void render_menu_item(menu_t *menu, menu_item_t *item, int index) {
    int is_selected = (index == menu->selected_index);
    uint32_t color = is_selected ? item->color_highlight : item->color_normal;

    /* Since we don't have real framebuffers, show on text display */
    int voffset = 35 + index; /* Below existing system messages */

    if (voffset > 22) { /* Don't overwrite existing messages */
        return;
    }

    /* Clear previous line */
    vga_print("                                                      ", 0, voffset, VGA_COLOR_BLACK);

    if (item->type == MENU_ITEM_BUTTON) {
        if (is_selected) {
            vga_print(">", item->x, voffset, VGA_COLOR_YELLOW);
            vga_print(item->label, item->x + 2, voffset, VGA_COLOR_CYAN);
        } else {
            vga_print(" ", item->x, voffset, VGA_COLOR_WHITE);
            vga_print(item->label, item->x + 2, voffset, color);
        }
    }
}

/* Render entire menu */
void render_menu(menu_t *menu) {
    if (!menu) return;

    /* Show title */
    vga_print("=== ", 0, 33, VGA_COLOR_GREEN);
    vga_print(menu->title, 4, 33, VGA_COLOR_WHITE);
    vga_print(" ===", 4 + strlen(menu->title), 33, VGA_COLOR_GREEN);

    /* Render items */
    for (int i = 0; i < menu->num_items; i++) {
        render_menu_item(menu, &menu->items[i], i);
    }
}

/* Select next menu item */
void menu_select_next(menu_t *menu) {
    if (!menu || menu->num_items == 0) return;

    menu->selected_index = (menu->selected_index + 1) % menu->num_items;
    render_menu(menu);
}

/* Select previous menu item */
void menu_select_prev(menu_t *menu) {
    if (!menu || menu->num_items == 0) return;

    menu->selected_index = (menu->selected_index - 1 + menu->num_items) % menu->num_items;
    render_menu(menu);
}

/* Activate selected menu item */
void menu_activate(menu_t *menu) {
    if (!menu || menu->num_items == 0) return;

    menu_item_t *item = &menu->items[menu->selected_index];
    if (item->callback) {
        item->callback();
    }
}

/* Destroy menu and free memory */
void destroy_menu(menu_t *menu) {
    if (!menu) return;

    if (menu->items) {
        kfree(menu->items);
    }
    kfree(menu);
}

/* Universal AI file selector */
#define MAX_AVAILABLE_FILES 20
typedef struct {
    char filename[256];
    uint32_t size;
    uint8_t format_detected;
} available_file_t;

available_file_t available_files[MAX_AVAILABLE_FILES];
int num_available_files = 0;
int current_selection = 0;

/* Scan directory for all files */
void scan_available_ai_files() {
    fat32_dir_t dir;
    fat32_dir_entry_t entry;
    char name_buffer[256];

    num_available_files = 0;

    if (fat32_opendir("/", &dir) == 0) {
        while (fat32_readdir(&dir, &entry, name_buffer, sizeof(name_buffer)) == 0 &&
               num_available_files < MAX_AVAILABLE_FILES) {

            /* Skip directories for now */
            if (!(entry.attr & 0x10)) {
                uint32_t size_mb = entry.size;
                strcpy(available_files[num_available_files].filename, name_buffer);
                available_files[num_available_files].size = entry.size;
                available_files[num_available_files].format_detected = 0; /* Will detect later */
                num_available_files++;
            }
        }
        fat32_closedir(&dir);
    }
}

/* Auto-detect AI format from file content */
uint8_t detect_ai_format(const char *filename) {
    uint8_t buffer[16]; /* Just read first 16 bytes for magic detection */

    /* TODO: Implement file reading for format detection */
    /* For now, return unknown - file selection will handle logic */

    return AI_FORMAT_CUSTOM; /* Safe default */
}

/* Display file selection menu */
void display_file_selection_menu(int selected_index) {
    vga_print("SELEZIONA FILE IA (Qualsiasi dimensione/formato):", 0, 33, VGA_COLOR_GREEN);

    for (int i = 0; i < num_available_files && i < 6; i++) { /* Show first 6 files */
        char size_str[16];
        char line[80];
        memset(line, 0, sizeof(line));

        if (i == selected_index) {
            strcpy(line, "> ");
            strcat(line, available_files[i].filename);
            vga_print(line, 0, 35 + i, VGA_COLOR_CYAN);

            /* Show file size */
            if (available_files[i].size >= 1024*1024) {
                itoa(available_files[i].size / (1024*1024), size_str, 10);
                strcpy(line, "  Dimensione: ");
                strcat(line, size_str);
                strcat(line, "MB");
                vga_print(line, 40, 35 + i, VGA_COLOR_LIGHT_BLUE);
            } else {
                itoa(available_files[i].size, size_str, 10);
                strcpy(line, "  Dimensione: ");
                strcat(line, size_str);
                strcat(line, " byte");
                vga_print(line, 40, 35 + i, VGA_COLOR_LIGHT_BLUE);
            }
        } else {
            strcpy(line, "  ");
            strcat(line, available_files[i].filename);
            vga_print(line, 0, 35 + i, VGA_COLOR_WHITE);
        }
    }

    if (num_available_files > 6) {
        vga_print("  ... (piu' file disponibili)", 0, 41, VGA_COLOR_LIGHT_GREY);
    }

    vga_print("Usa controlli per selezionare file IA. ENTER per caricare!", 0, 48, VGA_COLOR_YELLOW);
}

/* Universal AI File Selector - Main Callback */
void callback_insert_ai() {
    static int current_selection = 0;

    if (num_available_files == 0) {
        /* First time - scan directory */
        vga_print("Scansionando directory per file IA disponibili...", 0, 38, VGA_COLOR_MAGENTA);
        fat32_init();
        fat32_mount();
        scan_available_ai_files();
    }

    if (num_available_files == 0) {
        vga_print("NESSUN FILE TROVATO nella directory!", 0, 39, VGA_COLOR_RED);
        vga_print("Inserisci un file AI (qualsiasi formato/dimensione)", 0, 40, VGA_COLOR_RED);
        vga_print("nel root del FAT32 filesystem del dispositivo.", 0, 41, VGA_COLOR_RED);
        return;
    }

    /* Display file selection menu */
    display_file_selection_menu(current_selection);

    vga_print("Caricamento UNIVERSALE: Qualsiasi formato, qualsiasi dimensione!", 0, 50, VGA_COLOR_LIGHT_MAGENTA);
}

void select_next_ai_file() {
    if (num_available_files > 0) {
        current_selection = (current_selection + 1) % num_available_files;
        display_file_selection_menu(current_selection);
    }
}

void select_prev_ai_file() {
    if (num_available_files > 0) {
        current_selection = (current_selection - 1 + num_available_files) % num_available_files;
        display_file_selection_menu(current_selection);
    }
}

void load_selected_ai_file() {
    if (num_available_files == 0 || current_selection >= num_available_files) {
        return;
    }

    const char *selected_file = available_files[current_selection].filename;
    uint32_t file_size = available_files[current_selection].size;

    vga_print("Caricando file selezionato: ", 0, 45, VGA_COLOR_GREEN);
    vga_print(selected_file, 26, 45, VGA_COLOR_GREEN);

    /* Detect format from content */
    uint8_t detected_format = detect_ai_format(selected_file);
    const char *format_names[] = {"Sconosciuto", "ONNX", "TensorFlow Lite", "Custom", "Core ML", "TensorRT", "SafeTensors", "GGUF"};

    char fmt_msg[40] = "Formato rilevato: ";
    strcat(fmt_msg, format_names[detected_format]);
    vga_print(fmt_msg, 0, 46, VGA_COLOR_LIGHT_BLUE);

    /* Load the model using universal loader */
    ai_loaded_model_t loaded_model;

    vga_print("Elaborazione modello universale (lazy loading per grandi dimensioni)...", 0, 47, VGA_COLOR_MAGENTA);

    if (ai_loader_load_model(selected_file, &loaded_model) == 0) {
        /* Use loaded AI model for real inference */
        sensor_data_t accel_data = read_sensor(SENSOR_TYPE_ACCELEROMETER);
        sensor_data_t cpu_data = read_sensor(SENSOR_TYPE_CPU_USAGE);
        sensor_data_t time_data = read_sensor(SENSOR_TYPE_TIME_OF_DAY);

        /* Create AI context with real sensor data */
        ai_context_t ai_context = {
            .accelerometer = accel_data,
            .cpu_usage = cpu_data,
            .memory_usage = read_sensor(SENSOR_TYPE_MEMORY_USAGE),
            .user_activity = read_sensor(SENSOR_TYPE_USER_ACTIVITY),
            .time_of_day = time_data,
            .system_uptime = get_tick_count()
        };

        /* Run real AI inference with loaded model */
        ai_decision_t decision = run_ai_inference(&loaded_model.runtime_model, &ai_context);

        /* Display intelligent decision */
        const char *decision_texts[] = {
            "Errore", "ATTIVO", "LAVORO", "GAMING", "RIPOSO"
        };

        char buffer[32];
        vga_print("DECISIONE IA: ", 0, 48, VGA_COLOR_YELLOW);
        vga_print(decision_texts[decision], 15, 48, VGA_COLOR_CYAN);

        vga_print("IA CARICATA CON SUCCESSO - L'AI E' TUTTA TUAA!", 0, 50, VGA_COLOR_MAGENTA);
        vga_print("Qualsiasi dimensione, qualsiasi formato! Computer gestisce tutto!", 0, 51, VGA_COLOR_LIGHT_MAGENTA);

    } else {
        vga_print("ERRORE nel caricamento IA - Riprova o usa file diverso", 0, 47, VGA_COLOR_RED);
    }
}

void callback_exit() {
    vga_print("Spegnimento sistema operativo AI-centrico...", 0, 42, VGA_COLOR_RED);
    vga_print("Grazie per aver esplorato il futuro del computing!", 0, 43, VGA_COLOR_WHITE);
}

void callback_info() {
    vga_print("My OS - Sistema Operativo AI-centrico", 0, 38, VGA_COLOR_WHITE);
    vga_print("CPU: Fine-grained multitasking con scheduler round-robin", 0, 39, VGA_COLOR_LIGHT_BLUE);
    vga_print("Memoria: Heap management con 1MB allocabile dinamicamente", 0, 40, VGA_COLOR_LIGHT_BLUE);
    vga_print("Sensori: 5 tipi AI-ready (accelerometro, CPU, memoria, orario, attivita)", 0, 41, VGA_COLOR_LIGHT_BLUE);
    vga_print("GUI: Framebuffer engine con linee Bresenham e colori RGB", 0, 42, VGA_COLOR_LIGHT_BLUE);
    vga_print("Carica un modello AI per attivare l'intelligenza(context-aware)!", 0, 43, VGA_COLOR_YELLOW);
}
