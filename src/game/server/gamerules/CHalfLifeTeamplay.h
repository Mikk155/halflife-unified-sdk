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

#include <memory>

#include "CHalfLifeMultiplay.h"

#define MAX_TEAMNAME_LENGTH 16
#define MAX_TEAMS 32

#define TEAMPLAY_TEAMLISTLENGTH MAX_TEAMS* MAX_TEAMNAME_LENGTH

class CHalfLifeTeamplay : public CHalfLifeMultiplay
{
public:
    static constexpr char GameModeName[] = "teamplay";

    CHalfLifeTeamplay();

    const char* GetGameModeName() const override { return GameModeName; }

    void ClientUserInfoChanged( CBasePlayer* pPlayer, char* infobuffer ) override;
    bool IsTeamplay() override { return true; }
    bool FPlayerCanTakeDamage( CBasePlayer* pPlayer, CBaseEntity* pAttacker ) override;
    int PlayerRelationship( CBasePlayer* pPlayer, CBaseEntity* pTarget ) override;
    const char* GetTeamID( CBaseEntity* pEntity ) override;
    bool ShouldAutoAim( CBasePlayer* pPlayer, CBaseEntity* target ) override;
    int IPointsForKill( CBasePlayer* pAttacker, CBasePlayer* pKilled ) override;
    void InitHUD( CBasePlayer* pl ) override;
    void DeathNotice( CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* inflictor ) override;
    void PlayerKilled( CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* inflictor ) override;
    void Think() override;
    int GetTeamIndex( const char* pTeamName ) override;
    const char* GetIndexedTeamName( int teamIndex ) override;
    bool IsValidTeam( const char* pTeamName ) override;
    const char* SetDefaultPlayerTeam( CBasePlayer* pPlayer ) override;
    void ChangePlayerTeam( CBasePlayer* pPlayer, const char* pTeamName, bool bKill, bool bGib ) override;

private:
    void RecountTeams( bool bResendInfo = false );
    const char* TeamWithFewestPlayers();

    bool m_DisableDeathMessages;
    bool m_DisableDeathPenalty;
    bool m_teamLimit; // This means the server set only some teams as valid
    char m_szTeamList[TEAMPLAY_TEAMLISTLENGTH];
};
