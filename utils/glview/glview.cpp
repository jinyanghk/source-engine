//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fully working Linux-native port of glview using X11 and GLX.
//          Bypasses the legacy Win32 glaux library completely.
//
//=============================================================================//
#include "glos.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "cmdlib.h"
#include "mathlib/mathlib.h"
#include "cmodel.h"
#include "tier1/strtools.h"
#include "physdll.h"
#include "phyfile.h"
#include "vphysics_interface.h"
#include "tier0/icommandline.h"
#include "tier0/vprof.h"

// X11 / GLX Native Window Loop Variables
Display             *g_pDisplay = NULL;
Window              g_Window = 0;
GLXContext          g_GlxContext = NULL; 

Vector origin(32.0f, 32.0f, 48.0f);
QAngle angles(0.0f, 0.0f, 0.0f);
Vector forward_v, right_v, vup, vpn, vright;
float	width = 1024;
float	height = 768;

float g_flMovementSpeed	= 320.f;		// Units / second (run speed of HL)
#define	SPEED_TURN	90		// Degrees / second

Vector g_Center;               // Center of all read points, so camera is in a sensible place
int g_nTotalPoints	   = 0;    // Total points read, for calculating center
int g_UseBlending      = 0;	   // Toggle to use blending mode or not
BOOL g_bReadPortals    = 0;	   // Did we read in a portal file?
BOOL g_bNoDepthPortals = 0;    // Do we zbuffer the lines of the portals?
int g_nPortalHighlight = -1;   // The leaf we're viewing
int g_nLeafHighlight = -1;     // The leaf we're viewing
BOOL g_bShowList1      = 1;	   // Show regular polygons?
BOOL g_bShowList2      = 1;	   // Show portals?
BOOL g_bShowLines      = 0;    // Show outlines of faces
BOOL g_Active = TRUE;
BOOL g_Update = TRUE;
BOOL g_bDisp = FALSE;
IPhysicsCollision *physcollision = NULL;

static int g_Keys[65536]; 
void KeyDown(int key);

BOOL ReadDisplacementFile( const char *filename );
void DrawDisplacementData( void );
void Draw(void);

unsigned int GetLinuxTimeMilliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void Error (char *error, ...)
{
	va_list argptr;
	char	text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

	printf("\n########################################\n");
	printf("GLVIEW CRITICAL ERROR: %s\n", text);
	printf("########################################\n\n");
	exit (1);
}

void KeyDown (int key)
{
	switch (key)
	{
	case XK_Escape:
		g_Active = FALSE;
		break;
	case XK_F1:
		glEnable (GL_CULL_FACE);
		glCullFace (GL_FRONT);
		break;
	case 'b':
	case 'B':
		g_UseBlending ^= 1;
		if (g_UseBlending)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		break;
	case '1':
		g_bShowList1 ^= 1;
		break;
	case '2':
		g_bShowList2 ^= 1;
		break;
	case 'p':
	case 'P':
		g_bNoDepthPortals ^= 1;
		break;
	case 'l':
	case 'L':
		g_bShowLines ^= 1;
		break;
	}
	g_Update = TRUE;
}

int Test_Key( int key )
{
	int r = (g_Keys[ key ] != 0);
	g_Keys[ key ] &= 0x01; // clear out debounce bit
	if (r)
		g_Update = TRUE;
	return r;
}

