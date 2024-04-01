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
        FireTargets( STRING( pev->message ), pActivator, this, USE_TOGGLE, 0 );
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
    else if( FStrEq( pkvd->szKeyName, "m_iszValueName" ) || FStrEq( pkvd->szKeyName, "m_iszDstValueName" ) )
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

std::string CKeyValueLogic :: ModifyVector( Vector VecOld, float x, float y, float z )
{
    if( !FBitSet( pev->spawnflags, SF_DONT_USE_X ) )
    {
        if( m_iszValueType == KeyValueLogicFlags::ADD )
            VecOld.x = VecOld.x + x;
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
            VecOld.x = VecOld.x - x;
        else if( m_iszValueType == KeyValueLogicFlags::MUL )
        {
            if( x == 0 ) { CBaseEntity::Logger->debug( "{} {} can not multiply by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.x = VecOld.x * x; }
        }
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
        {
            if( x == 0 ) { CBaseEntity::Logger->debug( "{} {} can not divide by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.x = VecOld.x / x; }
        }
    }
    if( !FBitSet( pev->spawnflags, SF_DONT_USE_Y ) )
    {
        if( m_iszValueType == KeyValueLogicFlags::ADD )
            VecOld.y = VecOld.y + y;
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
            VecOld.y = VecOld.y - y;
        else if( m_iszValueType == KeyValueLogicFlags::MUL )
        {
            if( y == 0 ) { CBaseEntity::Logger->debug( "{} {} can not multiply by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.y = VecOld.y * y; }
        }
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
        {
            if( y == 0 ) { CBaseEntity::Logger->debug( "{} {} can not divide by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.y = VecOld.y / y; }
        }
    }
    if( !FBitSet( pev->spawnflags, SF_DONT_USE_Z ) )
    {
        if( m_iszValueType == KeyValueLogicFlags::ADD )
            VecOld.z = VecOld.z + z;
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
            VecOld.z = VecOld.z - z;
        else if( m_iszValueType == KeyValueLogicFlags::MUL )
        {
            if( z == 0 ) { CBaseEntity::Logger->debug( "{} {} can not multiply by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.z = VecOld.z * z; }
        }
        else if( m_iszValueType == KeyValueLogicFlags::DIV )
        {
            if( y == 0 ) { CBaseEntity::Logger->debug( "{} {} can not divide by zero!", STRING(pev->classname), STRING(pev->targetname) ); }
            else { VecOld.z = VecOld.z / z; }
        }
    }

    return std::string( VecOld.ToString() );
}

bool CKeyValueLogic :: IsVector( std::string m_szKey )
{
    std::istringstream iss( m_szKey );
    float x, y, z;
    char delimiter;

    if( !( iss >> x >> delimiter >> y >> delimiter >> z ) )
        return false;

    std::string rest;

    if( iss >> rest )
        return false;

    return true;
}

bool CKeyValueLogic :: IsFloat( const std::string str )
{
    bool puntoDecimal = false;
    bool tieneDigitos = false;

    for (char c : str) {
        if (isdigit(c)) {
            tieneDigitos = true;
        } else if (c == '.') {
            if (puntoDecimal) {
                return false;
            }
            puntoDecimal = true;
        } else {
            return false;
        }
    }

    return tieneDigitos && (puntoDecimal || str.back() != '.');
}

std::string CKeyValueLogic :: FloatConversion( std::string szValue )
{
    if( IsFloat( szValue ) )
    {
        switch( m_iFloatConversion )
        {
            case FC_INT_ROUND:
            {
                szValue = std::to_string( atoi( szValue.c_str() ) );
                break;
            }
            case FC_INT_ROUND_DOWN:
            {
                szValue = std::to_string( (int)floor( atof( szValue.c_str() ) ) );
                break;
            }
            case FC_INT_ROUND_UP:
            {
                szValue = std::to_string( (int)ceil( atof( szValue.c_str() ) ) );
                break;
            }
            case FC_6_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 7 );
                break;
            }
            case FC_5_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 6 );
                break;
            }
            case FC_4_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 5 );
                break;
            }
            case FC_3_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 4 );
                break;
            }
            case FC_2_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 3 );
                break;
            }
            case FC_1_DECIMALS:
            {
                szValue = std::to_string( atof( szValue.c_str() ) ).substr( 0, szValue.find('.') + 2 );
                break;
            }
        }
    }
    return szValue;
}

