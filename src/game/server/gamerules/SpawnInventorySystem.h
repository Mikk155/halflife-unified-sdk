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

#include <spdlog/logger.h>

#include "PlayerInventory.h"

/**
 *    @brief Stores the inventory for players when they (re-)spawn.
 */
class SpawnInventorySystem final
{
public:
    const PlayerInventory* GetInventory() const { return &m_Inventory; }

    PlayerInventory* GetInventory() { return &m_Inventory; }

    void SetInventory( PlayerInventory&& inventory )
    {
        m_Inventory = std::move( inventory );
    }

private:
    PlayerInventory m_Inventory;
};

inline SpawnInventorySystem g_SpawnInventory;
