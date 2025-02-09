/***
 *
 *    Copyright (c) 2003', Valve LLC. All rights reserved.
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

inline hud_player_info_t g_PlayerInfoList[MAX_PLAYERS_HUD + 1];       // player info from the engine
inline extra_player_info_t g_PlayerExtraInfo[MAX_PLAYERS_HUD + 1]; // additional player info sent directly to the client dll
inline team_info_t g_TeamInfo[MAX_TEAMS + 1];
inline int g_IsSpectator[MAX_PLAYERS_HUD + 1];
