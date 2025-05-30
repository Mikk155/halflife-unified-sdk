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
#include "CPython.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS( weapon_357, CPython );

inline bool UseLaserSight()
{
    return g_cfg.GetValue<bool>( "revolver_laser_sight"sv, false );
}

void CPython::OnCreate()
{
    CBasePlayerWeapon::OnCreate();
    m_iId = WEAPON_PYTHON;
    m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;
    m_WorldModel = pev->model = MAKE_STRING( "models/w_357.mdl" );
}

bool CPython::GetWeaponInfo( WeaponInfo& info )
{
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].AmmoType = "357";
    info.AttackModeInfo[0].MagazineSize = PYTHON_MAX_CLIP;
    info.Slot = 1;
    info.Position = 1;
    info.Id = WEAPON_PYTHON;
    info.Weight = PYTHON_WEIGHT;

    return true;
}

void CPython::IncrementAmmo( CBasePlayer* pPlayer )
{
    if( pPlayer->GiveAmmo( 1, "357" ) >= 0 )
    {
        pPlayer->EmitSound( CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM );
    }
}

void CPython::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( "models/v_357.mdl" );
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/p_357.mdl" );

    PrecacheModel( "models/w_357ammobox.mdl" );
    PrecacheSound( "items/9mmclip1.wav" );

    PrecacheSound( "weapons/357_reload1.wav" );
    PrecacheSound( "weapons/357_cock1.wav" );
    PrecacheSound( "weapons/357_shot1.wav" );
    PrecacheSound( "weapons/357_shot2.wav" );

    m_usFirePython = PRECACHE_EVENT( 1, "events/python.sc" );
}

bool CPython::Deploy()
{
    if( UseLaserSight() )
    {
        // enable laser sight geometry.
        pev->body = 1;
    }
    else
    {
        pev->body = 0;
    }

    return DefaultDeploy( "models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python" );
}

void CPython::Holster()
{
    m_fInReload = false; // cancel any reload in progress.

    if( m_pPlayer->m_iFOV != 0 )
    {
        SecondaryAttack();
    }

    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
    SendWeaponAnim( PYTHON_HOLSTER );
}

void CPython::SecondaryAttack()
{
    if( !UseLaserSight() )
    {
        return;
    }

    if( m_pPlayer->m_iFOV != 0 )
    {
        m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
    }
    else if( m_pPlayer->m_iFOV != 40 )
    {
        m_pPlayer->m_iFOV = 40;
    }

    m_flNextSecondaryAttack = 0.5;
}

void CPython::PrimaryAttack()
{
    // don't fire underwater
    if( m_pPlayer->pev->waterlevel == WaterLevel::Head )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = 0.15;
        return;
    }

    if( GetMagazine1() <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = 0.15;
        }

        return;
    }

    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

    AdjustMagazine1( -1 );

    m_pPlayer->pev->effects = m_pPlayer->pev->effects | EF_MUZZLEFLASH;

    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );


    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    Vector vecDir;
    vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer, m_pPlayer->random_seed );

    int flags;
#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFirePython, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

    if( 0 == GetMagazine1() && m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
        // HEV suit - indicate out of ammo condition
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", 0 );

    m_flNextPrimaryAttack = 0.75;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CPython::Reload()
{
    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
        return;

    if( m_pPlayer->m_iFOV != 0 )
    {
        m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
    }

    DefaultReload( 6, PYTHON_RELOAD, 2.0 );
}

void CPython::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    int iAnim;
    float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
    if( flRand <= 0.5 )
    {
        iAnim = PYTHON_IDLE1;
        m_flTimeWeaponIdle = ( 70.0 / 30.0 );
    }
    else if( flRand <= 0.7 )
    {
        iAnim = PYTHON_IDLE2;
        m_flTimeWeaponIdle = ( 60.0 / 30.0 );
    }
    else if( flRand <= 0.9 )
    {
        iAnim = PYTHON_IDLE3;
        m_flTimeWeaponIdle = ( 88.0 / 30.0 );
    }
    else
    {
        iAnim = PYTHON_FIDGET;
        m_flTimeWeaponIdle = ( 170.0 / 30.0 );
    }

    SendWeaponAnim( iAnim );
}

void CPython::ItemPostFrame()
{
    const int currentBody = UseLaserSight() ? 1 : 0;

    // Check if we need to reset the laser sight.
    if( currentBody != pev->body )
    {
        pev->body = currentBody;

        m_flTimeWeaponIdle = 0;

        if( !UseLaserSight() && m_pPlayer->m_iFOV != 0 )
        {
            m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
        }
    }

    BaseClass::ItemPostFrame();
}

class CPythonAmmo : public CBasePlayerAmmo
{
public:
    void OnCreate() override
    {
        CBasePlayerAmmo::OnCreate();
        m_AmmoAmount = AMMO_357BOX_GIVE;
        m_AmmoName = MAKE_STRING( "357" );
        pev->model = MAKE_STRING( "models/w_357ammobox.mdl" );
    }
};

LINK_ENTITY_TO_CLASS( ammo_357, CPythonAmmo );
