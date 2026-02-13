#ifndef STDIO_H_
#define STDIO_H_

#include <stdarg.h>
#include "os.h"
#include "stdlib.h"
#include "string.h"

void PRINT(const char *format, ...);
void READ(const char *format, ...);

#endif  /* STDIO_H_ */