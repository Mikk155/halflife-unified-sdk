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

#include "discord.h"
#include "cbase.h"
#include "game.h"
#include "UserMessages.h"
#include "util.h"
#include "CGameRules.h"

void CDiscordServer :: SendPresence()
{
	if( gpGlobals->time < m_flNextThink )
		return;

	MESSAGE_BEGIN( MSG_ALL, gmsgDiscordRPC, nullptr, nullptr );
		WRITE_STRING( cl_discord_rpc_image.string );
		WRITE_STRING( cl_discord_rpc_header.string );
		WRITE_STRING( cl_discord_rpc_description.string );
	MESSAGE_END();

	m_flNextThink = gpGlobals->time + 5.0f;
}

void CDiscordServer :: ClearPresence()
{
	CVAR_SET_STRING( "cl_discord_rpc_image", "" );
	CVAR_SET_STRING( "cl_discord_rpc_header", "" );
	CVAR_SET_STRING( "cl_discord_rpc_description", "" );
}

void CDiscordServer :: UpdatePresence()
{
	const char* s =
		( g_pGameRules->IsCoOp() ? "Co-Operative"
		: g_pGameRules->IsTeamplay() ? "Team Play"
		: g_pGameRules->IsCTF() ? "Capture The Flag"
		: g_pGameRules->IsDeathmatch() ? "Death Match"
		: "Single Player"
	);

	char szBuffer[128];
	sprintf( szBuffer, "Playing %s", s );
	CVAR_SET_STRING( "cl_discord_rpc_header", szBuffer );
	CVAR_SET_STRING( "cl_discord_rpc_description",
		( g_pGameRules->IsMultiplayer()
		? CVAR_GET_STRING( "hostname" )
		: STRING( gpGlobals->mapname ) )
	);
}