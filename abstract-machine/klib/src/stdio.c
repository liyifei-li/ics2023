#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int gputch(unsigned char type, char *ch, int c) {
  switch(type) {
    case 0:
      putch(c);
      break;
    case 2:
      *ch = c;
      break;
    default:
      assert(0);
  }
  return c;
}
int gprintf(unsigned char type, char *str, const char *fmt, va_list ap) {
  size_t i = 0, j = 0;
  size_t k;
  int dptr;
  char *sptr;
  char minint[15] = "2147483638";
  char dtos[15] = {0};
  bool isneg = 0;
  while (fmt[j] != '\0') {
    if (fmt[j] != '%') {
      gputch(type, str + i, fmt[j]);
      i++;
      j++;
    }
    else {
      /*
      while (j++) {
        assert(fmt[j] != '\0');
      }
      */
      switch(fmt[j + 1]) {
        case 's':
          sptr = va_arg(ap, char*);
          strcpy(str + i, sptr);
          i += strlen(sptr);
          j += 2;
          break;
        case 'd':
          dptr = va_arg(ap, int);
          if (dptr == 0) {
            strcpy(dtos, "0");
            k = 1;
          }
          else if (dptr == 0x80000000) {
            strcpy(dtos, minint);
            isneg = 1;
            k = 10;
          }
          else {
            k = 0;
            if (dptr < 0) {
              dptr = -dptr;
              isneg = 1;
            }
            while (dptr) {
              dtos[k++] = dptr % 10 + '0';
              dptr /= 10;
            }
          }
          if (isneg)
            str[i++] = '-';
          while (k)
            str[i++] = dtos[--k];
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

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return gprintf(0, NULL, fmt, ap);
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  return gprintf(2, str, format, ap);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
