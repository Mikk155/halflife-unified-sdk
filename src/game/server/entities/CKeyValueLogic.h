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

#define TRIGONOMETRIC_DEGREES_IN 0
#define TRIGONOMETRIC_RADIAN_MEASURE 1

#define SF_DONT_USE_X       ( 1 << 0 ) // Do not use X (Vector)
#define SF_DONT_USE_Y       ( 1 << 1 ) // Do not use Y (Vector)
#define SF_DONT_USE_Z       ( 1 << 2 ) // Do not use Z (Vector)
#define SF_INVERT_TARGET    ( 1 << 5 ) // Invert target value
#define SF_INVERT_SOURCE    ( 1 << 6 ) // Invert source value
#define SF_MULTIPLE_DEST    ( 1 << 7 ) // Multiple destinations

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
    ARCCOT = 24,            // Arccot (= arccot(source))
    CON6D = 0,              // 6 decimal places (Default)
    CON5D = 1,              // 5 d. p., rounded to 5 d. p.
    CON4D = 4,              // 4 d. p., rounded to 4 d. p.
    CON3D = 7,              // 3 d. p., rounded to 3 d. p.
    CON3D = 10,             // 2 d. p., rounded to 2 d. p.
    CON3D = 13,             // 1 d. p., rounded to 1 d. p.
    CONIR = 16,             // Integer, rounded
    CONIU = 17,             // Integer, rounded up
    CONID = 18,             // Integer, rounded down
};

class CKeyValueLogic : public CPointEntity
{
	DECLARE_CLASS( CKeyValueLogic, CPointEntity );
	DECLARE_DATAMAP();

    public:
        void Spawn() override;
        void ModifyValue( CBaseEntity* pTarget );
        bool KeyValue(KeyValueData* pkvd) override;
        void FindTarget( CBaseEntity* pActivator, CBaseEntity* pCaller );
        string_t GetValueOfKey( CBaseEntity* pTarget, string_t m_szKey );
        void SetKeyValue( CBaseEntity* pTarget, string_t m_szKey, string_t m_szValue );
        void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value ) override;

    private:
        int m_iszValueType = KeyValueLogicFlags::REPLACE;
        int m_trigonometricBehaviour = TRIGONOMETRIC_DEGREES_IN;
        int m_iAppendSpaces = 0;
        int m_iFloatConversion = KeyValueLogicFlags::CON6D;
        string_t m_iszValueName;
        string_t m_iszNewValue;
};
