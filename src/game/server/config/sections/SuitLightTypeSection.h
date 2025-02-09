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

#include <string>

#include "cdll_dll.h"
#include "ServerConfigContext.h"

#include "config/GameConfig.h"

#include "utils/JSONSystem.h"
#include "utils/shared_utils.h"

/**
 *    @brief Allows a configuration file to specify the player's suit light type.
 */
class SuitLightTypeSection final : public GameConfigSection<ServerConfigContext>
{
public:
    explicit SuitLightTypeSection() = default;

    std::string_view GetName() const override final { return "SuitLightType"; }

    json::value_t GetType() const override final { return json::value_t::string; }

    std::string GetSchema() const override final
    {
        const auto types = []()
        {
            std::string types;

            bool first = true;

            for( const auto& type : SuitLightTypes )
            {
                if( !first )
                {
                    types += ',';
                }
                else
                {
                    first = false;
                }

                types += '"';
                types += type.Name;
                types += '"';
            }

            return types;
        }();

        return fmt::format( R"("enum": [{}])", types );
    }

    bool TryParse( GameConfigContext<ServerConfigContext>& context ) const override final
    {
        const auto type = context.Input.get<std::string>();

        if( !type.empty() )
        {
            if( auto value = SuitLightTypeFromString( type ); value )
            {
                context.Data.State.m_LightType = *value;
            }
        }

        return true;
    }
};
