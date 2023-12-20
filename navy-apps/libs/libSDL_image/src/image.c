#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#include <fcntl.h>
#include <stdio.h>

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *fd = fopen(filename, "rb");
  assert(fd);
  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  void *buf = malloc(size);
  fseek(fd, 0, SEEK_SET);
  fread(buf, 1, size, fd);
  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, size);
  free(buf);
  fclose(fd);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
