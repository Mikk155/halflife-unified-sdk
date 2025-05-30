//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_SpectatorPanel.h"
#include "hltv.h"

#include "pm_shared.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "entity_types.h"

// these are included for the math functions
#include "com_model.h"
#include "demo_api.h"
#include "event_api.h"
#include "screenfade.h"
#include "view.h"

#include "sound/ISoundSystem.h"

extern bool iJumpSpectator;
extern Vector vJumpOrigin;
extern Vector vJumpAngles;

Vector* GetClientColor( int clientIndex );

// Same color as in TeamFortressViewport::UpdateSpectatorPanel
Vector DefaultPlayerColor = {143 / 255.f, 143 / 255.f, 54 / 255.f};

void UnpackRGB( int& r, int& g, int& b, unsigned long ulRGB )
{
    r = ( ulRGB & 0xFF0000 ) >> 16;
    g = ( ulRGB & 0xFF00 ) >> 8;
    b = ulRGB & 0xFF;
}

#if 0 
const char* GetSpectatorLabel( int iMode )
{
    switch ( iMode )
    {
    case OBS_CHASE_LOCKED:
        return "#OBS_CHASE_LOCKED";

    case OBS_CHASE_FREE:
        return "#OBS_CHASE_FREE";

    case OBS_ROAMING:
        return "#OBS_ROAMING";

    case OBS_IN_EYE:
        return "#OBS_IN_EYE";

    case OBS_MAP_FREE:
        return "#OBS_MAP_FREE";

    case OBS_MAP_CHASE:
        return "#OBS_MAP_CHASE";

    case OBS_NONE:
    default:
        return "#OBS_NONE";
    }
}

#endif

void SpectatorMode()
{


    if( gEngfuncs.Cmd_Argc() <= 1 )
    {
        gEngfuncs.Con_Printf( "usage:  spec_mode <Main Mode> [<Inset Mode>]\n" );
        return;
    }

    // SetModes() will decide if command is executed on server or local
    if( gEngfuncs.Cmd_Argc() == 2 )
        gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), -1 );
    else if( gEngfuncs.Cmd_Argc() == 3 )
        gHUD.m_Spectator.SetModes( atoi( gEngfuncs.Cmd_Argv( 1 ) ), atoi( gEngfuncs.Cmd_Argv( 2 ) ) );
}

void SpectatorSpray()
{
    Vector forward;
    char string[128];

    if( 0 == gEngfuncs.IsSpectateOnly() )
        return;

    AngleVectors( v_angles, &forward, nullptr, nullptr );
    forward = forward * 128;
    forward = forward + v_origin;
    pmtrace_t* trace = gEngfuncs.PM_TraceLine( v_origin, forward, PM_TRACELINE_PHYSENTSONLY, 2, -1 );
    if( trace->fraction != 1.0 )
    {
        sprintf( string, "drc_spray %.2f %.2f %.2f %i",
            trace->endpos[0], trace->endpos[1], trace->endpos[2], trace->ent );
        gEngfuncs.pfnServerCmd( string );
    }
}
void SpectatorHelp()
{
    if( gViewPort )
    {
        // TODO: none of this spectator stuff exists in Op4
        // gViewPort->ShowVGUIMenu( MENU_SPECHELP );
    }
    else
    {
        char* text = CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help_Text" );

        if( text )
        {
            while( '\0' != *text )
            {
                if( *text != 13 )
                    gEngfuncs.Con_Printf( "%c", *text );
                text++;
            }
        }
    }
}

void SpectatorMenu()
{
    if( gEngfuncs.Cmd_Argc() <= 1 )
    {
        gEngfuncs.Con_Printf( "usage:  spec_menu <0|1>\n" );
        return;
    }

    gViewPort->m_pSpectatorPanel->ShowMenu( atoi( gEngfuncs.Cmd_Argv( 1 ) ) != 0 );
}

