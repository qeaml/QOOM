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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <SDL2/SDL_net.h>

#include <errno.h>

#include "qoom/i/system.h"
#include "qoom/d/event.h"
#include "qoom/d/net.h"
#include "qoom/m/argv.h"

#include "qoom/doomstat.h"

#include "qoom/i/net.h"

// For some odd reason...
#define ntohl(x) \
  ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                       (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                       (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                       (((unsigned long int)(x) & 0xff000000U) >> 24)))

#define ntohs(x) \
  ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                        (((unsigned short int)(x) & 0xff00) >> 8))) \

#define htonl(x) ntohl(x)
#define htons(x) ntohs(x)

void NetSend(void);
boolean NetListen(void);

//
// NETWORKING
//

int DOOMPORT = (5000 + 0x1d );

UDPsocket sendsocket = NULL, insocket = NULL;
IPaddress sendaddress[MAXNETNODES];

void (*netget)(void);
void (*netsend)(void);

//
// UDPsocket
//
UDPsocket I_UDPsocket(void) {
  UDPsocket s = SDLNet_UDP_Open(0);
  if(s == NULL)
    I_Error("can't create socket: %s", SDLNet_GetError());

  return s;
}

int sendchannel;

//
// BindToLocalPort
//
void BindToLocalPort(UDPsocket s, int port) {
  IPaddress address = {INADDR_ANY, port};
  sendchannel = SDLNet_UDP_Bind(s, -1, &address);
  if(sendchannel == -1)
    I_Error("BindToPort: bind: %s", SDLNet_GetError());
}

UDPpacket *sendpacket;

//
// PacketSend
//
void PacketSend(void) {
  doomdata_t sw = {
    htonl(netbuffer->checksum),
    netbuffer->retransmitfrom,
    netbuffer->starttic,
    netbuffer->player,
    netbuffer->numtics,
    {0}
  };

  for(int c = 0; c < netbuffer->numtics; c++) {
    sw.cmds[c].forwardmove = netbuffer->cmds[c].forwardmove;
    sw.cmds[c].sidemove = netbuffer->cmds[c].sidemove;
    sw.cmds[c].angleturn = htons(netbuffer->cmds[c].angleturn);
    sw.cmds[c].consistancy = htons(netbuffer->cmds[c].consistancy);
    sw.cmds[c].chatchar = netbuffer->cmds[c].chatchar;
    sw.cmds[c].buttons = netbuffer->cmds[c].buttons;
  }

  // sendpacket = SDLNet_AllocPacket(sizeof(sw));
  // if(sendpacket == NULL)
  //   I_Error("can't allocate UDP packet: %s", SDLNet_GetError());

  sendpacket->len = doomcom->datalength;
  sendpacket->address = sendaddress[doomcom->remotenode];
  memcpy(sendpacket->data, &sw, doomcom->datalength);

  SDLNet_UDP_Send(sendsocket, -1, sendpacket);
}

UDPpacket inpacket;

//
// PacketGet
//
void PacketGet(void) {
  int res = SDLNet_UDP_Recv(insocket, &inpacket);
  if(res == -1)
    I_Error("GetPacket: %s", SDLNet_GetError());

  if(res == 0) {
    doomcom->remotenode = -1; // no packet
    return;
  }

  doomdata_t sw;
  memcpy(&sw, inpacket.data, inpacket.len);

  static int first = 1;
  if(first) {
    printf("len=%d:p=[0x%x 0x%x] \n", res, *(int*)&sw, *((int*)&sw+1));
	  first = 0;
  }

  // find remote node number
  int remoteno = 0;
  for(; remoteno < doomcom->numnodes; remoteno++)
    if(inpacket.address.host == sendaddress[remoteno].host)
      break;

  if(remoteno == doomcom->numnodes) {
    // packet is not from one of the players (new game broadcast)
    doomcom->remotenode = -1; // no packet
    return;
  }

  doomcom->remotenode = remoteno; // good packet from a game player
  doomcom->datalength = inpacket.len;

  // byte swap
  netbuffer->checksum = ntohl(sw.checksum);
  netbuffer->player = sw.player;
  netbuffer->retransmitfrom = sw.retransmitfrom;
  netbuffer->starttic = sw.starttic;
  netbuffer->numtics = sw.numtics;

  for(int c = 0; c < netbuffer->numtics; c++) {
    netbuffer->cmds[c].forwardmove = sw.cmds[c].forwardmove;
    netbuffer->cmds[c].sidemove = sw.cmds[c].sidemove;
    netbuffer->cmds[c].angleturn = ntohs(sw.cmds[c].angleturn);
    netbuffer->cmds[c].consistancy = ntohs(sw.cmds[c].consistancy);
    netbuffer->cmds[c].chatchar = sw.cmds[c].chatchar;
    netbuffer->cmds[c].buttons = sw.cmds[c].buttons;
  }
}

