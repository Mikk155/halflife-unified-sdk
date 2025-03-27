﻿using Utilities.Entities;
using Utilities.Tools.UpgradeTool;
using System.Collections.Immutable;

namespace MapUpgrader.Upgrades.Common
{
    /// <summary>
    /// Update entity's old USE_TYPE systems with our new global one.
    /// </summary>
    internal sealed class UpdateUseTypes : MapUpgrade
    {
        private static readonly ImmutableArray<string> ClassNames = ImmutableArray.Create(
            "trigger_relay",
            "trigger_auto",
            "trigger_ctfgeneric",
            "game_team_master",
            "trigger_entity_iterator"
        );

        protected override void ApplyCore( MapUpgradeContext context )
        {
            foreach( var entity in context.Map.Entities.Where( e => ClassNames.Contains( e.ClassName ) ) )
            {
                if( !entity.ContainsKey( "triggerstate" ) )
                {
                    continue;
                }

                var trigger_type = entity.GetInteger( "triggerstate" ) switch
                {
                    0 => 0, // Off
                    1 => 1, // On
                    2 => 3, // TOGGLE
                    3 => 2, // Sven coop's USE_SET
                    4 => 4, // Sven coop's USE_KILL
                    _ => 1 // On (Default in the game's code)
                };

                entity.Remove( "triggerstate" );
                entity.SetInteger( "m_UseType", trigger_type );
            }
        }
    }
}
