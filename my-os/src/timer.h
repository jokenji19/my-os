/**
 * @file timer.h
 * @brief PIT (Programmable Interval Timer) 8253/8254 management
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* PIT ports and commands */
#define PIT_CMD_REG    0x43
#define PIT_CHANNEL0   0x40

#define PIT_FREQUENCY  1193182  /* PIT internal frequency in Hz */

/* PIT command register bits */
#define PIT_CMD_BINARY      0x00  /* Binary mode */
#define PIT_CMD_BCD         0x01  /* BCD mode */
#define PIT_CMD_MODE0       0x00  /* Interrupt on terminal count */
#define PIT_CMD_MODE1       0x02  /* Hardware re-triggerable one-shot */
#define PIT_CMD_MODE2       0x04  /* Rate generator */
#define PIT_CMD_MODE3       0x06  /* Square wave generator */
#define PIT_CMD_MODE4       0x08  /* Software triggered strobe */
#define PIT_CMD_MODE5       0x0A  /* Hardware triggered strobe */
#define PIT_CMD_LSB         0x10  /* Access LSB only */
#define PIT_CMD_MSB         0x20  /* Access MSB only */
#define PIT_CMD_BOTH        0x30  /* Access LSB first, then MSB */
#define PIT_CMD_CHANNEL0    0x00  /* Select channel 0 */
#define PIT_CMD_CHANNEL1    0x40  /* Select channel 1 */
#define PIT_CMD_CHANNEL2    0x80  /* Select channel 2 */
#define PIT_CMD_READBACK    0xC0  /* Read-back command */

/* PIT modes */
#define PIT_MODE_SQUARE_WAVE  PIT_CMD_MODE3 | PIT_CMD_BOTH | PIT_CMD_BINARY

/* Function prototypes */
void pit_init(uint32_t frequency);
void timer_handler();
uint32_t get_tick_count();

#endif
