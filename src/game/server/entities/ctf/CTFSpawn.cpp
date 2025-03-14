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

#include "CTFSpawn.h"

const char* const sTeamSpawnNames[] =
    {
        "ctfs0",
        "ctfs1",
        "ctfs2",
};

LINK_ENTITY_TO_CLASS( info_ctfspawn, CTFSpawn );

bool CTFSpawn::KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( "team_no", pkvd->szKeyName ) )
    {
        team_no = static_cast<CTFTeam>( atoi( pkvd->szValue ) );
        return true;
    }

    return CBaseEntity::KeyValue( pkvd );
}

bool CTFSpawn::Spawn()
{
    if( team_no < CTFTeam::None || team_no > CTFTeam::OpposingForce )
    {
        Logger->debug( "Teamspawnpoint with an invalid team_no of {}", static_cast<int>( team_no ) );
        return false;
    }

    m_sMaster = pev->classname;

    // Change the classname to the owning team's spawn name
    pev->classname = MAKE_STRING( sTeamSpawnNames[static_cast<int>( team_no )] );
    m_fState = true;

    return true;
}

bool CTFSpawn::IsTriggered( CBaseEntity* pEntity )
{
    if( !FStringNull( pev->targetname ) && STRING( pev->targetname ) )
        return m_fState;
    else
        return !( IsLockedByMaster( pEntity ) );
}
