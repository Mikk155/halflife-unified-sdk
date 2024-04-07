/***
 *
 *	Copyright (c) 1999, Valve LLC. All rights reserved.
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

#include "CDiscord.h"

static DiscordRichPresence discordPresence;
extern cl_enginefunc_t gEngfuncs;

// Blank handlers; not required for singleplayer Half-Life
static void HandleDiscordReady(const DiscordUser* connectedUser) {}
static void HandleDiscordDisconnected(int errcode, const char* message) {}
static void HandleDiscordError(int errcode, const char* message) {}
static void HandleDiscordJoin(const char* secret) {}
static void HandleDiscordSpectate(const char* secret) {}
static void HandleDiscordJoinRequest(const DiscordUser* request) {}

void CDiscord :: RPCStartUp()
{
	int64_t startTime = time(0);

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = HandleDiscordReady;
	handlers.disconnected = HandleDiscordDisconnected;
	handlers.errored = HandleDiscordError;
	handlers.joinGame = HandleDiscordJoin;
	handlers.spectateGame = HandleDiscordSpectate;
	handlers.joinRequest = HandleDiscordJoinRequest;

	Discord_Initialize("1176277342751031336", &handlers, 1, 0);

	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.startTimestamp = startTime;
	discordPresence.largeImageKey = m_szDefaultLogo;
	Discord_UpdatePresence(&discordPresence);
}

void CDiscord :: RPCStateUpdate()
{
	if( !g_Discord.m_szHeader || g_Discord.m_szHeader[0] == '\0' || !gEngfuncs.GetEntityByIndex(0) )
	{
		g_Discord.m_szHeader = "In main menu";
	}

	if( FStrEq( g_Discord.m_szHeader, "In main menu" )
	&& !FStrEq( g_Discord.m_szDescription, "Just started playing." )
	&& !gEngfuncs.GetEntityByIndex(0)
	// NO FUCKING IDEA WHY THEY'RE BOTH EQUALS BUT THEY ARE.
	|| FStrEq( g_Discord.m_szHeader, g_Discord.m_szDescription )
	) {
		g_Discord.m_szDescription = "";
	}

	discordPresence.state = g_Discord.m_szDescription;
	discordPresence.details = g_Discord.m_szHeader;
	Discord_UpdatePresence(&discordPresence);
}

void CDiscord :: RPCShutDown()
{
	Discord_Shutdown();
}