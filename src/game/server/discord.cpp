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

void CDiscordServer :: SendPresence()
{
	MESSAGE_BEGIN( MSG_ALL, gmsgDiscordRPC, nullptr, nullptr );
		WRITE_STRING( cl_discord_rpc_image.string );
		WRITE_STRING( cl_discord_rpc_header.string );
		WRITE_STRING( cl_discord_rpc_description.string );
	MESSAGE_END();
}