/***
 *
 *    Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
//
// hud_playerbrowse.cpp
//
#include "hud.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "event_api.h"

bool CHudFlagIcons::Init()
{
    g_ClientUserMessages.RegisterHandler( "FlagIcon", &CHudFlagIcons::MsgFunc_FlagIcon, this );
    g_ClientUserMessages.RegisterHandler( "FlagTimer", &CHudFlagIcons::MsgFunc_FlagTimer, this );

    gHUD.AddHudElem( this );

    memset( m_FlagList, 0, sizeof( m_FlagList ) );
    m_flTimeStart = 0;
    m_flTimeLimit = 0;
    m_bIsTimer = false;
    m_bTimerReset = false;

    return true;
}

bool CHudFlagIcons::VidInit()
{
    m_bIsTimer = false;
    return true;
}

void CHudFlagIcons::InitHUDData()
{
    memset( m_FlagList, 0, sizeof( m_FlagList ) );
}

bool CHudFlagIcons::Draw( float flTime )
{
    // TODO: can this ever return 2?
    if( gEngfuncs.IsSpectateOnly() != 2 )
    {
        int y = ScreenHeight - 64;

        for( int i = 0; i < MAX_FLAGSPRITES; ++i )
        {
            const auto& flag = m_FlagList[i];

            if( 0 != flag.spr )
            {
                y += flag.rc.top - flag.rc.bottom - 5;

                gEngfuncs.pfnSPR_Set( flag.spr, flag.r, flag.g, flag.b );
                gEngfuncs.pfnSPR_DrawAdditive( 0, 5, y, &flag.rc );
                gHUD.DrawHudNumber( 
                    flag.rc.right - flag.rc.left + 10,
                    y + ( ( flag.rc.bottom - flag.rc.top ) / 2 ) - 5,
                    DHN_DRAWZERO | DHN_2DIGITS,
                    flag.score, {flag.r, flag.g, flag.b} );
            }
        }

        if( m_bIsTimer )
        {
            if( m_bTimerReset )
            {
                m_flTimeStart = gHUD.m_flTime;
                m_bTimerReset = false;
            }

            const int totalSecondsLeft = static_cast<int>( m_flTimeLimit - ( gHUD.m_flTime - m_flTimeStart ) );

            const int minutesLeft = std::max( 0, totalSecondsLeft / 60 );
            const int secondsLeft = std::max( 0, totalSecondsLeft - ( 60 * minutesLeft ) );

            // TODO: this buffer is static in vanilla Op4
            char szBuf[40];
            sprintf( szBuf, "%s %d:%02d", CHudTextMessage::BufferedLocaliseTextString( "#CTFTimeRemain" ), minutesLeft, secondsLeft );
            gHUD.DrawHudString( 5, ScreenHeight - 60, 200, szBuf, gHUD.m_HudColor );
        }
    }

    return true;
}

void CHudFlagIcons::EnableFlag( const char* pszFlagName, unsigned char team_idx, unsigned char red, unsigned char green, unsigned char blue, unsigned char score )
{
    if( team_idx < MAX_FLAGSPRITES )
    {
        auto& flag = m_FlagList[team_idx];

        const int spriteIndex = gHUD.GetSpriteIndex( pszFlagName );

        flag.spr = gHUD.GetSprite( spriteIndex );
        flag.rc = gHUD.GetSpriteRect( spriteIndex );
        flag.r = red;
        flag.g = green;
        flag.b = blue;
        flag.score = score;

        auto& teamInfo = g_TeamInfo[team_idx + 1];
        teamInfo.scores_overriden = true;
        teamInfo.frags = score;

        strcpy( flag.szSpriteName, pszFlagName );
    }
}

void CHudFlagIcons::DisableFlag( const char* pszFlagName, unsigned char team_idx )
{
    if( team_idx < MAX_FLAGSPRITES )
    {
        memset( &m_FlagList[team_idx], 0, sizeof( m_FlagList[team_idx] ) );
    }
    else
    {
        m_iFlags &= ~HUD_ACTIVE;
    }
}

void CHudFlagIcons::MsgFunc_FlagIcon( const char* pszName, BufferReader& reader )
{
    const bool isActive = 0 != reader.ReadByte();
    const char* flagName = reader.ReadString();
    const int team_idx = reader.ReadByte();

    if( isActive )
    {
        const int r = reader.ReadByte();
        const int g = reader.ReadByte();
        const int b = reader.ReadByte();
        const int score = reader.ReadByte();
        EnableFlag( flagName, team_idx, r, g, b, score );
        m_iFlags |= HUD_ACTIVE;
    }
    else
    {
        DisableFlag( flagName, team_idx );
    }
}

void CHudFlagIcons::MsgFunc_FlagTimer( const char* pszName, BufferReader& reader )
{
    m_bIsTimer = reader.ReadByte() != 0;

    if( m_bIsTimer )
    {
        m_flTimeLimit = reader.ReadShort();
        m_bTimerReset = true;
    }
}
