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

/**
 *	@brief random tracers with gravity, ricochet sprite
 */
class CTempEntSparks : public CTempEntity
{
	DECLARE_CLASS( CTempEntSparks, CTempEntity );

	public:
		void RenderEffect( CBasePlayer* player ) override;
};

LINK_ENTITY_TO_CLASS( env_effect_sparks, CTempEntSparks );

void CTempEntSparks :: RenderEffect( CBasePlayer* player )
{
	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, nullptr, player );
		WRITE_BYTE( TE_SPARKS );
		WRITE_COORD( VecStartPos.x );
		WRITE_COORD( VecStartPos.y );
		WRITE_COORD( VecStartPos.z );
	MESSAGE_END();
}
