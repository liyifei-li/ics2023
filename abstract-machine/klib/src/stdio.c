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
    case 1: case 2: case 3: case 4:
      *ch = c;
      break;
    default:
      assert(0);
  }
  return c;
}

static const char numlist[16] = "0123456789abcdef";

int gprintf(size_t n, unsigned char type, char *str, const char *fmt, va_list ap) {
  size_t cnt = 0;
  size_t j = 0;
  int32_t slen;
  uint64_t u;
  int64_t d;
  char ch;
  char *sptr = NULL;
  bool isneg;
  char numstr[30];
  char flags;
  int32_t width;
  int32_t precision;
  int32_t length;
  uint32_t son;
  while (fmt[j] != '\0') {
    if (cnt == n) break;
    if (fmt[j] != '%') {
      gputch(type, str + cnt, fmt[j]);
      cnt++;
      if (cnt == n) break;
      j++;
    }
    else {
      j++;
      flags = 0;
      width = 0;
      precision = 0;
      length = 32;
      
      if (fmt[j] == '0') {
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
        case 'c': case 's':
          assert(!flags || flags == '-');
          if (fmt[j] == 'c') {
            ch = va_arg(ap, int);
            sptr = &ch;
            slen = 1;
          }
          else {
            sptr = va_arg(ap, char*);
            slen = strlen(sptr);
          }
          if (flags == 0) {
            for (int i = 0; i < slen; i++) {
              gputch(type, str + cnt, *(sptr + i));
              cnt++;
              if (cnt == n) break;
            }
            for (int i = 0; i < width - slen; i++) {
              gputch(type, str + cnt, ' ');
              cnt++;
              if (cnt == n) break;
            }
          }
          else {
            for (int i = 0; i < width - slen; i++) {
              gputch(type, str + cnt, ' ');
              cnt++;
              if (cnt == n) break;
            }
            for (int i = 0; i < slen; i++) {
              gputch(type, str + cnt, *(sptr + i));
              cnt++;
              if (cnt == n) break;
            }
          }
          j++;
        break;
          case 'o': case 'd': case 'x': case 'u':
          d = 0;
          u = 0;
          if (fmt[j] == 'd') {
            switch(length) {
              case 8:  d = (int8_t) va_arg(ap, int32_t); break;
              case 16: d = (int16_t)va_arg(ap, int32_t); break;
              case 32: d = (int32_t)va_arg(ap, int32_t); break;
              case 64: d = (int64_t)va_arg(ap, int64_t); break;
              default: assert(0);
            }
            if (d < 0) {
              isneg = 1;
              u = -d;
            }
            else {
              isneg = 0;
              u = d;
            }
          }
          else {
            switch(length) {
              case 8:  u = (uint8_t) va_arg(ap, uint32_t); break;
              case 16: u = (uint16_t)va_arg(ap, uint32_t); break;
              case 32: u = (uint32_t)va_arg(ap, uint32_t); break;
              case 64: u = (uint64_t)va_arg(ap, uint64_t); break;
              default: assert(0);
            }
            isneg = 0;
          }
          if (u == 0) {
            numstr[0] = '0';
            slen = 1;
          }
          else {
            slen = 0;
            son = fmt[j] == 'o' ? 8
                : fmt[j] == 'd' || fmt[j] == 'u' ? 10
                : fmt[j] == 'x' ? 16
                : 0;
            while (u) {
              numstr[slen++] = numlist[u % son];
              u /= son;
            }
            if (isneg) {
              numstr[slen++] = '-';
            }
          }
          for (int i = 0; i < width - precision && i < width - slen; i++) {
            if (flags == 0) {
              gputch(type, str + cnt, ' ');
              if (cnt == n) break;
            }
            else if (flags == '0') {
              gputch(type, str + cnt, '0');
              if (cnt == n) break;
            }
            cnt++;
          }
          for (int i = 0; i < precision - slen; i++) {
            gputch(type, str + cnt, '0');
            cnt++;
            if (cnt == n) break;
          }
          for (int i = 0; i < slen; i++) {
            gputch(type, str + cnt, numstr[slen - i - 1]);
            cnt++;
            if (cnt == n) break;
          }
          j++;
          break;
        default:
          printf(" %c unsupported\n", fmt[j]);
          assert(0);
      }
    }
  }
  va_end(ap);
  gputch(type, str + cnt, '\0');
  return cnt;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return gprintf(-1, 0, NULL, fmt, ap);
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return gprintf(-1, 1, out, fmt, ap);
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  return gprintf(-1, 2, str, format, ap);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return gprintf(-1, 3, out, fmt, ap);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return gprintf(-1, 4, out, fmt, ap);
}

#endif
