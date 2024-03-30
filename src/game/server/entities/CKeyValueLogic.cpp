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

void CKeyValueLogic :: Spawn()
{
    pev->solid = SOLID_NOT;
}

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
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i + d ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::APPEND:
        {
            std::string str = GetValueOfKey( pTarget, m_iszValueName );
            for( int i = 0; i < m_iAppendSpaces; i++ )
                str += " ";
            str += STRING( m_iszNewValue );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( str.c_str() ) );
            break;
        }
        case KeyValueLogicFlags::MUL:
        {
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i * d ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::SUB:
        {
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i - d ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::DIV:
        {
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i / d ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::AND:
        {
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            ClearBits( i, d );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::OR:
        {
            int i = atoi( GetValueOfKey( pTarget, m_iszValueName ) );
            int d = atoi( STRING( m_iszNewValue ) );
            SetBits( i, d );
            SetKeyValue( pTarget, m_iszValueName, MAKE_STRING( std::to_string( i ).c_str() ) );
            break;
        }
        case KeyValueLogicFlags::NAND:
        {
            break;
        }
        case KeyValueLogicFlags::NOR:
        {
            break;
        }
        case KeyValueLogicFlags::DANGLES:
        {
            break;
        }
        case KeyValueLogicFlags::ADIRECTION:
        {
            break;
        }
        case KeyValueLogicFlags::MOD:
        {
            break;
        }
        case KeyValueLogicFlags::XOR:
        {
            break;
        }
        case KeyValueLogicFlags::NXOR:
        {
            break;
        }
        case KeyValueLogicFlags::POW:
        {
            break;
        }
        case KeyValueLogicFlags::SIN:
        {
            break;
        }
        case KeyValueLogicFlags::COS:
        {
            break;
        }
        case KeyValueLogicFlags::TAN:
        {
            break;
        }
        case KeyValueLogicFlags::ARCSIN:
        {
            break;
        }
        case KeyValueLogicFlags::ARCCOS:
        {
            break;
        }
        case KeyValueLogicFlags::ARCTAN:
        {
            break;
        }
        case KeyValueLogicFlags::COT:
        {
            break;
        }
        case KeyValueLogicFlags::ARCCOT:
        {
            break;
        }
        default:
        {
            SetKeyValue( pTarget, m_iszValueName, m_iszNewValue );
        }
    }
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

const char* CKeyValueLogic :: ToString( Vector VecValue )
{
    std::string str = std::to_string( VecValue.x ) + " " + std::to_string( VecValue.y ) + " " + std::to_string( VecValue.z );
    return str.c_str();
}

const char* CKeyValueLogic :: GetValueOfKey( CBaseEntity* pTarget, string_t m_szKey )
{
    const char* szKey = STRING( m_szKey );

    if( !szKey ) return "";
    else if( FStrEq( szKey, "classname" ) ) return STRING( pTarget->pev->classname );
    else if( FStrEq( szKey, "globalname" ) ) return STRING( pTarget->pev->globalname );
    else if( FStrEq( szKey, "model" ) ) return STRING( pTarget->pev->model );
    else if( FStrEq( szKey, "viewmodel" ) ) return STRING( pTarget->pev->viewmodel );
    else if( FStrEq( szKey, "weaponmodel" ) ) return STRING( pTarget->pev->weaponmodel );
    else if( FStrEq( szKey, "team" ) ) return STRING( pTarget->pev->team );
    else if( FStrEq( szKey, "target" ) ) return STRING( pTarget->pev->target );
    else if( FStrEq( szKey, "targetname" ) ) return STRING( pTarget->pev->targetname );
    else if( FStrEq( szKey, "netname" ) ) return STRING( pTarget->pev->netname );
    else if( FStrEq( szKey, "message" ) ) return STRING( pTarget->pev->message );
    else if( FStrEq( szKey, "noise" ) ) return STRING( pTarget->pev->noise );
    else if( FStrEq( szKey, "noise1" ) ) return STRING( pTarget->pev->noise1 );
    else if( FStrEq( szKey, "noise2" ) ) return STRING( pTarget->pev->noise2 );
    else if( FStrEq( szKey, "noise3" ) ) return STRING( pTarget->pev->noise3 );
    else if( FStrEq( szKey, "fixangle" ) ) return std::to_string( pTarget->pev->fixangle ).c_str();
    else if( FStrEq( szKey, "modelindex" ) ) return std::to_string( pTarget->pev->modelindex ).c_str();
    else if( FStrEq( szKey, "movetype" ) ) return std::to_string( pTarget->pev->movetype ).c_str();
    else if( FStrEq( szKey, "solid" ) ) return std::to_string( pTarget->pev->solid ).c_str();
    else if( FStrEq( szKey, "skin" ) ) return std::to_string( pTarget->pev->skin ).c_str();
    else if( FStrEq( szKey, "body" ) ) return std::to_string( pTarget->pev->body ).c_str();
    else if( FStrEq( szKey, "effects" ) ) return std::to_string( pTarget->pev->effects ).c_str();
    else if( FStrEq( szKey, "light_level" ) ) return std::to_string( pTarget->pev->light_level ).c_str();
    else if( FStrEq( szKey, "sequence" ) ) return std::to_string( pTarget->pev->sequence ).c_str();
    else if( FStrEq( szKey, "gaitsequence" ) ) return std::to_string( pTarget->pev->gaitsequence ).c_str();
    else if( FStrEq( szKey, "rendermode" ) ) return std::to_string( pTarget->pev->rendermode ).c_str();
    else if( FStrEq( szKey, "renderfx" ) ) return std::to_string( pTarget->pev->renderfx ).c_str();
    else if( FStrEq( szKey, "weapons" ) ) return std::to_string( pTarget->pev->weapons ).c_str();
    else if( FStrEq( szKey, "deadflag" ) ) return std::to_string( pTarget->pev->deadflag ).c_str();
    else if( FStrEq( szKey, "button" ) ) return std::to_string( pTarget->pev->button ).c_str();
    else if( FStrEq( szKey, "impulse" ) ) return std::to_string( pTarget->pev->impulse ).c_str();
    else if( FStrEq( szKey, "spawnflags" ) ) return std::to_string( pTarget->pev->spawnflags ).c_str();
    else if( FStrEq( szKey, "flags" ) ) return std::to_string( pTarget->pev->flags ).c_str();
    else if( FStrEq( szKey, "colormap" ) ) return std::to_string( pTarget->pev->colormap ).c_str();
    else if( FStrEq( szKey, "watertype" ) ) return std::to_string( pTarget->pev->watertype ).c_str();
    else if( FStrEq( szKey, "playerclass" ) ) return std::to_string( pTarget->pev->playerclass ).c_str();
    else if( FStrEq( szKey, "weaponanim" ) ) return std::to_string( pTarget->pev->weaponanim ).c_str();
    else if( FStrEq( szKey, "pushmsec" ) ) return std::to_string( pTarget->pev->pushmsec ).c_str();
    else if( FStrEq( szKey, "bInDuck" ) ) return std::to_string( pTarget->pev->bInDuck ).c_str();
    else if( FStrEq( szKey, "flTimeStepSound" ) ) return std::to_string( pTarget->pev->flTimeStepSound ).c_str();
    else if( FStrEq( szKey, "flSwimTime" ) ) return std::to_string( pTarget->pev->flSwimTime ).c_str();
    else if( FStrEq( szKey, "flDuckTime" ) ) return std::to_string( pTarget->pev->flDuckTime ).c_str();
    else if( FStrEq( szKey, "iStepLeft" ) ) return std::to_string( pTarget->pev->iStepLeft ).c_str();
    else if( FStrEq( szKey, "gamestate" ) ) return std::to_string( pTarget->pev->gamestate ).c_str();
    else if( FStrEq( szKey, "oldbuttons" ) ) return std::to_string( pTarget->pev->oldbuttons ).c_str();
    else if( FStrEq( szKey, "groupinfo" ) ) return std::to_string( pTarget->pev->groupinfo ).c_str();
    else if( FStrEq( szKey, "iuser1" ) ) return std::to_string( pTarget->pev->iuser1 ).c_str();
    else if( FStrEq( szKey, "iuser2" ) ) return std::to_string( pTarget->pev->iuser2 ).c_str();
    else if( FStrEq( szKey, "iuser3" ) ) return std::to_string( pTarget->pev->iuser3 ).c_str();
    else if( FStrEq( szKey, "iuser4" ) ) return std::to_string( pTarget->pev->iuser4 ).c_str();
    else if( FStrEq( szKey, "waterlevel" ) ) return std::to_string( (int)pTarget->pev->waterlevel ).c_str();
    else if( FStrEq( szKey, "impacttime" ) ) return std::to_string( pTarget->pev->impacttime ).c_str();
    else if( FStrEq( szKey, "starttime" ) ) return std::to_string( pTarget->pev->starttime ).c_str();
    else if( FStrEq( szKey, "idealpitch" ) ) return std::to_string( pTarget->pev->idealpitch ).c_str();
    else if( FStrEq( szKey, "pitch_speed" ) ) return std::to_string( pTarget->pev->pitch_speed ).c_str();
    else if( FStrEq( szKey, "ideal_yaw" ) ) return std::to_string( pTarget->pev->ideal_yaw ).c_str();
    else if( FStrEq( szKey, "yaw_speed" ) ) return std::to_string( pTarget->pev->yaw_speed ).c_str();
    else if( FStrEq( szKey, "ltime" ) ) return std::to_string( pTarget->pev->ltime ).c_str();
    else if( FStrEq( szKey, "nextthink" ) ) return std::to_string( pTarget->pev->nextthink ).c_str();
    else if( FStrEq( szKey, "gravity" ) ) return std::to_string( pTarget->pev->gravity ).c_str();
    else if( FStrEq( szKey, "friction" ) ) return std::to_string( pTarget->pev->friction ).c_str();
    else if( FStrEq( szKey, "frame" ) ) return std::to_string( pTarget->pev->frame ).c_str();
    else if( FStrEq( szKey, "animtime" ) ) return std::to_string( pTarget->pev->animtime ).c_str();
    else if( FStrEq( szKey, "framerate" ) ) return std::to_string( pTarget->pev->framerate ).c_str();
    else if( FStrEq( szKey, "scale" ) ) return std::to_string( pTarget->pev->scale ).c_str();
    else if( FStrEq( szKey, "renderamt" ) ) return std::to_string( pTarget->pev->renderamt ).c_str();
    else if( FStrEq( szKey, "health" ) ) return std::to_string( pTarget->pev->health ).c_str();
    else if( FStrEq( szKey, "frags" ) ) return std::to_string( pTarget->pev->frags ).c_str();
    else if( FStrEq( szKey, "takedamage" ) ) return std::to_string( pTarget->pev->takedamage ).c_str();
    else if( FStrEq( szKey, "max_health" ) ) return std::to_string( pTarget->pev->max_health ).c_str();
    else if( FStrEq( szKey, "teleport_time" ) ) return std::to_string( pTarget->pev->teleport_time ).c_str();
    else if( FStrEq( szKey, "armortype" ) ) return std::to_string( pTarget->pev->armortype ).c_str();
    else if( FStrEq( szKey, "armorvalue" ) ) return std::to_string( pTarget->pev->armorvalue ).c_str();
    else if( FStrEq( szKey, "dmg_take" ) ) return std::to_string( pTarget->pev->dmg_take ).c_str();
    else if( FStrEq( szKey, "dmg_save" ) ) return std::to_string( pTarget->pev->dmg_save ).c_str();
    else if( FStrEq( szKey, "dmg" ) ) return std::to_string( pTarget->pev->dmg ).c_str();
    else if( FStrEq( szKey, "dmgtime" ) ) return std::to_string( pTarget->pev->dmgtime ).c_str();
    else if( FStrEq( szKey, "speed" ) ) return std::to_string( pTarget->pev->speed ).c_str();
    else if( FStrEq( szKey, "air_finished" ) ) return std::to_string( pTarget->pev->air_finished ).c_str();
    else if( FStrEq( szKey, "pain_finished" ) ) return std::to_string( pTarget->pev->pain_finished ).c_str();
    else if( FStrEq( szKey, "radsuit_finished" ) ) return std::to_string( pTarget->pev->radsuit_finished ).c_str();
    else if( FStrEq( szKey, "maxspeed" ) ) return std::to_string( pTarget->pev->maxspeed ).c_str();
    else if( FStrEq( szKey, "fov" ) ) return std::to_string( pTarget->pev->fov ).c_str();
    else if( FStrEq( szKey, "flFallVelocity" ) ) return std::to_string( pTarget->pev->flFallVelocity ).c_str();
    else if( FStrEq( szKey, "fuser1" ) ) return std::to_string( pTarget->pev->fuser1 ).c_str();
    else if( FStrEq( szKey, "fuser2" ) ) return std::to_string( pTarget->pev->fuser2 ).c_str();
    else if( FStrEq( szKey, "fuser3" ) ) return std::to_string( pTarget->pev->fuser3 ).c_str();
    else if( FStrEq( szKey, "fuser4" ) ) return std::to_string( pTarget->pev->fuser4 ).c_str();
    else if( FStrEq( szKey, "origin" ) ) return ToString( pTarget->pev->origin );
    else if( FStrEq( szKey, "oldorigin" ) ) return ToString( pTarget->pev->oldorigin );
    else if( FStrEq( szKey, "velocity" ) ) return ToString( pTarget->pev->velocity );
    else if( FStrEq( szKey, "basevelocity" ) ) return ToString( pTarget->pev->basevelocity );
    else if( FStrEq( szKey, "clbasevelocity" ) ) return ToString( pTarget->pev->clbasevelocity );
    else if( FStrEq( szKey, "movedir" ) ) return ToString( pTarget->pev->movedir );
    else if( FStrEq( szKey, "angles" ) ) return ToString( pTarget->pev->angles );
    else if( FStrEq( szKey, "avelocity" ) ) return ToString( pTarget->pev->avelocity );
    else if( FStrEq( szKey, "punchangle" ) ) return ToString( pTarget->pev->punchangle );
    else if( FStrEq( szKey, "v_angle" ) ) return ToString( pTarget->pev->v_angle );
    else if( FStrEq( szKey, "endpos" ) ) return ToString( pTarget->pev->endpos );
    else if( FStrEq( szKey, "startpos" ) ) return ToString( pTarget->pev->startpos );
    else if( FStrEq( szKey, "absmin" ) ) return ToString( pTarget->pev->absmin );
    else if( FStrEq( szKey, "absmax" ) ) return ToString( pTarget->pev->absmax );
    else if( FStrEq( szKey, "mins" ) ) return ToString( pTarget->pev->mins );
    else if( FStrEq( szKey, "maxs" ) ) return ToString( pTarget->pev->maxs );
    else if( FStrEq( szKey, "size" ) ) return ToString( pTarget->pev->size );
    else if( FStrEq( szKey, "rendercolor" ) ) return ToString( pTarget->pev->rendercolor );
    else if( FStrEq( szKey, "view_ofs" ) ) return ToString( pTarget->pev->view_ofs );
    else if( FStrEq( szKey, "vuser1" ) ) return ToString( pTarget->pev->vuser1 );
    else if( FStrEq( szKey, "vuser2" ) ) return ToString( pTarget->pev->vuser2 );
    else if( FStrEq( szKey, "vuser3" ) ) return ToString( pTarget->pev->vuser3 );
    else if( FStrEq( szKey, "vuser4" ) ) return ToString( pTarget->pev->vuser4 );

    /*
	byte controller[NUM_ENT_CONTROLLERS]; // bone controller setting (0..255)
	byte blending[NUM_ENT_BLENDERS];	  // blending amount between sub-sequences (0..255)
	edict_t* chain; // Entity pointer when linked into a linked list
	edict_t* dmg_inflictor;
	edict_t* enemy;	 //!< Used by some NPC movement code in the engine to determine movement destination
	edict_t* aiment; // entity pointer when MOVETYPE_FOLLOW
	edict_t* owner;
	edict_t* groundentity;
	edict_t* pContainingEntity;
	edict_t* euser1;
	edict_t* euser2;
	edict_t* euser3;
	edict_t* euser4;
    */

    if( pTarget->KeyValueDatai > 0 )
    {
        for( int i = 0; i < pTarget->KeyValueDatai; ++i )
        {
            if( FStrEq( szKey, STRING( pTarget->KeyValueKeys[ i ] ) ) )
            {
                return STRING( pTarget->KeyValueValues[ i ] );
            }
        }
    }
    return "";
}
