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

#pragma once

#include "hud.h"
#include <string.h>
#include "discord_rpc.h"
#include <time.h>

/*
-TODO

    - Delayed loading of libraries and move them to cl_dlls/

    - Cvar for server operators for a discord webhook so anything that happens in the server chat is sent to that channel
    - If possible, do the opposite direction as well for the chat bridge.

    - Create Usermessages so the server can send their custom rpc by mappers

    - Add buttons, so people can "Join server" or "Download map"
*/

/**
 *	@brief Discord
 */
class CDiscord final
{
public:
    void RPCStartUp();
    void RPCStateUpdate();
    void RPCShutDown();
    const char* m_szDefaultLogo = "logo";
    const char* m_szImage;
    const char* m_szHeader;
    const char* m_szDescription;
};

inline CDiscord g_Discord;