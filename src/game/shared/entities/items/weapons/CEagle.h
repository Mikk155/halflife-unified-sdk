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

class CEagleLaser;

enum DesertEagleAnim
{
    EAGLE_IDLE1 = 0,
    EAGLE_IDLE2,
    EAGLE_IDLE3,
    EAGLE_IDLE4,
    EAGLE_IDLE5,
    EAGLE_SHOOT,
    EAGLE_SHOOT_EMPTY,
    EAGLE_RELOAD_NOSHOT,
    EAGLE_RELOAD,
    EAGLE_DRAW,
    EAGLE_HOLSTER
};

class CEagle : public CBasePlayerWeapon
{
    DECLARE_CLASS( CEagle, CBasePlayerWeapon );
    DECLARE_DATAMAP();

public:
    void OnCreate() override;

    void Precache() override;

    bool Deploy() override;

    void Holster() override;

    void WeaponIdle() override;

    // So the laser spot is always updated.
    bool ShouldWeaponIdle() override { return true; }

    void PrimaryAttack() override;

    void SecondaryAttack() override;

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
    void UpdateLaser();

private:
    int m_iShell;
    unsigned short m_usFireEagle;

    bool m_bSpotVisible;
    bool m_bLaserActive;
    CEagleLaser* m_pLaser;
};