void ToggleScores()
{
    if( gViewPort )
    {
        if( gViewPort->IsScoreBoardVisible() )
        {
            gViewPort->HideScoreBoard();
        }
        else
        {
            gViewPort->ShowScoreBoard();
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudSpectator::Init()
{
    gHUD.AddHudElem( this );

    m_iFlags |= HUD_ACTIVE;
    m_flNextObserverInput = 0.0f;
    m_zoomDelta = 0.0f;
    m_moveDelta = 0.0f;
    m_FOV = 90.0f;
    m_chatEnabled = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );
    iJumpSpectator = false;

    memset( &m_OverviewData, 0, sizeof( m_OverviewData ) );
    memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );
    m_lastPrimaryObject = m_lastSecondaryObject = 0;

    gEngfuncs.pfnAddCommand( "spec_mode", SpectatorMode );
    gEngfuncs.pfnAddCommand( "spec_decal", SpectatorSpray );
    gEngfuncs.pfnAddCommand( "spec_help", SpectatorHelp );
    gEngfuncs.pfnAddCommand( "spec_menu", SpectatorMenu );
    // gEngfuncs.pfnAddCommand("togglescores", ToggleScores);

    m_drawnames = gEngfuncs.pfnRegisterVariable( "spec_drawnames", "1", 0 );
    m_drawcone = gEngfuncs.pfnRegisterVariable( "spec_drawcone", "1", 0 );
    m_drawstatus = gEngfuncs.pfnRegisterVariable( "spec_drawstatus", "1", 0 );
    m_autoDirector = gEngfuncs.pfnRegisterVariable( "spec_autodirector", "1", 0 );
    m_pip = gEngfuncs.pfnRegisterVariable( "spec_pip", "1", 0 );

    if( !m_drawnames || !m_drawcone || !m_drawstatus || !m_autoDirector || !m_pip )
    {
        gEngfuncs.Con_Printf( "ERROR! Couldn't register all spectator variables.\n" );
        return false;
    }

    return true;
}

bool UTIL_FindEntityInMap( const char* name, Vector& origin, Vector& angle )
{
    bool found = false;
    int n;
    char keyname[256];
    char token[1024];

    cl_entity_t* pEnt = gEngfuncs.GetEntityByIndex( 0 ); // get world model

    if( !pEnt )
        return false;

    if( !pEnt->model )
        return false;

    char* data = pEnt->model->entities;

    while( data )
    {
        data = gEngfuncs.COM_ParseFile( data, token );

        if( ( token[0] == '}' ) || ( token[0] == 0 ) )
            break;

        if( !data )
        {
            gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
            return false;
        }

        if( token[0] != '{' )
        {
            gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: expected {\n" );
            return false;
        }

        // we parse the first { now parse entities properties

        while( true )
        {
            // parse key
            data = gEngfuncs.COM_ParseFile( data, token );
            if( token[0] == '}' )
                break; // finish parsing this entity

            if( !data )
            {
                gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
                return false;
            }

            strcpy( keyname, token );

            // another hack to fix keynames with trailing spaces
            n = strlen( keyname );
            while( 0 != n && keyname[n - 1] == ' ' )
            {
                keyname[n - 1] = 0;
                n--;
            }

            // parse value
            data = gEngfuncs.COM_ParseFile( data, token );
            if( !data )
            {
                gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: EOF without closing brace\n" );
                return false;
            }

            if( token[0] == '}' )
            {
                gEngfuncs.Con_DPrintf( "UTIL_FindEntityInMap: closing brace without data" );
                return false;
            }

            if( 0 == strcmp( keyname, "classname" ) )
            {
                if( 0 == strcmp( token, name ) )
                {
                    found = true; // thats our entity
                }
            }

            if( 0 == strcmp( keyname, "angle" ) )
            {
                float y = atof( token );

                if( y >= 0 )
                {
                    angle[0] = 0.0f;
                    angle[1] = y;
                }
                else if( (int)y == -1 )
                {
                    angle[0] = -90.0f;
                    angle[1] = 0.0f;
                }
                else
                {
                    angle[0] = 90.0f;
                    angle[1] = 0.0f;
                }

                angle[2] = 0.0f;
            }

            if( 0 == strcmp( keyname, "angles" ) )
            {
                UTIL_StringToVector( angle, token );
            }

            if( 0 == strcmp( keyname, "origin" ) )
            {
                UTIL_StringToVector( origin, token );
            }
        } // while(true)

        if( found )
            return true;
    }

    return false; // we search all entities, but didn't found the correct
}

//-----------------------------------------------------------------------------
// SetSpectatorStartPosition():
// Get valid map position and 'beam' spectator to this position
//-----------------------------------------------------------------------------

void CHudSpectator::SetSpectatorStartPosition()
{
    // search for info_player start
    if( UTIL_FindEntityInMap( "trigger_camera", m_cameraOrigin, m_cameraAngles ) )
        iJumpSpectator = true;

    else if( UTIL_FindEntityInMap( "info_player_start", m_cameraOrigin, m_cameraAngles ) )
        iJumpSpectator = true;

    else if( UTIL_FindEntityInMap( "info_player_start_mp", m_cameraOrigin, m_cameraAngles ) )
        iJumpSpectator = true;

    else
    {
        // jump to 0,0,0 if no better position was found
        m_cameraOrigin = vec3_origin;
        m_cameraAngles = vec3_origin;
    }

    vJumpOrigin = m_cameraOrigin;
    vJumpAngles = m_cameraAngles;

    iJumpSpectator = true; // jump anyway
}


void CHudSpectator::SetCameraView( Vector pos, Vector angle, float fov )
{
    m_FOV = fov;
    vJumpOrigin = pos;
    vJumpAngles = angle;
    gEngfuncs.SetViewAngles( vJumpAngles );
    iJumpSpectator = true; // jump anyway
}

void CHudSpectator::AddWaypoint( float time, Vector pos, Vector angle, float fov, int flags )
{
    if( flags == 0 && time == 0.0f )
    {
        // switch instantly to this camera view
        SetCameraView( pos, angle, fov );
        return;
    }

    if( m_NumWayPoints >= MAX_CAM_WAYPOINTS )
    {
        gEngfuncs.Con_Printf( "Too many camera waypoints!\n" );
        return;
    }

    m_CamPath[m_NumWayPoints].angle = angle;
    m_CamPath[m_NumWayPoints].position = pos;
    m_CamPath[m_NumWayPoints].flags = flags;
    m_CamPath[m_NumWayPoints].fov = fov;
    m_CamPath[m_NumWayPoints].time = time;

    gEngfuncs.Con_DPrintf( "Added waypoint %i\n", m_NumWayPoints );

    m_NumWayPoints++;
}

void CHudSpectator::SetWayInterpolation( cameraWayPoint_t* prev, cameraWayPoint_t* start, cameraWayPoint_t* end, cameraWayPoint_t* next )
{
    m_WayInterpolation.SetViewAngles( start->angle, end->angle );

    m_WayInterpolation.SetFOVs( start->fov, end->fov );

    m_WayInterpolation.SetSmoothing( ( start->flags & DRC_FLAG_SLOWSTART ) != 0,
        ( start->flags & DRC_FLAG_SLOWEND ) != 0 );

    if( prev && next )
    {
        m_WayInterpolation.SetWaypoints( &prev->position, start->position, end->position, &next->position );
    }
    else if( prev )
    {
        m_WayInterpolation.SetWaypoints( &prev->position, start->position, end->position, nullptr );
    }
    else if( next )
    {
        m_WayInterpolation.SetWaypoints( nullptr, start->position, end->position, &next->position );
    }
    else
    {
        m_WayInterpolation.SetWaypoints( nullptr, start->position, end->position, nullptr );
    }
}

bool CHudSpectator::GetDirectorCamera( Vector& position, Vector& angle )
{
    float now = gHUD.m_flTime;
    float fov = 90.0f;

    if( 0 != m_ChaseEntity )
    {
        cl_entity_t* ent = gEngfuncs.GetEntityByIndex( m_ChaseEntity );

        if( ent )
        {
            Vector vt = ent->curstate.origin;

            if( m_ChaseEntity <= gEngfuncs.GetMaxClients() )
            {
                if( ent->curstate.solid == SOLID_NOT )
                {
                    vt = vt + VEC_DEAD_VIEW;
                }
                else if( ent->curstate.usehull == 1 )
                {
                    vt = vt + VEC_DUCK_VIEW;
                }
                else
                {
                    vt = vt + VEC_VIEW;
                }
            }

            vt = vt - position;
            VectorAngles( vt, angle );
            angle[0] = -angle[0];
            return true;
        }
        else
        {
            return false;
        }
    }

    if( !m_IsInterpolating )
        return false;

    if( m_WayPoint < 0 || m_WayPoint >= ( m_NumWayPoints - 1 ) )
        return false;

    cameraWayPoint_t* wp1 = &m_CamPath[m_WayPoint];
    cameraWayPoint_t* wp2 = &m_CamPath[m_WayPoint + 1];

    if( now < wp1->time )
        return false;

    while( now > wp2->time )
    {
        // go to next waypoint, if possible
        m_WayPoint++;

        if( m_WayPoint >= ( m_NumWayPoints - 1 ) )
        {
            m_IsInterpolating = false;
            return false; // there is no following waypoint
        }

        wp1 = wp2;
        wp2 = &m_CamPath[m_WayPoint + 1];

        if( m_WayPoint > 0 )
        {
            // we have a predecessor

            if( m_WayPoint < ( m_NumWayPoints - 1 ) )
            {
                // we have also a successor
                SetWayInterpolation( &m_CamPath[m_WayPoint - 1], wp1, wp2, &m_CamPath[m_WayPoint + 2] );
            }
            else
            {
                SetWayInterpolation( &m_CamPath[m_WayPoint - 1], wp1, wp2, nullptr );
            }
        }
        else if( m_WayPoint < ( m_NumWayPoints - 1 ) )
        {
            // we only have a successor
            SetWayInterpolation( nullptr, wp1, wp2, &m_CamPath[m_WayPoint + 2] );
        }
        else
        {
            // we have only two waypoints
            SetWayInterpolation( nullptr, wp1, wp2, nullptr );
        }
    }

    if( wp2->time <= wp1->time )
        return false;

    float fraction = ( now - wp1->time ) / ( wp2->time - wp1->time );

    if( fraction < 0.0f )
        fraction = 0.0f;
    else if( fraction > 1.0f )
        fraction = 1.0f;

    m_WayInterpolation.Interpolate( fraction, position, angle, &fov );

    // gEngfuncs.Con_Printf("Interpolate time: %.2f, fraction %.2f, point : %.2f,%.2f,%.2f\n", now, fraction, position[0], position[1], position[2] );

    SetCameraView( position, angle, fov );

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Loads new icons
//-----------------------------------------------------------------------------
bool CHudSpectator::VidInit()
{
    m_hsprPlayer = SPR_Load( "sprites/iplayer.spr" );
    m_hsprPlayerBlue = SPR_Load( "sprites/iplayerblue.spr" );
    m_hsprPlayerRed = SPR_Load( "sprites/iplayerred.spr" );
    m_hsprPlayerDead = SPR_Load( "sprites/iplayerdead.spr" );
    m_hsprUnkownMap = SPR_Load( "sprites/tile.spr" );
    m_hsprBeam = SPR_Load( "sprites/laserbeam.spr" );
    m_hsprCamera = SPR_Load( "sprites/camera.spr" );
    m_hCrosshair = SPR_Load( "sprites/crosshairs.spr" );

    m_lastPrimaryObject = m_lastSecondaryObject = 0;
    m_flNextObserverInput = 0.0f;
    m_lastHudMessage = 0;
    m_iSpectatorNumber = 0;
    iJumpSpectator = false;
    g_iUser1 = g_iUser2 = 0;

    return true;
}

float CHudSpectator::GetFOV()
{
    return m_FOV;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : flTime -
//            intermission -
//-----------------------------------------------------------------------------
bool CHudSpectator::Draw( float flTime )
{
    int lx;

    char string[256];
    Vector* color;

    // draw only in spectator mode
    if( 0 == g_iUser1 )
        return false;

    // if user pressed zoom, aplly changes
    if( ( m_zoomDelta != 0.0f ) && ( g_iUser1 == OBS_MAP_FREE ) )
    {
        m_mapZoom += m_zoomDelta;

        if( m_mapZoom > 3.0f )
            m_mapZoom = 3.0f;

        if( m_mapZoom < 0.5f )
            m_mapZoom = 0.5f;
    }

    // if user moves in map mode, change map origin
    if( ( m_moveDelta != 0.0f ) && ( g_iUser1 != OBS_ROAMING ) )
    {
        Vector right;
        AngleVectors( v_angles, nullptr, &right, nullptr );
        VectorNormalize( right );
        right = right * m_moveDelta;

        m_mapOrigin = m_mapOrigin + right;
    }

    // Only draw the icon names only if map mode is in Main Mode
    if( g_iUser1 < OBS_MAP_FREE )
        return true;

    if( 0 == m_drawnames->value )
        return true;

    // make sure we have player info
    gViewPort->GetAllPlayersInfo();


    // loop through all the players and draw additional infos to their sprites on the map
    for( int i = 0; i < MAX_PLAYERS_HUD; i++ )
    {

        if( m_vPlayerPos[i][2] < 0 ) // marked as invisible ?
            continue;

        // check if name would be in inset window
        if( m_pip->value != INSET_OFF )
        {
            if( m_vPlayerPos[i][0] > XRES( m_OverviewData.insetWindowX ) &&
                m_vPlayerPos[i][1] > YRES( m_OverviewData.insetWindowY ) &&
                m_vPlayerPos[i][0] < XRES( m_OverviewData.insetWindowX + m_OverviewData.insetWindowWidth ) &&
                m_vPlayerPos[i][1] < YRES( m_OverviewData.insetWindowY + m_OverviewData.insetWindowHeight ) )
                continue;
        }

        color = GetClientColor( i + 1 );

        // TODO: this is pretty ugly, need a better way.
        if( !color )
        {
            color = &DefaultPlayerColor;
        }

        // draw the players name and health underneath
        sprintf( string, "%s", g_PlayerInfoList[i + 1].name );

        lx = strlen( string ) * 3; // 3 is avg. character length :)

        gEngfuncs.pfnDrawSetTextColor( color->x, color->y, color->z );
        DrawConsoleString( m_vPlayerPos[i][0] - lx, m_vPlayerPos[i][1], string );
    }

    return true;
}


void CHudSpectator::DirectorMessage( BufferReader& reader )
{
    float f1, f2;
    char* string;
    Vector v1, v2;
    int i1, i2, i3;

    int cmd = reader.ReadByte();

    switch ( cmd ) // director command byte
    {
    case DRC_CMD_START:
    {
        // now we have to do some things clientside, since the proxy doesn't know our mod
        g_iPlayerClass = 0;
        g_iTeamNumber = 0;

        // fake a InitHUD & ResetHUD message
        BufferReader dummyReader;
        gHUD.MsgFunc_InitHUD( nullptr, dummyReader );
        gHUD.MsgFunc_ResetHUD( nullptr, dummyReader );

        break;
    }

    case DRC_CMD_EVENT: // old director style message
        m_lastPrimaryObject = reader.ReadShort();
        m_lastSecondaryObject = reader.ReadShort();
        m_iObserverFlags = reader.ReadLong();

        if( 0 != m_autoDirector->value )
        {
            if( ( g_iUser2 != m_lastPrimaryObject ) || ( g_iUser3 != m_lastSecondaryObject ) )
                V_ResetChaseCam();

            g_iUser2 = m_lastPrimaryObject;
            g_iUser3 = m_lastSecondaryObject;
            m_IsInterpolating = false;
            m_ChaseEntity = 0;
        }

        // gEngfuncs.Con_Printf("Director Camera: %i %i\n", firstObject, secondObject);
        break;
    case DRC_CMD_MODE:
        if( 0 != m_autoDirector->value )
        {
            SetModes( reader.ReadByte(), -1 );
        }
        break;


    case DRC_CMD_CAMERA:
        v1[0] = reader.ReadCoord(); // position
        v1[1] = reader.ReadCoord();
        v1[2] = reader.ReadCoord(); // vJumpOrigin

        v2[0] = reader.ReadCoord(); // view angle
        v2[1] = reader.ReadCoord(); // vJumpAngles
        v2[2] = reader.ReadCoord();
        f1 = reader.ReadByte();     // fov
        i1 = reader.ReadShort(); // target

        if( 0 != m_autoDirector->value )
        {
            SetModes( OBS_ROAMING, -1 );
            SetCameraView( v1, v2, f1 );
            m_ChaseEntity = i1;
        }
        break;

    case DRC_CMD_MESSAGE:
    {
        client_textmessage_t* msg = &m_HUDMessages[m_lastHudMessage];

        msg->effect = reader.ReadByte(); // effect

        UnpackRGB( ( int& )msg->r1, ( int& )msg->g1, ( int& )msg->b1, reader.ReadLong() ); // color
        msg->r2 = msg->r1;
        msg->g2 = msg->g1;
        msg->b2 = msg->b1;
        msg->a2 = msg->a1 = 0xFF; // not transparent

        msg->x = reader.ReadFloat(); // x pos
        msg->y = reader.ReadFloat(); // y pos

        msg->fadein = reader.ReadFloat();    // fadein
        msg->fadeout = reader.ReadFloat();    // fadeout
        msg->holdtime = reader.ReadFloat(); // holdtime
        msg->fxtime = reader.ReadFloat();    // fxtime;

        strncpy( m_HUDMessageText[m_lastHudMessage], reader.ReadString(), 128 );
        m_HUDMessageText[m_lastHudMessage][127] = 0; // text

        msg->pMessage = m_HUDMessageText[m_lastHudMessage];
        msg->pName = "HUD_MESSAGE";

        gHUD.m_Message.MessageAdd( msg );

        m_lastHudMessage++;
        m_lastHudMessage %= MAX_SPEC_HUD_MESSAGES;
    }

    break;

    case DRC_CMD_SOUND:
        string = reader.ReadString();
        f1 = reader.ReadFloat();

        // gEngfuncs.Con_Printf("DRC_CMD_FX_SOUND: %s %.2f\n", string, value );
        EV_PlaySound( 0, v_origin, CHAN_BODY, string, f1, ATTN_NORM, 0, PITCH_NORM );

        break;

    case DRC_CMD_TIMESCALE:
        f1 = reader.ReadFloat(); // ignore this command (maybe show slowmo sign)
        break;



    case DRC_CMD_STATUS:
        reader.ReadLong();                        // total number of spectator slots
        m_iSpectatorNumber = reader.ReadLong(); // total number of spectator
        reader.ReadShort();                        // total number of relay proxies

        gViewPort->UpdateSpectatorPanel();
        break;

    case DRC_CMD_BANNER:
        // gEngfuncs.Con_DPrintf("GUI: Banner %s\n",reader.ReadString() ); // name of banner tga eg gfx/temp/7454562234563475.tga
        gViewPort->m_pSpectatorPanel->m_TopBanner->LoadImage( reader.ReadString() );
        gViewPort->UpdateSpectatorPanel();
        break;

    case DRC_CMD_STUFFTEXT:
        gEngfuncs.pfnFilteredClientCmd( reader.ReadString() );
        break;

    case DRC_CMD_CAMPATH:
        v1[0] = reader.ReadCoord(); // position
        v1[1] = reader.ReadCoord();
        v1[2] = reader.ReadCoord(); // vJumpOrigin

        v2[0] = reader.ReadCoord(); // view angle
        v2[1] = reader.ReadCoord(); // vJumpAngles
        v2[2] = reader.ReadCoord();
        f1 = reader.ReadByte(); // FOV
        i1 = reader.ReadByte(); // flags

        if( 0 != m_autoDirector->value )
        {
            SetModes( OBS_ROAMING, -1 );
            SetCameraView( v1, v2, f1 );
        }
        break;

    case DRC_CMD_WAYPOINTS:
        i1 = reader.ReadByte();
        m_NumWayPoints = 0;
        m_WayPoint = 0;
        for( i2 = 0; i2 < i1; i2++ )
        {
            f1 = gHUD.m_flTime + (float)( reader.ReadShort() ) / 100.0f;

            v1[0] = reader.ReadCoord(); // position
            v1[1] = reader.ReadCoord();
            v1[2] = reader.ReadCoord(); // vJumpOrigin

            v2[0] = reader.ReadCoord(); // view angle
            v2[1] = reader.ReadCoord(); // vJumpAngles
            v2[2] = reader.ReadCoord();
            f2 = reader.ReadByte(); // fov
            i3 = reader.ReadByte(); // flags

            AddWaypoint( f1, v1, v2, f2, i3 );
        }

        // gEngfuncs.Con_Printf("CHudSpectator::DirectorMessage: waypoints %i.\n", m_NumWayPoints );
        if( 0 == m_autoDirector->value )
        {
            // ignore waypoints
            m_NumWayPoints = 0;
            break;
        }

        SetModes( OBS_ROAMING, -1 );

        m_IsInterpolating = true;

        if( m_NumWayPoints > 2 )
        {
            SetWayInterpolation( nullptr, &m_CamPath[0], &m_CamPath[1], &m_CamPath[2] );
        }
        else
        {
            SetWayInterpolation( nullptr, &m_CamPath[0], &m_CamPath[1], nullptr );
        }
        break;

    default:
        gEngfuncs.Con_DPrintf( "CHudSpectator::DirectorMessage: unknown command %i.\n", cmd );
    }
}

void CHudSpectator::FindNextPlayer( bool bReverse )
{
    // MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
    //                only a subset of the players. e.g. Make it check the target's team.

    int iStart;
    cl_entity_t* pEnt = nullptr;

    // if we are NOT in HLTV mode, spectator targets are set on server
    if( 0 == gEngfuncs.IsSpectateOnly() )
    {
        char cmdstring[32];
        // forward command to server
        sprintf( cmdstring, "follownext %i", bReverse ? 1 : 0 );
        gEngfuncs.pfnServerCmd( cmdstring );
        return;
    }

    if( 0 != g_iUser2 )
        iStart = g_iUser2;
    else
        iStart = 1;

    g_iUser2 = 0;

    int iCurrent = iStart;

    int iDir = bReverse ? -1 : 1;

    // make sure we have player info
    gViewPort->GetAllPlayersInfo();


    do
    {
        iCurrent += iDir;

        // Loop through the clients
        if( iCurrent > MAX_PLAYERS_HUD )
            iCurrent = 1;
        if( iCurrent < 1 )
            iCurrent = MAX_PLAYERS_HUD;

        pEnt = gEngfuncs.GetEntityByIndex( iCurrent );

        if( !IsActivePlayer( pEnt ) )
            continue;

        // MOD AUTHORS: Add checks on target here.

        g_iUser2 = iCurrent;
        break;

    } while( iCurrent != iStart );

    // Did we find a target?
    if( 0 == g_iUser2 )
    {
        gEngfuncs.Con_DPrintf( "No observer targets.\n" );
        // take save camera position
        vJumpOrigin = m_cameraOrigin;
        vJumpAngles = m_cameraAngles;
    }
    else
    {
        // use new entity position for roaming
        vJumpOrigin = pEnt->origin;
        vJumpAngles = pEnt->angles;
    }

    iJumpSpectator = true;
    BufferReader reader;
    gViewPort->MsgFunc_ResetFade( nullptr, reader );
}


void CHudSpectator::FindPlayer( const char* name )
{
    // MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
    //                only a subset of the players. e.g. Make it check the target's team.

    // if we are NOT in HLTV mode, spectator targets are set on server
    if( 0 == gEngfuncs.IsSpectateOnly() )
    {
        char cmdstring[32];
        // forward command to server
        sprintf( cmdstring, "follow %s", name );
        gEngfuncs.pfnServerCmd( cmdstring );
        return;
    }

    g_iUser2 = 0;

    // make sure we have player info
    gViewPort->GetAllPlayersInfo();

    cl_entity_t* pEnt = nullptr;

    for( int i = 1; i < MAX_PLAYERS_HUD; i++ )
    {

        pEnt = gEngfuncs.GetEntityByIndex( i );

        if( !IsActivePlayer( pEnt ) )
            continue;

        if( !stricmp( g_PlayerInfoList[pEnt->index].name, name ) )
        {
            g_iUser2 = i;
            break;
        }
    }

    // Did we find a target?
    if( 0 == g_iUser2 )
    {
        gEngfuncs.Con_DPrintf( "No observer targets.\n" );
        // take save camera position
        vJumpOrigin = m_cameraOrigin;
        vJumpAngles = m_cameraAngles;
    }
    else
    {
        // use new entity position for roaming
        vJumpOrigin = pEnt->origin;
        vJumpAngles = pEnt->angles;
    }

    iJumpSpectator = true;
    BufferReader reader;
    gViewPort->MsgFunc_ResetFade( nullptr, reader );
}

void CHudSpectator::HandleButtonsDown( int ButtonPressed )
{
    double time = gEngfuncs.GetClientTime();

    int newMainMode = g_iUser1;
    int newInsetMode = m_pip->value;

    // gEngfuncs.Con_Printf(" HandleButtons:%i\n", ButtonPressed );

    if( !gViewPort )
        return;

    // Not in intermission.
    if( gHUD.m_iIntermission )
        return;

    if( 0 == g_iUser1 )
        return; // dont do anything if not in spectator mode

    // don't handle buttons during normal demo playback
    if( 0 != gEngfuncs.pDemoAPI->IsPlayingback() && 0 == gEngfuncs.IsSpectateOnly() )
        return;
    // Slow down mouse clicks.
    if( m_flNextObserverInput > time )
        return;

    // enable spectator screen
    if( ( ButtonPressed & IN_DUCK ) != 0 )
        gViewPort->m_pSpectatorPanel->ShowMenu( !gViewPort->m_pSpectatorPanel->m_menuVisible );

    //  'Use' changes inset window mode
    if( ( ButtonPressed & IN_USE ) != 0 )
    {
        newInsetMode = ToggleInset( true );
    }

    // if not in HLTV mode, buttons are handled server side
    if( 0 != gEngfuncs.IsSpectateOnly() )
    {
        // changing target or chase mode not in overviewmode without inset window

        // Jump changes main window modes
        if( ( ButtonPressed & IN_JUMP ) != 0 )
        {
            if( g_iUser1 == OBS_CHASE_LOCKED )
                newMainMode = OBS_CHASE_FREE;

            else if( g_iUser1 == OBS_CHASE_FREE )
                newMainMode = OBS_IN_EYE;

            else if( g_iUser1 == OBS_IN_EYE )
                newMainMode = OBS_ROAMING;

            else if( g_iUser1 == OBS_ROAMING )
                newMainMode = OBS_MAP_FREE;

            else if( g_iUser1 == OBS_MAP_FREE )
                newMainMode = OBS_MAP_CHASE;

            else
                newMainMode = OBS_CHASE_FREE; // don't use OBS_CHASE_LOCKED anymore
        }

        // Attack moves to the next player
        if( ( ButtonPressed & ( IN_ATTACK | IN_ATTACK2 ) ) != 0 )
        {
            FindNextPlayer( ( ButtonPressed & IN_ATTACK2 ) != 0 );

            if( g_iUser1 == OBS_ROAMING )
            {
                gEngfuncs.SetViewAngles( vJumpAngles );
                iJumpSpectator = true;
            }
            // release directed mode if player wants to see another player
            m_autoDirector->value = 0.0f;
        }
    }

    SetModes( newMainMode, newInsetMode );

    if( g_iUser1 == OBS_MAP_FREE )
    {
        if( ( ButtonPressed & IN_FORWARD ) != 0 )
            m_zoomDelta = 0.01f;

        if( ( ButtonPressed & IN_BACK ) != 0 )
            m_zoomDelta = -0.01f;

        if( ( ButtonPressed & IN_MOVELEFT ) != 0 )
            m_moveDelta = -12.0f;

        if( ( ButtonPressed & IN_MOVERIGHT ) != 0 )
            m_moveDelta = 12.0f;
    }

    m_flNextObserverInput = time + 0.2;
}

void CHudSpectator::HandleButtonsUp( int ButtonPressed )
{
    if( !gViewPort )
        return;

    if( !gViewPort->m_pSpectatorPanel->isVisible() )
        return; // dont do anything if not in spectator mode

    if( ( ButtonPressed & ( IN_FORWARD | IN_BACK ) ) != 0 )
        m_zoomDelta = 0.0f;

    if( ( ButtonPressed & ( IN_MOVELEFT | IN_MOVERIGHT ) ) != 0 )
        m_moveDelta = 0.0f;
}

void CHudSpectator::SetModes( int iNewMainMode, int iNewInsetMode )
{
    // if value == -1 keep old value
    if( iNewMainMode == -1 )
        iNewMainMode = g_iUser1;

    if( iNewInsetMode == -1 )
        iNewInsetMode = m_pip->value;

    // inset mode is handled only clients side
    m_pip->value = iNewInsetMode;

    if( iNewMainMode < OBS_CHASE_LOCKED || iNewMainMode > OBS_MAP_CHASE )
    {
        gEngfuncs.Con_Printf( "Invalid spectator mode.\n" );
        return;
    }

    m_IsInterpolating = false;
    m_ChaseEntity = 0;

    // main mode settings will override inset window settings
    if( iNewMainMode != g_iUser1 )
    {
        // if we are NOT in HLTV mode, main spectator mode is set on server
        if( 0 == gEngfuncs.IsSpectateOnly() )
        {
            char cmdstring[32];
            // forward command to server
            sprintf( cmdstring, "specmode %i", iNewMainMode );
            gEngfuncs.pfnServerCmd( cmdstring );
            return;
        }

        if( 0 == g_iUser2 && ( iNewMainMode != OBS_ROAMING ) ) // make sure we have a target
        {
            // choose last Director object if still available
            if( IsActivePlayer( gEngfuncs.GetEntityByIndex( m_lastPrimaryObject ) ) )
            {
                g_iUser2 = m_lastPrimaryObject;
                g_iUser3 = m_lastSecondaryObject;
            }
            else
                FindNextPlayer( false ); // find any target
        }

        switch ( iNewMainMode )
        {
        case OBS_CHASE_LOCKED:
            g_iUser1 = OBS_CHASE_LOCKED;
            break;

        case OBS_CHASE_FREE:
            g_iUser1 = OBS_CHASE_FREE;
            break;

        case OBS_ROAMING: // jump to current vJumpOrigin/angle
            g_iUser1 = OBS_ROAMING;
            if( 0 != g_iUser2 )
            {
                V_GetChasePos( g_iUser2, &v_cl_angles, vJumpOrigin, vJumpAngles );
                gEngfuncs.SetViewAngles( vJumpAngles );
                iJumpSpectator = true;
            }
            break;

        case OBS_IN_EYE:
            g_iUser1 = OBS_IN_EYE;
            break;

        case OBS_MAP_FREE:
            g_iUser1 = OBS_MAP_FREE;
            // reset user values
            m_mapZoom = m_OverviewData.zoom;
            m_mapOrigin = m_OverviewData.origin;
            break;

        case OBS_MAP_CHASE:
            g_iUser1 = OBS_MAP_CHASE;
            // reset user values
            m_mapZoom = m_OverviewData.zoom;
            m_mapOrigin = m_OverviewData.origin;
            break;
        }

        if( ( g_iUser1 == OBS_IN_EYE ) || ( g_iUser1 == OBS_ROAMING ) )
        {
            m_crosshairRect.left = 24;
            m_crosshairRect.top = 0;
            m_crosshairRect.right = 48;
            m_crosshairRect.bottom = 24;

            gHUD.m_Ammo.SetDrawCrosshair( true );
            gHUD.m_Ammo.SetCrosshair( m_hCrosshair, m_crosshairRect );
        }
        else
        {
            memset( &m_crosshairRect, 0, sizeof( m_crosshairRect ) );
            gHUD.m_Ammo.SetDrawCrosshair( false );
            gHUD.m_Ammo.SetCrosshair( 0, m_crosshairRect );
        }

        BufferReader reader;
        gViewPort->MsgFunc_ResetFade( nullptr, reader );

        char string[128];
        sprintf( string, "#Spec_Mode%d", g_iUser1 );
        sprintf( string, "%c%s", HUD_PRINTCENTER, CHudTextMessage::BufferedLocaliseTextString( string ) );
        reader = BufferReader{{reinterpret_cast<std::byte*>( string ), strlen( string ) + 1}};
        gHUD.m_TextMessage.MsgFunc_TextMsg( nullptr, reader );
    }

    gViewPort->UpdateSpectatorPanel();
}

bool CHudSpectator::IsActivePlayer( cl_entity_t* ent )
{
    return ( nullptr != ent &&
            0 != ent->player &&
            ent->curstate.solid != SOLID_NOT &&
            ent != gEngfuncs.GetLocalPlayer() &&
            g_PlayerInfoList[ent->index].name != nullptr );
}


bool CHudSpectator::ParseOverviewFile()
{
    memset( &m_OverviewData, 0, sizeof( m_OverviewData ) );

    // fill in standrd values
    m_OverviewData.insetWindowX = 4; // upper left corner
    m_OverviewData.insetWindowY = 4;
    m_OverviewData.insetWindowHeight = 180;
    m_OverviewData.insetWindowWidth = 240;
    m_OverviewData.origin[0] = 0.0f;
    m_OverviewData.origin[1] = 0.0f;
    m_OverviewData.origin[2] = 0.0f;
    m_OverviewData.zoom = 1.0f;
    m_OverviewData.layers = 0;
    m_OverviewData.layersHeights[0] = 0.0f;
    strcpy( m_OverviewData.map, gEngfuncs.pfnGetLevelName() );

    if( strlen( m_OverviewData.map ) == 0 )
        return false; // not active yet

    Filename levelname = m_OverviewData.map + 5;
    levelname.resize( levelname.size() - 4 );

    Filename filename;
    fmt::format_to( std::back_inserter( filename ), "overviews/{}.txt", levelname.c_str() );

    const auto fileContents = FileSystem_LoadFileIntoBuffer( filename.c_str(), FileContentFormat::Text );

    if( fileContents.empty() )
    {
        gEngfuncs.Con_DPrintf( "Couldn't open file %s. Using default values for overview mode.\n", filename.c_str() );
        return false;
    }

    auto pfile = reinterpret_cast<const char*>( fileContents.data() );

    char token[1024];

    while( true )
    {
        pfile = gEngfuncs.COM_ParseFile( pfile, token );

        if( !pfile )
            break;

        if( !stricmp( token, "global" ) )
        {
            // parse the global data
            pfile = gEngfuncs.COM_ParseFile( pfile, token );
            if( stricmp( token, "{" ) )
            {
                gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename.c_str() );
                return false;
            }

            pfile = gEngfuncs.COM_ParseFile( pfile, token );

            while( stricmp( token, "}" ) )
            {
                if( !stricmp( token, "zoom" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.zoom = atof( token );
                }
                else if( !stricmp( token, "origin" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.origin[0] = atof( token );
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.origin[1] = atof( token );
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.origin[2] = atof( token );
                }
                else if( !stricmp( token, "rotated" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.rotated = 0 != atoi( token );
                }
                else if( !stricmp( token, "inset" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.insetWindowX = atof( token );
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.insetWindowY = atof( token );
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.insetWindowWidth = atof( token );
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    m_OverviewData.insetWindowHeight = atof( token );
                }
                else
                {
                    gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename.c_str(), token );
                    return false;
                }

                pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
            }
        }
        else if( !stricmp( token, "layer" ) )
        {
            // parse a layer data

            if( m_OverviewData.layers == OVERVIEW_MAX_LAYERS )
            {
                gEngfuncs.Con_Printf( "Error parsing overview file %s. ( too many layers )\n", filename.c_str() );
                return false;
            }

            pfile = gEngfuncs.COM_ParseFile( pfile, token );


            if( stricmp( token, "{" ) )
            {
                gEngfuncs.Con_Printf( "Error parsing overview file %s. (expected { )\n", filename.c_str() );
                return false;
            }

            pfile = gEngfuncs.COM_ParseFile( pfile, token );

            while( stricmp( token, "}" ) )
            {
                if( !stricmp( token, "image" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    strcpy( m_OverviewData.layersImages[m_OverviewData.layers], token );
                }
                else if( !stricmp( token, "height" ) )
                {
                    pfile = gEngfuncs.COM_ParseFile( pfile, token );
                    const float height = atof( token );
                    m_OverviewData.layersHeights[m_OverviewData.layers] = height;
                }
                else
                {
                    gEngfuncs.Con_Printf( "Error parsing overview file %s. (%s unkown)\n", filename.c_str(), token );
                    return false;
                }

                pfile = gEngfuncs.COM_ParseFile( pfile, token ); // parse next token
            }

            m_OverviewData.layers++;
        }
    }

    m_mapZoom = m_OverviewData.zoom;
    m_mapOrigin = m_OverviewData.origin;

    return true;
}

void CHudSpectator::LoadMapSprites()
{
    // right now only support for one map layer
    if( m_OverviewData.layers > 0 )
    {
        m_MapSprite = gEngfuncs.LoadMapSprite( m_OverviewData.layersImages[0] );
    }
    else
        m_MapSprite = nullptr; // the standard "unkown map" sprite will be used instead
}

void CHudSpectator::DrawOverviewLayer()
{
    float screenaspect, xs, ys, xStep, yStep, x, y, z;
    int ix, iy, i, xTiles, yTiles, frame;

    bool hasMapImage = nullptr != m_MapSprite;
    model_t* dummySprite = ( model_t* )gEngfuncs.GetSpritePointer( m_hsprUnkownMap );

    if( hasMapImage )
    {
        i = m_MapSprite->numframes / ( 4 * 3 );
        i = sqrt( (float)i );
        xTiles = i * 4;
        yTiles = i * 3;
    }
    else
    {
        xTiles = 8;
        yTiles = 6;
    }


    screenaspect = 4.0f / 3.0f;


    xs = m_OverviewData.origin[0];
    ys = m_OverviewData.origin[1];
    z = ( 90.0f - v_angles[0] ) / 90.0f;
    z *= m_OverviewData.layersHeights[0]; // gOverviewData.z_min - 32;

    // i = r_overviewTexture + ( layer*OVERVIEW_X_TILES*OVERVIEW_Y_TILES );

    gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );
    gEngfuncs.pTriAPI->CullFace( TRI_NONE );
    gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );

    frame = 0;


    // rotated view ?
    if( m_OverviewData.rotated )
    {
        xStep = ( 2 * 4096.0f / m_OverviewData.zoom ) / xTiles;
        yStep = -( 2 * 4096.0f / ( m_OverviewData.zoom * screenaspect ) ) / yTiles;

        y = ys + ( 4096.0f / ( m_OverviewData.zoom * screenaspect ) );

        for( iy = 0; iy < yTiles; iy++ )
        {
            x = xs - ( 4096.0f / ( m_OverviewData.zoom ) );

            for( ix = 0; ix < xTiles; ix++ )
            {
                if( hasMapImage )
                    gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );
                else
                    gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

                gEngfuncs.pTriAPI->Begin( TRI_QUADS );
                gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
                gEngfuncs.pTriAPI->Vertex3f( x, y, z );

                gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
                gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z );

                gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
                gEngfuncs.pTriAPI->Vertex3f( x + xStep, y + yStep, z );

                gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
                gEngfuncs.pTriAPI->Vertex3f( x, y + yStep, z );
                gEngfuncs.pTriAPI->End();

                frame++;
                x += xStep;
            }

            y += yStep;
        }
    }
    else
    {
        xStep = -( 2 * 4096.0f / m_OverviewData.zoom ) / xTiles;
        yStep = -( 2 * 4096.0f / ( m_OverviewData.zoom * screenaspect ) ) / yTiles;


        x = xs + ( 4096.0f / ( m_OverviewData.zoom * screenaspect ) );



        for( ix = 0; ix < yTiles; ix++ )
        {

            y = ys + ( 4096.0f / ( m_OverviewData.zoom ) );

            for( iy = 0; iy < xTiles; iy++ )
            {
                if( hasMapImage )
                    gEngfuncs.pTriAPI->SpriteTexture( m_MapSprite, frame );
                else
                    gEngfuncs.pTriAPI->SpriteTexture( dummySprite, 0 );

                gEngfuncs.pTriAPI->Begin( TRI_QUADS );
                gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
                gEngfuncs.pTriAPI->Vertex3f( x, y, z );

                gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
                gEngfuncs.pTriAPI->Vertex3f( x + xStep, y, z );

                gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
                gEngfuncs.pTriAPI->Vertex3f( x + xStep, y + yStep, z );

                gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
                gEngfuncs.pTriAPI->Vertex3f( x, y + yStep, z );
                gEngfuncs.pTriAPI->End();

                frame++;

                y += yStep;
            }

            x += xStep;
        }
    }
}

void CHudSpectator::DrawOverviewEntities()
{
    int i;
    model_t* hSpriteModel;
    Vector origin, angles, point, forward, right, left, up, screen, offset;
    float x, y, z, r, g, b, sizeScale = 4.0f;
    cl_entity_t* ent;
    float rmatrix[3][4]; // transformation matrix

    float zScale = ( 90.0f - v_angles[0] ) / 90.0f;


    z = m_OverviewData.layersHeights[0] * zScale;
    // get yellow/brown HUD color
    const auto color = gHUD.m_HudColor;
    r = (float)color.Red / 255.0f;
    g = (float)color.Green / 255.0f;
    b = (float)color.Blue / 255.0f;

    gEngfuncs.pTriAPI->CullFace( TRI_NONE );

    for( i = 0; i < MAX_PLAYERS_HUD; i++ )
        m_vPlayerPos[i][2] = -1; // mark as invisible

    // draw all players
    for( i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
    {
        if( 0 == m_OverviewEntities[i].hSprite )
            continue;

        hSpriteModel = ( model_t* )gEngfuncs.GetSpritePointer( m_OverviewEntities[i].hSprite );
        ent = m_OverviewEntities[i].entity;

        gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );
        gEngfuncs.pTriAPI->RenderMode( kRenderTransTexture );

        // see R_DrawSpriteModel
        // draws players sprite

        AngleVectors( ent->angles, &right, &up, nullptr );

        origin = ent->origin;

        gEngfuncs.pTriAPI->Begin( TRI_QUADS );

        gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );

        gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
        point = origin + ( ( 16.0f * sizeScale ) * up );
        point = point + ( ( 16.0f * sizeScale ) * right );
        point[2] *= zScale;
        gEngfuncs.pTriAPI->Vertex3fv( point );

        gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );

        point = origin + ( ( 16.0f * sizeScale ) * up );
        point = point + ( ( -16.0f * sizeScale ) * right );
        point[2] *= zScale;
        gEngfuncs.pTriAPI->Vertex3fv( point );

        gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
        point = origin + ( ( -16.0f * sizeScale ) * up );
        point = point + ( ( -16.0f * sizeScale ) * right );
        point[2] *= zScale;
        gEngfuncs.pTriAPI->Vertex3fv( point );

        gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
        point = origin + ( ( -16.0f * sizeScale ) * up );
        point = point + ( ( 16.0f * sizeScale ) * right );
        point[2] *= zScale;
        gEngfuncs.pTriAPI->Vertex3fv( point );

        gEngfuncs.pTriAPI->End();


        if( 0 == ent->player )
            continue;
        // draw line under player icons
        origin[2] *= zScale;

        gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );

        hSpriteModel = ( model_t* )gEngfuncs.GetSpritePointer( m_hsprBeam );
        gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );

        gEngfuncs.pTriAPI->Color4f( r, g, b, 0.3 );

        gEngfuncs.pTriAPI->Begin( TRI_QUADS );
        gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] + 4, origin[2] - zScale );
        gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] - 4, origin[2] - zScale );
        gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] - 4, z );
        gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] + 4, z );
        gEngfuncs.pTriAPI->End();

        gEngfuncs.pTriAPI->Begin( TRI_QUADS );
        gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] + 4, origin[2] - zScale );
        gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] - 4, origin[2] - zScale );
        gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] + 4, origin[1] - 4, z );
        gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
        gEngfuncs.pTriAPI->Vertex3f( origin[0] - 4, origin[1] + 4, z );
        gEngfuncs.pTriAPI->End();

        // calculate screen position for name and infromation in hud::draw()
        if( 0 == gEngfuncs.pTriAPI->WorldToScreen( origin, screen ) )
            continue; // object is behind viewer

        screen[0] = XPROJECT( screen[0] );
        screen[1] = YPROJECT( screen[1] );
        screen[2] = 0.0f;

        // calculate some offset under the icon
        origin[0] += 32.0f;
        origin[1] += 32.0f;

        gEngfuncs.pTriAPI->WorldToScreen( origin, offset );

        offset[0] = XPROJECT( offset[0] );
        offset[1] = YPROJECT( offset[1] );
        offset[2] = 0.0f;

        offset = offset - screen;

        int playerNum = ent->index - 1;

        m_vPlayerPos[playerNum][0] = screen[0];
        m_vPlayerPos[playerNum][1] = screen[1] + offset.Length();
        m_vPlayerPos[playerNum][2] = 1; // mark player as visible
    }

    if( 0 == m_pip->value || 0 == m_drawcone->value )
        return;

    // get current camera position and angle

    if( m_pip->value == INSET_IN_EYE || g_iUser1 == OBS_IN_EYE )
    {
        V_GetInEyePos( g_iUser2, origin, angles );
    }
    else if( m_pip->value == INSET_CHASE_FREE || g_iUser1 == OBS_CHASE_FREE )
    {
        V_GetChasePos( g_iUser2, &v_cl_angles, origin, angles );
    }
    else if( g_iUser1 == OBS_ROAMING )
    {
        origin = v_sim_org;
        angles = v_cl_angles;
    }
    else
        V_GetChasePos( g_iUser2, nullptr, origin, angles );


    // draw camera sprite

    x = origin[0];
    y = origin[1];
    z = origin[2];

    angles[0] = 0; // always show horizontal camera sprite

    hSpriteModel = ( model_t* )gEngfuncs.GetSpritePointer( m_hsprCamera );
    gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
    gEngfuncs.pTriAPI->SpriteTexture( hSpriteModel, 0 );


    gEngfuncs.pTriAPI->Color4f( r, g, b, 1.0 );

    AngleVectors( angles, &forward, nullptr, nullptr );
    forward = forward * 512.0f;

    offset[0] = 0.0f;
    offset[1] = 45.0f;
    offset[2] = 0.0f;

    AngleMatrix( offset, rmatrix );
    VectorTransform( forward, rmatrix, right );

    offset[1] = -45.0f;
    AngleMatrix( offset, rmatrix );
    VectorTransform( forward, rmatrix, left );

    gEngfuncs.pTriAPI->Begin( TRI_TRIANGLES );
    gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
    gEngfuncs.pTriAPI->Vertex3f( x + right[0], y + right[1], ( z + right[2] ) * zScale );

    gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
    gEngfuncs.pTriAPI->Vertex3f( x, y, z * zScale );

    gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
    gEngfuncs.pTriAPI->Vertex3f( x + left[0], y + left[1], ( z + left[2] ) * zScale );
    gEngfuncs.pTriAPI->End();
}



void CHudSpectator::DrawOverview()
{
    // draw only in sepctator mode
    if( 0 == g_iUser1 )
        return;

    // Only draw the overview if Map Mode is selected for this view
    if( m_iDrawCycle == 0 && ( ( g_iUser1 != OBS_MAP_FREE ) && ( g_iUser1 != OBS_MAP_CHASE ) ) )
        return;

    if( m_iDrawCycle == 1 && m_pip->value < INSET_MAP_FREE )
        return;

    DrawOverviewLayer();
    DrawOverviewEntities();
    CheckOverviewEntities();
}
void CHudSpectator::CheckOverviewEntities()
{
    double time = gEngfuncs.GetClientTime();

    // removes old entities from list
    for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
    {
        // remove entity from list if it is too old
        if( m_OverviewEntities[i].killTime < time )
        {
            memset( &m_OverviewEntities[i], 0, sizeof( overviewEntity_t ) );
        }
    }
}

bool CHudSpectator::AddOverviewEntity( int type, cl_entity_t* ent, const char* modelname )
{
    HSPRITE hSprite = 0;
    double duration = -1.0f; // duration -1 means show it only this frame;

    if( !ent )
        return false;

    if( type == ET_PLAYER )
    {
        if( ent->curstate.solid != SOLID_NOT )
        {
            switch ( g_PlayerExtraInfo[ent->index].teamnumber )
            {
                // blue and red teams are swapped in CS and TFC
            case 1:
                hSprite = m_hsprPlayerBlue;
                break;
            case 2:
                hSprite = m_hsprPlayerRed;
                break;
            default:
                hSprite = m_hsprPlayer;
                break;
            }
        }
        else
            return false; // it's an spectator
    }
    else if( type == ET_NORMAL )
    {
        return false;
    }
    else
        return false;

    return AddOverviewEntityToList( hSprite, ent, gEngfuncs.GetClientTime() + duration );
}

void CHudSpectator::DeathMessage( int victim )
{
    // find out where the victim is
    cl_entity_t* pl = gEngfuncs.GetEntityByIndex( victim );

    if( pl && 0 != pl->player )
        AddOverviewEntityToList( m_hsprPlayerDead, pl, gEngfuncs.GetClientTime() + 2.0f );
}

bool CHudSpectator::AddOverviewEntityToList( HSPRITE sprite, cl_entity_t* ent, double killTime )
{
    for( int i = 0; i < MAX_OVERVIEW_ENTITIES; i++ )
    {
        // find empty entity slot
        if( m_OverviewEntities[i].entity == nullptr )
        {
            m_OverviewEntities[i].entity = ent;
            m_OverviewEntities[i].hSprite = sprite;
            m_OverviewEntities[i].killTime = killTime;
            return true;
        }
    }

    return false; // maximum overview entities reached
}
void CHudSpectator::CheckSettings()
{
    // disallow same inset mode as main mode:

    m_pip->value = (int)m_pip->value;

    if( ( g_iUser1 < OBS_MAP_FREE ) && ( m_pip->value == INSET_CHASE_FREE || m_pip->value == INSET_IN_EYE ) )
    {
        // otherwise both would show in World picures
        m_pip->value = INSET_MAP_FREE;
    }

    if( ( g_iUser1 >= OBS_MAP_FREE ) && ( m_pip->value >= INSET_MAP_FREE ) )
    {
        // both would show map views
        m_pip->value = INSET_CHASE_FREE;
    }

    // disble in intermission screen
    if( gHUD.m_iIntermission )
        m_pip->value = INSET_OFF;

    // check chat mode
    if( m_chatEnabled != ( gHUD.m_SayText.m_HUD_saytext->value != 0 ) )
    {
        // hud_saytext changed
        m_chatEnabled = ( gHUD.m_SayText.m_HUD_saytext->value != 0 );

        if( 0 != gEngfuncs.IsSpectateOnly() )
        {
            // tell proxy our new chat mode
            char chatcmd[32];
            sprintf( chatcmd, "ignoremsg %i", m_chatEnabled ? 0 : 1 );
            gEngfuncs.pfnServerCmd( chatcmd );
        }
    }

    // HL/TFC has no oberserver corsshair, so set it client side
    if( ( g_iUser1 == OBS_IN_EYE ) || ( g_iUser1 == OBS_ROAMING ) )
    {
        m_crosshairRect.left = 24;
        m_crosshairRect.top = 0;
        m_crosshairRect.right = 48;
        m_crosshairRect.bottom = 24;

        gHUD.m_Ammo.SetDrawCrosshair( true );
        gHUD.m_Ammo.SetCrosshair( m_hCrosshair, m_crosshairRect );
    }
    else
    {
        memset( &m_crosshairRect, 0, sizeof( m_crosshairRect ) );
        gHUD.m_Ammo.SetDrawCrosshair( false );
        gHUD.m_Ammo.SetCrosshair( 0, m_crosshairRect );
    }



    // if we are a real player on server don't allow inset window
    // in First Person mode since this is our resticted forcecamera mode 2
    // team number 3 = SPECTATOR see player.h

    if( ( ( g_iTeamNumber == 1 ) || ( g_iTeamNumber == 2 ) ) && ( g_iUser1 == OBS_IN_EYE ) )
        m_pip->value = INSET_OFF;

    // draw small border around inset view, adjust upper black bar
    gViewPort->m_pSpectatorPanel->EnableInsetView( m_pip->value != INSET_OFF );
}

