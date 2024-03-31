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

#define SF_DONT_USE_X       ( 1 << 0 ) // Do not use X (Vector)
#define SF_DONT_USE_Y       ( 1 << 1 ) // Do not use Y (Vector)
#define SF_DONT_USE_Z       ( 1 << 2 ) // Do not use Z (Vector)
#define SF_INVERT_TARGET    ( 1 << 5 ) // Invert target value
#define SF_INVERT_SOURCE    ( 1 << 6 ) // Invert source value
#define SF_MULTIPLE_DEST    ( 1 << 7 ) // Multiple destinations

#define FC_6_DECIMALS       0          // 6 decimal places (Default)
#define FC_5_DECIMALS       1          // 5 d. p., rounded to 5 d. p.
#define FC_4_DECIMALS       4          // 4 d. p., rounded to 4 d. p.
#define FC_3_DECIMALS       7          // 3 d. p., rounded to 3 d. p.
#define FC_2_DECIMALS       10         // 2 d. p., rounded to 2 d. p.
#define FC_1_DECIMALS       13         // 1 d. p., rounded to 1 d. p.
#define FC_INT_ROUND        16         // Integer, rounded
#define FC_INT_ROUND_UP     17         // Integer, rounded up
#define FC_INT_ROUND_DOWN   18         // Integer, rounded down

#define TRM_DEGREES_IN      0          // Degrees in (out for arc.)
#define TRM_RADIAN_MEASURE  1          // Radian measure in (out for arc.)

enum KeyValueLogicFlags : int
{
    REPLACE = 0,            // Replace (= source)
    ADD = 1,                // Add (= old + source)
    MUL = 2,                // Mul (= old * source)
    SUB = 3,                // Sub (= old - source)
    DIV = 4,                // Div (= old / source)
    AND = 5,                // AND (= old & source)
    OR = 6,                 // OR (= old | source)
    NAND = 7,               // NAND (= !(old & source))
    NOR = 8,                // NOR (= !(old | source))
    DANGLES = 9,            // Direction to Angles
    ADIRECTION = 10,        // Angles to Direction
    APPEND = 11,            // Append (String concatenation)
    MOD = 12,               // Mod (= old % source)
    XOR = 13,               // XOR (= old ^ source)
    NXOR = 14,              // NXOR (= !(old ^ source))
    POW = 16,               // Pow (= old ^ source)
    SIN = 17,               // Sin (= sin(source))
    COS = 18,               // Cos (= cos(source))
    TAN = 19,               // Tan (= tan(source))
    ARCSIN = 20,            // Arcsin (= arcsin(source))
    ARCCOS = 21,            // Arccos (= arccos(source))
    ARCTAN = 22,            // Arctan (= arctan(source))
    COT = 23,               // Cot (= cot(source))
    ARCCOT = 24             // Arccot (= arccot(source))
};

class CKeyValueLogic : public CPointEntity
{
	DECLARE_CLASS( CKeyValueLogic, CPointEntity );
	DECLARE_DATAMAP();

    public:
        void Spawn() override;
        const char* ToString( Vector VecValue );
        void ModifyValue( CBaseEntity* pTarget );
        bool KeyValue(KeyValueData* pkvd) override;
        void FindTarget( CBaseEntity* pActivator, CBaseEntity* pCaller );
        const char* GetValueOfKey( CBaseEntity* pTarget, string_t m_szKey );
        void SetKeyValue( CBaseEntity* pTarget, string_t m_szKey, string_t m_szValue );
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

        string_t m_iszValueName;
        string_t m_iszNewValue;

    private:
        int m_iszValueType = KeyValueLogicFlags::REPLACE;
        int m_trigonometricBehaviour = TRM_DEGREES_IN;
        int m_iAppendSpaces = 0;
        int m_iFloatConversion = FC_6_DECIMALS;
};
