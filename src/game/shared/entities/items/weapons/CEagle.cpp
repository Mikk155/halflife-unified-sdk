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
#include "UserMessages.h"

#ifndef CLIENT_DLL
#include "weapons/CEagleLaser.h"
#endif

#include "CEagle.h"

BEGIN_DATAMAP( CEagle )
    DEFINE_FIELD( m_bSpotVisible, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_bLaserActive, FIELD_BOOLEAN ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( weapon_eagle, CEagle );

void CEagle::OnCreate()
{
    BaseClass::OnCreate();
    m_iId = WEAPON_EAGLE;
    m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;
    m_WorldModel = pev->model = MAKE_STRING( "models/w_desert_eagle.mdl" );
}

void CEagle::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( "models/v_desert_eagle.mdl" );
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/p_desert_eagle.mdl" );
    m_iShell = PrecacheModel( "models/shell.mdl" );
    PrecacheSound( "weapons/desert_eagle_fire.wav" );
    PrecacheSound( "weapons/desert_eagle_reload.wav" );
    PrecacheSound( "weapons/desert_eagle_sight.wav" );
    PrecacheSound( "weapons/desert_eagle_sight2.wav" );
    m_usFireEagle = PRECACHE_EVENT( 1, "events/eagle.sc" );
}

bool CEagle::Deploy()
{
    m_bSpotVisible = true;

    return DefaultDeploy( 
        "models/v_desert_eagle.mdl", "models/p_desert_eagle.mdl",
        EAGLE_DRAW,
        "onehanded" );
}

void CEagle::Holster()
{
    m_fInReload = false;

#ifndef CLIENT_DLL
    if( m_pLaser )
    {
        UTIL_Remove( m_pLaser );
        m_pLaser = nullptr;
        m_bSpotVisible = false;
    }
#endif

    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );

    SendWeaponAnim( EAGLE_HOLSTER );
}

void CEagle::WeaponIdle()
{
#ifndef CLIENT_DLL
    UpdateLaser();
#endif

    ResetEmptySound();

    // Update autoaim
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() && 0 != GetMagazine1() )
    {
        const float flNextIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

        int iAnim;

        if( m_bLaserActive )
        {
            if( flNextIdle > 0.5 )
            {
                iAnim = EAGLE_IDLE5;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
            }
            else
            {
                iAnim = EAGLE_IDLE4;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
            }
        }
        else
        {
            if( flNextIdle <= 0.3 )
            {
                iAnim = EAGLE_IDLE1;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
            }
            else
            {
                if( flNextIdle > 0.6 )
                {
                    iAnim = EAGLE_IDLE3;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.633;
                }
                else
                {
                    iAnim = EAGLE_IDLE2;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
                }
            }
        }

        SendWeaponAnim( iAnim );
    }
}

void CEagle::PrimaryAttack()
{
    if( m_pPlayer->pev->waterlevel == WaterLevel::Head )
    {
        PlayEmptySound();

        // Note: this is broken in original Op4 since it uses gpGlobals->time when using prediction
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
        return;
    }

    if( GetMagazine1() <= 0 )
    {
        if( !m_fInReload )
        {
            if( m_fFireOnEmpty )
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
            }
            // Don't do this because it glitches the animation
            // else
            //{
            //    Reload();
            // }
        }

        return;
    }

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

    AdjustMagazine1( -1 );

    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL
    if( m_pLaser && m_bLaserActive )
    {
        m_pLaser->pev->effects |= EF_NODRAW;
        m_pLaser->SetThink( &CEagleLaser::Revive );
        m_pLaser->pev->nextthink = gpGlobals->time + 0.6;
    }
#endif

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecSrc = m_pPlayer->GetGunPosition();

    Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    const float flSpread = m_bLaserActive ? 0.001 : 0.1;

    const Vector vecSpread = m_pPlayer->FireBulletsPlayer( 
        1,
        vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ),
        8192.0, BULLET_PLAYER_EAGLE, 0, 0,
        m_pPlayer, m_pPlayer->random_seed );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + ( m_bLaserActive ? 0.5 : 0.22 );

    int flags;
