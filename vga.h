#ifndef VGA_H
#define VGA_H
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "klib.h"

#define TEXT_SCREEN_WIDTH 80
#define TEXT_SCREEN_HEIGHT 25
#define TEXT_VIDEO_MEMORY_ADDRESS 0xB8000

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static int pos = 0;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void update_cursor(int position)
{
    outb(0x3D4, 0x0F); // Set low byte of cursor position
    outb(0x3D5, (uint8_t)(position & 0xFF));
    outb(0x3D4, 0x0E); // Set high byte of cursor position
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ (
        "inb %1, %0"       // Read from the I/O port into the result
        : "=a"(result)      // Output operand (store result in the 'result' variable)
        : "d"(port)         // Input operand (pass the I/O port value in the 'port' variable)
    );
    return result;
}

void scroll()
{
    for (int row = 0; row < TEXT_SCREEN_HEIGHT - 1; row++) {
        for (int col = 0; col < TEXT_SCREEN_WIDTH; col++) {
            int from = row * TEXT_SCREEN_WIDTH + col;
            int to = (row + 1) * TEXT_SCREEN_WIDTH + col;
            *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + from * 2) =
                *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + to * 2);
        }
    }

    // Clear the last row
    for (int col = 0; col < TEXT_SCREEN_WIDTH; col++) {
        int pos = (TEXT_SCREEN_HEIGHT - 1) * TEXT_SCREEN_WIDTH + col;
        *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + pos * 2) =
            (uint16_t)(' ' | ((uint16_t)(VGA_COLOR_WHITE | VGA_COLOR_BLUE << 4)) << 8);
    }
}

void clear()
{
    for (uint16_t i = 0; i < (TEXT_SCREEN_WIDTH * TEXT_SCREEN_HEIGHT); i++) {
        *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + i * 2) =
            (uint16_t)(' ' | ((uint16_t)(VGA_COLOR_WHITE | VGA_COLOR_BLUE << 4)) << 8);
    }
    pos = 0;
    update_cursor(pos);
}

void putchar(char c)
{
    if (c == '\n') {
        pos += TEXT_SCREEN_WIDTH - (pos % TEXT_SCREEN_WIDTH);
    }
    else if (c == '\b') { // Handle backspace
        if (pos > 0) {
            pos--; // Move cursor back
            *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + pos * 2) =
                (uint16_t)(' ' | ((uint16_t)(VGA_COLOR_WHITE | VGA_COLOR_BLUE << 4)) << 8); // Replace with space
        }
    } 
     else {
        *(uint16_t*)(uintptr_t)(TEXT_VIDEO_MEMORY_ADDRESS + pos * 2) =
            (uint16_t)(c | ((uint16_t)(VGA_COLOR_WHITE | VGA_COLOR_BLUE << 4)) << 8);
        pos++;
    }

    // Handle line wrapping and scrolling
    if (pos >= TEXT_SCREEN_WIDTH * TEXT_SCREEN_HEIGHT) {
        scroll();
        pos -= TEXT_SCREEN_WIDTH;
    }

    update_cursor(pos);
}

void puts(const char* s)
{
    for (size_t i = 0; i < strlen(s); i++) {
        putchar(s[i]);
    }
}

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[32]; // Temporary buffer for numbers
    const char *str;
    int num;
    double fnum;

    for (; *format != '\0'; format++) {
        if (*format == '%') {
            format++; // Move past '%'

            switch (*format) {
                case 's': // String
                    str = va_arg(args, const char *);
                    puts(str);
                    break;

                case 'd': // Decimal integer
                case 'i': // Integer
                    num = va_arg(args, int);
                    itoa(num, buffer, 10);
                    puts(buffer);
                    break;

                case 'x': // Hexadecimal
                    num = va_arg(args, int);
                    itoa(num, buffer, 16);
                    puts(buffer);
                    break;

                case 'c': // Character
                    num = va_arg(args, int);
                    char charBuffer[2] = {(char)num, '\0'};
                    puts(charBuffer);
                    break;

                case 'f': // Floating-point
                    fnum = va_arg(args, double);
                    ftoa(fnum, buffer, 10); // Default precision: 6
                    puts(buffer);
                    break;    

                case '%': // Literal '%'
                    puts("%");
                    break;

                default: // Unknown specifier
                    puts("?");
                    break;
            }
        } else {
            // Print characters as-is
            char charBuffer[2] = {*format, '\0'};
            puts(charBuffer);
        }
    }

    va_end(args);
}

#endif // VGA_H
