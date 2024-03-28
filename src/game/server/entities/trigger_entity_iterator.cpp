/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "cbase.h"
#include "trigger_entity_iterator.h"

BEGIN_DATAMAP( CTriggerEntityIterator )
	DEFINE_FUNCTION( IteratorThink ),
	DEFINE_FIELD( run_mode, FIELD_INTEGER ),
	DEFINE_FIELD( name_filter, FIELD_STRING ),
	DEFINE_FIELD( triggerstate, FIELD_INTEGER ),
	DEFINE_FIELD( maximum_runs, FIELD_INTEGER ),
	DEFINE_FIELD( current_runs, FIELD_INTEGER ),
	DEFINE_FIELD( status_filter, FIELD_INTEGER ),
	DEFINE_FIELD( classname_filter, FIELD_STRING ),
	DEFINE_FIELD( trigger_after_run, FIELD_STRING ),
	DEFINE_FIELD( delay_between_runs, FIELD_FLOAT ),
	DEFINE_FIELD( delay_between_triggers, FIELD_FLOAT ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( trigger_entity_iterator, CTriggerEntityIterator );

void CTriggerEntityIterator :: Spawn()
{
	pev->solid = SOLID_NOT;

	if( FBitSet( pev->spawnflags, TEI_FLAGS::spawnflags::START_ON ) )
	{
		SetThink( &CTriggerEntityIterator::IteratorThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CTriggerEntityIterator :: IteratorThink()
{
	if( run_mode == TEI_FLAGS::run_mode::TOGGLE && ( maximum_runs == 0 || current_runs < maximum_runs ) )
	{
		pev->nextthink = gpGlobals->time + delay_between_runs;

		if( maximum_runs != 0 )
		{
			current_runs++;
		}
	}

	IteratorFind();
}

bool CTriggerEntityIterator :: KeyValue( KeyValueData* pkvd )
{
	if( FStrEq( pkvd->szKeyName, "maximum_runs" ) )
	{
		maximum_runs = atoi( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "name_filter" ) )
	{
		name_filter = ALLOC_STRING( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "classname_filter" ) )
	{
		classname_filter = ALLOC_STRING( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "status_filter" ) )
	{
		status_filter = atoi( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "triggerstate" ) )
	{
		triggerstate = atoi( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "run_mode" ) )
	{
		run_mode = atoi( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "maximum_runs" ) )
	{
		maximum_runs = atoi( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "delay_between_triggers" ) )
	{
		delay_between_triggers = atof( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "delay_between_runs" ) )
	{
		delay_between_runs = atof( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "trigger_after_run" ) )
	{
		trigger_after_run = ALLOC_STRING( pkvd->szValue );
	}
	else
	{
		return CBaseEntity::KeyValue( pkvd );
	}

	return true;
}

void CTriggerEntityIterator :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if( run_mode == TEI_FLAGS::run_mode::TOGGLE )
	{
		if( &CTriggerEntityIterator::IteratorThink )
		{
			SetThink( &CTriggerEntityIterator::IteratorThink );
			pev->nextthink = gpGlobals->time + 0.1;
		}
		else
		{
			SetThink( nullptr );
		}
	}
	else
	{
		IteratorFind();
	}
}

void CTriggerEntityIterator :: IteratorFind()
{
	if( !FStringNull( name_filter ) )
	{
		for( auto pTarget : UTIL_FindEntitiesByTargetname( STRING( name_filter ) ) )
		{
			if( pTarget )
			{
				IteratorTrigger( pTarget );
			}
		}
	}
	else if( !FStringNull( classname_filter ) )
	{
		for( auto pTarget : UTIL_FindEntitiesByClassname( STRING( classname_filter ) ) )
		{
			if( pTarget )
			{
				IteratorTrigger( pTarget );
			}
		}
	}

	flNextDelayTrigger = 0.0;

	if( !FStringNull( trigger_after_run ) )
	{
		FireTargets( STRING( trigger_after_run ), this, this, USE_TOGGLE, 0 );
	}
}

void CTriggerEntityIterator :: IteratorTrigger( CBaseEntity* pTarget )
{
	if(( !FStringNull( classname_filter ) && !pTarget->ClassnameIs( STRING( classname_filter ) ) )
	or ( !FStringNull( name_filter ) && !FStrEq( STRING( pTarget->pev->targetname ), STRING( name_filter ) ) )
	or ( status_filter == TEI_FLAGS::status_filter::ONLY_DEAD && pTarget->IsAlive() )
	or ( status_filter == TEI_FLAGS::status_filter::ONLY_LIVING && !pTarget->IsAlive() )
	) { return; }

	USE_TYPE UseType = static_cast<USE_TYPE>( triggerstate );

/*
	if( UseType == USE_UNSET )
	{
		UseType = pTarget->m_UseType;
	}
*/

	if( delay_between_triggers > 0.0 )
	{
		flNextDelayTrigger += delay_between_triggers;

		// create a temp object to fire at a later time
		CBaseDelay* pTemp = g_EntityDictionary->Create<CBaseDelay>("entity_iterator_delayed");

		pTemp->pev->nextthink = gpGlobals->time + flNextDelayTrigger;

		pTemp->SetThink(&CBaseDelay::DelayThink);

		pTemp->pev->button = (int)UseType;
		pTemp->pev->target = pev->target;
		pTemp->m_hActivator = pTarget;

	}
	else
	{
		FireTargets( STRING( pev->target ), pTarget, this, UseType, 0 );
	}
}
