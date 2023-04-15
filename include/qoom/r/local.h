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
//	Refresh (R_*) module, global header.
//	All the rendering/drawing stuff is here.
//
//-----------------------------------------------------------------------------

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
#include "qoom/tables.h"

// Screen size related parameters.
#include "qoom/doomdef.h"

// Include the refresh/render data structs.
#include "qoom/r/data.h"



//
// Separate header file for each module.
//
#include "qoom/r/main.h"
#include "qoom/r/bsp.h"
#include "qoom/r/segs.h"
#include "qoom/r/plane.h"
#include "qoom/r/data.h"
#include "qoom/r/things.h"
#include "qoom/r/draw.h"

#endif		// __R_LOCAL__
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
