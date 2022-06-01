//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <windows.h>
//#include <gl/gl.h>
#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

//#include "com_model.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#define DLLEXPORT __declspec( dllexport )

#define GL_TEXTURE_RECTANGLE_NV 0x84F5

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};

#include "r_studioint.h"

extern engine_studio_api_t IEngineStudio;

bool InitCameraEffect(void);
void GrabCameraTexture(void);
void RenderCameraEffect(void);

/*
// FUNCTIONS
void BeginOrtho(void);
void EndOrtho(void);

void DrawQuad(int width, int height, int ofsX = 0, int ofsY = 0);
void DrawQuad2(int Width, int Height, int PosX, int PosY);

bool InitScreenGlow(void);
void RenderScreenGlow(void);

*/

/*
void BeginOrtho(void)
{
     glMatrixMode(GL_PROJECTION);
     glPushMatrix();
     glLoadIdentity();
     //glOrtho(0, 1, 1, 0, 0.1, 100);
     glOrtho(0, ScreenWidth, 0, ScreenHeight, -1, 1);

     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     glLoadIdentity();
}

void EndOrtho(void)
{
     glMatrixMode(GL_PROJECTION);
     glPopMatrix();
     glMatrixMode(GL_MODELVIEW);
     glPopMatrix();
}

void DrawQuad2(int Width, int Height, int PosX, int PosY)
{
  glBegin(GL_QUADS);
    glTexCoord2f( 0.0, ScreenHeight );
    glVertex3i (PosX,PosY+Height,-1);
    glTexCoord2f( ScreenWidth, ScreenHeight );
    glVertex3i (PosX+Width,PosY+Height,-1);
    glTexCoord2f( ScreenWidth, 0.0 );
    glVertex3i (PosX+Width,PosY,-1);
    glTexCoord2f( 0.0, 0.0 );
    glVertex3i (PosX,PosY,-1);
  glEnd();
}
*/
/*
void DrawQuad(int width, int height, int ofsX, int ofsY)
{
     glTexCoord2f(ofsX,height+ofsY);
     glVertex3i(0, ScreenHeight, -1);
     glTexCoord2f(ofsX,ofsY);
     glVertex3i(0, 0, -1);
     glTexCoord2f(width+ofsX,ofsY);
     glVertex3i(ScreenWidth, 0, -1);
     glTexCoord2f(width+ofsX,height+ofsY);
     glVertex3i(ScreenWidth, ScreenHeight, -1);
}

bool InitScreenGlow(void)
{
     // register the CVARs
     gEngfuncs.pfnRegisterVariable("effect_blur", "1", 0);
     gEngfuncs.pfnRegisterVariable("glow_blur_steps", "1", 0);
     gEngfuncs.pfnRegisterVariable("glow_darken_steps", "3", 0);
     gEngfuncs.pfnRegisterVariable("glow_strength", "2", 0);

     // create a load of blank pixels to create textures with
     unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
     memset(pBlankTex, 0, ScreenWidth*ScreenHeight*3);

     // Create the SCREEN-HOLDING TEXTURE
     glGenTextures(1, &gHUD.g_uiScreenTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiScreenTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

      // Create the BLURRED TEXTURE
     glGenTextures(1, &gHUD.g_uiGlowTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiGlowTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth/2, ScreenHeight/2, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

      // free the memory
     delete[] pBlankTex;

     return true;
}

void RenderScreenGlow(void)
{
	 int QuadWidth, QuadHeight;
 	 QuadWidth = ScreenWidth;
 	 QuadHeight = ScreenHeight;

     // check to see if (a) we can render it, and (b) we're meant to render it

	 if ((int)gEngfuncs.pfnGetCvarFloat("effect_blur") == 0)
		 return;

     if ((int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps") == 0 || (int)gEngfuncs.pfnGetCvarFloat("glow_strength") == 0)
         return;

     // enable some OpenGL stuff
     glColor3f(1,1,1);
     glDisable(GL_DEPTH_TEST);
	 glEnable(GL_TEXTURE_RECTANGLE_NV);

     // STEP 1: Grab the screen and put it into a texture
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiScreenTex);
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

     // STEP 2: Set up an orthogonal projection
     BeginOrtho();

     // STEP 3: Render the current scene to a new, lower-res texture, darkening non-bright areas of the scene
     // by multiplying it with itself a few times.

     glViewport(0, 0, QuadWidth, QuadHeight);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiScreenTex);
     glBlendFunc(GL_DST_COLOR, GL_ZERO);     
     glDisable(GL_BLEND);

     glBegin(GL_QUADS);
       DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();

     glEnable(GL_BLEND);

     glBegin(GL_QUADS);
     for (int i = 0; i < (int)gEngfuncs.pfnGetCvarFloat("glow_darken_steps"); i++)
       DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiGlowTex);
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, QuadWidth, QuadHeight, 0);

     // STEP 4: Blur the now darkened scene in the horizontal direction.

     float blurAlpha = 1 / (gEngfuncs.pfnGetCvarFloat("glow_blur_steps")*2 + 1);

     glColor4f(1,1,1,blurAlpha);

     glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

     glBegin(GL_QUADS);
       DrawQuad(QuadWidth, QuadHeight);
     glEnd();

     glBlendFunc(GL_SRC_ALPHA, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i <= (int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps"); i++) {
          DrawQuad(QuadWidth, QuadHeight, -i, 0);
          DrawQuad(QuadWidth, QuadHeight, i, 0);
     }
     glEnd();
     
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, QuadWidth, QuadHeight, 0);

     // STEP 5: Blur the horizontally blurred image in the vertical direction.

     glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

     glBegin(GL_QUADS);
       DrawQuad(QuadWidth, QuadHeight);
     glEnd();

     glBlendFunc(GL_SRC_ALPHA, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i <= (int)gEngfuncs.pfnGetCvarFloat("glow_blur_steps"); i++) {
          DrawQuad(QuadWidth, QuadHeight, 0, -i);
          DrawQuad(QuadWidth, QuadHeight, 0, i);
     }
     glEnd();
     
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, QuadWidth, QuadHeight, 0);

     // STEP 6: Combine the blur with the original image.

     glViewport(0, 0, ScreenWidth, ScreenHeight);

     glDisable(GL_BLEND);

     glBegin(GL_QUADS);
       DrawQuad(QuadWidth, QuadHeight);
     glEnd();

     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);

     glBegin(GL_QUADS);
     for (i = 1; i < (int)gEngfuncs.pfnGetCvarFloat("glow_strength"); i++) {
          DrawQuad(QuadWidth, QuadHeight);
     }
     glEnd();

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiScreenTex);
     glBegin(GL_QUADS);
         DrawQuad(ScreenWidth, ScreenHeight);
     glEnd();
     glDisable(GL_BLEND);

     // STEP 7: Restore the original projection and modelview matrices and disable rectangular textures.
	 EndOrtho();

     glDisable(GL_TEXTURE_RECTANGLE_NV);
     glEnable(GL_DEPTH_TEST);
}
*/

