#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  int free_srcrect = 0;
  int free_dstrect = 0;
  if (srcrect == NULL) {
    srcrect = malloc(sizeof(SDL_Rect));
    assert(srcrect);
    free_srcrect = 1;
    srcrect->x = 0;
    srcrect->y = 0;
    srcrect->w = src->w;
    srcrect->h = src->h;
  }
  if (dstrect == NULL) {
    dstrect = malloc(sizeof(SDL_Rect));
    assert(dstrect);
    free_dstrect = 1;
    dstrect->x = 0;
    dstrect->y = 0;
  }
  uint8_t BytesPerPixel = src->format->BytesPerPixel;
  assert(srcrect->x >= 0 && srcrect->x + srcrect->w <= src->w);
  assert(srcrect->y >= 0 && srcrect->y + srcrect->h <= src->h);
  void *pixels = malloc(BytesPerPixel * srcrect->w * srcrect->h);
  void *pos;
  for (int i = 0; i < srcrect->h; i++) {
    pos = src->pixels + BytesPerPixel * (srcrect->x + (i + srcrect->y) * src->w);
    memcpy(pixels + BytesPerPixel * i * srcrect->w, pos, BytesPerPixel * srcrect->w);
  }
  assert(dstrect->x >= 0 && dstrect->x + srcrect->w <= dst->w);
  assert(dstrect->y >= 0 && dstrect->y + srcrect->h <= dst->h);
  for (int i = 0; i < srcrect->h; i++) {
    pos = dst->pixels + BytesPerPixel * ((dstrect->x) + (i + dstrect->y) * dst->w);
    memcpy(pos, pixels + BytesPerPixel * i * srcrect->w, BytesPerPixel * srcrect->w);
  }
  if (free_srcrect) free(srcrect);
  if (free_dstrect) free(dstrect);
  free(pixels);
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
  int free_dstrect = 0;
  assert(dst);
  if (dstrect == NULL) {
    dstrect = malloc(sizeof(SDL_Rect));
    assert(dstrect);
    free_dstrect = 1;
    dstrect->x = 0;
    dstrect->y = 0;
    dstrect->w = dst->w;
    dstrect->h = dst->h;
  }
  uint8_t BytesPerPixel = dst->format->BytesPerPixel;
  if (dstrect->x < 0) dstrect->x = 0;
  if (dstrect->y < 0) dstrect->y = 0;
  if (dstrect->x + dstrect->w > dst->w) dstrect->w = dst->w - dstrect->x;
  if (dstrect->y + dstrect->h > dst->h) dstrect->h = dst->h - dstrect->y;
  void *pos;
  assert(BytesPerPixel == 1 || BytesPerPixel == 4);
  if (BytesPerPixel == 1) {
    for (int i = 0; i < dstrect->h; i++) {
      pos = dst->pixels + (dstrect->x + (i + dstrect->y) * dst->w);
      memset(pos, color, dstrect->w);
    }
  }
  else {
    for (int i = 0; i < dstrect->h; i++) {
      for (int j = 0; j < dstrect->w; j++) {
        pos = dst->pixels + BytesPerPixel * (dstrect->x + j + (i + dstrect->y) * dst->w);
        memcpy(pos, &color, BytesPerPixel);
      }
    }
  }
  if (free_dstrect) free(dstrect);
}

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  assert(s);
  if (x == 0 && y == 0 && w == 0 && h == 0) {
    w = s->w;
    h = s->h;
  }

  assert(x >= 0 && x + w <= s->w);
  assert(y >= 0 && y + h <= s->h);
  uint8_t BytesPerPixel = s->format->BytesPerPixel;
  int fd = open("/dev/fb", O_WRONLY);
  uint32_t *pixels = malloc(4 * w * h);
  assert(BytesPerPixel == 1 || BytesPerPixel == 4);
  if (BytesPerPixel == 1) {
    uint8_t *pos;
    SDL_Color *colors = s->format->palette->colors;
    uint32_t color;

    for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        pos = s->pixels + x + j + (i + y) * s->w;
        color = colors[*pos].r << 16 | colors[*pos].g << 8 | colors[*pos].b;
        memcpy(pixels + j + i * w, &color, 4);
      }
    }

  }
  else {
    for (int i = 0; i < h; i++) {
      memcpy(pixels + i * w, s->pixels + BytesPerPixel * (x + (i + y) * s->w), BytesPerPixel * w);
    }
  }
  NDL_DrawRect(pixels, x, y, w, h);
  close(fd);
  free(pixels);
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
}
