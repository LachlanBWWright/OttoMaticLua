#pragma once

#ifdef NDS
// NDS: no mouse smoothing needed
typedef struct { float xrel, yrel; } SDL_MouseMotionEvent;
static inline void MouseSmoothing_ResetState(void) {}
static inline void MouseSmoothing_StartFrame(void) {}
static inline void MouseSmoothing_OnMouseMotion(const SDL_MouseMotionEvent* motion) { (void)motion; }
static inline void MouseSmoothing_GetDelta(int* dxOut, int* dyOut) { *dxOut = 0; *dyOut = 0; }
#else
#include <SDL3/SDL.h>

void MouseSmoothing_ResetState(void);

void MouseSmoothing_StartFrame(void);

void MouseSmoothing_OnMouseMotion(const SDL_MouseMotionEvent* motion);

void MouseSmoothing_GetDelta(int* dxOut, int* dyOut);
#endif
