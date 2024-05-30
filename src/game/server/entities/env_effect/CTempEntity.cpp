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

#include "CTempEntity.h"

BEGIN_DATAMAP( CTempEntity )
	DEFINE_FIELD( m_VecOffSet, FIELD_VECTOR ),
END_DATAMAP();

void CTempEntity :: RenderEffect( CBasePlayer* player ) { }

void CTempEntity :: Precache() { }

bool CTempEntity :: KeyValue( KeyValueData* pkvd )
{
	if( FStrEq( pkvd->szKeyName, "m_VecOffSet" ) )
	{
		UTIL_StringToVector( m_VecOffSet, pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_VecOffSetEnd" ) )
	{
		UTIL_StringToVector( m_VecOffSetEnd, pkvd->szValue );
	}
	else
	{
		return BaseClass::KeyValue( pkvd );
	}
	return true;
}

void CTempEntity :: ModelSet( const char* szModel )
{
	if( FStringNull( pev->model ) )
	{
		m_iModel = PrecacheModel( szModel );
		m_szModel = MAKE_STRING( szModel );
	}
	else
	{
		m_iModel = PrecacheModel( STRING( pev->model ) );
		m_szModel = pev->model;
	}
}

void CTempEntity :: Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}

Vector CTempEntity :: GetPosition( string_t m_szTarget, CBaseEntity* pActivator, CBaseEntity* pCaller, Vector VecOffSet )
{
	Vector VecPos;

	if( !FStringNull( m_szTarget ) )
	{
		CBaseEntity* pEntity = UTIL_FindEntityByTargetname( nullptr, STRING( m_szTarget ), pActivator, pCaller );

		if( pEntity != nullptr )
		{
			VecPos = pEntity->pev->origin;
		}
		else
		{
			UTIL_StringToVector( VecPos, STRING( m_szTarget ) );
		}
	}

	if( !VecPos || VecPos == g_vecZero )
	{
		if( !FStringNull( m_szTarget ) )
		{
			CBaseEntity::Logger->error("Couldn't found {} for entity {} returning pev->origin", STRING( m_szTarget ), STRING( pev->classname ) );
		}

		VecPos = pev->origin;
	}

	if( VecOffSet != g_vecZero )
	{
		if( VecOffSet.x > 0 ) VecPos.x += VecOffSet.x; else VecPos.x -= VecOffSet.x;
		if( VecOffSet.y > 0 ) VecPos.y += VecOffSet.y; else VecPos.y -= VecOffSet.y;
		if( VecOffSet.z > 0 ) VecPos.z += VecOffSet.z; else VecPos.z -= VecOffSet.z;
	}

	return VecPos;
}

void CTempEntity :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	VecStartPos = GetPosition( pev->netname, pActivator, pCaller, m_VecOffSet );

	if( !FStringNull( pev->message ) )
	{
		VecEndPos = GetPosition( pev->message, pActivator, pCaller, m_VecOffSetEnd );
	}

	for( auto player : UTIL_FindPlayers() )
	{
		if( CanSeeFX( player, pActivator ) )
		{
			RenderEffect( player );
		}
	}

	if( !FStringNull( pev->target ) )
	{
		SUB_UseTargets( pActivator, USE_TOGGLE, 0 );
	}

	if( !FBitSet( pev->spawnflags, SF_TEMPENT_REPEATABLE ) )
	{
		UTIL_Remove( this );
	}
}

bool CTempEntity :: CanSeeFX( CBasePlayer* player, CBaseEntity* pActivator )
{
	if( g_pGameRules->IsMultiplayer() ) {
		if( ( !FBitSet( pev->spawnflags, SF_TEMPENT_ALL_PLAYERS ) && !FBitSet( pev->spawnflags, SF_TEMPENT_ACTIVATOR ) )
			|| ( !FBitSet( pev->spawnflags, SF_TEMPENT_ACTIVATOR ) && player == pActivator )
				|| ( !FBitSet( pev->spawnflags, SF_TEMPENT_ALL_PLAYERS ) && player != pActivator ) ) {
					return false; } }
	return ( player && player != nullptr && player->IsConnected() );
}
