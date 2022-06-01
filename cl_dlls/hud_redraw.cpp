/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
/***
*
*	(C) 2008 Vyacheslav Dzhura
*
****/
//
// hud_redraw.cpp
//
#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include "triangleapi.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "event_api.h"

#include "vgui_TeamFortressViewport.h"

#define MAX_LOGO_FRAMES 56

extern vec3_t v_origin;
int    g_iFrameSize;


int grgLogoFrame[MAX_LOGO_FRAMES] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
	16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
	29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31 
};


extern int g_iVisibleMouse;

float HUD_GetFOV( void );

extern cvar_t *sensitivity;

// Think
void CHud::Think(void)
{
	int newfov;
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if (pList->p->m_iFlags & HUD_ACTIVE)
			pList->p->Think();
		pList = pList->pNext;
	}

	newfov = HUD_GetFOV();
	if ( newfov == 0 )
	{
		m_iFOV = default_fov->value;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iFOV == default_fov->value )
	{  
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{  
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)default_fov->value) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if ( m_iFOV == 0 )
	{  // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = max( default_fov->value, 90 );  
	}
}

void ReturnFrameHint(char* szHint,int Id)
{
	switch(Id){
	case 0:
		sprintf(szHint,"Button");
		break;
	case 1:
		sprintf(szHint,"Object");
		break;
	case 2:
        sprintf(szHint,"Item");
		break;
	case 3:
		sprintf(szHint,"File");
		break;
	case 4:
		sprintf(szHint,"Documents");
		break;
    case 5:
		sprintf(szHint,"Book");
		break;
	case 6:
		sprintf(szHint,"Monitor");
		break;
	case 7:
		sprintf(szHint,"Computer");
		break;
	case 8:
		sprintf(szHint,"Compact Disc");
		break;
	case 9:
		sprintf(szHint,"Extinguisher");
		break;
    case 10:
		sprintf(szHint,"Security Card");
		break;
    case 11:
		sprintf(szHint,"Key");
		break;
	case 12:
		sprintf(szHint,"Syringe");
		break;
	case 13:
		sprintf(szHint,"Cleansuit");
		break;
    case 14:
		sprintf(szHint,"Wrench");
		break;
	case 15:
        sprintf(szHint,"Retinal Scanner");
		break;
	case 16:
		sprintf(szHint,"Health Charger");
		break;
	case 17:
		sprintf(szHint,"HEV Charger");
		break;
	default:
        sprintf(szHint,"Unknown");
	}
}

void DrawFrameCorner(int x,int y,int u1,int u2,int u3,int u4,int v1,int v2,int v3,int v4)
{
	gEngfuncs.pTriAPI->TexCoord2f( u1, v1 );
	gEngfuncs.pTriAPI->Vertex3f( x, y, 0.0 );
	gEngfuncs.pTriAPI->TexCoord2f( u2, v2 );
	gEngfuncs.pTriAPI->Vertex3f( x, y+g_iFrameSize, 0.0 );
	gEngfuncs.pTriAPI->TexCoord2f( u3, v3 );
	gEngfuncs.pTriAPI->Vertex3f(  x+g_iFrameSize, y+g_iFrameSize, 0.0 );
	gEngfuncs.pTriAPI->TexCoord2f( u4, v4 );
	gEngfuncs.pTriAPI->Vertex3f(  x+g_iFrameSize, y, 0.0  );
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud :: Redraw( float flTime, int intermission )
{
	m_fOldTime = m_flTime;	// save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static m_flShotTime = 0;
	
	// Clock was reset, reset delta
	if ( m_flTimeDelta < 0 )
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if ( m_iIntermission && !intermission )
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			gViewPort->UpdateSpectatorPanel();
		}
		else if ( !m_iIntermission && intermission )
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if ( CVAR_GET_FLOAT( "hud_takesshots" ) != 0 )
				m_flShotTime = flTime + 1.0;	// Take a screenshot in a second
		}
	}

	if (m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;
	
	if ( m_pCvarDraw->value )
	{
		HUDLIST *pList = m_pHudList;

		while (pList)
		{
			if ( !m_bAlienMode )
			{
				if ( !intermission )
				{
					if ( (pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL) )
						pList->p->Draw(flTime);
				}
				else
				{  // it's an intermission,  so only draw hud elements that are set to draw during intermissions
					if ( pList->p->m_iFlags & HUD_INTERMISSION )
						pList->p->Draw( flTime );
				}
			} else
			{  // alien mode!!!
				if ( pList->p->m_iFlags & HUD_ALIEN )
					if ( (pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL) )
						pList->p->Draw( flTime );
			}

			pList = pList->pNext;
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250 );
		
		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0)/2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}

	// draw selection frame around entity
	if (m_iFrameIndex !=0)
	{
        vec3_t v_center, v_maxs;
		
		gEngfuncs.pTriAPI->WorldToScreen(m_vAimFrameCoords,v_center);
		gEngfuncs.pTriAPI->WorldToScreen(m_vAimFrameMaxs,v_maxs);
		
		v_center[0] = XPROJECT(v_center[0]);
		v_center[1] = YPROJECT(v_center[1]);
		v_center[2] = 0.0f;
		v_maxs[0] = XPROJECT(v_maxs[0]);
		v_maxs[1] = YPROJECT(v_maxs[1]);
		v_maxs[2] = 0.0f;

		g_iFrameSize = m_iFrameSize;

		gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pFrameTexture, 0 );
		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
        gEngfuncs.pTriAPI->Begin( TRI_QUADS ); 
		
		if (v_center[0]>v_maxs[0])
		{
			//right bottom
			DrawFrameCorner(v_center[0],v_center[1],1,1,0,0,1,0,0,1);
			//left bottom
			DrawFrameCorner(v_center[0]-(v_center[0]-v_maxs[0]),v_center[1],1,0,0,1,0,0,1,1);
			//left up
			DrawFrameCorner(v_maxs[0], v_maxs[1],0,0,1,1,0,1,1,0);
			//left right
			DrawFrameCorner(v_maxs[0]+(v_center[0]-v_maxs[0]),v_maxs[1],0,1,1,0,1,1,0,0);
		}
		else
		{
			//right bottom
			DrawFrameCorner(v_center[0],v_center[1],1,0,0,1,0,0,1,1);
			//left bottom
			DrawFrameCorner(v_center[0]-(v_center[0]-v_maxs[0]),v_center[1],1,1,0,0,1,0,0,1);
			//left up
			DrawFrameCorner(v_maxs[0],v_maxs[1],0,1,1,0,1,1,0,0);
			//left right
			DrawFrameCorner(v_maxs[0]+(v_center[0]-v_maxs[0]),v_maxs[1],0,0,1,1,0,1,1,0);
		}
		gEngfuncs.pTriAPI->End();
		gEngfuncs.pTriAPI->RenderMode( kRenderNormal );

		//to make that working, modify in decay.dll player.cpp blank message value of frame kind to -1
		//if (!m_iFrameKind==-1)
		if (m_iFrameKind != -1)
		{
			char szMes[25];
			ReturnFrameHint(szMes,m_iFrameKind);
			gHUD.DrawHudString( v_maxs[0]+5, v_maxs[1]+4, ScreenWidth, szMes, 255, 180, 0 );
		}
	}

	/*
	char szMes[255];
	if ( m_bAlienMode )
	  sprintf( szMes, "Alien slave (vortigaunt) mode on" ); 
	else
	  sprintf( szMes, "Normal mode" );
  */

	//ReturnFrameHint(szMes,m_iFrameKind);
