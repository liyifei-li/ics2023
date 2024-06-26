#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY);
  return read(fd, buf, len) != 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  return;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", O_WRONLY);
  for (int i = 0; i < h; i++) {
    lseek(fd, 4 * (x + (y + i) * screen_w), SEEK_SET);\
    write(fd, (void *)pixels, 4 * w);
    pixels += w;
  }
  return;
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  int fd = open("/proc/dispinfo", O_RDONLY);
  char buf[64];
  read(fd, buf, 64);
  char *ptr;
  ptr = strstr(buf, "WIDTH");
  assert(ptr != NULL);
  while(*ptr != '\0' && !isdigit(*ptr)) {
    ptr++;
  }
  screen_w = atoi(ptr);
  ptr = strstr(buf, "HEIGHT");
  assert(ptr != NULL);
  while(*ptr != '\0' && !isdigit(*ptr)) {
    ptr++;
  }
  screen_h = atoi(ptr);
  close(fd);
  return 0;
}

void NDL_Quit() {
}
