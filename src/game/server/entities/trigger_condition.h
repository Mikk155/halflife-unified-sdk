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

#define SF_DONT_USE_X       ( 1 << 0 ) // Start Off
#define SF_DONT_USE_X       ( 1 << 1 ) // Do not use X (Vector)
#define SF_DONT_USE_Y       ( 1 << 2 ) // Do not use Y (Vector)
#define SF_DONT_USE_Z       ( 1 << 3 ) // Do not use Z (Vector)
#define SF_DONT_USE_W       ( 1 << 4 ) // Don't use W (A)
#define SF_DONT_USE_W       ( 1 << 5 ) // Cyclic; no toggle, Check once when fired - do not toggle
#define SF_KEEP_ACTIVATOR   ( 1 << 6 ) // Keep '!activator'
#define SF_IGNORE_1STRESULT ( 1 << 6 ) // Ignore initial result

#define CHECK_ALTERNATINGLY 0   // Fire true/false alternatingly
#define CHECK_ONLY_FALSE 1      // Only wait after false
#define CHECK_ONLY_TRUE 2       // Only wait after true
#define CHECK_TRUE_FALSE 3      // Always fire for both

enum TriggerConditionCheck : int
{
    EQUAL = 0,              // Ent keyvalue equals mappers value
    NOT_EQUAL = 1,          // Ent keyvalue does not equal mappers value
    LESS = 2,               // Ent keyvalue is smaller than mappers value
	GREATER = 3,            // Ent keyvalue is greater than mappers value
	LESS_OR_EQUAL = 4,      // Ent keyvalue is less than or equal to mappers value
	GREATER_OR_EQUAL = 5,   // Ent keyvalue is greater than or equal to mappers value
	LOGICAL_AND = 6         // (Ent keyvalue & mappers value) does not equal 0 (Flag check)
};

class CTriggerCondition : public CPointEntity
{
	DECLARE_CLASS( CTriggerCondition, CPointEntity );
	DECLARE_DATAMAP();

    public:
        void Spawn() override;
        bool KeyValue(KeyValueData* pkvd) override;
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;


    private:
        string_t m_iszValueName;    // Monitored key
        string_t m_iszSourceName;   // Compare-entity
        string_t m_iszSourceKey;    // Compare-key
        string_t m_iszCheckValue;   // Compare-value (alternative)
        int m_iCheckType = TriggerConditionCheck::EQUAL;  // Comparator; mon. val. -> comp.-val.
        float m_fCheckInterval; // Check-interval (seconds)
        int m_iCheckBehaviour = // Constant mode trigger behaviour
};

/*
	target(target_destination) : "Monitored entity"
	netname(string) : "Target for 'true'-case"
	message(string) : "Target for 'false'-case"
*/