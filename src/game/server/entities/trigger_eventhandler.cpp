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

#define EVENTHANDLER_OFF ( 1 << 0 )

class CGameEventHandler : public CBaseEntity
{
public:
	void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;
	void FireHandlers( int iEventType, CBaseEntity* pActivator, CBaseEntity* pCaller, float value );
};

LINK_ENTITY_TO_CLASS( trigger_eventhandler, CGameEventHandler );

void CGameEventHandler :: FireHandlers( int iEventType, CBaseEntity* pActivator, CBaseEntity* pCaller, float value )
{
	if( iEventType == (int)pev->frags && !FBitSet( pev->spawnflags, EVENTHANDLER_OFF ) )
	{
		FireTargets( STRING( pev->target ), ( pActivator != nullptr ? pActivator : this ), ( pCaller != nullptr ? pCaller : this ), USE_TOGGLE, value );
	}
}

void CGameEventHandler :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	switch( useType )
	{
		case USE_ON:
			SetBits( pev->spawnflags, EVENTHANDLER_OFF );
		break;
		case USE_OFF:
			ClearBits( pev->spawnflags, EVENTHANDLER_OFF );
		break;
		case USE_TOGGLE:
			if( FBitSet( pev->spawnflags, EVENTHANDLER_OFF ) )
				ClearBits( pev->spawnflags, EVENTHANDLER_OFF );
			else
				SetBits( pev->spawnflags, EVENTHANDLER_OFF );
		break;
	}
}

void HandleEvent( int iEventType, CBaseEntity* pActivator, CBaseEntity* pCaller, float value )
{
	CGameEventHandler* pEntity = nullptr;

	while( ( pEntity = static_cast<CGameEventHandler*>( UTIL_FindEntityByClassname( pEntity, "trigger_eventhandler" ) ) ) != nullptr )
	{
		pEntity->FireHandlers( iEventType, pActivator, pCaller, value );
	}
}
