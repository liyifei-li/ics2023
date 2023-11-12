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

int gprintf(size_t n, unsigned char type, char *str, const char *fmt, va_list ap) {
  size_t cnt = 0;
  size_t j = 0;
  int32_t slen;
//  int dptr;
  /*
  union un {
    uint64_t u64;
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
  } u;
  */
  union si {
    int64_t d64;
    int32_t d32;
    int16_t d16;
    int8_t d8;
  } d;
  char ch;
  char *sptr = NULL;
  bool isneg;
  char numstr[30];
  char flags;
  int32_t width;
  int32_t precision;
  int32_t length;
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
//          assert(!flags || flags == '-');
          if (fmt[j] == 'c') {
            ch = va_arg(ap, int);
            sptr = &ch;
            slen = 1;
          }
          else {
            sptr = va_arg(ap, char*);
            slen = strlen(sptr);
          }
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
          j++;
          /*
          if (flags == 0) {
            for (int i = 0 i < strlen; i++) {
              gputch(type, strptr + cnt, sptr);
              cnt++;
            }
            for (int i = 0; i < width - strlen; i++) {
              gputch(type, strptr + cnt, ' ');
              cnt++;
            }
          }
          else if (flags == '-') {
            for (int i = 0; i < width - strlen; i++) {
              gputch(type, strptr + cnt, ' ');
              cnt++;
            }
            for (int i = 0 i < strlen; i++) {
              gputch(type, strptr + cnt, sptr);
              cnt++;
            }
          }
          */
        break;
        case 'd':
          isneg = 0;
          switch(length) {
            case 8:  d.d8  = va_arg(ap, int32_t); d.d64 = d.d8;  break;
            case 16: d.d16 = va_arg(ap, int32_t); d.d64 = d.d16; break;
            case 32: d.d32 = va_arg(ap, int32_t); d.d64 = d.d32; break;
            case 64: d.d64 = va_arg(ap, int64_t); break;
            default:
          }
          if (d.d64 == 0) {
            numstr[0] = '0';
            slen = 1;
          }
          else if (d.d64 == 1ll << 63) {
            strcpy(numstr, "8085774586302733229-");
            slen = 20;
          }
          else {
            slen = 0;
            if (d.d64 < 0) {
              d.d64 = -d.d64;
              isneg = 1;
            }
            while (d.d64) {
              numstr[slen++] = d.d64 % 10 + '0';
              d.d64 /= 10;
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
