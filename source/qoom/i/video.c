#include "qoom/i/system.h"
#include "qoom/i/video.h"
#include "qoom/v_video.h"
#include "qoom/m/argv.h"
#include "qoom/doomdef.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>

SDL_Window *gWindow = NULL;
int gWindowWidth = SCREENWIDTH,
    gWindowHeight = SCREENHEIGHT;
SDL_Renderer *gRenderer = NULL;
SDL_Palette *gPalette = NULL;
SDL_Surface *gFrame = NULL;

void I_InitGraphics(void) {
  int multiply = 1;
  if(M_CheckParm("-2")) multiply = 2;
  if(M_CheckParm("-3")) multiply = 3;
  if(M_CheckParm("-4")) multiply = 4;
  gWindowHeight *= multiply;
  gWindowWidth *= multiply;

  gWindow = SDL_CreateWindow("QOOM",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    gWindowWidth, gWindowHeight, 0);
  if(gWindow == NULL)
    I_Error("can't create window: %s", SDL_GetError());

  gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
  if(gRenderer == NULL)
    I_Error("can't create renderer: %s", SDL_GetError());

  gFrame = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0);
  if(gFrame == NULL)
    I_Error("can't allocate frame: %s", SDL_GetError());

  gPalette = SDL_AllocPalette(256);
  if(gPalette == NULL)
    I_Error("can't create palette: %s", SDL_GetError());

  SDL_SetSurfacePalette(gFrame, gPalette);
}

void I_ShutdownGraphics(void) {
  if(gFrame) SDL_FreeSurface(gFrame);
  if(gRenderer) SDL_DestroyRenderer(gRenderer);
  if(gWindow) SDL_DestroyWindow(gWindow);
}

void I_SetPalette(byte *palette) {
  SDL_Color colors[256];
  for(int i = 0; i < 256; i++) {
    colors[i].r = gammatable[usegamma][*palette++];
    colors[i].g = gammatable[usegamma][*palette++];
    colors[i].b = gammatable[usegamma][*palette++];
    colors[i].a = 0xFF;
  }
  SDL_SetPaletteColors(gPalette, colors, 0, 256);
}

void I_UpdateNoBlit(void) {}

void I_FinishUpdate(void) {
  memcpy(gFrame, screens[0], SCREENWIDTH*SCREENWIDTH);

  SDL_Texture *tex = SDL_CreateTextureFromSurface(gRenderer, gFrame);
  if(tex == NULL)
    I_Error("can't upload texture to renderer: %s", SDL_GetError());

  SDL_Rect srcrect = {0, 0, SCREENWIDTH, SCREENHEIGHT};
  SDL_Rect dstrect = {0, 0, gWindowWidth, gWindowHeight};
  SDL_RenderCopy(gRenderer, tex, &srcrect, &dstrect);

  SDL_DestroyTexture(tex);
}

void I_ReadScreen(byte *scr) {
  memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}
