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
  /*
  union si {
    int64_t d64;
    int32_t d32;
    int16_t d16;
    int8_t d8;
  } d;
  */
  int32_t dd;
  char ch;
  char *sptr = NULL;
  char numstr[30];
//  char flags;
  int32_t width;
  int32_t precision;
//  int32_t length;
  while (fmt[j] != '\0') {
    if (fmt[j] != '%') {
      gputch(type, str + cnt, fmt[j]);
      cnt++;
      j++;
    }
    else {
      j++;
//      flags = 0;
      width = 0;
      precision = 0;
//      length = 32;
      /*
      if (fmt[j] == '-' || fmt[j] == '+' || fmt[j] == ' ' || fmt[j] == '#' || fmt[j] == '0') {
        flags = fmt[j];
        j++;
      }
      */
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
/*
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
*/
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
          }
          for (int i = 0; i < width - slen; i++) {
            gputch(type, str + cnt, ' ');
            cnt++;
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
          j++;
          /*
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
            strcpy(numstr, "-9223372036854775808");
            slen = 20;
          }
          else {
            slen = 0;
            if (d.d64 < 0) {
              d.d64 = -d.d64;
              numstr[slen++] = '-';
            }
            while (d.d64) {
              numstr[slen++] = d.d64 % 10 + '0';
              d.d64 /= 10;
            }
          }
          */
          if (dd == 0) {
            numstr[0] = '0';
            slen = 1;
          }
          else {
            slen = 0;
            if (dd < 0) {
              dd = -dd;
              numstr[slen++] = '-';
            }
            while (dd) {
              numstr[slen++] = dd % 10 + '0';
              dd /= 10;
            }
          }
          for (int i = 0; i < width - precision && i < width - slen; i++) {
            gputch(type, str + cnt, ' ');
            cnt++;
          }
          for (int i = 0; i < precision - slen; i++) {
            gputch(type, str + cnt, '0');
            cnt++;
          }
          for (int i = 0; i < slen; i++) {\
            gputch(type, str + cnt, numstr[i]);
            cnt++;
          }
          break;
        default:
          assert(0);
      }

      /*
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
