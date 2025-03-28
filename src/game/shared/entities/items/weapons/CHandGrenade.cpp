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
#include "CHandGrenade.h"

#define HANDGRENADE_PRIMARY_VOLUME 450

LINK_ENTITY_TO_CLASS( weapon_handgrenade, CHandGrenade );

void CHandGrenade::OnCreate()
{
    CBasePlayerWeapon::OnCreate();
    m_iId = WEAPON_HANDGRENADE;
    m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
#ifndef CLIENT_DLL
    pev->dmg = g_cfg.GetValue<float>( "plr_hand_grenade"sv, 100, this );
#endif
    m_WorldModel = pev->model = MAKE_STRING( "models/w_grenade.mdl" );
}

void CHandGrenade::Precache()
{
    CBasePlayerWeapon::Precache();
    PrecacheModel( STRING( m_WorldModel ) );
    PrecacheModel( "models/v_grenade.mdl" );
    PrecacheModel( "models/p_grenade.mdl" );
}

bool CHandGrenade::GetWeaponInfo( WeaponInfo& info )
{
    info.Name = STRING( pev->classname );
    info.AttackModeInfo[0].AmmoType = "Hand Grenade";
    info.AttackModeInfo[0].MagazineSize = WEAPON_NOCLIP;
    info.Slot = 4;
    info.Position = 0;
    info.Id = WEAPON_HANDGRENADE;
    info.Weight = HANDGRENADE_WEIGHT;
    info.Flags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return true;
}

void CHandGrenade::IncrementAmmo( CBasePlayer* pPlayer )
{
#ifndef CLIENT_DLL
    // TODO: not sure how useful this is given that the player has to have this weapon for this method to be called
    if( !pPlayer->HasNamedPlayerWeapon( "weapon_handgrenade" ) )
    {
        pPlayer->GiveNamedItem( "weapon_handgrenade" );
    }
#endif

    if( pPlayer->GiveAmmo( 1, "Hand Grenade" ) >= 0 )
    {
        pPlayer->EmitSound( CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM );
    }
}

bool CHandGrenade::Deploy()
{
    m_flReleaseThrow = -1;
    return DefaultDeploy( "models/v_grenade.mdl", "models/p_grenade.mdl", HANDGRENADE_DRAW, "crowbar" );
}

bool CHandGrenade::CanHolster()
{
    // can only holster hand grenades when not primed!
    return ( m_flStartThrow == 0 );
}

void CHandGrenade::Holster()
{
    // Stop any throw that was in process so players don't blow themselves or somebody else up when the weapon is deployed again.
    m_flStartThrow = 0;

    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( 0 != m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
    {
        SendWeaponAnim( HANDGRENADE_HOLSTER );
    }
    else
    {
        // no more grenades!
        m_pPlayer->ClearWeaponBit( m_iId );
        SetThink( &CHandGrenade::DestroyItem );
        pev->nextthink = gpGlobals->time + 0.1;
    }

    m_pPlayer->EmitSound( CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM );
}

void CHandGrenade::PrimaryAttack()
{
    if( 0 == m_flStartThrow && m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) > 0 )
    {
        m_flStartThrow = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( HANDGRENADE_PINPULL );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CHandGrenade::WeaponIdle()
{
    if( m_flReleaseThrow == 0 && 0 != m_flStartThrow )
        m_flReleaseThrow = gpGlobals->time;

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
        return;

    if( 0 != m_flStartThrow )
    {
        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if( angThrow.x < 0 )
            angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

        float flVel = ( 90 - angThrow.x ) * 4;
        if( flVel > 500 )
            flVel = 500;

        UTIL_MakeVectors( angThrow );

        Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        // alway explode 3 seconds after the pin was pulled
        float time = m_flStartThrow - gpGlobals->time + 3.0;
        if( time < 0 )
            time = 0;

        CGrenade::ShootTimed( m_pPlayer, vecSrc, vecThrow, time );

        if( flVel < 500 )
        {
            SendWeaponAnim( HANDGRENADE_THROW1 );
        }
        else if( flVel < 1000 )
        {
            SendWeaponAnim( HANDGRENADE_THROW2 );
        }
        else
        {
            SendWeaponAnim( HANDGRENADE_THROW3 );
        }

        // player "shoot" animation
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        // m_flReleaseThrow = 0;
        m_flStartThrow = 0;
        m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

        m_pPlayer->AdjustAmmoByIndex( m_iPrimaryAmmoType, -1 );

        if( 0 == m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
        {
            // just threw last grenade
            // set attack times in the future, and weapon idle in the future so we can see the whole throw
            // animation, weapon idle will automatically retire the weapon for us.
            m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 ); // ensure that the animation can finish playing
        }
        return;
    }
    else if( m_flReleaseThrow > 0 )
    {
        // we've finished the throw, restart.
        m_flStartThrow = 0;

        if( 0 != m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
        {
            SendWeaponAnim( HANDGRENADE_DRAW );
        }
        else
        {
            RetireWeapon();
            return;
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
        m_flReleaseThrow = -1;
        return;
    }

    if( 0 != m_pPlayer->GetAmmoCountByIndex( m_iPrimaryAmmoType ) )
    {
        int iAnim;
        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
        if( flRand <= 0.75 )
        {
            iAnim = HANDGRENADE_IDLE;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
        }
        else
        {
            iAnim = HANDGRENADE_FIDGET;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
        }

        SendWeaponAnim( iAnim );
    }
}

void CHandGrenade::GetWeaponData( weapon_data_t& data )
{
    data.fuser2 = m_flStartThrow;
    data.fuser3 = m_flReleaseThrow;
}

void CHandGrenade::SetWeaponData( const weapon_data_t& data )
{
    m_flStartThrow = data.fuser2;
    m_flReleaseThrow = data.fuser3;
}
