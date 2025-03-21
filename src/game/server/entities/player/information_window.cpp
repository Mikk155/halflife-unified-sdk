/***
 *
 * Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 * This product contains software technology licensed from Id
 * Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 * All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "ui/hud/InformationWindow.h"

class CPlayerInformationWindow : public CPointEntity
{
    DECLARE_CLASS( CPlayerInformationWindow, CPointEntity );
    DECLARE_DATAMAP();

    public:
        bool Spawn() override;
        bool KeyValue( KeyValueData* pkvd ) override;
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value = {} ) override;

    private:
        string_t m_title;
        string_t m_body;
        int m_page;
};

LINK_ENTITY_TO_CLASS( player_information_window, CPlayerInformationWindow );

BEGIN_DATAMAP( CPlayerInformationWindow )
    DEFINE_FIELD( m_title, FIELD_STRING ),
    DEFINE_FIELD( m_body, FIELD_STRING ),
    DEFINE_FIELD( m_page, FIELD_INTEGER ),
END_DATAMAP();

bool CPlayerInformationWindow::Spawn()
{
    if( m_page == 0 )
    {
        CBaseEntity::Logger->warn( "Entity \"player_information_window\" is using page 0 but is reserved for the Server's motd. Removing entity." );
        return false;
    }
    return BaseClass::Spawn();
}

bool CPlayerInformationWindow :: KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "m_title" ) )
    {
        m_title = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_body" ) )
    {
        m_body = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_page" ) )
    {
        m_page = g_InformationWindow.ClampPages( atoi( pkvd->szValue ) );
    }
    else
    {
        return CPointEntity::KeyValue(pkvd);
    }
    return true;
}

void CPlayerInformationWindow :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value )
{
    if( ( pev->spawnflags & 2 ) != 0 )
    {
        for( auto player : UTIL_FindPlayers() )
        {
            if( player != nullptr )
            {
                if( useType == USE_OFF )
                    g_InformationWindow.ClearInformation( player, m_page );
                else
                    g_InformationWindow.SendInformation( player, m_page, STRING( m_title ), STRING( m_body ) );
            }
        }
    }
    else
    {
        CBasePlayer* player = ToBasePlayer( pActivator );

        if( !player && !g_pGameRules->IsMultiplayer() )
        {
            player = UTIL_GetLocalPlayer();
        }

        if( player != nullptr )
        {
            if( useType == USE_OFF )
                g_InformationWindow.ClearInformation( player, m_page );
            else
                g_InformationWindow.SendInformation( player, m_page, STRING( m_title ), STRING( m_body ) );
        }
    }
}