#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    PLAYBACK_EVENT_FULL( 
        flags, m_pPlayer->edict(), m_usFireEagle, 0,
        g_vecZero, g_vecZero,
        vecSpread.x, vecSpread.y,
        0, 0,
        static_cast<int>( GetMagazine1() == 0 ), 0 );

    if( 0 == GetMagazine1() )
    {
        if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
            m_pPlayer->SetSuitUpdate( "!HEV_AMO0", SUIT_REPEAT_OK );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );

#ifndef CLIENT_DLL
    UpdateLaser();
#endif
}

void CEagle::SecondaryAttack()
{
#ifndef CLIENT_DLL
    m_bLaserActive = !m_bLaserActive;

    if( !m_bLaserActive )
    {
        if( m_pLaser )
        {
            UTIL_Remove( m_pLaser );
            m_pLaser = nullptr;

            EmitSound( CHAN_WEAPON, "weapons/desert_eagle_sight2.wav", VOL_NORM, ATTN_NORM );
        }
    }
#endif

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CEagle::Reload()
{
    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
    {
        return;
    }

    const bool bResult = DefaultReload( EAGLE_MAX_CLIP, 0 != GetMagazine1() ? EAGLE_RELOAD : EAGLE_RELOAD_NOSHOT, 1.5 );

#ifndef CLIENT_DLL
    // Only turn it off if we're actually reloading
    if( bResult && m_pLaser && m_bLaserActive )
    {
        m_pLaser->pev->effects |= EF_NODRAW;
        m_pLaser->SetThink( &CEagleLaser::Revive );
        m_pLaser->pev->nextthink = gpGlobals->time + 1.6;

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5;
    }
#endif

    if( bResult )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );
    }
}

void CEagle::UpdateLaser()
{
#ifndef CLIENT_DLL
    // Don't turn on the laser if we're in the middle of a reload.
    if( m_fInReload )
    {
        return;
    }

    if( m_bLaserActive && m_bSpotVisible )
    {
        if( !m_pLaser )
        {
            m_pLaser = CEagleLaser::CreateSpot();

            EmitSound( CHAN_WEAPON, "weapons/desert_eagle_sight.wav", VOL_NORM, ATTN_NORM );
        }

        UTIL_MakeVectors( m_pPlayer->pev->v_angle );

        Vector vecSrc = m_pPlayer->GetGunPosition();

        Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192.0;

        TraceResult tr;

        UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr );

        m_pLaser->SetOrigin( tr.vecEndPos );
    }
#endif
}

bool CEagle::GetWeaponInfo( WeaponInfo& info )
{
    info.AttackModeInfo[0].AmmoType = "357";
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].MagazineSize = EAGLE_MAX_CLIP;
    info.Slot = 1;
    info.Position = 2;
    info.Id = WEAPON_EAGLE;
    info.Weight = EAGLE_WEIGHT;
    return true;
}

void CEagle::IncrementAmmo( CBasePlayer* pPlayer )
{
    if( pPlayer->GiveAmmo( 1, "357" ) >= 0 )
    {
        pPlayer->EmitSound( CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM );
    }
}

void CEagle::GetWeaponData( weapon_data_t& data )
{
    BaseClass::GetWeaponData( data );

    data.iuser1 = static_cast<int>( m_bLaserActive );
}

void CEagle::SetWeaponData( const weapon_data_t& data )
{
    BaseClass::SetWeaponData( data );

    m_bLaserActive = data.iuser1 != 0;
}

class CEagleAmmo : public CBasePlayerAmmo
{
public:
    void OnCreate() override
    {
        CBasePlayerAmmo::OnCreate();
        m_AmmoAmount = AMMO_EAGLE_GIVE;
        m_AmmoName = MAKE_STRING( "357" );
        // TODO: could probably use a better model
        pev->model = MAKE_STRING( "models/w_9mmclip.mdl" );
    }
};

LINK_ENTITY_TO_CLASS( ammo_eagleclip, CEagleAmmo );
