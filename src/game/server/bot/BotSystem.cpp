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

#include "cbase.h"
#include "BotSystem.h"
#include "client.h"

bool BotSystem::Initialize()
{
    m_Logger = g_Logging.CreateLogger( "bot" );

    g_ConCommands.CreateCommand( "addbot", [this]( const auto& args )
        {
            if( args.Count() != 2 )
            {
                Con_Printf( "Usage: %s <bot_name>\n", args.Argument( 0 ) );
                return;
            }

            const char* name = args.Argument( 1 );

            AddBot( name ); } );

    return true;
}

void BotSystem::Shutdown()
{
    g_Logging.RemoveLogger( m_Logger );
    m_Logger.reset();
}

void BotSystem::RunFrame()
{
    // Handle level changes and other problematic time changes.
    float frametime = gpGlobals->time - m_LastUpdateTime;

    if( frametime > 0.25f || frametime < 0 )
    {
        frametime = 0;
    }

    const byte msec = byte( frametime * 1000 );

    m_LastUpdateTime = gpGlobals->time;

    for( auto player : UTIL_FindPlayers() )
    {
        if( !player->IsConnected() || !player->IsBot() )
        {
            continue;
        }

        // If bot is newly created finish setup here.

        // Run bot think here.

        int buttons = 0;

        // Hack: detect when dead to force respawn.
        if( player->pev->deadflag == DEAD_RESPAWNABLE )
        {
            buttons |= IN_ATTACK;
        }

        player->pev->button = buttons;

        // Now update the bot.
        g_engfuncs.pfnRunPlayerMove( player->edict(), player->pev->angles, 0, 0, 0, player->pev->button, player->pev->impulse, msec );
    }
}

void BotSystem::AddBot( const char* name )
{
    // The engine will validate the name and change it to "unnamed" if it's not valid.
    // Any duplicates will be disambiguated by prepending a (<number>).
    const auto fakeClient = g_engfuncs.pfnCreateFakeClient( name );

    if( !fakeClient )
    {
        return;
    }

    // Use the netname variable here in case the engine changed it for some reason (usually duplicate names).
    // Use localhost as the IP address.
    char reject[128];
    if( 0 == ClientConnect( fakeClient, STRING( fakeClient->v.netname ), "127.0.0.1", reject ) )
    {
        // Bot was refused connection, kick it from the server to free the slot.
        SERVER_COMMAND( UTIL_VarArgs( "kick %s\n", STRING( fakeClient->v.netname ) ) );
        return;
    }

    // Finish connecting, create player.
    ClientPutInServer( fakeClient );

    auto bot = GET_PRIVATE<CBasePlayer>( fakeClient );

    auto infobuffer = g_engfuncs.pfnGetInfoKeyBuffer( fakeClient );

    // Pick a random color to distinguish between bots.
    g_engfuncs.pfnSetClientKeyValue( bot->entindex(), infobuffer, "topcolor", std::to_string( RANDOM_LONG( 0, 255 ) ).c_str() );
    g_engfuncs.pfnSetClientKeyValue( bot->entindex(), infobuffer, "bottomcolor", std::to_string( RANDOM_LONG( 0, 255 ) ).c_str() );

    // Do remaining logic at least one frame later to avoid race conditions.
}
