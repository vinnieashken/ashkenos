#ifndef KLIB_H
#define KLIB_H
#include <stdbool.h>
#include <stdint.h>

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}
// Implementation of itoa()
// void itoa(int value, char *str, int base) {
//     char *ptr = str, *ptr1 = str, tmp_char;
//     int tmp_value;

//     // Handle negative numbers for base 10
//     if (value < 0 && base == 10) {
//         *ptr++ = '-';
//         value = -value;
//     }

//     // Convert the number to the given base
//     do {
//         tmp_value = value % base;
//         *ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'A');
//     } while (value /= base);

//     *ptr-- = '\0'; // Null-terminate the string

//     // Reverse the string
//     while (ptr1 < ptr) {
//         tmp_char = *ptr;
//         *ptr-- = *ptr1;
//         *ptr1++ = tmp_char;
//     }
// }


void itoa(int32_t value, char *str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0'; // Invalid base
        return;
    }

    char *ptr = str;
    char *end = str;
    int is_negative = (value < 0 && base == 10); // Only base 10 supports negative numbers
    uint32_t uvalue = (uint32_t)value;

    if (is_negative) {
        uvalue = (uint32_t)(-value); // Handle negative values by converting to unsigned
    }

    // Convert the integer to the given base
    do {
        int digit = uvalue % base;
        *end++ = (digit > 9) ? ('A' + digit - 10) : ('0' + digit);
        uvalue /= base;
    } while (uvalue);

    if (is_negative) {
        *end++ = '-';
    }

    *end = '\0'; // Null-terminate the string

    // Reverse the string
    while (end > ptr) {
        char temp = *--end;
        *end = *ptr;
        *ptr++ = temp;
    }
}

void ftoa(double value, char *str, int precision) {

    if (str == NULL) {
        return; // Ensure the buffer is not NULL
    }

     // Preserve the current value of str
    char *ptr = str;
    // Handle negative values
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }

    // Extract the integer part
    int integer_part = (int64_t)value;

    // Extract the fractional part
    double fractional_part = value - (double)integer_part;

    // Convert the integer part to a string
    char temp[32]; // Temporary buffer for integer conversion

    itoa((int)integer_part, temp, 10); // Ensure integer_part fits in `int`

    // Use a separate pointer for clarity
    char *temp_ptr = temp; 
   
   // Copy the integer part into the final string
    while (*temp_ptr != '\0') {
        *ptr = *temp_ptr;
        ptr++;
        temp_ptr++;
    }

    // Add the decimal point
    if (precision > 0) {
        *ptr++ = '.';
              
       // Convert the fractional part
        for (int i = 0; i < precision; i++) {
            fractional_part *= 10;
            int digit = (int64_t)fractional_part;
            *ptr++ = '0' + digit;
            fractional_part -= digit;
        }
    }

    // Null-terminate the string
    *ptr = '\0';
}

int sprintf(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *buf_ptr = buffer;
    const char *fmt_ptr = format;

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++; // Skip '%'

            switch (*fmt_ptr) {
                case 'd': { // Signed integer
                    int value = va_arg(args, int);
                    char temp[12]; // Temporary buffer for integer
                    itoa(value, temp, 10); // Convert integer to string
                    char *temp_ptr = temp;
                    while (*temp_ptr) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'u': { // Unsigned integer
                    unsigned int value = va_arg(args, unsigned int);
                    char temp[12];
                    itoa((int)value, temp, 10); // Use itoa for unsigned
                    char *temp_ptr = temp;
                    while (*temp_ptr) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'x': { // Hexadecimal
                    unsigned int value = va_arg(args, unsigned int);
                    char temp[12];
                    itoa((int)value, temp, 16); // Convert to hexadecimal
                    char *temp_ptr = temp;
                    while (*temp_ptr) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'f': { // Floating-point
                    double value = va_arg(args, double);
                    char temp[32]; // Temporary buffer for float
                    ftoa(value, temp, 6); // Convert float to string with precision 6
                    char *temp_ptr = temp;
                    while (*temp_ptr) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'c': { // Character
                    char value = (char)va_arg(args, int);
                    *buf_ptr++ = value;
                    break;
                }
                case 's': { // String
                    char *value = va_arg(args, char *);
                    while (*value) {
                        *buf_ptr++ = *value++;
                    }
                    break;
                }
                default: {
                    // Handle unknown format specifiers
                    *buf_ptr++ = '%';
                    *buf_ptr++ = *fmt_ptr;
                    break;
                }
            }
        } else {
            *buf_ptr++ = *fmt_ptr; // Copy regular characters
        }
        fmt_ptr++;
    }

    *buf_ptr = '\0'; // Null-terminate the buffer
    va_end(args);

    return (buf_ptr - buffer); // Return the length of the formatted string
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;
    while (n--)
    *d++ = *s++;
    return dst;
}

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    while (n--)
    *p++ = c;
    return buf;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
    *d++ = *src++;
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
    if (*s1 != *s2)
    break;
    s1++;
    s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

#endif