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

#pragma once

#include "cbase.h"

#define NAME_FILTER_LIMIT 32

namespace TEI_FLAGS
{
	const int NONE = 0;

	namespace spawnflags {
		const int START_ON = ( 1 << 0 );
	}
	namespace status_filter {

		const int ONLY_LIVING = 1;
		const int ONLY_DEAD = 2;
	}
	namespace run_mode {
		const int RUN_ONCE = 0;
		const int ONCE_MULTI_THREADED = 1;
		const int TOGGLE = 2;
	}
}

/*
enum TEI_FLAGS : int
{
	NONE = 0,
	START_ON = ( 1 << 0 ),
	ONLY_LIVING = 1,
	ONLY_DEAD = 2,
	CLASSNAME = ( 1 << 0 ),
	TARGETNAME = ( 1 << 1 )
};
*/

class CTriggerEntityIterator : public CBaseEntity
{
	DECLARE_CLASS( CTriggerEntityIterator, CBaseEntity );
	DECLARE_DATAMAP();

	public:
		void Spawn() override;
		void IteratorThink();
		void IteratorFind();
		void IteratorTrigger( CBaseEntity* pTarget );
		bool KeyValue( KeyValueData* pkvd ) override;
		void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

	private:
		string_t name_filter;
		string_t classname_filter;
		string_t trigger_after_run;
		int status_filter = TEI_FLAGS::NONE;
		int triggerstate = 2;
		int run_mode = 0;
		int maximum_runs = 0;
		int current_runs = 0;
		float delay_between_runs = 0.5;
		float delay_between_triggers = 0.0;
		float flNextDelayTrigger = 0.0;
};