#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while (s[i] != '\0')
    i++;
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while (src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for (; i < n; i++)
    dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i = 0;
  while (src[i] != '\0') {
    dst[dst_len + i] = src[i];
    i++;
  }
  dst[dst_len + i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  while (1) {
    if (s1[i] - s2[i] != 0) return s1[i] - s2[i];
    if (s1[i] == '\0' && s2[i] == '\0') return 0;
    i++;
  }
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  while (i < n) {
    if (s1[i] - s2[i] != 0) return s1[i] - s2[i];
    if (s1[i] == '\0' && s2[i] == '\0') return 0;
    i++;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  size_t i = 0;
  while (i < n) {
    *(uint8_t *)(s + i) = c;
    i++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  size_t i;
  if (dst > src) {
    i = 0;
    while (i < n) {
      *(uint8_t *)(dst + i) = *(uint8_t *)(src + i);
      i++;
    }
  }
  else {
    i = n - 1;
    while (i > 0) {
      *(uint8_t *)(dst + i) = *(uint8_t *)(src + i);
      i--;
    }
    *(uint8_t *)dst = *(uint8_t *)src;
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i = 0;
  while (i < n) {
    *(uint8_t *)(out + i) = *(uint8_t *)in + i;
    i++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i = 0;
  while (i < n) {
    if (*(uint8_t *)s1 - *(uint8_t *)s2 != 0) return *(uint8_t *)s1 - *(uint8_t *)s2;
    if (*(uint8_t *)s1 == '\0' && *(uint8_t *)s2 == '\0') return 0;
    i++;
  }
  return 0;
}

/*
int sprintf(char *str, const char *format, ...) {
  size_t i = 0, j = 0;
  int dptr;
  char *sptr;
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
        if (dptr < 0) {
          dptr = -dptr;
          str[i++] = '-';
        }
        if (dptr == 0) {
          str[i++] = '0';
        }
        else {
          while (dptr) {
            str[i++] = dptr % 10;
            dptr /= 10;
          }
        }
        j += 2;
        break;
      default:
        assert(0);
        break;
      }
    }
  }
  str[i] = '\0';
  va_end(ap);
  return i;
}
*/

#endif
