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

class CKeyValueLogic : public CPointEntity
{
	DECLARE_CLASS( CKeyValueLogic, CPointEntity );

public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

private:
};

void CKeyValueLogic :: Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
    if( !FStringNull( pev->netname ) )
    {
        FireTargets( STRING( pev->netname ), pActivator, pCaller, useType, value );
    }
}