void CKeyValueLogic :: ModifyValue( CBaseEntity* pTarget )
{
    std::string m_szOldValue = GetValueOfKey( pTarget, m_iszValueName );

    std::string m_szNewValue = std::string( STRING( m_iszNewValue ) );

    if( m_iszValueType == KeyValueLogicFlags::AND )
    {
        int i = atoi( m_szOldValue.c_str() );
        ClearBits( i, atoi( m_szNewValue.c_str() ) );
        m_szNewValue = std::to_string( i );
    }
    else if( m_iszValueType == KeyValueLogicFlags::OR )
    {
        int i = atoi( m_szOldValue.c_str() );
        SetBits( i, atoi( m_szNewValue.c_str() ) );
        m_szNewValue = std::to_string( i );
    }
    else if( m_iszValueType == KeyValueLogicFlags::APPEND )
    {
        std::string str = m_szOldValue;
        for( int i = 0; i < m_iAppendSpaces; i++ ) { str += " "; }
        m_szNewValue = FloatConversion( m_szNewValue );
        m_szNewValue = str + m_szNewValue;
    }
    else if( IsVector( m_szOldValue ) )
    {
        Vector VecOld;
        UTIL_StringToVector( VecOld, m_szOldValue );

        if( IsVector( m_szNewValue ) )
        {
            Vector VecNew;
            UTIL_StringToVector( VecNew, m_szNewValue );

            m_szNewValue = ModifyVector( VecOld, VecNew.x, VecNew.y, VecNew.z );
        }
        else
        {
            float i = atof( m_szNewValue.c_str() );
            m_szNewValue = ModifyVector( VecOld, i, i, i );
        }
    }
    else if( m_iszValueType == KeyValueLogicFlags::ADD )
    {
        m_szNewValue = std::to_string( atof( m_szOldValue.c_str() ) + atof( m_szNewValue.c_str() ) );
    }
    else if( m_iszValueType == KeyValueLogicFlags::MUL )
    {
        if( atof( m_szNewValue.c_str() ) == 0 )
        {
            CBaseEntity::Logger->debug( "{} {} can not multiply by zero!", STRING(pev->classname), STRING(pev->targetname) );
            return;
        }
        m_szNewValue = std::to_string( atof( m_szOldValue.c_str() ) * atof( m_szNewValue.c_str() ) );
    }
    else if( m_iszValueType == KeyValueLogicFlags::SUB )
    {
        m_szNewValue = std::to_string( atof( m_szOldValue.c_str() ) - atof( m_szNewValue.c_str() ) );
    }
    else if( m_iszValueType == KeyValueLogicFlags::DIV )
    {
        if( atoi( m_szNewValue.c_str() ) == 0 )
        {
            CBaseEntity::Logger->debug( "{} {} can not divide by zero!", STRING(pev->classname), STRING(pev->targetname) );
            return;
        }
        m_szNewValue = std::to_string( atoi( m_szOldValue.c_str() ) / atoi( m_szNewValue.c_str() ) );
    }

    m_szNewValue = FloatConversion( m_szNewValue );

    SetKeyValue( pTarget, m_iszValueName, ALLOC_STRING( m_szNewValue.c_str() ) );
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

std::string CKeyValueLogic :: GetValueOfKey( CBaseEntity* pTarget, string_t m_szKey )
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
    else if( FStrEq( szKey, "origin" ) ) return pTarget->pev->origin.ToString();
    else if( FStrEq( szKey, "oldorigin" ) ) return pTarget->pev->oldorigin.ToString();
    else if( FStrEq( szKey, "velocity" ) ) return pTarget->pev->velocity.ToString();
    else if( FStrEq( szKey, "basevelocity" ) ) return pTarget->pev->basevelocity.ToString();
    else if( FStrEq( szKey, "clbasevelocity" ) ) return pTarget->pev->clbasevelocity.ToString();
    else if( FStrEq( szKey, "movedir" ) ) return pTarget->pev->movedir.ToString();
    else if( FStrEq( szKey, "angles" ) ) return pTarget->pev->angles.ToString();
    else if( FStrEq( szKey, "avelocity" ) ) return pTarget->pev->avelocity.ToString();
    else if( FStrEq( szKey, "punchangle" ) ) return pTarget->pev->punchangle.ToString();
    else if( FStrEq( szKey, "v_angle" ) ) return pTarget->pev->v_angle.ToString();
    else if( FStrEq( szKey, "endpos" ) ) return pTarget->pev->endpos.ToString();
    else if( FStrEq( szKey, "startpos" ) ) return pTarget->pev->startpos.ToString();
    else if( FStrEq( szKey, "absmin" ) ) return pTarget->pev->absmin.ToString();
    else if( FStrEq( szKey, "absmax" ) ) return pTarget->pev->absmax.ToString();
    else if( FStrEq( szKey, "mins" ) ) return pTarget->pev->mins.ToString();
    else if( FStrEq( szKey, "maxs" ) ) return pTarget->pev->maxs.ToString();
    else if( FStrEq( szKey, "size" ) ) return pTarget->pev->size.ToString();
    else if( FStrEq( szKey, "rendercolor" ) ) return pTarget->pev->rendercolor.ToString();
    else if( FStrEq( szKey, "view_ofs" ) ) return pTarget->pev->view_ofs.ToString();
    else if( FStrEq( szKey, "vuser1" ) ) return pTarget->pev->vuser1.ToString();
    else if( FStrEq( szKey, "vuser2" ) ) return pTarget->pev->vuser2.ToString();
    else if( FStrEq( szKey, "vuser3" ) ) return pTarget->pev->vuser3.ToString();
    else if( FStrEq( szKey, "vuser4" ) ) return pTarget->pev->vuser4.ToString();

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