bool InitCameraEffect(void)
{
     // register the CVARs
     gEngfuncs.pfnRegisterVariable("effect_camera", "1", 0);

     // create a load of blank pixels to create textures with
     unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
     memset(pBlankTex, 0, ScreenWidth*ScreenHeight*3);

     // Create the SCREEN-HOLDING TEXTURE
/*     glGenTextures(1, &gHUD.g_uiCameraTex);
     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiCameraTex);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

      // free the memory
     delete[] pBlankTex;
*/
     return true;
}

void GrabCameraTexture(void)
{
	if (gHUD.m_iCamMode != CAM_ON)
		return;
/*
	 glColor3f(1,1,1);
     glDisable(GL_DEPTH_TEST);

     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiCameraTex);
     glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

     glEnable(GL_DEPTH_TEST);*/
}

void RenderCameraEffect(void)
{
    if ((int)gEngfuncs.pfnGetCvarFloat("effect_camera") == 0)
        return;

	if (gHUD.m_iCamMode != CAM_ON)
		return;
	/*
    BeginOrtho();
      glEnable(GL_TEXTURE_RECTANGLE_NV);     
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, gHUD.g_uiCameraTex);
      DrawQuad2(128, 128, 10, 10); //2
	  //DrawQuad2(ScreenWidth, ScreenHeight, 0, 0);
	  glDisable(GL_TEXTURE_RECTANGLE_NV);
    EndOrtho();
	*/
/*	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
		return;

  glPointSize(5);
  glColor3f(0, 1, 0);
  glBegin(GL_POINTS);
	//glVertex3f (gHUD.m_vecCamPos.x, gHUD.m_vecCamPos.y, gHUD.m_vecCamPos.z);
  glVertex3f(org.x, org.y, org.z);
  glEnd(); */


/*
     texture_t* CameraTex;
     model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
     msurface_t** s=world->marksurfaces;
     glpoly_t* p;

     for(int i=0;i<world->nummarksurfaces;i++)
     {
		if(s[i]->visframe!=-1)
		{
			CameraTex=s[i]->texinfo->texture;
			//if(strncmp("screenhl2", CameraTex->name, 16)==0)
			//{ 
				//yes, we've found screen polygon
				//CameraTex->gl_texturenum = gHUD.g_uiCameraTex;
				
				glBindTexture(GL_TEXTURE_2D, gHUD.g_uiCameraTex);
				p=s[i]->polys;
				glBegin (GL_POLYGON);
				float* v = p->verts[0];
				for (int t=0 ; t<p->numverts ; t++, v+= VERTEXSIZE)
				{
					glTexCoord2f (v[3], v[4]);
					glVertex3fv (v);
				}
				glEnd ();
				s[i]->visframe=-1;
			//}
		}
	}*/
}

//#define TEST_IT
#if defined( TEST_IT )

/*
=================
Draw_Triangles

Example routine.  Draws a sprite offset from the player origin.
=================
*/
void Draw_Triangles( void )
{
	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
		return;

	org = player->origin;

	org.x += 50;
	org.y += 50;

	if (gHUD.m_hsprCursor == 0)
	{
		char sz[256];
		sprintf( sz, "sprites/enter1.spr" );
		gHUD.m_hsprCursor = SPR_Load( sz );
	}

	if ( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ))
	{
		return;
	}
	
	// Create a triangle, sigh
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	// Overload p->color with index into tracer palette, p->packedColor with brightness
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );
	// UNDONE: This gouraud shading causes tracers to disappear on some cards (permedia2)
	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y, org.z );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

#endif

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{

	gHUD.m_Spectator.DrawOverview();
	
#if defined( TEST_IT )
	Draw_Triangles();
#endif
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{

#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}