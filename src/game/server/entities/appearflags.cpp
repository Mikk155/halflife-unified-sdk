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

#include "appearflags.h"

bool ShouldAppear( CBaseEntity* pEntity )
{
	if( pEntity->m_appearflag_notin != (int)appearflags::DEFAULT )
	{
		if(
			( ( pEntity->m_appearflag_notin & appearflags::GM_SINGLEPLAYER )   != 0 &&    !g_pGameRules->IsMultiplayer() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::GM_MULTIPLAYER )    != 0 &&     g_pGameRules->IsMultiplayer() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::GM_COOPERATIVE )    != 0 &&     g_pGameRules->IsCoOp() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::SKILL_EASY )        != 0 &&     g_Skill.GetSkillLevel() == SkillLevel::Easy )
		||
			( ( pEntity->m_appearflag_notin & appearflags::SKILL_MEDIUM )      != 0 &&     g_Skill.GetSkillLevel() == SkillLevel::Medium )
		||
			( ( pEntity->m_appearflag_notin & appearflags::SKILL_HARD )        != 0 &&     g_Skill.GetSkillLevel() == SkillLevel::Hard )
		||
			( ( pEntity->m_appearflag_notin & appearflags::GM_TEAMPLAY )       != 0 &&    !g_pGameRules->IsTeamplay() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::GM_DEATHMATCH )     != 0 &&    !g_pGameRules->IsDeathmatch() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::GM_CAPTURETHEFLAG ) != 0 &&    !g_pGameRules->IsCTF() )
		||
			( ( pEntity->m_appearflag_notin & appearflags::SV_DEDICATED )      != 0 &&    !IS_DEDICATED_SERVER() )
		){ return false; }
	}

	if( pEntity->m_appearflag_onlyin != (int)appearflags::DEFAULT )
	{
		if(
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_SINGLEPLAYER )  != 0 &&     g_pGameRules->IsMultiplayer() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_MULTIPLAYER )   != 0 &&    !g_pGameRules->IsMultiplayer() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_COOPERATIVE )   != 0 &&    !g_pGameRules->IsCoOp() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::SKILL_EASY )       != 0 &&     g_Skill.GetSkillLevel() != SkillLevel::Easy )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::SKILL_MEDIUM )     != 0 &&     g_Skill.GetSkillLevel() != SkillLevel::Medium )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::SKILL_HARD )       != 0 &&     g_Skill.GetSkillLevel() != SkillLevel::Hard )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_TEAMPLAY )      != 0 &&     g_pGameRules->IsTeamplay() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_DEATHMATCH )    != 0 &&     g_pGameRules->IsDeathmatch() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::GM_CAPTURETHEFLAG )!= 0 &&     g_pGameRules->IsCTF() )
		||
			( ( pEntity->m_appearflag_onlyin & appearflags::SV_DEDICATED )     != 0 &&     IS_DEDICATED_SERVER() )
		){ return false; }
	}
	return true;
}