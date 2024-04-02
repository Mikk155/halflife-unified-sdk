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

#define MAX_PRECACHE_ITEMS 32

class CInfoPrecache : public CPointEntity
{
    DECLARE_CLASS( CInfoPrecache, CPointEntity );
    DECLARE_DATAMAP();

    public:
        void Spawn() override;
        void Precache() override;
        bool KeyValue( KeyValueData* pkvd ) override;

    private:
        string_t m_changeTargetName;
        int m_cTargets = 0;
        string_t m_iKey[ MAX_PRECACHE_ITEMS ];
        string_t m_iValue[ MAX_PRECACHE_ITEMS ];
};

BEGIN_DATAMAP( CInfoPrecache )
	DEFINE_FIELD( m_cTargets, FIELD_INTEGER ),
	DEFINE_ARRAY( m_iKey, FIELD_STRING, MAX_PRECACHE_ITEMS ),
	DEFINE_ARRAY( m_iValue, FIELD_STRING, MAX_PRECACHE_ITEMS ),
END_DATAMAP();

LINK_ENTITY_TO_CLASS( info_precache, CInfoPrecache );

bool CInfoPrecache :: KeyValue( KeyValueData* pkvd )
{
	if( m_cTargets < MAX_PRECACHE_ITEMS )
    {
        if( std::string( pkvd->szKeyName ).find( "model" ) )
        {
		    m_iKey[ m_cTargets ] = MAKE_STRING( "model" );
        }
        else if( std::string( pkvd->szKeyName ).find( "sound" ) )
        {
		    m_iKey[ m_cTargets ] = MAKE_STRING( "sound" );
        }
        else if( std::string( pkvd->szKeyName ).find( "generic" ) )
        {
		    m_iKey[ m_cTargets ] = MAKE_STRING( "generic" );
        }
        else if( std::string( pkvd->szKeyName ).find( "other" ) )
        {
		    m_iKey[ m_cTargets ] = MAKE_STRING( "other" );
        }
        else
        {
            return false;
        }

		m_iValue[ m_cTargets ] = ALLOC_STRING( pkvd->szValue );

		m_cTargets++;
		return true;
	}
    return false;
}

void CInfoPrecache :: Spawn()
{
    Precache();
    pev->solid = SOLID_NOT;
}

void CInfoPrecache :: Precache()
{
    if( m_cTargets > 0 )
    {
        for( int i = 0; i < m_cTargets; ++i )
        {
            if( FStrEq( STRING( m_iKey[i] ), "model" ) )
            {
                PrecacheModel( STRING( m_iValue[i] ) );
            }
            else if( FStrEq( STRING( m_iKey[i] ), "sound" ) )
            {
                PrecacheSound( STRING( m_iValue[i] ) );
            }
            else if( FStrEq( STRING( m_iKey[i] ), "generic" ) )
            {
                // PrecacheGeneric
            }
            else if( FStrEq( STRING( m_iKey[i] ), "other" ) )
            {
                UTIL_PrecacheOther( STRING( m_iValue[i] ) );
            }
        }
    }
}