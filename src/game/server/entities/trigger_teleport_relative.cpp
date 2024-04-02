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

#include "cbase.h"

enum CTelRelFlags : int
{
    // Only teleport people on the inside upon trigger
    OnlyTrigger = ( 1 << 0 ),
    // Allow monsters
    AllowMonsters = ( 1 << 1 ),
    // Don't allow players
    NoClients = ( 1 << 2 ),
    // Set velocity to g_vecZero
    KillVelocity = ( 1 << 3 )
};

class CTriggerTeleportRelative : public CBaseEntity
{
    public:
        void Spawn() override;
        void Touch( CBaseEntity* pOther ) override;
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;
        int m_ttrFlags() { return ( ( FBitSet( pev->spawnflags, CTelRelFlags::AllowMonsters ) ? FL_MONSTER : 0 ) | ( FBitSet( pev->spawnflags, CTelRelFlags::NoClients ) ? 0 : FL_CLIENT ) ); };
        Vector GetLandMark( string_t m_iszChar );
        void RelativeTeleport( CBaseEntity* pEntity );
};

LINK_ENTITY_TO_CLASS( trigger_teleport_relative, CTriggerTeleportRelative );

void CTriggerTeleportRelative :: Spawn()
{
    pev->solid = SOLID_TRIGGER;
    pev->effects |= EF_NODRAW;
    pev->movetype = MOVETYPE_NONE;
    SetModel( STRING( pev->model ) );
}

// Teleport everyone that's inside this entity's volumen
void CTriggerTeleportRelative :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    CBaseEntity* pList[256];

    int count = UTIL_EntitiesInBox( pList, 256, pev->mins, pev->maxs, m_ttrFlags() );

    if( 0 != count )
    {
        for (int i = 0; i < count; i++)
        {
            RelativeTeleport( pList[i] );
        }
    }
}

void CTriggerTeleportRelative :: Touch( CBaseEntity* pOther )
{
    if( !FBitSet( pev->spawnflags, CTelRelFlags::OnlyTrigger ) )
    {
        RelativeTeleport( pOther );
    }
}

void CTriggerTeleportRelative :: RelativeTeleport( CBaseEntity* pEntity )
{
    if( !pEntity
    || pEntity->IsPlayer() && FBitSet( pev->spawnflags, CTelRelFlags::NoClients )
    || pEntity->IsMonster() && !FBitSet( pev->spawnflags, CTelRelFlags::AllowMonsters )
    ){
        return;
    }

    Vector VecStart = GetLandMark( pev->netname );
    Vector VecEnd = GetLandMark( pev->message );

    Vector VecDif = ( VecStart - pEntity->pev->origin );
    Vector VecRes = ( VecEnd - VecDif );
    pEntity->pev->origin = VecRes;

    if( FBitSet( pev->spawnflags, CTelRelFlags::KillVelocity ) )
        pEntity->pev->velocity = g_vecZero;

    FireTargets( STRING( pev->target ), pEntity, this, USE_TOGGLE, 0 );
}

Vector CTriggerTeleportRelative :: GetLandMark( string_t m_iszChar )
{
    Vector VecSrc;

    CBaseEntity* pLandmark = UTIL_FindEntityByTargetname( NULL, STRING( m_iszChar ) );

    if( pLandmark )
    {
        VecSrc = pLandmark->pev->origin;
    }
    else
    {
        UTIL_StringToVector( VecSrc, STRING( m_iszChar ) );
    }

    return VecSrc;
}