/***
 *
 *  Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *  This product contains software technology licensed from Id
 *  Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *  All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

#define FCVAR_ARCHIVE (1 << 0)          // set to cause it to be saved to vars.rc
#define FCVAR_USERINFO (1 << 1)         // changes the client's info string
#define FCVAR_SERVER (1 << 2)           // notifies players when changed
#define FCVAR_EXTDLL (1 << 3)           // defined by external DLL
#define FCVAR_CLIENTDLL (1 << 4)        // defined by the client dll
#define FCVAR_PROTECTED (1 << 5)        // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY (1 << 6)           // This cvar cannot be changed by clients connected to a multiplayer server.
#define FCVAR_PRINTABLEONLY (1 << 7)    // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
#define FCVAR_UNLOGGED (1 << 8)         // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_NOEXTRAWHITEPACE (1 << 9) // strip trailing/leading white space from this cvar
#define FCVAR_PRIVILEGED (1 << 10)      // only available in privileged mode
#define FCVAR_FILTERABLE (1 << 11)      // filtered in unprivileged mode if cl_filterstuffcmd is 1
#define FCVAR_ISEXECUTED (1 << 12)      // This cvar's string contains a value that will be executed as a cfg file; don't allow commands to be appended to it
#define FCVAR_ISPATH (1 << 13)          // This cvar's string is a path or filename; don't allow absolute paths, escaping to another directory or backslashes

struct cvar_t
{
    const char* name;
    // Technically this should be non-const but that only matters to engine code
    const char* string;
    int flags;
    float value;
    cvar_t* next;
};
