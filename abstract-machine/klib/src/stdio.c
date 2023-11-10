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
  size_t cnt = 0;
  size_t j = 0;
//  int dptr;
  char *sptr = NULL;
  char *strptr = NULL;
  char flags;
  uint32_t width;
  uint32_t precision;
  uint32_t length;
/*
  char minint[15] = "2147483638";
  char dtos[15] = {0};
  bool isneg = 0;
*/
  while (fmt[j] != '\0') {
    if (fmt[j] != '%') {
      gputch(type, strptr, fmt[j]);
      cnt++;
      if (strptr != NULL)
        strptr++;
      j++;
    }
    else {
      flags = 0;
      if (flags) {};
      width = 0;
      precision = 0;
      length = 32;
      if (length) {};
      if (fmt[j] == '-' || fmt[j] == '+' || fmt[j] == ' ' || fmt[j] == '#' || fmt[j] == '0') {
        flags = fmt[j];
        j++;
      }

      if (fmt[j] == '*') {
        width = va_arg(ap, uint32_t);
        j++;
      }
      else {
        while (fmt[j] >= '0' && fmt[j] <= '9') {
          width = 10 * width + fmt[j] - '0';
          j++;
        }
      }

      if (fmt[j] == '.') {
        j++;
        if (fmt[j] == '*') {
          precision = va_arg(ap, uint32_t);
          j++;
        }
        else {
          assert(fmt[j] >= '0' && fmt[j] <= '9');
          while (fmt[j] >= '0' && fmt[j] <= '9') {
            precision = 10 * precision + fmt[j] - '0';
            j++;
          }
        }
      }

      if (fmt[j] == 'h') {
        j++;
        if (fmt[j] == 'h') {
          j++;
          length = 8;
        }
        else {
          length = 16;
        }
      }
      else if (fmt[j] == 'l') {
        j++;
        if (fmt[j] == 'l') {
          j++;
          length = 64;
        }
        else {
          length = 32;
        }
      }
      switch(fmt[j]) {
        case 'c':
          int c = va_arg(ap, int);
          gputch(type, strptr, c);
          cnt++;
        break;
        case 's':
          sptr = va_arg(ap, char*);
          while (*sptr != '\0') {
            gputch(type, strptr, *sptr);
            cnt++;
            if (strptr != NULL)
              strptr++;
            j++;
          }
          break;
        case 'd': case 'u':
          break;
      }
      /*
      while (j++) {
        switch(fmt[j]) {
          case '\0':
            assert(0);
          case '-': case '+': case ' ': case '#':
            assert(proc == 0);
            flags = fmt[j];
            proc = 1;
            break;
          case '0':
            if (proc == 0) {
              flags = '0';
              proc = 1;
            }
            else {
              assert(proc == 1);
            }
          case '1': 
        }
      }
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
    */
    }
  }
  va_end(ap);
  return cnt;
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
