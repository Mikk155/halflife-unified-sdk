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
#include "basemonster.h"

class CPlayerSetHudColor : public CPointEntity
{
    DECLARE_CLASS( CPlayerSetHudColor, CPointEntity );
    DECLARE_DATAMAP();

public:
    enum class Action
    {
        Set = 0,
        Reset = 1
    };

    bool KeyValue( KeyValueData* pkvd ) override;

    void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value = {} ) override;

private:
    Vector m_HudColor{vec3_origin};
    Action m_Action{Action::Set};
};

LINK_ENTITY_TO_CLASS( player_sethudcolor, CPlayerSetHudColor );

BEGIN_DATAMAP( CPlayerSetHudColor )
    DEFINE_FIELD( m_HudColor, FIELD_VECTOR ),
    DEFINE_FIELD( m_Action, FIELD_INTEGER ),
END_DATAMAP();

bool CPlayerSetHudColor::KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "hud_color" ) )
    {
        UTIL_StringToVector( m_HudColor, pkvd->szValue );
        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "action" ) )
    {
        m_Action = static_cast<Action>( atoi( pkvd->szValue ) );
        return true;
    }

    return CPointEntity::KeyValue( pkvd );
}

void CPlayerSetHudColor::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value )
{
    auto player = ToBasePlayer( pActivator );

    if( !player || !player->IsNetClient() )
    {
        return;
    }

    const RGB24 color = [this]()
    {
        switch ( m_Action )
        {
        case Action::Set:
            return RGB24{
                static_cast<std::uint8_t>( m_HudColor.x ),
                static_cast<std::uint8_t>( m_HudColor.y ),
                static_cast<std::uint8_t>( m_HudColor.z )};

        default:
        case Action::Reset:
            return RGB_HUD_COLOR;
        }
    }();

    player->SetHudColor( color );
}

class CPlayerSetCrosshairColor : public CPointEntity
{
    DECLARE_CLASS( CPlayerSetCrosshairColor, CPointEntity );
    DECLARE_DATAMAP();

public:
    enum class Action
    {
        Set = 0,
        Reset = 1
    };

    bool KeyValue( KeyValueData* pkvd ) override;

    void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value = {} ) override;

private:
    Vector m_CrosshairColor{vec3_origin};
    Action m_Action{Action::Set};
};

LINK_ENTITY_TO_CLASS( player_setcrosshaircolor, CPlayerSetCrosshairColor );

BEGIN_DATAMAP( CPlayerSetCrosshairColor )
    DEFINE_FIELD( m_CrosshairColor, FIELD_VECTOR ),
    DEFINE_FIELD( m_Action, FIELD_INTEGER ),
END_DATAMAP();

bool CPlayerSetCrosshairColor::KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "crosshair_color" ) )
    {
        UTIL_StringToVector( m_CrosshairColor, pkvd->szValue );
        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "action" ) )
    {
        m_Action = static_cast<Action>( atoi( pkvd->szValue ) );
        return true;
    }

    return CPointEntity::KeyValue( pkvd );
}

void CPlayerSetCrosshairColor::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value )
{
    auto player = ToBasePlayer( pActivator );

    if( !player || !player->IsNetClient() )
    {
        return;
    }

    const RGB24 color = [this]()
    {
        switch ( m_Action )
        {
        case Action::Set:
            return RGB24{
                static_cast<std::uint8_t>( m_CrosshairColor.x ),
                static_cast<std::uint8_t>( m_CrosshairColor.y ),
                static_cast<std::uint8_t>( m_CrosshairColor.z )};

        default:
        case Action::Reset:
            return RGB_HUD_COLOR;
        }
    }();

    player->SetCrosshairColor( color );
}

class CPlayerSetSuitLightType : public CPointEntity
{
    DECLARE_CLASS( CPlayerSetSuitLightType, CPointEntity );
    DECLARE_DATAMAP();

public:
    bool KeyValue( KeyValueData* pkvd ) override;

    void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value = {} ) override;

private:
    bool m_AllPlayers = false;
    SuitLightType m_Type = SuitLightType::Flashlight;
};

LINK_ENTITY_TO_CLASS( player_setsuitlighttype, CPlayerSetSuitLightType );

