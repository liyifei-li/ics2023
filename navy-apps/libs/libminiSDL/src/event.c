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

static uint8_t keystate[256] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[32];
  char eventtype[8];
  char key[16];
  if (!NDL_PollEvent(buf, 32)) {
    return 0;
  }
  sscanf(buf, "%s %s", eventtype, key);
  int keycnt = sizeof(keyname) / sizeof(keyname[0]);
  ev->key.keysym.sym = SDLK_NONE;
  for (int i = 0; i < keycnt; i++) {
    if (strcmp(keyname[i], key) == 0) {
      ev->key.keysym.sym = i;
      if (strcmp(eventtype, "kd") == 0) {
        ev->type = SDL_KEYDOWN;
        keystate[i] = 1;
      }
      else if (strcmp(eventtype, "ku") == 0) {
        ev->type = SDL_KEYUP;
        keystate[i] = 0;
      }
      else {
        assert(0);
      }
      break;
    }
  }
  assert(ev->key.keysym.sym != 0);
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  while(SDL_PollEvent(event));
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL) {
    *numkeys = 83;
  }
  return keystate;
}
