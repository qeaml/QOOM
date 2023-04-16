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
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------

/*
Modified by QOOM! 16 apr 23
*/

#include "qoom/doomtype.h"
#include "qoom/m/swap.h"
#include "qoom/i/system.h"
#include "qoom/z_zone.h"
#include "qoom/w_wad.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//
// GLOBALS
//

lumpinfo_t *lumpinfo; // Location of each lump on disk
int numlumps;
void **lumpcache;

int filelength(FILE *handle) {
  int old = ftell(handle);

  if(fseek(handle, 0, SEEK_END))
    I_Error("Error seeking");
  int size = ftell(handle);

  if(fseek(handle, old, SEEK_SET))
    I_Error("Error seeking");

  return size;
}

void ExtractFileBase(char *path, char *dest) {
  char *src = path + strlen(path) - 1;

  // back up until a \ or the start
  while(src != path && *(src-1) != '\\' && *(src-1) != '/')
    src--;

  // copy up to eight characters
  memset(dest,0,8);
  int length = 0;

  while(*src && *src != '.') {
    if(++length == 9)
      I_Error ("Filename base of %s >8 chars",path);

    *dest++ = toupper((int)*src++);
  }
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

int reloadlump;
char *reloadname;

void W_AddFile(char *filename) {
  // open the file and add to directory

  // handle reload indicator.
  if(filename[0] == '~') {
    filename++;
    reloadname = filename;
    reloadlump = numlumps;
  }

  FILE *handle;
  if((handle = fopen(filename, "rb")) == NULL) {
    printf(" counldn't open %s\n", filename);
    return;
  }

  printf(" adding %s\n", filename);
  int startlump = numlumps;

  filelump_t *fileinfo;
  filelump_t singleinfo;
  if(_strcmpi(filename+strlen(filename)-3, "wad")) {
    // single lump file
    fileinfo = &singleinfo;
    singleinfo.filepos = 0;
    singleinfo.size = LONG(filelength(handle));
    ExtractFileBase(filename, singleinfo.name);
    numlumps++;
  } else {
    // WAD file
    wadinfo_t header;
    fread(&header, sizeof(header), 1, handle);
    if(strncmp(header.identification, "IWAD", 4)) {
      // Homebrew levels?
      if(strncmp(header.identification, "PWAD", 4)) {
        I_Error("Wad file %s doesn't have IWAD or PWAD id\n", filename);
      }
      // ???modifiedgame = true;
    }
    header.numlumps = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    int length = header.numlumps * sizeof(filelump_t);
    fileinfo = _alloca(length);
    fseek(handle, header.infotableofs, SEEK_SET);
    fread(fileinfo, length, 1, handle);
    numlumps += header.numlumps;
  }

  // Fill in lumpinfo
  lumpinfo = realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));
  if(!lumpinfo)
    I_Error("Couldn't realloc lumpinfo");

  lumpinfo_t *lump_p = &lumpinfo[startlump];
  FILE *storehandle = reloadname ? NULL : handle;

  for(unsigned i = startlump; i < numlumps; i++, lump_p++, fileinfo++) {
    lump_p->handle = storehandle;
    lump_p->position = LONG(fileinfo->filepos);
    lump_p->size = LONG(fileinfo->size);
    strncpy(lump_p->name, fileinfo->name, 8);
  }

  if(reloadname)
    fclose(handle);
}

//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload(void) {

  if(!reloadname)
    return;

  FILE *handle;
  if((handle = fopen(reloadname, "rb")) == NULL)
    I_Error("W_Reload: couldn't open %s", reloadname);

  wadinfo_t header;
  fread(&header, sizeof(header), 1, handle);

  int lumpcount = LONG(header.numlumps);
  header.infotableofs = LONG(header.infotableofs);
  int length = lumpcount * sizeof(filelump_t);

  filelump_t *fileinfo;
  fseek(handle, header.infotableofs, SEEK_SET);
  fread(fileinfo, length, 1, handle);

  // Fill in lumpinfo
  lumpinfo_t *lump_p = &lumpinfo[reloadlump];

  for(unsigned i = reloadlump; i < reloadlump+lumpcount; i++, lump_p++, fileinfo++) {
    if(lumpcache[i])
      Z_Free(lumpcache[i]);

    lump_p->position = LONG(fileinfo->filepos);
    lump_p->size = LONG(fileinfo->size);
  }

  fclose(handle);
}