void Cam_Update( float frametime )
{
	if ( Test_Key( 'w' ) || Test_Key( 'W' ) )
	{
		VectorMA (origin.Base(), g_flMovementSpeed*frametime, vpn.Base(), origin.Base());
	}
	if ( Test_Key( 's' ) || Test_Key( 'S' ) )
	{
		VectorMA (origin.Base(), -g_flMovementSpeed*frametime, vpn.Base(), origin.Base());
	}
	if ( Test_Key( 'a' ) || Test_Key( 'A' ) )
	{
		VectorMA (origin.Base(), -g_flMovementSpeed*frametime, vright.Base(), origin.Base());
	}
	if ( Test_Key( 'd' ) || Test_Key( 'D' ) )
	{
		VectorMA (origin.Base(), g_flMovementSpeed*frametime, vright.Base(), origin.Base());
	}
	if ( Test_Key( XK_Up ) )
	{
		VectorMA (origin.Base(), g_flMovementSpeed*frametime, forward_v.Base(), origin.Base());
	}
	if ( Test_Key( XK_Down ) )
	{
		VectorMA (origin.Base(), -g_flMovementSpeed*frametime, forward_v.Base(), origin.Base());
	}
	if ( Test_Key( XK_Left ) )
	{
		angles[1] += SPEED_TURN * frametime;
	}
	if ( Test_Key( XK_Right ) )
	{
		angles[1] -= SPEED_TURN * frametime;
	}
	if ( Test_Key( 'f' ) || Test_Key( 'F' ) )
	{
		origin[2] += g_flMovementSpeed*frametime;
	}
	if ( Test_Key( 'c' ) || Test_Key( 'C' ) )
	{
		origin[2] -= g_flMovementSpeed*frametime;
	}
	if ( Test_Key( XK_Insert ) )
	{
		angles[0] += SPEED_TURN * frametime;
		if (angles[0] > 85)
			angles[0] = 85;
	}
	if ( Test_Key( XK_Delete ) )
	{
		angles[0] -= SPEED_TURN * frametime;
		if (angles[0] < -85)
			angles[0] = -85;
	}
}

void Cam_BuildMatrix (void)
{
	float	xa, ya;
	float	matrix[4][4];
	int		i;

	xa = angles[0]/180*M_PI;
	ya = angles[1]/180*M_PI;

    forward_v[0] = cos(ya);
    forward_v[1] = sin(ya);
    right_v[0] = forward_v[1];
    right_v[1] = -forward_v[0];

	glGetFloatv (GL_PROJECTION_MATRIX, &matrix[0][0]);

	for (i=0 ; i<3 ; i++)
	{
		vright[i] = matrix[i][0];
		vup[i] = matrix[i][1];
		vpn[i] = matrix[i][2];
	}

	VectorNormalize (vright);
	VectorNormalize (vup);
	VectorNormalize (vpn);
}

