//-----------------------------------------------------------------------------
// Mailsrvd
//  (c) Copyright Hyper-Active Systems, 2006.
//
// Hyper-Active Systems
// ABN: 98 400 498 123
// 66 Brown Cr
// Seville Grove, WA, 6112
// Australia
// Phone: +61 434 895 695
//        0434 895 695
// contact@hyper-active.com.au
// 
//-----------------------------------------------------------------------------
// Defaults.h
//
//  Default constants and such are included in this file.  The problem with 
//  this is that it couples the modules together more tightly (not a good thing 
//  if decoupling is your goal), but it does make it easier to find all the 
//  different constants that affect the program.  Since the different objects 
//  would not be very useful apart from this project, then we will sacrifice 
//  pure-ness and go for easy-ness.
//
//-----------------------------------------------------------------------------

#ifndef __DEFAULTS_H
#define __DEFAULTS_H



#define MAX_POP_ITEMS 50
#define IDLE_LIMIT	1000

#define MAX_LOCAL_ADDRESSES		1000
#define MAX_REMOTE_ADDRESSES	100

#define BUSY_SOCKET_TRIES		15
#define BUSY_SOCKET_PAUSE		30


#endif
