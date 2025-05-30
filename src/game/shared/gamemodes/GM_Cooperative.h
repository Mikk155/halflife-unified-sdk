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

#include "GameMode.h"
#include "GM_Multiplayer.h"

class GM_Cooperative : public GM_Multiplayer
{
    DECLARE_CLASS( GM_Cooperative, GM_Multiplayer );
    GM_Cooperative() = default;

public:

    static constexpr char GameModeName[] = "coop";
    const char* GetName() const override { return GameModeName; }
    const char* GetBaseName() const override { return BaseClass::GetName(); }
    const char* GetGameDescription() const override { return "Co-Operative"; }

    void OnClientInit( CBasePlayer* player ) override;
};
