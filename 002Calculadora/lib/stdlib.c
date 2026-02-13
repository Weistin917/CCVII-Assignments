#include "stdlib.h"

/**
 * Function to convert string to integer
 * @param s: string representing the integer
 * @return int: converted integer
 */
int atoi(const char *s) {
  int num = 0;
  int sign = 1;
  int i = 0;

  // Handle optional sign
  if (s[i] == '-') {
    sign = -1;
    i++;
  } else if (s[i] == '+') i++;

  for (; s[i] >= '0' && s[i] <= '9'; i++) {
    num = num * 10 + (s[i] - '0');
  }

  return sign * num;
}

/**
 * Function to convert integer to string representation
 * @param num: number to convert
 * @param buffer: pointer to the space where the string will be stored
 */
void itoa(int num, char *buffer) {
  int i = 0;
  int is_negative = 0;

  if (num == 0) {
    buffer[i++] = '0';
    buffer[i] = '\0';
    return;
  }

  if (num < 0) {
    is_negative = 1;
    num = -num;
  }

  while (num > 0 && i < 14) { // Reserve space for sign and null terminator
    buffer[i++] = '0' + (num % 10);
    num /= 10;
  }

  if (is_negative) {
    buffer[i++] = '-';
  }

  buffer[i] = '\0';

  // Reverse the string
  int start = 0, end = i - 1;
  char temp;
  while (start < end) {
    temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }
}

/**
 * Function to convert string to floating point number
 * @param s: string containing the represented number
 * @return float: converted number or 0 if there's invalid character
 */
float atof(const char *s) {
  float num, sign = 1;
  char isFraction;
  int pos = 10;

  if (*s == '-') {
    sign = -1;
    s++;
  } else if (*s == '+') s++;

  while (*s) {
    if (!isFraction && *s == '.') {
      isFraction = 1;
      s++;
    } else if (*s >= '0' && *s <= '9') {
      if (isFraction) {
        num += (float)(*s++ - '0') / pos;
        pos *= 10;
      } else {
        num = num * 10 + (*s++ - '0');
      }
    } else {
      return 0;
    }
  }
  
  return sign * num;
}

/**
 * Function to convert float to string
 * @param num: number to convert
 * @param buffer: memory space to store the resulting string
 */
void ftoa(float num, char *buffer) {
  int i = 0, isNegative = 0, integer = (int)num;

  if (num == 0) {
    buffer[i++] = '0';
    buffer[i++] = '.';
    buffer[i++] = '0';
    buffer[i++] = '0';
    buffer[i] = '\0';
    return;
  }

  if (num < 0) {
    isNegative = 1;
    num = -num;
  }

  // integer part of the number
  while (integer > 0 && i < 14) { // Reserve space for sign and null terminator
    buffer[i++] = '0' + (integer % 10);
    integer /= 10;
  }

  if (isNegative) {
    buffer[i++] = '-';
  }

  // Reverse the string
  int start = 0, end = i - 1;
  char temp;
  while (start < end) {
    temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }

  // decimal part
  buffer[i++] = '.';
  int limit = i + 5, temp_num;
  while (i < limit && (num - (float)temp_num)) {
    temp_num = (int)(num * 10);
    buffer[i++] = '0' + (temp_num % 10);
    num *= 10;
  }
  // null terminator
  buffer[i] = '\0';
}