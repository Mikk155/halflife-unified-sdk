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
// pm_shared.h
//

#pragma once

#include "Platform.h"
#include "sound/MaterialSystem.h"

struct playermove_t;

void PM_Init( playermove_t* ppmove );
void PM_Move( playermove_t* ppmove, qboolean server );

/**
 *    @brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
 */
bool PM_GetHullBounds( int hullnumber, float* mins, float* maxs );

int PM_GetVisEntInfo( int ent );
int PM_GetPhysEntInfo( int ent );

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
#define OBS_NONE 0
#define OBS_CHASE_LOCKED 1
#define OBS_CHASE_FREE 2
#define OBS_ROAMING 3
#define OBS_IN_EYE 4
#define OBS_MAP_FREE 5
#define OBS_MAP_CHASE 6

extern playermove_t* pmove;

inline bool g_CheckForPlayerStuck = false;
