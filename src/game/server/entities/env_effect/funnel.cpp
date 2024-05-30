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

#include "CTempEntity.h"

#define SF_FUNNEL_REVERSE 1 // funnel effect repels particles instead of attracting them.

/**
 *	@brief Funnel
 */
class CTempEntFunnel : public CTempEntity
{
	DECLARE_CLASS( CTempEntFunnel, CTempEntity );

	public:
		void Precache() override;
		void RenderEffect( CBasePlayer* player ) override;
};

LINK_ENTITY_TO_CLASS( env_effect_funnel, CTempEntFunnel );

void CTempEntFunnel :: Precache()
{
	BaseClass::ModelSet( "sprites/flare6.spr" );
}

void CTempEntFunnel :: RenderEffect( CBasePlayer* player )
{
	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, nullptr, player );
		WRITE_BYTE( TE_LARGEFUNNEL );
		WRITE_COORD( VecStartPos.x );
		WRITE_COORD( VecStartPos.y );
		WRITE_COORD( VecStartPos.z );
		WRITE_SHORT( m_iModel );
		WRITE_SHORT( ( (int)pev->health == SF_FUNNEL_REVERSE ? 1 : 0 ) );
	MESSAGE_END();
}
