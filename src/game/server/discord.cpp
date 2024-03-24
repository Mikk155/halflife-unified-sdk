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

	MESSAGE_BEGIN( MSG_ALL, gmsgDiscordRPC );
		WRITE_STRING( GetArguments( CVAR_GET_STRING( "discord_rpc_header" ) ) );
		WRITE_STRING( GetArguments( CVAR_GET_STRING( "discord_rpc_description" ) ) );
	MESSAGE_END();

	m_flNextThink = gpGlobals->time + 2.0f;
}

const char* CDiscordServer :: GetArguments( const char* szCommand )
{
	if( !szCommand )
		return "";

	std::string szString = std::string( szCommand );

	CBaseEntity::Logger->error( "szCommand {}", szCommand );

	if( szString.find( "$players$") != std::string::npos )
	{
		int iplayer = 0;

		for( int i = 0; i <= gpGlobals->maxClients; i++ )
		{
			if( auto player = UTIL_PlayerByIndex( i ); player && player->IsConnected() )
				iplayer++;
		}
		std::string szRep = std::to_string( iplayer ) + " of " + std::to_string( gpGlobals->maxClients );
		szString.replace( szString.find("$players$"), std::string("$players$").size(), szRep );
	}

	if( szString.find( "$gamemode$") != std::string::npos )
	{
		std::string szRep =
		std::string(
			( g_pGameRules->IsCoOp() ? "Co-Operative"
			: g_pGameRules->IsTeamplay() ? "Team Play"
			: g_pGameRules->IsCTF() ? "Capture The Flag"
			: g_pGameRules->IsDeathmatch() ? "Death Match"
			: "Single Player"
		));
		szString.replace( szString.find("$gamemode$"), std::string("$gamemode$").size(), std::string(szRep) );
	}

	if( szString.find( "$server$") != std::string::npos )
	{
		szString.replace( szString.find("$server$"), std::string("$server$").size(), std::string(CVAR_GET_STRING( "hostname" )) );
	}

	if( szString.find( "$map$") != std::string::npos )
	{
		szString.replace( szString.find("$map$"), std::string("$map$").size(), std::string(STRING(gpGlobals->mapname)) );
	}
	szCommand = szString.c_str();
	CBaseEntity::Logger->error( "szString {}", szCommand );

	return szCommand;
}

void CDiscordServer :: UpdatePresence()
{
	CVAR_SET_STRING( "discord_rpc_header", "Playing $gamemode$ $players$" );
	CVAR_SET_STRING( "discord_rpc_description", ( g_pGameRules->IsMultiplayer() ? "In server $server$" : "In map $map$" ) );
}