void Draw (void)
{
	float	screenaspect;
	float	yfov;

	glClearColor(0.0, 0.0, 0.0, 0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

    screenaspect = (float)width/height;
	yfov = 2*atan((float)height/width)*180/M_PI;
    gluPerspective (yfov,  screenaspect,  6,  20000);

    glRotatef (-90,  1, 0, 0);	    // put Z going up
    glRotatef (90,  0, 0, 1);	    // put Z going up
    glRotatef (angles[0],  0, 1, 0);
    glRotatef (-angles[1],  0, 0, 1);
    glTranslatef (-origin[0],  -origin[1],  -origin[2]);

	Cam_BuildMatrix ();

	glShadeModel (GL_SMOOTH);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (g_UseBlending)
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	glDepthFunc (GL_LEQUAL);

	if( g_bDisp )
	{
		DrawDisplacementData();
	}
	else
	{
		if (g_bShowList1)
			glCallList (1);
		
		if (g_bReadPortals)
		{
			if (g_bNoDepthPortals)
				glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			if (g_bShowList2)
				glCallList(2);
		};
		
		if (g_bShowLines)
			glCallList(3);
	}
}

void ReadPolyFileType(const char *name, int nList, BOOL drawLines)
{
	FILE	*f;
	int		i, j, numverts;
	float	v[6];
	int		c;
	int		r;
	float   divisor;

	f = fopen (name, "rt");
	if (!f)
		Error ("Couldn't open %s", name);

	if (g_bReadPortals)
		divisor = 2.0f;
	else 
		divisor = 1.0f;

	c = 0;
	glNewList (nList, GL_COMPILE);
	
	for (i = 0; i < 3; i++)
		g_Center[i] = 0.0f;

	if (drawLines)
		glLineWidth(1.5);

	while (1)
	{
		r = fscanf( f, "%i\n", &numverts);
		if (!r || r == EOF)
			break;

		if ( c > 65534*8)
			break;

		if (drawLines || numverts == 2)
			glBegin(GL_LINE_LOOP);
		else
			glBegin (GL_POLYGON);

		for (i=0 ; i<numverts ; i++)
		{
			r = fscanf( f, "%f %f %f %f %f %f\n", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);

			if (drawLines)
				glColor4f(1.0, 1.0, 0.0, 0.5);
			else
			{
				if (g_bReadPortals)
				{
					if (fabs(fabs(v[5]) - 1.0f) < 0.01)
					{
						glColor4f (v[3],v[4],v[5],0.5);   
					}	
					else
					{
						v[3] += v[4] + v[5];
						v[3] /= 3.0f;
						glColor4f (v[3]/divisor, v[3]/divisor, v[3]/divisor, 0.6);   
					}
				}
				else 
				{
					v[3] = pow( v[3], (float)(1.0 / 2.2) );
					v[4] = pow( v[4], (float)(1.0 / 2.2) );
					v[5] = pow( v[5], (float)(1.0 / 2.2) );

					glColor4f (v[3]/divisor, v[4]/divisor, 	v[5]/divisor, 0.6);
				};
			};
			glVertex3f (v[0], v[1], v[2]);

			for (j = 0; j < 3; j++)
			{
				g_Center[j] += v[j];
			}
			g_nTotalPoints++;
		}
		glEnd ();
		c++;
	}

	if (f)
		fclose(f);

	glEndList ();

	if (g_nTotalPoints > 0)
	{
		for (i = 0; i < 3; i++)
		{
			g_Center[i] = g_Center[i]/(float)g_nTotalPoints;
			origin[i] = g_Center[i];
		}
	}
}

struct phyviewparams_t
{ 
	Vector mins;
	Vector maxs;
	Vector offset;
	QAngle angles;
	int outputType;
	
	void Defaults()
	{
		ClearBounds(mins, maxs);
		offset.Init();
		outputType = GL_POLYGON;
		angles.Init();
	}
};

void AddVCollideToList( phyheader_t &header, vcollide_t &collide, phyviewparams_t &params )
{
	matrix3x4_t xform;
	AngleMatrix( params.angles, params.offset, xform );
	ClearBounds( params.mins, params.maxs );
	for ( int i = 0; i < header.solidCount; i++ )
	{
		ICollisionQuery *pQuery = physcollision->CreateQueryModel( collide.solids[i] );
		for ( int j = 0; j < pQuery->ConvexCount(); j++ )
		{
			for ( int k = 0; k < pQuery->TriangleCount(j); k++ )
			{
				Vector verts[3];
				pQuery->GetTriangleVerts( j, k, verts );
				Vector v0,v1,v2;
				VectorTransform( verts[0], xform, v0 );
				VectorTransform( verts[1], xform, v1 );
				VectorTransform( verts[2], xform, v2 );
				AddPointToBounds( v0, params.mins, params.maxs );
				AddPointToBounds( v1, params.mins, params.maxs );
				AddPointToBounds( v2, params.mins, params.maxs );

				glBegin(params.outputType);
				glColor3ub( 255, 0, 0 );
				glVertex3fv( v0.Base() );
				glColor3ub( 0, 255, 0 );
				glVertex3fv( v1.Base() );
				glColor3ub( 0, 0, 255 );
				glVertex3fv( v2.Base() );
				glEnd();
			}
		}
		physcollision->DestroyQueryModel( pQuery );
	}
}

void ReadPHYFile(const char *name, phyviewparams_t &params )
{
	FILE *fp = fopen (name, "rb");
	if (!fp)
		Error ("Couldn't open %s", name);

	phyheader_t header;
	fread( &header, sizeof(header), 1, fp );
	if ( header.size != sizeof(header) || header.solidCount <= 0 )
    {
        fclose(fp);
		return;
    }

	int pos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	int fileSize = ftell(fp) - pos;
	fseek( fp, pos, SEEK_SET );

	char *buf = (char *)malloc( fileSize );
	fread( buf, fileSize, 1, fp );
	fclose( fp );

	vcollide_t collide;
	physcollision->VCollideLoad( &collide, header.solidCount, (const char *)buf, fileSize );
	AddVCollideToList( header, collide, params );
    free(buf);
}

void ReadPolyFile (const char *name)
{
	char ext[4];
	Q_ExtractFileExtension( name, ext, 4 );

	bool isPHY = !Q_stricmp( ext, "phy" );
	if ( isPHY )
	{
		CreateInterfaceFn physicsFactory = GetPhysicsFactory();
		physcollision = (IPhysicsCollision *)physicsFactory( VPHYSICS_COLLISION_INTERFACE_VERSION, NULL );
		if ( physcollision )
		{
			phyviewparams_t params;
			params.Defaults();
			glNewList (1, GL_COMPILE);
			ReadPHYFile( name, params );
			Vector tmp = (params.mins + params.maxs) * 0.5;
			tmp.CopyToArray(origin.Base());
			glEndList ();
		}
	}
	else
	{
		ReadPolyFileType(name, 1, false);
		ReadPolyFileType(name, 3, true);
	}
}

void ReadPortalFile (char *name)
{
	FILE	*f;
	int		i, numverts;
	float	v[3];
	int		c;
	int		r;

	char szDummy[80];
	int nNumLeafs;
	int nNumPortals;
	int nLeafIndex[2];

	f = fopen (name, "r");
	if (!f)
		Error ("Couldn't open %s", name);

	c = 0;
	glNewList (2, GL_COMPILE);

	fscanf(f, "%79s\n", szDummy);
	fscanf(f, "%i\n", &nNumLeafs);
	fscanf(f, "%i\n", &nNumPortals);

	glLineWidth(1.5);

	while (1)
	{
		r = fscanf(f, "%i %i %i ", &numverts, &nLeafIndex[0], &nLeafIndex[1]);
		if (!r || r == EOF)
			break;

		glBegin(GL_LINE_LOOP);
		for (i=0 ; i<numverts ; i++)
		{
			r = fscanf (f, "(%f %f %f )\n", &v[0], &v[1], &v[2]);
			if (!r || (r != 3) || r == EOF)
				break;

			if ( c == g_nPortalHighlight || nLeafIndex[0] == g_nLeafHighlight || nLeafIndex[1] == g_nLeafHighlight )
			{
				glColor4f (1.0, 0.0, 0.0, 1.0);   
			}
			else
			{
				glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
			}
			glVertex3f (v[0], v[1], v[2]);
		}
		glEnd ();
		c++;
	}

	if (f)
		fclose(f);

	glEndList ();
}

#define MAX_DISP_COUNT	4096
static Vector dispPoints[MAX_DISP_COUNT];
static Vector dispNormals[MAX_DISP_COUNT];
static int dispPointCount = 0;

BOOL ReadDisplacementFile( const char *filename )
{
	FILE	*pFile;
	int		fileCount;

	pFile = fopen( filename, "r" );
	if( !pFile )
		Error( "Couldn't open %s", filename );

	while( 1 )
	{
		if( dispPointCount >= MAX_DISP_COUNT )
			break;

		fileCount = fscanf( pFile, "%f %f %f %f %f %f",
			                &dispPoints[dispPointCount][0], &dispPoints[dispPointCount][1], &dispPoints[dispPointCount][2],
							&dispNormals[dispPointCount][0], &dispNormals[dispPointCount][1], &dispNormals[dispPointCount][2] );
		dispPointCount++;

		if( !fileCount || ( fileCount == EOF ) )
			break;
	}

	fclose( pFile );
	return TRUE;
}

void DrawDisplacementData( void )
{
	int		i, j;
	int		width_d, halfCount;

	GLUquadricObj *pObject = gluNewQuadric();
	glEnable( GL_DEPTH_TEST );

	for( i = 0; i < dispPointCount; i++ )
	{
		glColor3f( 1.0f, 0.0f, 0.0f );
		glPushMatrix();
		glTranslatef( dispPoints[i][0], dispPoints[i][1], dispPoints[i][2] );
		gluSphere( pObject, 5, 5, 5 );
		glPopMatrix();

		glColor3f( 1.0f, 1.0f, 0.0f );
		glBegin( GL_LINES );
		glVertex3f( dispPoints[i][0], dispPoints[i][1], dispPoints[i][2] );
		glVertex3f( dispPoints[i][0] + ( dispNormals[i][0] * 50.0f ), dispPoints[i][1] + ( dispNormals[i][1] * 50.0f ), dispPoints[i][2] + ( dispNormals[i][2] * 50.0f ) );
		glEnd();
	}

	halfCount = dispPointCount / 2;
	width_d = sqrt( (float)halfCount );

	glDisable( GL_CULL_FACE );
	glColor3f( 0.0f, 0.0f, 1.0f );
	for( i = 0; i < width_d - 1; i++ )
	{
		for( j = 0; j < width_d - 1; j++ )
		{
			glBegin( GL_POLYGON );
			glVertex3f( dispPoints[i*width_d+j][0], dispPoints[i*width_d+j][1], dispPoints[i*width_d+j][2] );
			glVertex3f( dispPoints[(i+1)*width_d+j][0], dispPoints[(i+1)*width_d+j][1], dispPoints[(i+1)*width_d+j][2] );
			glVertex3f( dispPoints[(i+1)*width_d+(j+1)][0], dispPoints[(i+1)*width_d+(j+1)][1], dispPoints[(i+1)*width_d+(j+1)][2] );
			glVertex3f( dispPoints[i*width_d+(j+1)][0], dispPoints[i*width_d+(j+1)][1], dispPoints[i*width_d+(j+1)][2] );
			glEnd();
		}
	}

	glColor3f( 0.0f, 1.0f, 0.0f );
	for( i = 0; i < width_d - 1; i++ )
	{
		for( j = 0; j < width_d - 1; j++ )
		{
			glBegin( GL_POLYGON );
			glVertex3f( dispPoints[i*width_d+j][0] + ( dispNormals[i*width_d+j][0] * 150.0f ), dispPoints[i*width_d+j][1] + ( dispNormals[i*width_d+j][1] * 150.0f ), dispPoints[i*width_d+j][2] + ( dispNormals[i*width_d+j][2] * 150.0f ) );
			glVertex3f( dispPoints[(i+1)*width_d+j][0] + ( dispNormals[(i+1)*width_d+j][0] * 150.0f ), dispPoints[(i+1)*width_d+j][1] + ( dispNormals[(i+1)*width_d+j][1] * 150.0f ), dispPoints[(i+1)*width_d+j][2] + ( dispNormals[(i+1)*width_d+j][2] * 150.0f ) );
			glVertex3f( dispPoints[(i+1)*width_d+(j+1)][0] + ( dispNormals[(i+1)*width_d+(j+1)][0] * 150.0f ), dispPoints[(i+1)*width_d+(j+1)][1] + ( dispNormals[(i+1)*width_d+(j+1)][1] * 150.0f ), dispPoints[(i+1)*width_d+(j+1)][2] + ( dispNormals[(i+1)*width_d+(j+1)][2] * 150.0f ) );
			glVertex3f( dispPoints[i*width_d+(j+1)][0] + ( dispNormals[i*width_d+(j+1)][0] * 150.0f ), dispPoints[i*width_d+(j+1)][1] + ( dispNormals[i*width_d+(j+1)][1] * 150.0f ), dispPoints[i*width_d+(j+1)][2] + ( dispNormals[i*width_d+(j+1)][2] * 150.0f ) );
			glEnd();
		}
	}

	glDisable( GL_DEPTH_TEST );
	gluDeleteQuadric( pObject );
}

void CreateLinuxGLWindow(const char* title, int w, int h)
{
    g_pDisplay = XOpenDisplay(NULL);
    if (!g_pDisplay) {
        printf("Error: Cannot connect to X server\n");
        exit(1);
    }

    int attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *visual = glXChooseVisual(g_pDisplay, 0, attributes);
    if (!visual) {
        printf("Error: No appropriate visual found\n");
        exit(1);
    }
    
    Window root = DefaultRootWindow(g_pDisplay);
    XSetWindowAttributes winAttr;
    winAttr.colormap = XCreateColormap(g_pDisplay, root, visual->visual, AllocNone);
    winAttr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;

    g_Window = XCreateWindow(g_pDisplay, root, 0, 0, w, h, 0, 
                             visual->depth, InputOutput, visual->visual, 
                             CWColormap | CWEventMask, &winAttr);

    XMapWindow(g_pDisplay, g_Window);
    XStoreName(g_pDisplay, g_Window, title);

    g_GlxContext = glXCreateContext(g_pDisplay, visual, NULL, GL_TRUE);
    glXMakeCurrent(g_pDisplay, g_Window, g_GlxContext);
}

void ProcessLinuxEvents()
{
    XEvent event;
    while (XPending(g_pDisplay)) {
        XNextEvent(g_pDisplay, &event);
        KeySym keysym;
        switch (event.type) {
            case Expose:
                g_Update = TRUE;
                break;
            case ConfigureNotify:
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                glViewport(0, 0, width, height);
                g_Update = TRUE;
                break;
            case KeyPress:
                keysym = XLookupKeysym(&event.xkey, 0);
                if (keysym < 65536) {
                    g_Keys[keysym] = 0x03;
                    KeyDown(keysym);
                }
                break;
            case KeyRelease:
                keysym = XLookupKeysym(&event.xkey, 0);
                if (keysym < 65536) {
                    g_Keys[keysym] &= 0x02;
                }
                break;
        }
    }
}

SpewRetval_t Sys_SpewFunc( SpewType_t type, const char *pMsg )
{
	printf("%s", pMsg);
	if( type == SPEW_ASSERT )
		return SPEW_DEBUGGER;
	else if( type == SPEW_ERROR )
		return SPEW_ABORT;
	else
		return SPEW_CONTINUE;
}

int main(int argc, char* argv[])
{
	CommandLine()->CreateCmdLine( argc, argv );
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );

	if (CommandLine()->ParmCount() < 2)
		Error ("Usage: glview [-portal] [-disp] <filename.gl>");

	CreateLinuxGLWindow("Source Engine Geometry Viewer (GLView)", width, height);

	const char *pFileName = CommandLine()->GetParm( CommandLine()->ParmCount() - 1 );
	CmdLib_InitFileSystem( pFileName );

	if ( CommandLine()->CheckParm( "-portal") )
	{
		g_bReadPortals = 1;
		g_nPortalHighlight = CommandLine()->ParmValue( "-portalhighlight", -1 );
		g_nLeafHighlight = CommandLine()->ParmValue( "-leafhighlight", -1 );
	}
	g_flMovementSpeed = CommandLine()->ParmValue( "-speed", 320 );

	if( CommandLine()->CheckParm( "-disp") )
	{
		ReadDisplacementFile( pFileName );
		g_bDisp = TRUE;
	}
	SpewOutputFunc( Sys_SpewFunc );

	if (pFileName && pFileName && !g_bDisp )
	{
		ReadPolyFile( pFileName );
	}

	if (g_bReadPortals)
	{
		char szTempCmd[MAX_PATH];
		strcpy(szTempCmd, pFileName);
		char *pTmp = strrchr(szTempCmd, '.');
		if (pTmp) *pTmp = '\0';
		strcat(szTempCmd, ".prt");
		ReadPortalFile(szTempCmd);
	}

	unsigned int lastTime = GetLinuxTimeMilliseconds();

	while (g_Active)
	{
        ProcessLinuxEvents();

        unsigned int currentTime = GetLinuxTimeMilliseconds();
        float frametime = (currentTime - lastTime) * 0.001f;
        if (frametime > 0.2f) frametime = 0.2f;
        lastTime = currentTime;

        Cam_Update(frametime);

        if (g_Update)
        {
            Draw();
            glXSwapBuffers(g_pDisplay, g_Window);
            g_Update = FALSE;
        }
        else
        {
            usleep(1000); 
        }
	}

    if(g_GlxContext) glXDestroyContext(g_pDisplay, g_GlxContext);
    if(g_Window) XDestroyWindow(g_pDisplay, g_Window);
    if(g_pDisplay) XCloseDisplay(g_pDisplay);

    return 0;
}
