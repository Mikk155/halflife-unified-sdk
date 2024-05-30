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

// If not set, the entity is removed after it's triggered
#define SF_TEMPENT_REPEATABLE  ( 1 << 0 )

// All players (not activator) can see this FX
#define SF_TEMPENT_ALL_PLAYERS ( 1 << 1 )

// Activator can see this FX
#define SF_TEMPENT_ACTIVATOR   ( 1 << 2 )

class CTempEntity : public CBaseDelay
{
	DECLARE_CLASS( CTempEntity, CBaseDelay );
	DECLARE_DATAMAP();

	public:
		void Spawn() override;
		void Precache() override;
		bool KeyValue( KeyValueData* pkvd ) override;
		void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

		bool CanSeeFX( CBasePlayer* player, CBaseEntity* pActivator );
		Vector GetPosition( string_t m_szTarget, CBaseEntity* pActivator, CBaseEntity* pCaller, Vector VecOffSet );
		void ModelSet( const char* szModel );
		virtual void RenderEffect( CBasePlayer* player );

		// Model index for pev->model or default model
		int m_iModel;

		// Model name for pev->model or default model
		string_t m_szModel;

		// OffSet vector to apply to VecStartPos
		Vector m_VecOffSet;
		// OffSet vector to apply to VecEndPos
		Vector m_VecOffSetEnd;

		// Start position of the effect (pev->netname)
		Vector VecStartPos;

		// End position of the effect if needed (pev->message)
		Vector VecEndPos;
};
