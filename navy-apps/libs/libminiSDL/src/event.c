#include <NDL.h>
#include <SDL.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[32];
  char eventtype[8];
  char key[16];
  while (!NDL_PollEvent(buf, 64));
  sscanf(buf, "%s %s", eventtype, key);
  if (strcmp(eventtype, "kd") == 0) {
    event->type = SDL_KEYDOWN;
  }
  else if (strcmp(eventtype, "ku") == 0) {
    event->type = SDL_KEYUP;
  }
  else {
    assert(0);
  }
  int keycnt = sizeof(keyname) / sizeof(keyname[0]);
  event->key.keysym.sym = SDLK_NONE;
  for (int i = 0; i < keycnt; i++) {
    if (strcmp(keyname[i], key) == 0) {
      event->key.keysym.sym = i;
      break;
    }
  }
  assert(event->key.keysym.sym != 0);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
