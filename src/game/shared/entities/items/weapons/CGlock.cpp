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
#include "CGlock.h"

LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock );

void CGlock::OnCreate()
{
    CBasePlayerWeapon::OnCreate();
    m_iId = WEAPON_GLOCK;
    m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
    m_WorldModel = pev->model = MAKE_STRING( "models/w_9mmhandgun.mdl" );
}

void CGlock::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( "models/v_9mmhandgun.mdl" );
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/p_9mmhandgun.mdl" );

    m_iShell = PrecacheModel( "models/shell.mdl" ); // brass shell

    PrecacheSound( "items/9mmclip1.wav" );
    PrecacheSound( "items/9mmclip2.wav" );

    PrecacheSound( "weapons/pl_gun1.wav" ); // silenced handgun
    PrecacheSound( "weapons/pl_gun2.wav" ); // silenced handgun
    PrecacheSound( "weapons/pl_gun3.wav" ); // handgun

    m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
    m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

bool CGlock::GetWeaponInfo( WeaponInfo& info )
{
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].AmmoType = "9mm";
    info.AttackModeInfo[0].MagazineSize = GLOCK_MAX_CLIP;
    info.Slot = 1;
    info.Position = 0;
    info.Id = WEAPON_GLOCK;
    info.Weight = GLOCK_WEIGHT;

    return true;
}

void CGlock::IncrementAmmo( CBasePlayer* pPlayer )
{
    if( pPlayer->GiveAmmo( 1, "9mm" ) >= 0 )
    {
        pPlayer->EmitSound( CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM );
    }
}

bool CGlock::Deploy()
{
    // pev->body = 1;
    return DefaultDeploy( "models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded" );
}

void CGlock::SecondaryAttack()
{
    GlockFire( 0.1, 0.2, false );
}

void CGlock::PrimaryAttack()
{
    GlockFire( 0.01, 0.3, true );
}

void CGlock::GlockFire( float flSpread, float flCycleTime, bool fUseAutoAim )
{
    if( GetMagazine1() <= 0 )
    {
        // if (m_fFireOnEmpty)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.2 );
        }

        return;
    }

    AdjustMagazine1( -1 );

    m_pPlayer->pev->effects = m_pPlayer->pev->effects | EF_MUZZLEFLASH;

    int flags;

#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    // player "shoot" animation
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    // silenced
    if( pev->body == 1 )
    {
        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
    }
    else
    {
        // non-silenced
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
    }

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecAiming;

    if( fUseAutoAim )
    {
        vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
    }
    else
    {
        vecAiming = gpGlobals->v_forward;
    }

    Vector vecDir;
    vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0,
        g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, ( GetMagazine1() == 0 ) ? 1 : 0, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if( 0 == GetMagazine1() && m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
        // HEV suit - indicate out of ammo condition
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", 0 );

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CGlock::Reload()
{
    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
        return;

    bool iResult;

    if( GetMagazine1() == 0 )
        iResult = DefaultReload( 17, GLOCK_RELOAD, 1.5 );
    else
        iResult = DefaultReload( 17, GLOCK_RELOAD_NOT_EMPTY, 1.5 );

    if( iResult )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
    }
}

void CGlock::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    // only idle if the slid isn't back
    if( GetMagazine1() != 0 )
    {
        int iAnim;
        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

        if( flRand <= 0.3 + 0 * 0.75 )
        {
            iAnim = GLOCK_IDLE3;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
        }
        else if( flRand <= 0.6 + 0 * 0.875 )
        {
            iAnim = GLOCK_IDLE1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
        }
        else
        {
            iAnim = GLOCK_IDLE2;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
        }
        SendWeaponAnim( iAnim );
    }
}

class CGlockAmmo : public CBasePlayerAmmo
{
public:
    void OnCreate() override
    {
        CBasePlayerAmmo::OnCreate();
        m_AmmoAmount = AMMO_GLOCKCLIP_GIVE;
        m_AmmoName = MAKE_STRING( "9mm" );
        pev->model = MAKE_STRING( "models/w_9mmclip.mdl" );
    }
};

LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo );
