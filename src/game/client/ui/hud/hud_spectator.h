//========= Copyright ï¿½ 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "cl_entity.h"
#include "interpolation.h"

struct model_t;

#define INSET_OFF 0
#define INSET_CHASE_FREE 1
#define INSET_IN_EYE 2
#define INSET_MAP_FREE 3
#define INSET_MAP_CHASE 4

#define MAX_SPEC_HUD_MESSAGES 8

#define OVERVIEW_TILE_SIZE 128 // don't change this
#define OVERVIEW_MAX_LAYERS 1

//-----------------------------------------------------------------------------
// Purpose: Handles the drawing of the spectator stuff (camera & top-down map and all the things on it )
//-----------------------------------------------------------------------------

struct overviewInfo_t
{
    char map[64];  // cl.levelname or empty
    Vector origin; // center of map
    float zoom;       // zoom of map images
    int layers;       // how may layers do we have
    float layersHeights[OVERVIEW_MAX_LAYERS];
    char layersImages[OVERVIEW_MAX_LAYERS][255];
    bool rotated; // are map images rotated (90 degrees) ?

    int insetWindowX;
    int insetWindowY;
    int insetWindowHeight;
    int insetWindowWidth;
};

struct overviewEntity_t
{

    HSPRITE hSprite;
    cl_entity_t* entity;
    double killTime;
};

struct cameraWayPoint_t
{
    float time;
    Vector position;
    Vector angle;
    float fov;
    int flags;
};

#define MAX_OVERVIEW_ENTITIES 128
#define MAX_CAM_WAYPOINTS 32

class CHudSpectator : public CHudBase
{
public:
    void Reset() override;
    int ToggleInset( bool allowOff );
    void CheckSettings();
    void InitHUDData() override;
    bool AddOverviewEntityToList( HSPRITE sprite, cl_entity_t* ent, double killTime );
    void DeathMessage( int victim );
    bool AddOverviewEntity( int type, cl_entity_t* ent, const char* modelname );
    void CheckOverviewEntities();
    void DrawOverview();
    void DrawOverviewEntities();
    void DrawOverviewLayer();
    void LoadMapSprites();
    bool ParseOverviewFile();
    bool IsActivePlayer( cl_entity_t* ent );
    void SetModes( int iMainMode, int iInsetMode );
    void HandleButtonsDown( int ButtonPressed );
    void HandleButtonsUp( int ButtonPressed );
    void FindNextPlayer( bool bReverse );
    void FindPlayer( const char* name );
    void DirectorMessage( BufferReader& reader );
    void SetSpectatorStartPosition();
    bool Init() override;
    bool VidInit() override;

    bool Draw( float flTime ) override;

    void AddWaypoint( float time, Vector pos, Vector angle, float fov, int flags );
    void SetCameraView( Vector pos, Vector angle, float fov );
    float GetFOV();
    bool GetDirectorCamera( Vector& position, Vector& angle );
    void SetWayInterpolation( cameraWayPoint_t* prev, cameraWayPoint_t* start, cameraWayPoint_t* end, cameraWayPoint_t* next );


    int m_iDrawCycle;
    client_textmessage_t m_HUDMessages[MAX_SPEC_HUD_MESSAGES];
    char m_HUDMessageText[MAX_SPEC_HUD_MESSAGES][128];
    int m_lastHudMessage;
    overviewInfo_t m_OverviewData;
    overviewEntity_t m_OverviewEntities[MAX_OVERVIEW_ENTITIES];
    int m_iObserverFlags;
    int m_iSpectatorNumber;

    float m_mapZoom;    // zoom the user currently uses
    Vector m_mapOrigin; // origin where user rotates around
    cvar_t* m_drawnames;
    cvar_t* m_drawcone;
    cvar_t* m_drawstatus;
    cvar_t* m_autoDirector;
    cvar_t* m_pip;
    bool m_chatEnabled;

    bool m_IsInterpolating;
    int m_ChaseEntity;       // if != 0, follow this entity with viewangles
    int m_WayPoint;           // current waypoint 1
    int m_NumWayPoints;       // current number of waypoints
    Vector m_cameraOrigin; // a help camera
    Vector m_cameraAngles; // and it's angles
    CInterpolation m_WayInterpolation;

private:
    Vector m_vPlayerPos[MAX_PLAYERS_HUD];
    HSPRITE m_hsprPlayerBlue;
    HSPRITE m_hsprPlayerRed;
    HSPRITE m_hsprPlayer;
    HSPRITE m_hsprCamera;
    HSPRITE m_hsprPlayerDead;
    HSPRITE m_hsprViewcone;
    HSPRITE m_hsprUnkownMap;
    HSPRITE m_hsprBeam;
    HSPRITE m_hCrosshair;

    Rect m_crosshairRect;

    model_t* m_MapSprite; // each layer image is saved in one sprite, where each tile is a sprite frame
    float m_flNextObserverInput;
    float m_FOV;
    float m_zoomDelta;
    float m_moveDelta;
    int m_lastPrimaryObject;
    int m_lastSecondaryObject;

    cameraWayPoint_t m_CamPath[MAX_CAM_WAYPOINTS];
};
