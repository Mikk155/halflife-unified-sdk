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
#include "world.h"

#include <fmt/format.h>

void CDiscordServer :: SendMessage( const char* szCvar, int i )
{
    char str[61];

	std::string szStr = g_DiscordServer.format( szCvar );

    std::strcpy( str, szStr.c_str() );

	MESSAGE_BEGIN( MSG_ALL, gmsgDiscordRPC );
		WRITE_STRING( str );
		WRITE_BYTE( i );
	MESSAGE_END();
}

void CDiscordServer :: SendPresence()
{
	if( m_flNextThink < gpGlobals->time )
	{
		SendMessage( "discord_rpc_header", 1 );
		SendMessage( "discord_rpc_description", 0 );
		m_flNextThink = gpGlobals->time + 2.0f;
	}
}

std::string CDiscordServer :: format( const char* szCvar )
{
	std::string szStr = std::string( CVAR_GET_STRING( szCvar ) );

	if( !szStr.empty() )
	{
		if( szStr.find( "$players$" ) != std::string::npos )
		{
			int iPlayers = 0;

			for( auto player : UTIL_FindPlayers() )
			{
				if( player && player->IsPlayer() && player->IsConnected() )
				{
					iPlayers++;
				}
			}

			szStr.replace( szStr.find("$players$"), std::string("$players$").size(), fmt::format( "{} of {}", iPlayers, gpGlobals->maxClients ) );
		}

		if( szStr.find( "$gamemode$") != std::string::npos )
		{
			std::string szRep =
			std::string(
				( g_pGameRules->IsCoOp() ? "Co-Operative"
				: g_pGameRules->IsTeamplay() ? "Team Play"
				: g_pGameRules->IsCTF() ? "Capture The Flag"
				: g_pGameRules->IsDeathmatch() ? "Death Match"
				: "Single Player"
			));

			szStr.replace( szStr.find("$gamemode$"), std::string("$gamemode$").size(), std::string(szRep) );
		}

		if( szStr.find( "$server$") != std::string::npos )
		{
			szStr.replace( szStr.find("$server$"), std::string("$server$").size(), fmt::format( "{}", CVAR_GET_STRING( "hostname" ) ) );
		}

		if( szStr.find( "$map$") != std::string::npos )
		{
			CBaseEntity* pWorldspawn = CBaseEntity::Instance( INDEXENT(0) );
			string_t MapName = ( pWorldspawn && !FStringNull( pWorldspawn->pev->message ) ? pWorldspawn->pev->message : gpGlobals->mapname );
			szStr.replace( szStr.find("$map$"), std::string("$map$").size(), fmt::format( STRING( MapName ) ) );
		}
	}
	return fmt::format( "{:<60}", szStr );
}

void CDiscordServer :: UpdatePresence()
{
	CVAR_SET_STRING( "discord_rpc_header", "Playing $gamemode$ $players$ in $map$" );
	CVAR_SET_STRING( "discord_rpc_description", ( g_pGameRules->IsMultiplayer() ? "In server $server$" : "In $map$" ) );
}