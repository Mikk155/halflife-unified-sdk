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

#define SF_ENVMODEL_OFF			( 1 << 0 )
#define SF_ENVMODEL_DROPTOFLOOR	( 1 << 1 )
#define SF_ENVMODEL_SOLID		( 1 << 2 )

class CEnvModel : public CBaseAnimating
{
	DECLARE_CLASS( CEnvModel, CBaseAnimating );
	DECLARE_DATAMAP();

    public:
        void ModelThink();
        void SetSequence();
        void Spawn() override;
        void Precache() override;
        int ObjectCaps() override;
        bool KeyValue( KeyValueData* pkvd ) override;
        void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

    private:
        string_t m_iszSequence_On;
        string_t m_iszSequence_Off;
        int m_iAction_On;
        int m_iAction_Off;
};

LINK_ENTITY_TO_CLASS( env_model, CEnvModel );

BEGIN_DATAMAP( CEnvModel )
    DEFINE_FUNCTION( ModelThink ),
    DEFINE_FIELD( m_iAction_On, FIELD_INTEGER ),
    DEFINE_FIELD( m_iAction_Off, FIELD_INTEGER ),
    DEFINE_FIELD( m_iszSequence_On, FIELD_STRING ),
    DEFINE_FIELD( m_iszSequence_Off, FIELD_STRING ),
END_DATAMAP();

int CEnvModel :: ObjectCaps()
{
    return ( CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; );
}

bool CEnvModel :: KeyValue( KeyValueData* pkvd )
{
	if( FStrEq( pkvd->szKeyName, "m_iszSequence_On" ) )
	{
		m_iszSequence_On = ALLOC_STRING( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_iszSequence_Off" ) )
	{
		m_iszSequence_Off = ALLOC_STRING( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_iAction_On" ) )
	{
		m_iAction_On = ALLOC_STRING( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_iAction_Off" ) )
	{
		m_iAction_Off = ALLOC_STRING( pkvd->szValue );
	}
    else
    {
    	return BaseClass::KeyValue(pkvd);
    }

    return true;
}

void CEnvModel :: Precache()
{
    PrecacheModel( STRING( pev->model ) );
}

void CEnvModel :: Spawn()
{
	Precache();
    SetModel( STRING( pev->model ) );

    if( FBitSet( pev->spawnflags, SF_ENVMODEL_SOLID ) )
    {
        pev->solid = SOLID_SLIDEBOX;
        SetSize( Vector( -10, -10, -10 ), Vector( 10, 10, 10 ) );
    }

    if( FBitSet( pev->spawnflags, SF_ENVMODEL_DROPTOFLOOR ) )
    {
        pev->origin.z += 1;
        DROP_TO_FLOOR( edict() );
    }

    SetBoneController( 0, 0 );
    SetBoneController( 1, 0 );

    SetSequence();

    SetThink( &CEnvModel::ModelThink );
    pev->nextthink = gpGlobals->time + 0.1f;
}

void CEnvModel :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    switch( useType )
    {
        case USE_ON:
        {
            ClearBits( pev->spawnflags, SF_ENVMODEL_OFF );
            break;
        }
        case USE_OFF:
        {
            SetBits( pev->spawnflags, SF_ENVMODEL_OFF );
            break;
        }
        case USE_TOGGLE:
        default:
        {
            if( ShouldToggle( useType, ( pev->spawnflags & SF_ENVMODEL_OFF ) ) )
            {
                ClearBits( pev->spawnflags, SF_ENVMODEL_OFF );
            }
            else
            {
                SetBits( pev->spawnflags, SF_ENVMODEL_OFF );
            }
            break;
        }
        SetSequence();
        pev->nextthink + gpGlobals->time + 0.1f;
    }
}

void CEnvModel :: ModelThink()
{
	int iTemp = ( FBitSet( pev->spawnflags, SF_ENVMODEL_OFF ) ? m_iAction_Off : m_iAction_On );

    StudioFrameAdvance();

    if( m_fSequenceFinished && !m_fSequenceLoops )
    {
        switch( iTemp )
        {
            case 1: // Loop
            {
                pev->animtime = gpGlobals->time;
                m_fSequenceFinished = false;
                m_flLastEventCheck = gpGlobals->time;
                pev->frame = 0;
                break;
            }
            case 2: // Change state
            {
                if( FBitSet( pev->spawnflags & SF_ENVMODEL_OFF ) )
                {
                    ClearBits( pev->spawnflags, SF_ENVMODEL_OFF );
                }
                else
                {
                    SetBits( pev->spawnflags, SF_ENVMODEL_OFF );
                }
                SetSequence();
                break;
            }
            default:
            {
                return;
            }
        }
        pev->nextthink = gpGlobals->time + 0.1f;
    }
}

void CEnvModel :: SetSequence()
{
	int iszSeq = ( FBitSet( pev->spawnflags & SF_ENVMODEL_OFF ) ? m_iszSequence_Off : m_iszSequence_On );

    if( !iszSeq )
        return;

    pev->sequence = LookupSequence( STRING( iszSeq ) );

    if( pev->sequence == -1 )
    {
        BaseClass::Logger->error( "env_model {} unknown sequence {}", STRING( pev->targetname ), iszSeq );
        pev->sequence = 0;
    }

    pev->frame = 0;
    ResetSequenceInfo();

    if( FBitSet( pev->spawnflags, SF_ENVMODEL_OFF ) )
    {
        m_fSequenceLoops = ( m_iAction_Off == 1 ? 1 : 0 );
    }
    else
    {
        m_fSequenceLoops = ( m_iAction_On == 1 ? 1 : 0 );
    }
}

/*
*/
