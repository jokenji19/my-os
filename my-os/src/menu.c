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

/* Demo callbacks - moved to header for external access */
void callback_insert_ai() {
    nn_model_t demo_model;
    ai_context_t ai_context;

    vga_print("File IA caricato! Inizializzo modello di intelligenza artificiale...", 0, 38, VGA_COLOR_MAGENTA);

    /* SIMULATION: AI in azione per dimostrazione */
    vga_print("AI attiva! Analizzo contesto e prendo decisioni...", 0, 39, VGA_COLOR_GREEN);

    /* Finge analisi AI - usa sensori reali ma logica semplificata */
    sensor_data_t accel_data = read_sensor(SENSOR_TYPE_ACCELEROMETER);
    sensor_data_t cpu_data = read_sensor(SENSOR_TYPE_CPU_USAGE);
    sensor_data_t time_data = read_sensor(SENSOR_TYPE_TIME_OF_DAY);

    /* Simple "AI" logic per dimostrazione */
    char decision_text[40];
    char buffer[32];

    if (cpu_data.x_value > 50.0 && time_data.x_value >= 9.0 && time_data.x_value <= 18.0) {
        strcpy(decision_text, "Modo LAVORO - Focus produttivita!");
    } else if (accel_data.x_value > 12.0) {
        strcpy(decision_text, "Modo ATTIVO - Tocco e movimento!");
    } else if (time_data.x_value >= 22.0 || time_data.x_value <= 6.0) {
        strcpy(decision_text, "Modo RIPOSO - Risparmio energetico");
    } else {
        strcpy(decision_text, "Modo IDLE - Attesa interazione");
    }

    vga_print("Decisione AI: ", 0, 40, VGA_COLOR_YELLOW);
    vga_print(decision_text, 14, 40, VGA_COLOR_CYAN);

    /* Mostra dati sensori letti */
    itoa((int)(accel_data.x_value), buffer, 10);
    vga_print("Accelerometro: ", 0, 41, VGA_COLOR_WHITE);
    vga_print(buffer, 16, 41, VGA_COLOR_LIGHT_BLUE);

    itoa((int)cpu_data.x_value, buffer, 10);
    vga_print("CPU: ", 23, 41, VGA_COLOR_WHITE);
    vga_print(buffer, 28, 41, VGA_COLOR_LIGHT_BLUE);
    vga_print("%", 35, 41, VGA_COLOR_LIGHT_BLUE);

    vga_print("AI SIMULATION funzionante! Sistema intelligente attivo!", 0, 42, VGA_COLOR_GREEN);
    vga_print("Nota: AI semplificata per demo - Runtime completo su hardware reale", 0, 43, VGA_COLOR_WHITE);
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
