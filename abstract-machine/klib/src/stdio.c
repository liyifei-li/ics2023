#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *str, const char *format, ...) {
  size_t i = 0, j = 0;
  int dptr;
  char *sptr;
  char minint[15] = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
  va_list ap;
  va_start(ap, format);
  while (format[j] != '\0') {
    if (format[j] != '%') {
      str[i] = format[j];
      i++;
      j++;
    }
    else {
      switch(format[j + 1]) {
        case 's':
          sptr = va_arg(ap, char*);
          strcpy(str + i, sptr);
          i += strlen(sptr);
          j += 2;
          break;
        case 'd':
          dptr = *va_arg(ap, int*);
          if (dptr == 0) {
            str[i++] = '0';
          }
          else if (dptr == 0x80000000) {
            strcpy(str + i, minint);
            i += 11;
          }
          else {
            if (dptr < 0) {
              dptr = -dptr;
              str[i++] = '-';
            }
            while (dptr) {
              str[i++] = dptr % 10 + '0';
              dptr /= 10;
            }
          }
          j += 2;
          break;
        default:
          panic("Not implemented");
          break;
      }
    }
  }
  str[i] = '\0';
  va_end(ap);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
