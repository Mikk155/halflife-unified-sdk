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

#include "config/GameConfig.h"

#include "utils/JSONSystem.h"

/**
 *    @brief Simple section that echoes a message provided in the config file.
 */
template <typename DataContext>
class EchoSection final : public GameConfigSection<DataContext>
{
public:
    explicit EchoSection() = default;

    std::string_view GetName() const override final { return "Echo"; }

    json::value_t GetType() const override final { return json::value_t::string; }

    std::string GetSchema() const override final { return {}; }

    bool TryParse( GameConfigContext<DataContext>& context ) const override final
    {
        context.Logger.info( "{}", context.Input.template get<std::string>() );
        return true;
    }
};
