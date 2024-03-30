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

#include "CKeyValueLogic.h"

BEGIN_DATAMAP( CKeyValueLogic )
	DEFINE_FIELD( m_iszNewValue, FIELD_STRING ),
	DEFINE_FIELD( m_iszValueName, FIELD_STRING ),
	DEFINE_FIELD( m_iszValueType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iAppendSpaces, FIELD_INTEGER ),
	DEFINE_FIELD( m_iFloatConversion, FIELD_INTEGER ),
	DEFINE_FIELD( m_trigonometricBehaviour, FIELD_INTEGER ),
END_DATAMAP();

void CKeyValueLogic :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    if( !FStringNull( pev->message ) )
    {
        FireTargets( STRING( pev->message ), pActivator, pCaller, useType, value );
    }
}

bool CKeyValueLogic :: KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "m_iszValueType" ) )
    {
        m_iszValueType = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_trigonometricBehaviour" ) )
    {
        m_trigonometricBehaviour = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iAppendSpaces" ) )
    {
        m_iAppendSpaces = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iFloatConversion" ) )
    {
        m_iFloatConversion = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszValueName" ) )
    {
        m_iszValueName = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszNewValue" ) )
    {
        m_iszNewValue = ALLOC_STRING( pkvd->szValue );
    }
    else
    {
        return CPointEntity::KeyValue(pkvd);
    }
    return true;
}

void CKeyValueLogic :: FindTarget( CBaseEntity* pActivator, CBaseEntity* pCaller )
{
    if( !FStringNull( pev->target ) )
    {
        CBaseEntity* pTarget = nullptr;

        while( ( pTarget = UTIL_FindEntityByTargetname( pTarget, STRING( pev->target ), pActivator, pCaller ) ) )
        {
            ModifyValue( pTarget );

            if( !FBitSet( pev->spawnflags, SF_MULTIPLE_DEST ) )
            {
                break;
            }
        }
    }
}

void CKeyValueLogic :: ModifyValue( CBaseEntity* pTarget )
{
    switch( m_iszValueType )
    {
        case KeyValueLogicFlags::REPLACE:
        {
            SetKeyValue( pTarget, m_iszValueName, m_iszNewValue );
            break;
        }
        case KeyValueLogicFlags::ADD:
        {
            int i = atoi( STRING( GetValueOfKey( pTarget, m_iszValueName ) ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i + d ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::APPEND:
        {
            break;
        }
    }
}

string_t CKeyValueLogic :: GetValueOfKey( CBaseEntity* pTarget, string_t m_szKey )
{
    if( pTarget->KeyValueDatai > 0 )
    {
        for( int i = 0; i < pTarget->KeyValueDatai; ++i )
        {
            if( FStrEq( STRING( m_szKey ), STRING( pTarget->KeyValueKeys[ i ] ) ) )
            {
                return pTarget->KeyValueValues[ i ];
            }
        }
    }
    return string_t();
}

void CKeyValueLogic :: SetKeyValue( CBaseEntity* pTarget, string_t m_szKey, string_t m_szValue )
{
    if( pTarget && !FStringNull( m_szKey ) && !FStringNull( m_szValue ) )
    {
        auto edict = pTarget->edict();

        const char* classname = pTarget->GetClassname();

        KeyValueData kvd{.szClassName = classname};

        kvd.szKeyName = STRING( m_szKey );
        kvd.szValue = STRING( m_szValue );
        kvd.fHandled = 0;

        DispatchKeyValue( edict, &kvd );
    }
}