//	gHUD.DrawHudString( 10, 10, 512, szMes, 255, 180, 0 );

	//draw sun
    if (m_iLensIndex !=0)
	{
		vec3_t screen,ors;
		float tN[9];
		tN[0]=1.0;
		tN[1]=0.8;
		tN[2]=0.7;
		tN[3]=0.5;
		tN[4]=0.4;
        tN[5]=0.25;
		tN[6]=0.1;
		tN[7]=-0.1;
		tN[8]=-0.2;
		
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex(m_iLensIndex);
		if (ent)
		{
			vec3_t  forward, right, up;
	        AngleVectors ( ent->angles, forward, right, up );//get f/r/u vectors

			pmtrace_t tr;
			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)&v_origin, v_origin + (forward+up)/2 * 8192, PM_GLASS_IGNORE, -1, &tr );
			//pmtrace_t tr = *(gEngfuncs.PM_TraceLine( (float *)&v_origin, v_origin + (forward+up)/2 * 8192, 0, 2, -1 )); // PM_GLASS_IGNORE
			if (gEngfuncs.PM_PointContents( tr.endpos, NULL )!=CONTENTS_SKY)
			{
				return 0;
			}

			if (!gEngfuncs.pTriAPI->WorldToScreen(tr.endpos,ors)) 
			{
				int LWidth, LHeight;
				LWidth = NULL;
				LHeight = NULL;
				
				ors[0] = XPROJECT(ors[0]);
				ors[1] = YPROJECT(ors[1]);
				ors[2] = 0.0f;
				
				if (ors[0]<0 || ors[0]>ScreenWidth) return 0;
				if (ors[1]<0 || ors[1]>ScreenHeight) return 0;
				
				for (int i=0;i<9;i++)
				{
					if (i!=0) 
						SPR_Set(m_hsprLens[i], 50, 50, 50); //was 250 
					else 
						SPR_Set(m_hsprLens[i], 100, 100, 100); //make sun brighter then other lens
					LWidth = SPR_Width(m_hsprLens[i],0);
					LHeight = SPR_Height(m_hsprLens[i],0);
					
					screen=ors;
					screen[0] = ScreenWidth/2+(screen[0]-ScreenWidth/2)*tN[i];
					screen[1] = ScreenHeight/2+(screen[1]-ScreenHeight/2)*tN[i];
					screen[0] = screen[0]-(LWidth/2);
					screen[1] = screen[1]-(LHeight/2);
					
					SPR_DrawAdditive(0, screen[0],screen[1], NULL);
				}
			}
		}
	}

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );
		
		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );
		
		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/

	return 1;
}

void ScaleColors( int &r, int &g, int &b, int a )
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud :: DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b )
{
	// draw the string until we hit the null character or a newline character
	for ( ; *szIt != 0 && *szIt != '\n'; szIt++ )
	{
		int next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next > iMaxX )
			return xpos;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
		xpos = next;		
	}

	return xpos;
}

int CHud :: DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b )
{
	char szString[32];
	sprintf( szString, "%d", iNumber );
	return DrawHudStringReverse( xpos, ypos, iMinX, szString, r, g, b );

}

// draws a string from right to left (right-aligned)
int CHud :: DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b )
{
	// find the end of the string
	for ( char *szIt = szString; *szIt != 0; szIt++ )
	{ // we should count the length?		
	}

	// iterate throug the string in reverse
	for ( szIt--;  szIt != (szString-1);  szIt-- )	
	{
		int next = xpos - gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next < iMinX )
			return xpos;
		xpos = next;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
	}

	return xpos;
}

int CHud :: DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;
	
	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			 k = iNumber/100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100)/10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
		SPR_DrawAdditive(0,  x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	} 
	else if (iFlags & DHN_DRAWZERO) 
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b );

		// SPR_Draw 100's
		if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		
		SPR_DrawAdditive( 0,  x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth( int iNumber, int iFlags )
{
	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;

}	


