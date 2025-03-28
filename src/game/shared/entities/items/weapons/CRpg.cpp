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
#include "CRpg.h"
#include "UserMessages.h"

#ifndef CLIENT_DLL
#include "items/weapons/CLaserSpot.h"
#include "items/weapons/CRpgRocket.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_rpg, CRpg );

BEGIN_DATAMAP( CRpg )
    DEFINE_FIELD( m_fSpotActive, FIELD_BOOLEAN ),
    DEFINE_FIELD( m_cActiveRockets, FIELD_INTEGER ),
END_DATAMAP();

void CRpg::OnCreate()
{
    CBasePlayerWeapon::OnCreate();
    m_iId = WEAPON_RPG;

    m_fSpotActive = true;
    m_iDefaultAmmo = RPG_DEFAULT_GIVE;
    m_WorldModel = pev->model = MAKE_STRING( "models/w_rpg.mdl" );
}

void CRpg::Reload()
{
    if( GetMagazine1() == 1 )
    {
        // don't bother with any of this if don't need to reload.
        return;
    }

    if( m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) <= 0 )
        return;

    // because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
    // weapons code is constantly calling into this function, but is often denied because
    // a) missiles are in flight, but the LTD is on
    // or
    // b) player is totally out of ammo and has nothing to switch to, and should be allowed to
    //    shine the designator around
    //
    // Set the next attack time into the future so that WeaponIdle will get called more often
    // than reload, allowing the RPG LTD to be updated

    m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );

    if( 0 != m_cActiveRockets && m_fSpotActive )
    {
        // no reloading when there are active missiles tracking the designator.
        // ward off future autoreload attempts by setting next attack time into the future for a bit.
        return;
    }

#ifndef CLIENT_DLL
    if( m_pSpot && m_fSpotActive )
    {
        m_pSpot->Suspend( 2.1 );
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
    }
#endif

    if( GetMagazine1() == 0 )
    {
        const bool iResult = DefaultReload( RPG_MAX_CLIP, RPG_RELOAD, 2 );

        if( iResult )
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
    }
}

void CRpg::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/v_rpg.mdl" );
    PrecacheModel( "models/p_rpg.mdl" );

    PrecacheSound( "items/9mmclip1.wav" );

    UTIL_PrecacheOther( "laser_spot" );
    UTIL_PrecacheOther( "rpg_rocket" );

    PrecacheSound( "weapons/rocketfire1.wav" );
    PrecacheSound( "weapons/glauncher.wav" ); // alternative fire sound

    m_usRpg = PRECACHE_EVENT( 1, "events/rpg.sc" );
}

bool CRpg::GetWeaponInfo( WeaponInfo& info )
{
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].AmmoType = "rockets";
    info.AttackModeInfo[0].MagazineSize = RPG_MAX_CLIP;
    info.Slot = 3;
    info.Position = 0;
    info.Id = WEAPON_RPG;
    info.Weight = RPG_WEIGHT;

    return true;
}

void CRpg::IncrementAmmo( CBasePlayer* pPlayer )
{
    if( pPlayer->GiveAmmo( 1, "rockets" ) >= 0 )
    {
        pPlayer->EmitSound( CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM );
    }
}

bool CRpg::Deploy()
{
    if( GetMagazine1() == 0 )
    {
        return DefaultDeploy( "models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg" );
    }

    return DefaultDeploy( "models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW1, "rpg" );
}

bool CRpg::CanHolster()
{
    if( m_fSpotActive && 0 != m_cActiveRockets )
    {
        // can't put away while guiding a missile.
        return false;
    }

    return true;
}

void CRpg::Holster()
{
    m_fInReload = false; // cancel any reload in progress.

    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    SendWeaponAnim( RPG_HOLSTER1 );

#ifndef CLIENT_DLL
    if( m_pSpot )
    {
        UTIL_Remove( m_pSpot );
        m_pSpot = nullptr;
    }
#endif
}

void CRpg::PrimaryAttack()
{
    if( 0 != GetMagazine1() )
    {
        m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
        // player "shoot" animation
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        UTIL_MakeVectors( m_pPlayer->pev->v_angle );
        Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

        CRpgRocket* pRocket = CRpgRocket::CreateRpgRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );

        UTIL_MakeVectors( m_pPlayer->pev->v_angle ); // RpgRocket::Create stomps on globals, so remake.
        pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif

        // firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
        // Ken signed up for this as a global change (sjb)

        int flags;
#if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usRpg );

        AdjustMagazine1( -1 );

        m_flNextPrimaryAttack = GetNextAttackDelay( 1.5 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
    }
    else
    {
        PlayEmptySound();
    }
    UpdateSpot();
}

void CRpg::SecondaryAttack()
{
    m_fSpotActive = !m_fSpotActive;

#ifndef CLIENT_DLL
    if( !m_fSpotActive && m_pSpot )
    {
        UTIL_Remove( m_pSpot );
        m_pSpot = nullptr;
    }
#endif

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CRpg::WeaponIdle()
{
    if( ( m_pPlayer->pev->button & ( IN_ATTACK | IN_ATTACK2 ) ) == 0 )
    {
        ResetEmptySound();
    }

    UpdateSpot();

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    if( 0 != m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
    {
        int iAnim;
        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
        if( flRand <= 0.75 || m_fSpotActive )
        {
            if( GetMagazine1() == 0 )
                iAnim = RPG_IDLE_UL;
            else
                iAnim = RPG_IDLE;

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
        }
        else
        {
            if( GetMagazine1() == 0 )
                iAnim = RPG_FIDGET_UL;
            else
                iAnim = RPG_FIDGET;

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.1;
        }

        SendWeaponAnim( iAnim );
    }
    else
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
    }
}

void CRpg::UpdateSpot()
{

#ifndef CLIENT_DLL
    // Don't turn on the laser if we're in the middle of a reload.
    if( m_fInReload )
    {
        return;
    }

    if( m_fSpotActive )
    {
        if( !m_pSpot )
        {
            m_pSpot = CLaserSpot::CreateSpot();
        }

        UTIL_MakeVectors( m_pPlayer->pev->v_angle );
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAiming = gpGlobals->v_forward;

        TraceResult tr;
        UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr );

        m_pSpot->SetOrigin( tr.vecEndPos );
    }
#endif
}

void CRpg::GetWeaponData( weapon_data_t& data )
{
    data.fuser2 = static_cast<float>( m_fSpotActive );
    data.fuser3 = m_cActiveRockets;
}

void CRpg::SetWeaponData( const weapon_data_t& data )
{
    m_fSpotActive = data.fuser2 != 0;
    m_cActiveRockets = data.fuser3;
}

class CRpgAmmo : public CBasePlayerAmmo
{
public:
    void OnCreate() override
    {
        CBasePlayerAmmo::OnCreate();
        m_AmmoAmount = AMMO_RPGCLIP_GIVE;
        m_AmmoName = MAKE_STRING( "rockets" );
        pev->model = MAKE_STRING( "models/w_rpgammo.mdl" );
    }
};

LINK_ENTITY_TO_CLASS( ammo_rpgclip, CRpgAmmo );
