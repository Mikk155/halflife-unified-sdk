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

#define SF_START_ON ( 1 << 0 )

#define FILTER_STATUS_ONLY_LIVING 1
#define FILTER_STATUS_ONLY_DEAD 2

#define RUN_MODE_RUN_ONCE 0
#define RUN_MODE_ONCE_MULTI_THREADED 1
#define RUN_MODE_TOGGLE_ON_OFF 2

class CTriggerEntityIterator : public CBaseDelay
{
	DECLARE_CLASS( CTriggerEntityIterator, CBaseDelay );
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
		int status_filter = 0;
		int triggerstate = 2;
		int run_mode = 0;
		int maximum_runs = 0;
		int current_runs = 0;
		float delay_between_runs = 0.5;
		float delay_between_triggers = 0.0;
		float flNextDelayTrigger = 0.0;
		bool IsThinking = false;
};