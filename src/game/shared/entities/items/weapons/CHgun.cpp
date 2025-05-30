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
#include "CHgun.h"
#include "aliens/hornet.h"
#include "UserMessages.h"

enum firemode_e
{
    FIREMODE_TRACK = 0,
    FIREMODE_FAST
};

LINK_ENTITY_TO_CLASS( weapon_hornetgun, CHgun );

BEGIN_DATAMAP( CHgun )
    DEFINE_FIELD( m_flRechargeTime, FIELD_TIME ),
END_DATAMAP();

bool CHgun::IsUseable()
{
    return true;
}

void CHgun::OnCreate()
{
    CBasePlayerWeapon::OnCreate();
    m_iId = WEAPON_HORNETGUN;
    m_iDefaultAmmo = HIVEHAND_DEFAULT_GIVE;
    m_WorldModel = pev->model = MAKE_STRING( "models/w_hgun.mdl" );
}

void CHgun::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( "models/v_hgun.mdl" );
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/p_hgun.mdl" );

    m_usHornetFire = PRECACHE_EVENT( 1, "events/firehornet.sc" );

    UTIL_PrecacheOther( "hornet" );
}

void CHgun::AddToPlayer( CBasePlayer* pPlayer )
{
#ifndef CLIENT_DLL
    if( g_GameMode->IsMultiplayer() )
    {
        // in multiplayer, all hivehands come full.
        m_iDefaultAmmo = HORNET_MAX_CARRY;
    }
#endif

    CBasePlayerWeapon::AddToPlayer( pPlayer );
}

bool CHgun::GetWeaponInfo( WeaponInfo& info )
{
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].AmmoType = "Hornets";
    info.AttackModeInfo[0].MagazineSize = WEAPON_NOCLIP;
    info.Slot = 3;
    info.Position = 3;
    info.Id = WEAPON_HORNETGUN;
    info.Flags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_SELECTONEMPTY;
    info.Weight = HORNETGUN_WEIGHT;

    return true;
}

bool CHgun::Deploy()
{
    return DefaultDeploy( "models/v_hgun.mdl", "models/p_hgun.mdl", HGUN_UP, "hive" );
}

void CHgun::Holster()
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
    SendWeaponAnim( HGUN_DOWN );

    //!!!HACKHACK - can't select hornetgun if it's empty! no way to get ammo for it, either.
    if( 0 == m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
    {
        m_pPlayer->SetAmmoCountByIndex( m_iPrimaryAmmoType, 1 );
    }
}

void CHgun::PrimaryAttack()
{
    Reload();

    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
    {
        return;
    }

#ifndef CLIENT_DLL
    UTIL_MakeVectors( m_pPlayer->pev->v_angle );

    CBaseEntity* pHornet = CBaseEntity::Create( "hornet",
        m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12,
        m_pPlayer->pev->v_angle,
        m_pPlayer );

    pHornet->pev->velocity = gpGlobals->v_forward * 300;

    m_flRechargeTime = gpGlobals->time + 0.5;
#endif

    const bool wasLastShot = m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) == 1;

    m_pPlayer->AdjustAmmoByIndex( m_iPrimaryAmmoType, -1 );

    m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

    int flags;
#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    if( wasLastShot &&
        m_pPlayer->m_LastWeaponDataUpdateTime != 0 && m_pPlayer->m_LastWeaponDataUpdateTime < m_LastRechargeTime )
    {
        flags &= ~FEV_NOTHOST;
    }

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usHornetFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );


    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_flNextPrimaryAttack = GetNextAttackDelay( 0.25 );

    if( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
    {
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CHgun::SecondaryAttack()
{
    Reload();

    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
    {
        return;
    }

    // Wouldn't be a bad idea to completely predict these, since they fly so fast...
#ifndef CLIENT_DLL
    CBaseEntity* pHornet;
    Vector vecSrc;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle );

    vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12;

    m_iFirePhase++;
    switch ( m_iFirePhase )
    {
    case 1:
        vecSrc = vecSrc + gpGlobals->v_up * 8;
        break;
    case 2:
        vecSrc = vecSrc + gpGlobals->v_up * 8;
        vecSrc = vecSrc + gpGlobals->v_right * 8;
        break;
    case 3:
        vecSrc = vecSrc + gpGlobals->v_right * 8;
        break;
    case 4:
        vecSrc = vecSrc + gpGlobals->v_up * -8;
        vecSrc = vecSrc + gpGlobals->v_right * 8;
        break;
    case 5:
        vecSrc = vecSrc + gpGlobals->v_up * -8;
        break;
    case 6:
        vecSrc = vecSrc + gpGlobals->v_up * -8;
        vecSrc = vecSrc + gpGlobals->v_right * -8;
        break;
    case 7:
        vecSrc = vecSrc + gpGlobals->v_right * -8;
        break;
    case 8:
        vecSrc = vecSrc + gpGlobals->v_up * 8;
        vecSrc = vecSrc + gpGlobals->v_right * -8;
        m_iFirePhase = 0;
        break;
    }

    pHornet = CBaseEntity::Create( "hornet", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer );
    pHornet->pev->velocity = gpGlobals->v_forward * 1200;
    pHornet->pev->angles = UTIL_VecToAngles( pHornet->pev->velocity );

    pHornet->SetClassification( "player_bioweapon" );

    pHornet->SetThink( &CHornet::StartDart );

    m_flRechargeTime = gpGlobals->time + 0.5;
#endif

    int flags;
#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    const bool wasLastShot = m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) == 1;

    if( wasLastShot &&
        m_pPlayer->m_LastWeaponDataUpdateTime != 0 && m_pPlayer->m_LastWeaponDataUpdateTime < m_LastRechargeTime )
    {
        flags &= ~FEV_NOTHOST;
    }

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usHornetFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

    m_pPlayer->AdjustAmmoByIndex( m_iPrimaryAmmoType, -1 );
    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CHgun::Reload()
{
#ifndef CLIENT_DLL
    int ammoCount = m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType );

    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) >= HORNET_MAX_CARRY )
        return;

    bool updatedAmmo = false;

    while( ammoCount < HORNET_MAX_CARRY && m_flRechargeTime < gpGlobals->time )
    {
        ++ammoCount;
        m_flRechargeTime += 0.5;
        updatedAmmo = true;
    }

    m_pPlayer->SetAmmoCountByIndex( m_iPrimaryAmmoType, ammoCount );

    if( updatedAmmo )
    {
        m_LastRechargeTime = gpGlobals->time;
    }
#endif
}

void CHgun::WeaponIdle()
{
    Reload();

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    int iAnim;
    float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
    if( flRand <= 0.75 )
    {
        iAnim = HGUN_IDLE1;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * ( 2 );
    }
    else if( flRand <= 0.875 )
    {
        iAnim = HGUN_FIDGETSWAY;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
    }
    else
    {
        iAnim = HGUN_FIDGETSHAKE;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 35.0 / 16.0;
    }
    SendWeaponAnim( iAnim );
}
