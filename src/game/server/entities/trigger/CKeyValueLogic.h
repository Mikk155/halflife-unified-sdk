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
#include <iomanip>

enum KeyValueMath : int
{
    REPLACE = 0,
    ADD = 1,
    MULTIPLY = 2,
    SUBSTRACT = 3,
    DIVIDE = 4,
    AND = 5,
    OR = 6,
    DIRECTION_TO_ANGLES = 7,
    ANGLES_TO_DIRECTION = 8,
    APPEND_STRING = 9,

    EQUAL = 0,
    NOT_EQUAL = 1,
    LESS = 2,
    GREATER = 3,
    LESS_OR_EQUAL = 4,
    GREATER_OR_EQUAL = 5,
    LOGICAL_AND = 6
};

enum KeyValueFloatConversion : int
{
    NONE = 0,
    DECIMALS_6 = 1,
    DECIMALS_5 = 2,
    DECIMALS_4 = 3,
    DECIMALS_3 = 4,
    DECIMALS_2 = 5,
    DECIMALS_1 = 6,
    INTEGER = 7,
    INTEGER_RUP = 8,
    INTEGER_RDN = 9
};

enum KeyValueVector : int
{
    X = ( 1 << 0 ),
    Y = ( 1 << 1 ),
    Z = ( 1 << 2 ),
    A = ( 1 << 3 )
};

class CKeyValueLogic : public CBaseDelay
{
    DECLARE_CLASS( CKeyValueLogic, CBaseDelay );
    DECLARE_DATAMAP();

    public:
        void Spawn() override;
        bool KeyValue( KeyValueData* pkvd ) override;
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

        std::string format( std::string szValue );

        std::string GetValue( CBaseEntity* pActivator, CBaseEntity* pCaller );

        CBaseEntity* GetTarget( CBaseEntity* pActivator, CBaseEntity* pCaller );

        string_t m_sTargetEntity;
        string_t m_sKeyName;
        string_t m_sSourceEntity;
        string_t m_sSourceKeyName;
        string_t m_sValue;

        CBaseEntity* pLastFind = nullptr;

        KeyValueMath m_iSetType;

        int m_iAppendSpaces;

        int m_SkipVector;

        int m_iMaxTargets = 1;
        int m_iCurrentTarget = 0;

        KeyValueFloatConversion m_FloatConversion = KeyValueFloatConversion::DECIMALS_6;
};
