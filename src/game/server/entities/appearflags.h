/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "cbase.h"

#pragma once

enum appearflags : int
{
	NOT_IN = -1,
	DEFAULT = 0,
	ONLY_IN = 1,
	SKILL_EASY = 1,
	SKILL_MEDIUM = 2,
	SKILL_HARD = 3,
	GM_DEATHMATCH = 4,
	GM_COOPERATIVE = 5,
	GM_CAPTURETHEFLAG = 6,
	GM_TEAMPLAY = 7,
	GM_MULTIPLAYER = 8,
	GM_SINGLEPLAYER = 9,
	SV_DEDICATED = 10
};

bool ShouldAppear( CBaseEntity* pEntity );