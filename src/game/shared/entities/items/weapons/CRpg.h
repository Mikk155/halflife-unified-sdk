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

class CLaserSpot;

enum rpg_e
{
    RPG_IDLE = 0,
    RPG_FIDGET,
    RPG_RELOAD,       // to reload
    RPG_FIRE2,       // to empty
    RPG_HOLSTER1,  // loaded
    RPG_DRAW1,       // loaded
    RPG_HOLSTER2,  // unloaded
    RPG_DRAW_UL,   // unloaded
    RPG_IDLE_UL,   // unloaded idle
    RPG_FIDGET_UL, // unloaded fidget
};

class CRpg : public CBasePlayerWeapon
{
    DECLARE_CLASS( CRpg, CBasePlayerWeapon );
    DECLARE_DATAMAP();

public:
    void OnCreate() override;
    void Precache() override;
    void Reload() override;
    bool GetWeaponInfo( WeaponInfo& info ) override;
    void IncrementAmmo( CBasePlayer* pPlayer ) override;

    bool Deploy() override;
    bool CanHolster() override;
    void Holster() override;

    void PrimaryAttack() override;
    void SecondaryAttack() override;
    void WeaponIdle() override;

    void UpdateSpot();
    bool ShouldWeaponIdle() override { return true; }

    CLaserSpot* m_pSpot;
    bool m_fSpotActive;
    int m_cActiveRockets; // how many missiles in flight from this launcher right now?

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
    unsigned short m_usRpg;
};
