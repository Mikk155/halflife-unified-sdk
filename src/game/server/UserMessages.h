/***
 *
 *    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#pragma once

inline int gmsgShake = 0;
inline int gmsgFade = 0;
inline int gmsgFlashlight = 0;
inline int gmsgFlashBattery = 0;
inline int gmsgResetHUD = 0;
inline int gmsgInitHUD = 0;
inline int gmsgShowGameTitle = 0;
inline int gmsgCurWeapon = 0;
inline int gmsgHealth = 0;
inline int gmsgDamage = 0;
inline int gmsgBattery = 0;
inline int gmsgTrain = 0;
inline int gmsgLogo = 0;
inline int gmsgAmmoX = 0;
inline int gmsgHudText = 0;
inline int gmsgDeathMsg = 0;
inline int gmsgScoreInfo = 0;
inline int gmsgTeamInfo = 0;
inline int gmsgTeamScore = 0;
inline int gmsgMOTD = 0;
inline int gmsgServerName = 0;
inline int gmsgAmmoPickup = 0;
inline int gmsgWeapPickup = 0;
inline int gmsgItemPickup = 0;
inline int gmsgHideWeapon = 0;
inline int gmsgSetCurWeap = 0;
inline int gmsgSayText = 0;
inline int gmsgTextMsg = 0;
inline int gmsgSetFOV = 0;
inline int gmsgShowMenu = 0;
inline int gmsgGeigerRange = 0;
inline int gmsgTeamNames = 0;
inline int gmsgStatusText = 0;
inline int gmsgStatusValue = 0;
inline int gmsgSpectator = 0;
inline int gmsgStatusIcon = 0;
inline int gmsgPlayerBrowse = 0;
inline int gmsgFlagIcon = 0;
inline int gmsgFlagTimer = 0;
inline int gmsgPlayerIcon = 0;
inline int gmsgVGUIMenu = 0;
inline int gmsgAllowSpec = 0;
inline int gmsgSetMenuTeam = 0;
inline int gmsgCTFScore = 0;
inline int gmsgStatsInfo = 0;
inline int gmsgStatsPlayer = 0;
inline int gmsgTeamFull = 0;
inline int gmsgCustomIcon = 0;
inline int gmsgWeapons = 0;
inline int gmsgEntityInfo = 0;
inline int gmsgEmitSound = 0;
inline int gmsgTempEntity = 0;
inline int gmsgConfigVars = 0;
inline int gmsgTgtLaser = 0;
inline int gmsgFog = 0;
inline int gmsgClientGibs = 0;
inline int gmsgGameMode = 0;
inline int gmsgSendSteamID = 0;

#include "dlight.h"
inline int gmsgDLight = 0;

void LinkUserMessages();
