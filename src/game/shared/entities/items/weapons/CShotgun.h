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

#include "cbase.h"

// special deathmatch shotgun spreads
constexpr Vector VECTOR_CONE_DM_SHOTGUN{0.08716f, 0.04362f, 0.00f};          // 10 degrees by 5 degrees
constexpr Vector VECTOR_CONE_DM_DOUBLESHOTGUN{0.17365f, 0.04362f, 0.00f}; // 20 degrees by 5 degrees

enum shotgun_e
{
    SHOTGUN_IDLE = 0,
    SHOTGUN_FIRE,
    SHOTGUN_FIRE2,
    SHOTGUN_RELOAD,
    SHOTGUN_PUMP,
    SHOTGUN_START_RELOAD,
    SHOTGUN_DRAW,
    SHOTGUN_HOLSTER,
    SHOTGUN_IDLE4,
    SHOTGUN_IDLE_DEEP
};

class CShotgun : public CBasePlayerWeapon
{
    DECLARE_CLASS( CShotgun, CBasePlayerWeapon );
    DECLARE_DATAMAP();

public:
    void OnCreate() override;
    void Precache() override;
    bool GetWeaponInfo( WeaponInfo& info ) override;
    void IncrementAmmo( CBasePlayer* pPlayer ) override;

    void PrimaryAttack() override;
    void SecondaryAttack() override;
    bool Deploy() override;
    void Reload() override;
    void WeaponIdle() override;
    void ItemPostFrame() override;
    int m_fInReload; // TODO: not used, remove
    float m_flNextReload;
    int m_iShell;

    bool UseDecrement() override
    {
#if defined( CLIENT_WEAPONS )
        return true;
#else
        return false;
#endif
    }

private:
    unsigned short m_usDoubleFire;
    unsigned short m_usSingleFire;
};
