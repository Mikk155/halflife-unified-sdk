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

enum SniperRifleAnim
{
    SNIPERRIFLE_DRAW = 0,
    SNIPERRIFLE_SLOWIDLE,
    SNIPERRIFLE_FIRE,
    SNIPERRIFLE_FIRELASTROUND,
    SNIPERRIFLE_RELOAD1,
    SNIPERRIFLE_RELOAD2,
    SNIPERRIFLE_RELOAD3,
    SNIPERRIFLE_SLOWIDLE2,
    SNIPERRIFLE_HOLSTER
};

/**
 *    @brief Opposing force sniper rifle
 */
class CSniperRifle : public CBasePlayerWeapon
{
    DECLARE_CLASS( CSniperRifle, CBasePlayerWeapon );
    DECLARE_DATAMAP();

public:
    void OnCreate() override;
    void Precache() override;

    bool Deploy() override;

    void Holster() override;

    void WeaponIdle() override;

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

    void ToggleZoom();

private:
    unsigned short m_usSniper;

    bool m_bReloading;
    float m_flReloadStart;
};
