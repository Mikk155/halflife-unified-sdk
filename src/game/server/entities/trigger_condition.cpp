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

#include "trigger_condition.h"

BEGIN_DATAMAP( CTriggerCondition )
	DEFINE_FUNCTION( ThinkCondition ),
	DEFINE_FIELD( IsThinking, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_szCaller, FIELD_EHANDLE ),
	DEFINE_FIELD( LastFireCase, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iCheckType, FIELD_INTEGER ),
	DEFINE_FIELD( m_szActivator, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iszSourceKey, FIELD_STRING ),
	DEFINE_FIELD( m_iszValueName, FIELD_STRING ),
	DEFINE_FIELD( m_iszSourceName, FIELD_STRING ),
	DEFINE_FIELD( m_iszCheckValue, FIELD_STRING ),
	DEFINE_FIELD( m_fCheckInterval, FIELD_FLOAT ),
	DEFINE_FIELD( m_iCheckBehaviour, FIELD_INTEGER ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( trigger_condition, CTriggerCondition )

void CTriggerCondition :: Spawn()
{
    if( !FBitSet( pev->spawnflags, SF_START_OFF ) && !FBitSet( pev->spawnflags, SF_CYCLIC_NTOGGLE ) )
    {
        SetThink( &CTriggerCondition::ThinkCondition );
        IsThinking = true;
        pev->nextthink = gpGlobals->time + m_fCheckInterval;
    }
    pev->solid = SOLID_NOT;
}

bool CTriggerCondition :: KeyValue( KeyValueData* pkvd )
{
    if( FStrEq( pkvd->szKeyName, "m_iCheckBehaviour" ) )
    {
        m_iCheckBehaviour = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iCheckType" ) )
    {
        m_iCheckType = atoi( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_fCheckInterval" ) )
    {
        m_fCheckInterval = atof( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszSourceKey" ) )
    {
        m_iszSourceKey = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszValueName" ) )
    {
        m_iszValueName = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszSourceName" ) )
    {
        m_iszSourceName = ALLOC_STRING( pkvd->szValue );
    }
    else if( FStrEq( pkvd->szKeyName, "m_iszCheckValue" ) )
    {
        m_iszCheckValue = ALLOC_STRING( pkvd->szValue );
    }
    else
    {
        return CPointEntity::KeyValue(pkvd);
    }
    return true;
}

void CTriggerCondition :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    m_szActivator = pActivator;
    m_szCaller = pCaller;

    if( FBitSet( pev->spawnflags, SF_CYCLIC_NTOGGLE ) )
    {
        GetComparator();
    }
    else
    {
        if( IsThinking )
        {
            SetThink( nullptr );
            IsThinking = false;
        }
        else
        {
            SetThink( &CTriggerCondition::ThinkCondition );
            IsThinking = true;
            pev->nextthink = gpGlobals->time + m_fCheckInterval;
        }
    }
}

void CTriggerCondition :: ThinkCondition()
{
    GetComparator();
    pev->nextthink = gpGlobals->time + m_fCheckInterval;
}

void CTriggerCondition :: GetComparator()
{
    if( FBitSet( pev->spawnflags, SF_IGNORE_1STRESULT ) )
    {
        ClearBits( pev->spawnflags, SF_IGNORE_1STRESULT );
        return;
    }

    if( !FStringNull( pev->target ) )
    {
        CBaseEntity* pTarget = UTIL_FindEntityByTargetname( nullptr, STRING( pev->target ), m_szActivator.Get(), m_szCaller.Get() );

        if( pTarget )
        {
            std::string Destination;

            std::string Source = CKeyValueLogic::GetValueOfKey( pTarget, m_iszValueName );

            if( !FStringNull( m_iszCheckValue ) )
            {
                Destination = std::string( STRING( m_iszCheckValue ) );
            }
            else
            {
                CBaseEntity* pCompare = UTIL_FindEntityByTargetname( nullptr, STRING( m_iszSourceName ), m_szActivator.Get(), m_szCaller.Get() );

                if( pCompare )
                {
                    Destination = CKeyValueLogic::GetValueOfKey( pCompare, m_iszSourceKey );
                }
            }

            const char* szTarget;
            bool bPossitive = IsPossitive( Source, Destination );

            switch( m_iCheckBehaviour )
            {
                case CHECK_TRUE_FALSE:
                {
                    break;
                }
                case CHECK_ALTERNATINGLY:
                {
                    if( LastFireCase == bPossitive )
                    {
                        return;
                    }
                    LastFireCase = bPossitive;
                    break;
                }
                case CHECK_ONLY_FALSE:
                {
                    if( !bPossitive )
                    {
                        LastFireCase = true;
                    }

                    if( !LastFireCase )
                    {
                        return;
                    }

                    break;
                }
                case CHECK_ONLY_TRUE:
                {
                    if( bPossitive )
                    {
                        LastFireCase = true;
                    }

                    if( !LastFireCase )
                    {
                        return;
                    }

                    break;
                }
            }

            if( bPossitive )
            {
                szTarget = STRING( pev->netname );
            }
            else
            {
                szTarget = STRING( pev->message );
            }

            CBaseEntity* pActivator = this;

            if( FBitSet( pev->spawnflags, SF_KEEP_ACTIVATOR ) )
            {
                pActivator = m_szActivator.Get();
            }

            FireTargets( szTarget, pActivator, this, USE_TOGGLE, 0 );
        }
    }
}

bool CTriggerCondition :: IsPossitive( std::string Source, std::string Destination )
{
    switch( m_iCheckType )
    {
        case TriggerConditionCheck::NOT_EQUAL:
        {
            return ( Source != Destination );
        }
        case TriggerConditionCheck::LESS:
        {
            return ( atof( Source.c_str() ) < atof( Destination.c_str() ) );
        }
        case TriggerConditionCheck::LESS_OR_EQUAL:
        {
            return ( atof( Source.c_str() ) <= atof( Destination.c_str() ) );
        }
        case TriggerConditionCheck::GREATER:
        {
            return ( atof( Source.c_str() ) > atof( Destination.c_str() ) );
        }
        case TriggerConditionCheck::GREATER_OR_EQUAL:
        {
            return ( atof( Source.c_str() ) >= atof( Destination.c_str() ) );
        }
        case TriggerConditionCheck::LOGICAL_AND:
        {
            return FBitSet( atoi( Source.c_str() ), atoi( Destination.c_str() ) );
        }
        case TriggerConditionCheck::EQUAL:
        default:
        {
            return ( Source == Destination );
        }
    }
    return false;
}
