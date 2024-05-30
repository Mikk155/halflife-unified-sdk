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

#define EF_CTEMPENTLIGHT_DLIGHT 0
#define EF_CTEMPENTLIGHT_ELIGHT 1
#define EF_CTEMPENTLIGHT_DELIGHT 2

/**
 *	@brief dynamic light, effect world, minor entity effect and point entity light, no world effect
 */
class CTempEntLight : public CTempEntity
{
	DECLARE_CLASS( CTempEntLight, CTempEntity );
	DECLARE_DATAMAP();

	public:
		bool KeyValue( KeyValueData* pkvd ) override;
		void RenderEffect( CBasePlayer* player ) override;

	private:
		int m_iRadius = 32;
		float m_fRadius = 32;
		int m_iDecay;
		float m_fDecay;
		int m_iLife = 8;
		short m_Attachment;
};

LINK_ENTITY_TO_CLASS( env_effect_light, CTempEntLight );

BEGIN_DATAMAP( CTempEntLight )
	DEFINE_FIELD( m_iRadius, FIELD_INTEGER ),
	DEFINE_FIELD( m_fRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_iDecay, FIELD_INTEGER ),
	DEFINE_FIELD( m_fDecay, FIELD_FLOAT ),
	DEFINE_FIELD( m_iLife, FIELD_INTEGER ),
END_DATAMAP();

bool CTempEntLight :: KeyValue( KeyValueData* pkvd )
{
	if( FStrEq( pkvd->szKeyName, "m_fRadius" ) )
	{
		m_iRadius = atoi( pkvd->szValue );
		m_fRadius = atof( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_fDecay" ) )
	{
		m_iDecay = atoi( pkvd->szValue );
		m_fDecay = atof( pkvd->szValue );
	}
	else if( FStrEq( pkvd->szKeyName, "m_iLife" ) )
	{
		m_iLife = atoi( pkvd->szValue );
	}
	else
	{
		return BaseClass::KeyValue( pkvd );
	}
	return true;
}

void CTempEntLight :: RenderEffect( CBasePlayer* player )
{
	switch( (int)pev->health )
	{
		case EF_CTEMPENTLIGHT_DLIGHT:
		case EF_CTEMPENTLIGHT_DELIGHT:
			MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, nullptr, player );
				WRITE_BYTE( TE_DLIGHT );
				WRITE_COORD( VecStartPos.x );
				WRITE_COORD( VecStartPos.y );
				WRITE_COORD( VecStartPos.z );
				WRITE_BYTE( m_iRadius );
				WRITE_BYTE( (int)pev->rendercolor.x );
				WRITE_BYTE( (int)pev->rendercolor.y );
				WRITE_BYTE( (int)pev->rendercolor.z );
				WRITE_BYTE( m_iLife );
				WRITE_BYTE( m_iDecay );
			MESSAGE_END();
		break;
	}

	switch( (int)pev->health )
	{
		case EF_CTEMPENTLIGHT_ELIGHT:
		case EF_CTEMPENTLIGHT_DELIGHT:
			MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, nullptr, player );
				WRITE_BYTE( TE_ELIGHT );
				WRITE_SHORT( m_Attachment );
				WRITE_COORD( VecStartPos.x );
				WRITE_COORD( VecStartPos.y );
				WRITE_COORD( VecStartPos.z );
				WRITE_COORD( m_fRadius );
				WRITE_BYTE( (int)pev->rendercolor.x );
				WRITE_BYTE( (int)pev->rendercolor.y );
				WRITE_BYTE( (int)pev->rendercolor.z );
				WRITE_BYTE( m_iLife );
				WRITE_COORD( m_fDecay );
			MESSAGE_END();
		break;
	}
}