int CHudSpectator::ToggleInset( bool allowOff )
{
    int newInsetMode = (int)m_pip->value + 1;

    if( g_iUser1 < OBS_MAP_FREE )
    {
        if( newInsetMode > INSET_MAP_CHASE )
        {
            if( allowOff )
                newInsetMode = INSET_OFF;
            else
                newInsetMode = INSET_MAP_FREE;
        }

        if( newInsetMode == INSET_CHASE_FREE )
            newInsetMode = INSET_MAP_FREE;
    }
    else
    {
        if( newInsetMode > INSET_IN_EYE )
        {
            if( allowOff )
                newInsetMode = INSET_OFF;
            else
                newInsetMode = INSET_CHASE_FREE;
        }
    }

    return newInsetMode;
}
void CHudSpectator::Reset()
{
    // Reset HUD
    if( 0 != strcmp( m_OverviewData.map, gEngfuncs.pfnGetLevelName() ) )
    {
        // update level overview if level changed
        ParseOverviewFile();
        LoadMapSprites();
    }

    memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );

    m_FOV = 90.0f;

    m_IsInterpolating = false;

    m_ChaseEntity = 0;

    SetSpectatorStartPosition();
}

void CHudSpectator::InitHUDData()
{
    m_lastPrimaryObject = m_lastSecondaryObject = 0;
    m_flNextObserverInput = 0.0f;
    m_lastHudMessage = 0;
    m_iSpectatorNumber = 0;
    iJumpSpectator = false;
    g_iUser1 = g_iUser2 = 0;

    memset( &m_OverviewData, 0, sizeof( m_OverviewData ) );
    memset( &m_OverviewEntities, 0, sizeof( m_OverviewEntities ) );

    if( 0 != gEngfuncs.IsSpectateOnly() || 0 != gEngfuncs.pDemoAPI->IsPlayingback() )
        m_autoDirector->value = 1.0f;
    else
        m_autoDirector->value = 0.0f;

    Reset();

    SetModes( OBS_CHASE_LOCKED, INSET_OFF );

    g_iUser2 = 0; // fake not target until first camera command

    // reset HUD FOV
    gHUD.m_iFOV = CVAR_GET_FLOAT( "default_fov" );
}
