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
// DESCRIPTION:
//	WAD I/O functions.
//
//-----------------------------------------------------------------------------

/*
Modified by QOOM! 16 apr 23
*/

#ifndef __W_WAD__
#define __W_WAD__

#include <stdio.h>

//
// TYPES
//
typedef struct {
  // Should be "IWAD" or "PWAD".
  char identification[4];
  int numlumps, infotableofs;
} wadinfo_t;

typedef struct {
  int filepos, size;
  char name[8];
} filelump_t;

//
// WADFILE I/O related stuff.
//
typedef struct {
  char name[8];
  FILE *handle;
  int position, size;
} lumpinfo_t;


extern void **lumpcache;
extern lumpinfo_t *lumpinfo;
extern int numlumps;

void W_InitMultipleFiles(char** filenames);
void W_Reload(void);

int W_CheckNumForName(char* name);
int W_GetNumForName(char* name);

int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest);

void *W_CacheLumpNum(int lump, int tag);
void *W_CacheLumpName(char* name, int tag);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
