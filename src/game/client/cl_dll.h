/***
 *
 *    Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *    This product contains software technology licensed from Id
 *    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *    All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
//
//  cl_dll.h
//

#pragma once

// 4-23-98  JOHN

//
//  This DLL is linked by the client when they first initialize.
// This DLL is responsible for the following tasks:
//        - Loading the HUD graphics upon initialization
//        - Drawing the HUD graphics every frame
//        - Handling the custum HUD-update packets
//

#include "Platform.h"

typedef int ( *pfnUserMsgHook )( const char* pszName, int iSize, void* pbuf );

#include "mathlib.h"

#include "../engine/cdll_int.h"
#include "cdll_dll.h"

inline cl_enginefunc_t gEngfuncs;