BEGIN_DATAMAP( CPlayerSetSuitLightType )
    DEFINE_FIELD( m_AllPlayers, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_Type, FIELD_INTEGER ),
END_DATAMAP();

bool CPlayerSetSuitLightType::KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "all_players" ) )
    {
        m_AllPlayers = atoi( pkvd->szValue ) != 0;
        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "light_type" ) )
    {
        m_Type = static_cast<SuitLightType>( atoi( pkvd->szValue ) );
        return true;
    }

    return CPointEntity::KeyValue( pkvd );
}

void CPlayerSetSuitLightType::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value )
{
    const auto executor = [this]( CBasePlayer* player )
    {
        player->SetSuitLightType( m_Type );
    };

    if( m_AllPlayers )
    {
        for( auto player : UTIL_FindPlayers() )
        {
            executor( player );
        }
    }
    else
    {
        CBasePlayer* player = ToBasePlayer( pActivator );

        if( !player && !g_GameMode->IsMultiplayer() )
        {
            player = UTIL_GetLocalPlayer();
        }

        if( player )
        {
            executor( player );
        }
    }
}

class CPlayerSetHealth : public CPointEntity
{
    DECLARE_CLASS( CPlayerSetHealth, CPointEntity );
    DECLARE_DATAMAP();

public:
    static constexpr int FlagAllPlayers = 1 << 0;
    static constexpr int FlagSetHealth = 1 << 1;
    static constexpr int FlagSetArmor = 1 << 2;

    bool KeyValue( KeyValueData* pkvd ) override;

    void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value = {} ) override;

private:
    int m_Flags = 0;
    int m_Health = 100;
    int m_Armor = 0;
};

LINK_ENTITY_TO_CLASS( player_sethealth, CPlayerSetHealth );

BEGIN_DATAMAP( CPlayerSetHealth )
    DEFINE_FIELD( m_Flags, FIELD_INTEGER ),
    DEFINE_FIELD( m_Health, FIELD_INTEGER ),
    DEFINE_FIELD( m_Armor, FIELD_INTEGER ),
END_DATAMAP();

bool CPlayerSetHealth::KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "all_players" ) )
    {
        if( atoi( pkvd->szValue ) != 0 )
        {
            SetBits( m_Flags, FlagAllPlayers );
        }
        else
        {
            ClearBits( m_Flags, FlagAllPlayers );
        }

        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "set_health" ) )
    {
        if( atoi( pkvd->szValue ) != 0 )
        {
            SetBits( m_Flags, FlagSetHealth );
        }
        else
        {
            ClearBits( m_Flags, FlagSetHealth );
        }

        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "set_armor" ) )
    {
        if( atoi( pkvd->szValue ) != 0 )
        {
            SetBits( m_Flags, FlagSetArmor );
        }
        else
        {
            ClearBits( m_Flags, FlagSetArmor );
        }

        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "health_to_set" ) )
    {
        // Clamp to 1 so players don't end up in an invalid state
        m_Health = std::max( atoi( pkvd->szValue ), 1 );
        return true;
    }
    else if( FStrEq( pkvd->szKeyName, "armor_to_set" ) )
    {
        m_Armor = std::max( atoi( pkvd->szValue ), 0 );
        return true;
    }

    return CPointEntity::KeyValue( pkvd );
}

void CPlayerSetHealth::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, UseValue value )
{
    const auto executor = [this]( CBasePlayer* player )
    {
        if( ( m_Flags & FlagSetHealth ) != 0 )
        {
            // Clamp it to the current max health
            player->pev->health = std::min( static_cast<int>( std::floor( player->pev->max_health ) ), m_Health );
        }

        if( ( m_Flags & FlagSetArmor ) != 0 )
        {
            // Clamp it now, in case future changes allow for custom armor maximum
            player->pev->armorvalue = std::min( MAX_NORMAL_BATTERY, m_Armor );
        }
    };

    if( ( m_Flags & FlagAllPlayers ) != 0 )
    {
        for( auto player : UTIL_FindPlayers() )
        {
            executor( player );
        }
    }
    else
    {
        CBasePlayer* player = ToBasePlayer( pActivator );

        if( !player && !g_GameMode->IsMultiplayer() )
        {
            player = UTIL_GetLocalPlayer();
        }

        if( player )
        {
            executor( player );
        }
    }
}