IPaddress GetLocalAddress(void) {
  IPaddress localaddr;
  if(SDLNet_GetLocalAddresses(&localaddr, 1) == 0)
    I_Error("GetLocalAddress : %s", SDLNet_GetError());

  return localaddr;
}

//
// I_InitNetwork
//
void I_InitNetwork(void) {
  if(SDLNet_Init() != 0)
    I_Error("can't initialize network: %s", SDLNet_GetError());

  doomcom = malloc(sizeof(doomcom_t));
  memset(doomcom, 0, sizeof(doomcom_t));

  // set up for network
  doomcom->ticdup = 1;
  int i = M_CheckParm("-dup");
  if(i && i < myargc - 1) {
    doomcom->ticdup = myargv[i+1][0] - '0';
    if(doomcom->ticdup < 1)
      doomcom->ticdup = 1;
    if(doomcom->ticdup > 9)
      doomcom->ticdup = 9;
  }

  doomcom->extratics = M_CheckParm("-extratic") != 0;

  i = M_CheckParm("-port");
  if(i && i < myargc - 1) {
    DOOMPORT = atoi(myargv[i+1]);
    printf("using alternative port %i\n", DOOMPORT);
  }

  // parse network game options,
  //  -net <consoleplayer> <host> <host> ...
  i = M_CheckParm("-net");
  if(!i) {
    // single player game
    netgame = false;
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes = 1;
    doomcom->deathmatch = false;
    doomcom->consoleplayer = 0;
    return;
  }

  netsend = PacketSend;
  netget = PacketGet;
  netgame = true;

  // parse player number and host list
  doomcom->consoleplayer = myargv[i+1][0]-'1';

  doomcom->numnodes = 1; // this node for sure

  i++;
  while(++i < myargc && myargv[i][0] != '-') {
    int res = SDLNet_ResolveHost(&sendaddress[doomcom->numnodes],
      myargv[i]+1, htons(DOOMPORT));
    if(res != 0)
      I_Error("couldn't find %s", myargv[i]);

    doomcom->numnodes++;
  }

  doomcom->id = DOOMCOM_ID;
  doomcom->numplayers = doomcom->numnodes;

  insocket = I_UDPsocket();
  BindToLocalPort(insocket, htons(DOOMPORT));

  sendsocket = I_UDPsocket();
  sendpacket = SDLNet_AllocPacket(sizeof(doomdata_t));
}

void I_NetCmd(void) {
  if(doomcom->command == CMD_SEND)
    netsend();
  else if(doomcom->command == CMD_GET)
    netget();
  else
    I_Error("Bad net cmd: %i\n", doomcom->command);
}

void I_QuitNetwork() {
  if(sendpacket != NULL)
    SDLNet_FreePacket(sendpacket);

  if(sendsocket != NULL)
    SDLNet_UDP_Close(sendsocket);

  if(insocket != NULL)
    SDLNet_UDP_Close(insocket);

  SDLNet_Quit();
}
