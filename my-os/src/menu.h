/**
 * @file menu.h
 * @brief Interactive menu system for user interface
 */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>

/* Menu item types */
typedef enum {
    MENU_ITEM_BUTTON = 0,
    MENU_ITEM_CHECKBOX,
    MENU_ITEM_TEXT,
    MENU_ITEM_SEPARATOR
} menu_item_type_t;

/* Menu item structure */
typedef struct {
    menu_item_type_t type;
    const char *label;
    uint8_t selected;
    int x, y;  /* Position */
    int width, height;
    uint32_t color_normal;
    uint32_t color_highlight;
    void (*callback)(void);  /* Action when selected */
} menu_item_t;

/* Menu structure */
typedef struct {
    const char *title;
    menu_item_t *items;
    int num_items;
    int selected_index;
    int x_offset, y_offset;
} menu_t;

/* Function prototypes */
void init_menu_system();
menu_t *create_menu(const char *title, int x, int y);
int add_menu_item(menu_t *menu, menu_item_type_t type, const char *label,
                  int px, int py, int w, int h, uint32_t color_normal, uint32_t color_highlight, void (*callback)(void));
void render_menu(menu_t *menu);
void menu_select_next(menu_t *menu);
void menu_select_prev(menu_t *menu);
void menu_activate(menu_t *menu);
void destroy_menu(menu_t *menu);

#endif
