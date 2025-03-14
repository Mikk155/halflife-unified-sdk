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
#include "HudReplacementSystem.h"
#include "WeaponDataSystem.h"

bool HudReplacementSystem::Initialize()
{
    g_NetworkData.RegisterHandler( "HudReplacement", this );
    return true;
}

void HudReplacementSystem::HandleNetworkDataBlock( NetworkDataBlock& block )
{
    if( block.Mode == NetworkDataMode::Serialize )
    {
        block.Data = HudReplacementFileName;
    }
    else
    {
        HudReplacementFileName = block.Data.get<std::string>();
    }
}

void HudReplacementSystem::SetWeaponHudReplacementFiles( WeaponHudReplacements&& fileNames )
{
    for( auto& [key, value] : fileNames )
    {
        g_WeaponData.SetWeaponHudConfigFileName( key, std::move( value ) );
    }
}