//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
void W_InitMultipleFiles(char** filenames) {
  int size;

  // open all the files, load headers, and count lumps
  numlumps = 0;

  // will be realloced as lumps are added
  lumpinfo = malloc(1);

  for(; *filenames; filenames++)
    W_AddFile(*filenames);

  if(!numlumps)
    I_Error("W_InitFiles: no files found");

  // set up caching
  size = numlumps * sizeof(*lumpcache);
  lumpcache = malloc(size);

  if(!lumpcache)
    I_Error ("Couldn't allocate lumpcache");

  memset(lumpcache,0, size);
}

//
// W_InitFile
// Just initialize from a single file.
//
void W_InitFile(char* filename) {
  char *names[2] = {filename, NULL};
  W_InitMultipleFiles(names);
}

//
// W_NumLumps
//
int W_NumLumps(void) {
  return numlumps;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//
int W_CheckNumForName(char* name) {
  union {
    char s[9];
    int x[2];
  } name8;
  // make the name into two integers for easy compares
  strncpy(name8.s, name, 8);

  // in case the name was a full 8 chars
  name8.s[8] = 0;

  // case insensitive
  _strupr(name8.s);

  int v1 = name8.x[0],
      v2 = name8.x[1];

  // scan backwards so patch lump files take precedence
  lumpinfo_t *lump_p = lumpinfo + numlumps;

  while(lump_p-- != lumpinfo) {
    if(*(int*)&lump_p->name == v1
    &&(*(int*)&lump_p->name[4] == v2)) {
      return lump_p - lumpinfo;
    }
  }

  // TFB. Not found.
  return -1;
}

//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(char* name) {
  int i = W_CheckNumForName (name);

  if (i == -1)
    I_Error("W_GetNumForName: %s not found!", name);

  return i;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(int lump) {
  if (lump >= numlumps)
    I_Error ("W_LumpLength: %i >= numlumps",lump);

  return lumpinfo[lump].size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(int lump, void *dest) {

  if(lump >= numlumps)
    I_Error("W_ReadLump: %i >= numlumps",lump);

  lumpinfo_t *l = lumpinfo+lump;

  // ??? I_BeginRead ();

  FILE *handle;
  if(l->handle == NULL) {
    // reloadable file, so use open / read / close
    if((handle = fopen(reloadname, "rb")) == NULL)
      I_Error ("W_ReadLump: couldn't open %s",reloadname);
  } else {
    handle = l->handle;
  }

  fseek(handle, l->position, SEEK_SET);
  int c = fread(dest, l->size, 1, handle);

  if(c < l->size)
    I_Error("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);

  if(l->handle == NULL)
    fclose(handle);

  // ??? I_EndRead ();
}

//
// W_CacheLumpNum
//
void *W_CacheLumpNum(int lump, int tag) {
  if(lump >= numlumps)
    I_Error ("W_CacheLumpNum: %i >= numlumps",lump);

  if(!lumpcache[lump]) {
    // read the lump in

    //printf ("cache miss on lump %i\n",lump);
    byte *ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
    W_ReadLump (lump, lumpcache[lump]);
  } else {
    //printf ("cache hit on lump %i\n",lump);
    Z_ChangeTag (lumpcache[lump],tag);
  }

  return lumpcache[lump];
}

//
// W_CacheLumpName
//
void *W_CacheLumpName(char* name, int tag) {
  return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//
// W_Profile
//
int info[2500][10];
int profilecount;

void W_Profile(void) {
  FILE *f;
  char name[9];

  char ch;
  for(int i = 0; i < numlumps; i++) {
    void *ptr = lumpcache[i];
    if(!ptr) {
      ch = ' ';
      continue;
    } else {
      memblock_t *block = (memblock_t*)((byte*)ptr - sizeof(memblock_t));
      if(block->tag < PU_PURGELEVEL)
        ch = 'S';
      else
        ch = 'P';
    }
    info[i][profilecount] = ch;
  }
  profilecount++;

  f = fopen ("waddump.txt","w");
  name[8] = 0;

  for(int i = 0; i < numlumps; i++) {
    memcpy(name, lumpinfo[i].name, 8);

    int j;
    for(j = 0 ; j < 8; j++)
      if(!name[j]) break;

    for(; j < 8; j++)
      name[j] = ' ';

    fprintf(f, "%s ", name);

    for (j=0 ; j<profilecount ; j++)
      fprintf (f,"    %c",info[i][j]);

    fprintf(f, "\n");
  }

  fclose(f);
}
