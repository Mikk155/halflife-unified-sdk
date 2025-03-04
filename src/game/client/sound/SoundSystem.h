/***
 *
 *    Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *    This product contains software technology licensed from Id
 *    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *    All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

#include <memory>

#include <spdlog/logger.h>

#include "ISoundSystem.h"

namespace sound
{
class GameSoundSystem;
class MusicSystem;

class SoundSystem final : public ISoundSystem
{
public:
    ~SoundSystem() override;

    bool Create();

    void Update() override;

    void Block() override;

    void Unblock() override;

    void Pause() override;

    void Resume() override;

    IGameSoundSystem* GetGameSoundSystem() override;

    IMusicSystem* GetMusicSystem() override;

    const std::shared_ptr<spdlog::logger>& GetLogger() { return m_Logger; }

private:
    std::shared_ptr<spdlog::logger> m_Logger;

    std::unique_ptr<GameSoundSystem> m_GameSoundSystem;
    std::unique_ptr<MusicSystem> m_MusicSystem;

    bool m_Blocked{false};
    bool m_Paused{false};
};
}
