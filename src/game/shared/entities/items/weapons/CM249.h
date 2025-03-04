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

#pragma once

enum M249Anim
{
    M249_SLOWIDLE = 0,
    M249_IDLE2,
    M249_RELOAD_START,
    M249_RELOAD_END,
    M249_HOLSTER,
    M249_DRAW,
    M249_SHOOT1,
    M249_SHOOT2,
    M249_SHOOT3
};

class CM249 : public CBasePlayerWeapon
{
    DECLARE_CLASS( CM249, CBasePlayerWeapon );
    DECLARE_DATAMAP();

public:
    void OnCreate() override;

    void Precache() override;

    bool Deploy() override;

    void Holster() override;

    void WeaponIdle() override;

    void PrimaryAttack() override;

    void Reload() override;

    bool GetWeaponInfo( WeaponInfo& info ) override;

    void IncrementAmmo( CBasePlayer* pPlayer ) override;

    bool UseDecrement() override
    {
#if defined( CLIENT_WEAPONS )
        return true;
#else
        return false;
#endif
    }

    void GetWeaponData( weapon_data_t& data ) override;

    void SetWeaponData( const weapon_data_t& data ) override;

private:
    static int RecalculateBody( int iClip );

private:
    unsigned short m_usFireM249;

    float m_flNextAnimTime;

    int m_iShell;

    // Used to alternate between ejecting shells and links.
    bool m_bAlternatingEject = false;
    int m_iLink;
    int m_iSmoke;
    int m_iFire;

    bool m_bReloading;
    float m_flReloadStartTime;
    float m_flReloadStart;
};
