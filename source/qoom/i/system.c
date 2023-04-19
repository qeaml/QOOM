// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include <time.h>
#include <Windows.h>
#define __BYTEBOOL__ // to avoid redefinition in doomtype

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include "qoom/doomdef.h"
#include "qoom/m/misc.h"
#include "qoom/i/video.h"
#include "qoom/i/sound.h"

#include "qoom/d/net.h"
#include "qoom/g_game.h"

#include "qoom/i/system.h"


int mb_used = 6;

void I_Tactile(int on, int off, int total) {
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t emptycmd;

ticcmd_t *I_BaseTiccmd(void) {
  return &emptycmd;
}

int I_GetHeapSize(void) {
  return mb_used*1024*1024;
}

byte *I_ZoneBase(int *size) {
  *size = mb_used*1024*1024;
  return (byte *) malloc (*size);
}

//
// I_GetTime
// returns time in 1/70th second tics
//
int I_GetTime(void) {
  static clock_t basetime = 0;

  clock_t time = clock();
  if(!basetime)
    basetime = time;

  return time*TICRATE/CLOCKS_PER_SEC;
}

//
// I_Init
//
void I_Init (void) {
  I_InitSound();
  //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit(void) {
  D_QuitNetGame();
  I_ShutdownSound();
  I_ShutdownMusic();
  M_SaveDefaults();
  I_ShutdownGraphics();
  exit(0);
}

void I_WaitVBL(int count) {
  Sleep(count * (1000000/70));
}

void I_BeginRead(void) {}
void I_EndRead(void) {}

byte *I_AllocLow(int length) {
  byte *mem = malloc(length);
  memset(mem, 0, length);
  return mem;
}

//
// I_Error
//
extern boolean demorecording;

void I_Error(char *error, ...) {
  va_list	argptr;

  // Message first.
  va_start(argptr,error);
  fprintf(stderr, "Error: ");
  vfprintf(stderr,error,argptr);
  fprintf(stderr, "\n");
  va_end(argptr);

  // Shutdown. Here might be other errors.
  if(demorecording)
    G_CheckDemoStatus();

  D_QuitNetGame();
  I_ShutdownGraphics();

  exit(-1);
}
