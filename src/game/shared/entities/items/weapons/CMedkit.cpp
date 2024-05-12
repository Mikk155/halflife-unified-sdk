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

#include "CMedkit.h"

#define VIEW_MODEL "models/v_medkit.mdl"
#define WORLD_MODEL "models/w_medkit.mdl"
#define PLAYER_MODEL "models/p_medkit.mdl"
#define SND_SHOT "items/medshot4.wav"
#define SND_NO "items/medshotno1.wav"
#define SND_LOOP "items/medcharge4.wav"

BEGIN_DATAMAP( CMedkit )
END_DATAMAP();

LINK_ENTITY_TO_CLASS( weapon_medkit, CMedkit );

void CMedkit::OnCreate()
{
	BaseClass::OnCreate();
	m_iId = WEAPON_MEDKIT;
	SetMagazine1(WEAPON_NOCLIP);
	m_WorldModel = pev->model = MAKE_STRING( WORLD_MODEL );
}

void CMedkit::Precache()
{
	CBasePlayerWeapon::Precache();
	PrecacheModel( VIEW_MODEL );
	PrecacheModel( STRING(m_WorldModel) );
	PrecacheModel( PLAYER_MODEL );

	PrecacheSound( SND_SHOT );
	PrecacheSound( SND_NO );
	PrecacheSound( SND_LOOP );

	m_usMedkit = PRECACHE_EVENT(1, "events/medkit.sc");
}

bool CMedkit::Deploy()
{
	return DefaultDeploy( VIEW_MODEL, PLAYER_MODEL, MedkitAnim::Draw, "crowbar" );
}

void CMedkit::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim( MedkitAnim::Holster );
}

void CMedkit::SetNextAttack( float flNext = 1.0f )
{
	m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flNext;
}

void CMedkit::SecondaryAttack()
{
	if( m_flNextSecondaryAttack > UTIL_WeaponTimeBase() )
		return;

	if( /*m_iClip*/100 < g_Skill.GetValue( "plr_medkit_revive_ammo", 50 ) )
	{
		m_pPlayer->EmitSound( CHAN_STATIC, SND_NO, 0.5, ATTN_NORM );
		SetNextAttack();
		return;
	}

#ifndef CLIENT_DLL
	bool ShouldRevive = false;

	CBaseEntity* pTarget = nullptr;

	while ( ( pTarget = UTIL_FindEntityInSphere( pTarget, m_pPlayer->pev->origin, g_Skill.GetValue( "plr_medkit_revive_distance", 128 ) ) ) != nullptr )
	{
		if( pTarget->ClassnameIs( "deadplayer" ) )
		{
			auto player = UTIL_PlayerByIndex( (int)pTarget->pev->renderamt );

			if( player != nullptr )
			{
				ShouldRevive = true;
				break;
			}
		}
		else if( ( pTarget->IsPlayer() || pTarget->IsMonster() ) && !pTarget->IsAlive() && pTarget->pev->deadflag == DEAD_DEAD )
		{
			ShouldRevive = true;
			break;
		}
	}

	if( ShouldRevive )
	{
		SendWeaponAnim( MedkitAnim::LongUse );
		m_pPlayer->EmitSound( CHAN_STATIC, SND_SHOT, 0.5, ATTN_NORM );

		if( pTarget->IsPlayer() )
		{
			CBasePlayer* player = ToBasePlayer( pTarget );

			if( player != nullptr )
			{
				player->m_hCorpse = nullptr;
				player->Revive( m_pPlayer, g_Skill.GetValue( "plr_medkit_revive_ammo", 50 ) );
			}
		}
		else
		{
			CBaseMonster* pMonster = pTarget->MyMonsterPointer();

			if( pMonster != nullptr )
			{
				pMonster->Revive( m_pPlayer, g_Skill.GetValue( "plr_medkit_revive_ammo", 50 ) );
			}
		}

		// Loop sound
		// m_pPlayer->EmitSound( CHAN_STATIC, SND_LOOP, 0.5, ATTN_NORM );
	}
#endif

	SetNextAttack();
}

void CMedkit::PrimaryAttack()
{
	if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
		return;

	if( /*m_iClip*/100 < g_Skill.GetValue( "plr_medkit_heal_ammo", 5 ) )
	{
		m_pPlayer->EmitSound( CHAN_STATIC, SND_NO, 0.5, ATTN_NORM );
		SetNextAttack();
		return;
	}

	CBaseEntity* pTarget = m_pPlayer;

	if( m_pPlayer->pev->angles.x > -20 )
	{
		TraceResult tr;

		UTIL_TraceLine( m_pPlayer->GetGunPosition(),
			m_pPlayer->GetGunPosition() + gpGlobals->v_forward * g_Skill.GetValue( "plr_medkit_heal_distance", 5 ),
				dont_ignore_monsters, m_pPlayer->edict(), &tr
		);

		if( tr.pHit != nullptr )
		{
			pTarget = CBaseEntity::Instance( tr.pHit );
		}
	}

	if( pTarget != nullptr )
	{
		if( pTarget->pev->health < pTarget->pev->max_health )
		{
			SendWeaponAnim( MedkitAnim::ShortUse );
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			m_pPlayer->EmitSound( CHAN_STATIC, SND_SHOT, 0.5, ATTN_NORM );

			m_iClip = m_iClip - g_Skill.GetValue( "plr_medkit_heal_ammo", 5 );
			pTarget->GiveHealth( g_Skill.GetValue( "plr_medkit_heal_amt", 5 ), DMG_GENERIC );
		}
		else
		{
			m_pPlayer->EmitSound( CHAN_STATIC, SND_NO, 0.5, ATTN_NORM );
		}
	}
	else
	{
		m_pPlayer->EmitSound( CHAN_STATIC, SND_NO, 0.5, ATTN_NORM );
	}

	SetNextAttack();
}

void CMedkit::WeaponIdle()
{
	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		int iAnim = ( UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, 1 ) == 1 ? MedkitAnim::LongIdle : MedkitAnim::Idle );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ( iAnim == MedkitAnim::LongIdle ? 3 : 1 );
		SendWeaponAnim(iAnim);
	}
}

bool CMedkit::GetWeaponInfo(WeaponInfo& info)
{
	info.Name = STRING(pev->classname);
	info.AttackModeInfo[0].MagazineSize = WEAPON_NOCLIP;
	info.Slot = 0;
	info.Position = 4;
	info.Id = WEAPON_MEDKIT;
	info.Weight = 0;
	return true;
}
