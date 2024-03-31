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

#include "trigger_copyvalue.h"

BEGIN_DATAMAP( CTriggerCopyValue )
	DEFINE_FUNCTION( CopyThink ),
	DEFINE_FIELD( svalue, FIELD_FLOAT ),
	DEFINE_FIELD( suseType, FIELD_INTEGER ),
	DEFINE_FIELD( IsThinking, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iszSrcValueName, FIELD_STRING ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( trigger_copyvalue, CTriggerCopyValue );

void CTriggerCopyValue :: Spawn()
{
    if( FBitSet( pev->spawnflags, SF_START_ON ) )
    {
    	SetThink(&CTriggerCopyValue::CopyThink);
        IsThinking = true;
	    pev->nextthink = gpGlobals->time + pev->dmg;
    }
    CKeyValueLogic::Spawn();
}

bool CTriggerCopyValue :: KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "m_iszSrcValueName" ) )
    {
        m_iszSrcValueName = ALLOC_STRING( pkvd->szValue );
    }
    else
    {
        return CKeyValueLogic::KeyValue(pkvd);
    }
    return true;
}

void CTriggerCopyValue :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    suseType = useType;
    svalue = value;

    if( FBitSet( pev->spawnflags, SF_CONSTANT ) )
    {
        if( IsThinking )
        {
            SetThink( nullptr );
            IsThinking = false;
        }
        else
        {
            SetThink(&CTriggerCopyValue::CopyThink);
            pev->nextthink = gpGlobals->time;
            IsThinking = true;
        }
    }
    else
    {
        CopyValues( pActivator, pCaller );
    }
}

void CTriggerCopyValue :: CopyThink()
{
    CopyValues( nullptr, nullptr );

    if( FBitSet( pev->spawnflags, SF_CONSTANT ) )
    {
	    pev->nextthink = gpGlobals->time + pev->dmg;
    }
}

void CTriggerCopyValue :: CopyValues( CBaseEntity* pActivator, CBaseEntity* pCaller )
{
    if( !FStringNull( pev->netname ) )
    {
        CBaseEntity* pTarget = UTIL_FindEntityByTargetname( nullptr, STRING( pev->netname ), pActivator, pCaller );

        if( pTarget )
        {
            m_iszNewValue = MAKE_STRING( GetValueOfKey( pTarget, m_iszSrcValueName ) );
        }
    }
    FindTarget( pActivator, pCaller );
    CKeyValueLogic::Use( pActivator, pCaller, suseType, svalue );
}
