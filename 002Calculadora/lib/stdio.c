#include "stdio.h"

/**
 * Display the given message on the terminal
 * @param format: the string to be displayed. Can contain the format for additional inputs to be inserted.
 *                %s for strings, %d for integers, %f for floating point numbers
 * @param args: additional arguments to be passed according to the given format
 */
void PRINT(const char *format, ...) {
  va_list args;
  va_start(args, format);

  char conversion[15];
  
  while (*format) {
    if (*format == '%') {
      switch (*(format + 1)) {
        case 's':
          char *s = va_arg(args, char*);
          uart_puts(s);
          format++;
          break;
        case 'd':
          int d = va_arg(args, int);
          itoa(d, conversion);
          uart_puts(conversion);
          format++;
          break;
        case 'f':
          double f = va_arg(args, double);
          ftoa((float)f, conversion);
          uart_puts(conversion);
          format++;
          break;
        default:
          uart_putc(*format);
          break;
      }
      format++;
    } else {
      uart_putc(*format++);
    }
  }
}

/**
 * Reads the input from the terminal following the given format
 * @param format: the format of the input to be read
 *                %s to read string, %d to read integer, %f to read floating point number
 * @param args: pointers to the memory spaces of where the inputs are to be saved
 */
void READ(const char *format, ...) {
  va_list args;
  va_start(args, format);

  int endsInNL;
  char buffer[20];

  while (*format) {
    if (*format == ' ' || *format == '\t') format++;
    else if (*format == '%') {
      switch(*(format + 1)) {
        case 's':
          int s_length = uart_get_until_whitespace(buffer, 20, &endsInNL);
          char *s = va_arg(args, char *);
          my_strncpy(s, buffer, s_length);
          format++;
          break;
        case 'd':
          uart_get_until_whitespace(buffer, 20, &endsInNL);
          int *d = va_arg(args, int *);
          *d = atoi(buffer);
          format++;
          break;
        case 'f':
          uart_get_until_whitespace(buffer, 20, &endsInNL);
          float *f = va_arg(args, float *);
          *f = atof(buffer);
          format++;
          break;
        default:
          break;
      }
      format++;
    } else if (!uart_get_match(*format++)) {
      break;
    }
  }
  if (!endsInNL) uart_gets_input(buffer, 20);
}