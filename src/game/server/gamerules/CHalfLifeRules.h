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

#include "CGameRules.h"

/**
 *    @brief rules for the single player Half-Life game
 */
class CHalfLifeRules : public CGameRules
{
public:
    static constexpr char GameModeName[] = "singleplayer";

    CHalfLifeRules() = default;

    const char* GetGameModeName() const override { return GameModeName; }

    void Think() override;
    bool IsAllowedToSpawn( CBaseEntity* pEntity ) override;

    bool GetNextBestWeapon( CBasePlayer* pPlayer, CBasePlayerWeapon* pCurrentWeapon, bool alwaysSearch = false ) override;

    bool ClientConnected( edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128] ) override;
    void InitHUD( CBasePlayer* pl ) override;
    void ClientDisconnected( edict_t* pClient ) override;

    bool FPlayerCanRespawn( CBasePlayer* pPlayer ) override;
    float FlPlayerSpawnTime( CBasePlayer* pPlayer ) override;

    bool AllowAutoTargetCrosshair() override;

    int IPointsForKill( CBasePlayer* pAttacker, CBasePlayer* pKilled ) override;
    void PlayerKilled( CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* inflictor ) override;
    void DeathNotice( CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* inflictor ) override;

    void PlayerGotItem( CBasePlayer* player, CBaseItem* item ) override;

    int DeadPlayerWeapons( CBasePlayer* pPlayer ) override;

    int DeadPlayerAmmo( CBasePlayer* pPlayer ) override;

    const char* GetTeamID( CBaseEntity* pEntity ) override { return ""; }
    int PlayerRelationship( CBasePlayer* pPlayer, CBaseEntity* pTarget ) override;
};
