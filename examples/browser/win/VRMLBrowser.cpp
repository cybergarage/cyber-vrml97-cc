/******************************************************************
*
*	VRML library for C++
*
*	Copyright (C) Satoshi Konno 1996-1997
*
*	File:	VRMLBrowser.cpp
*
******************************************************************/
#include <windows.h>   
#include <windowsx.h>   
#include <commdlg.h>   
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "CyberVRML97.h"
#include "resource.h"

#define MOUSE_BUTTON_NONE	0
#define MOUSE_BUTTON_LEFT	1

static char		szTitle[] = "VRML Browser for WIN32";
static int		mouseButton = MOUSE_BUTTON_NONE;
static int		mxPos, myPos;

static SceneGraph		sceneGraph;
static PointLightNode	defaultLight;

enum {
OGL_RENDERING_WIRE,
OGL_RENDERING_SHADE,
OGL_RENDERING_TEXTURE,
};

LONG WINAPI WndProc( HWND, UINT, WPARAM, LPARAM );

////////////////////////////////////////////////////////// 
//  OpenGLSetup
////////////////////////////////////////////////////////// 

HGLRC OpenGLSetup( HWND hWnd )
{
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof (PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
        PFD_TYPE_RGBA,
        24,
        0, 0, 0,               
        0, 0, 0,               
        0, 0,        
        0, 0, 0, 0, 0, 
        32,	
        0,
        0,
        PFD_MAIN_PLANE, 
        0,	 
        0,	 
        0,	 
        0	 
    };

    int nMyPixelFormatID;
    HDC hDC;
    HGLRC hRC;

    hDC = GetDC( hWnd );
    nMyPixelFormatID = ChoosePixelFormat( hDC, &pfd );

    SetPixelFormat( hDC, nMyPixelFormatID, &pfd );

    hRC = wglCreateContext( hDC );
    ReleaseDC( hWnd, hDC );

    return hRC;
}

////////////////////////////////////////////////////////// 
//  OpenGLSetSize
////////////////////////////////////////////////////////// 

void OpenGLSetSize(SceneGraph *sg, int width, int height) 
{
	GLdouble aspect = (GLdouble)width/(GLdouble)height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ViewpointNode *view = sg->getViewpointNode();
	if (view == NULL)
		view = sg->getDefaultViewpointNode();

	float fov = (view->getFieldOfView() / 3.14f) * 180.0f;

	gluPerspective(fov, aspect, 0.1f, 10000.0f);

	glViewport( 0, 0, width, height );
}

////////////////////////////////////////////////////////// 
//  DrawSceneGraph
////////////////////////////////////////////////////////// 

static int gnLights;

void PushLightNode(LightNode *lightNode)
{
	if (!lightNode->isOn()) 
		return;

	GLint	nMaxLightMax;
	glGetIntegerv(GL_MAX_LIGHTS, &nMaxLightMax);

	if (nMaxLightMax < gnLights) {
		gnLights++;
		return;
	}

	float	color[4];
	float	pos[4];
	float	attenuation[3];
	float	direction[3];
	float	intensity;

	if (lightNode->isPointLightNode()) {
		
		PointLightNode *pLight = (PointLightNode *)lightNode;

		glEnable(GL_LIGHT0+gnLights);
		
		pLight->getAmbientColor(color);
		glLightfv(GL_LIGHT0+gnLights, GL_AMBIENT, color);

		pLight->getColor(color);
		intensity = pLight->getIntensity();
		color[0] *= intensity; 
		color[1] *= intensity; 
		color[2] *= intensity; 
		glLightfv(GL_LIGHT0+gnLights, GL_DIFFUSE, color);
		glLightfv(GL_LIGHT0+gnLights, GL_SPECULAR, color);

		pLight->getLocation(pos); pos[3] = 1.0f;
		glLightfv(GL_LIGHT0+gnLights, GL_POSITION, pos);

		direction[0] = 0.0f; direction[0] = 0.0f; direction[0] = 0.0f;
		glLightfv(GL_LIGHT0+gnLights, GL_SPOT_DIRECTION, direction);
		glLightf(GL_LIGHT0+gnLights, GL_SPOT_EXPONENT, 0.0f);
		glLightf(GL_LIGHT0+gnLights, GL_SPOT_CUTOFF, 180.0f);

		pLight->getAttenuation(attenuation);
		glLightf(GL_LIGHT0+gnLights, GL_CONSTANT_ATTENUATION, attenuation[0]);
		glLightf(GL_LIGHT0+gnLights, GL_LINEAR_ATTENUATION, attenuation[1]);
		glLightf(GL_LIGHT0+gnLights, GL_QUADRATIC_ATTENUATION, attenuation[2]);
		
		gnLights++;
	}
	else if (lightNode->isDirectionalLightNode()) {

		DirectionalLightNode *dLight = (DirectionalLightNode *)lightNode;
		
		glEnable(GL_LIGHT0+gnLights);
		
		dLight->getAmbientColor(color);
		glLightfv(GL_LIGHT0+gnLights, GL_AMBIENT, color);

		dLight->getColor(color);
		intensity = dLight->getIntensity();
		color[0] *= intensity; 
		color[1] *= intensity; 
		color[2] *= intensity; 
		glLightfv(GL_LIGHT0+gnLights, GL_DIFFUSE, color);
		glLightfv(GL_LIGHT0+gnLights, GL_SPECULAR, color);

		dLight->getDirection(pos); pos[3] = 0.0f;
		glLightfv(GL_LIGHT0+gnLights, GL_POSITION, pos);

		direction[0] = 0.0f; direction[0] = 0.0f; direction[0] = 0.0f;
		glLightfv(GL_LIGHT0+gnLights, GL_SPOT_DIRECTION, direction);
		glLightf(GL_LIGHT0+gnLights, GL_SPOT_EXPONENT, 0.0f);
		glLightf(GL_LIGHT0+gnLights, GL_SPOT_CUTOFF, 180.0f);

		glLightf(GL_LIGHT0+gnLights, GL_CONSTANT_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0+gnLights, GL_LINEAR_ATTENUATION, 0.0);
		glLightf(GL_LIGHT0+gnLights, GL_QUADRATIC_ATTENUATION, 0.0);

		gnLights++;
	}
	else if (lightNode->isSpotLightNode()) {

		SpotLightNode *sLight = (SpotLightNode *)lightNode;

		glEnable(GL_LIGHT0+gnLights);
		
		sLight->getAmbientColor(color);
		glLightfv(GL_LIGHT0+gnLights, GL_AMBIENT, color);

		sLight->getColor(color);
		intensity = sLight->getIntensity();
		color[0] *= intensity; 
		color[1] *= intensity; 
		color[2] *= intensity; 
		glLightfv(GL_LIGHT0+gnLights, GL_DIFFUSE, color);
		glLightfv(GL_LIGHT0+gnLights, GL_SPECULAR, color);

		sLight->getLocation(pos); pos[3] = 1.0f;
		glLightfv(GL_LIGHT0+gnLights, GL_POSITION, pos);

		sLight->getDirection(direction);
		glLightfv(GL_LIGHT0+gnLights, GL_SPOT_DIRECTION, direction);

		glLightf(GL_LIGHT0+gnLights, GL_SPOT_EXPONENT, 0.0f);
		glLightf(GL_LIGHT0+gnLights, GL_SPOT_CUTOFF, sLight->getCutOffAngle());

		sLight->getAttenuation(attenuation);
		glLightf(GL_LIGHT0+gnLights, GL_CONSTANT_ATTENUATION, attenuation[0]);
		glLightf(GL_LIGHT0+gnLights, GL_LINEAR_ATTENUATION, attenuation[1]);
		glLightf(GL_LIGHT0+gnLights, GL_QUADRATIC_ATTENUATION, attenuation[2]);

		gnLights++;
	}
}

void PopLightNode(LightNode *lightNode)
{
	if (!lightNode->isOn()) 
		return;

	GLint	nMaxLightMax;
	glGetIntegerv(GL_MAX_LIGHTS, &nMaxLightMax);

	gnLights--;
	
	if (gnLights < nMaxLightMax)
		glDisable(GL_LIGHT0+gnLights);
}

void DrawShapeNode(SceneGraph *sg, ShapeNode *shape, int drawMode)
{
	glPushMatrix ();

	/////////////////////////////////
	//	Appearance(Material)
	/////////////////////////////////

	float	color[4];
	color[3] = 1.0f;

	AppearanceNode			*appearance = shape->getAppearanceNodes();
	MaterialNode			*material = NULL;
	ImageTextureNode		*imgTexture = NULL;
	TextureTransformNode	*texTransform = NULL;

	bool				bEnableTexture = false;

	if (appearance) {

		// Texture Transform
		TextureTransformNode *texTransform = appearance->getTextureTransformNodes();
		if (texTransform) {
			float texCenter[2];
			float texScale[2];
			float texTranslation[2];
			float texRotation;

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();

			texTransform->getTranslation(texTranslation);
			glTranslatef(texTranslation[0], texTranslation[1], 0.0f);

			texTransform->getCenter(texCenter);
			glTranslatef(texCenter[0], texCenter[1], 0.0f);

			texRotation = texTransform->getRotation();
			glRotatef(0.0f, 0.0f, 1.0f, texRotation);

			texTransform->getScale(texScale);
			glScalef(texScale[0], texScale[1], 1.0f);

			glTranslatef(-texCenter[0], -texCenter[1], 0.0f);
		}
		else {
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(0.0f, 0.0f, 1.0f);
		}

		glMatrixMode(GL_MODELVIEW);

		// Texture
		imgTexture = appearance->getImageTextureNodes();
		if (imgTexture && drawMode == OGL_RENDERING_TEXTURE) {

			int width = imgTexture->getWidth();
			int height = imgTexture->getHeight();
			RGBAColor32 *color = imgTexture->getImage();

			if (0 < width && 0 < height && color != NULL) 
				bEnableTexture = true;

			if (bEnableTexture == true) {
				if (imgTexture->hasTransparencyColor() == true) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				else
					glDisable(GL_BLEND);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glBindTexture(GL_TEXTURE_2D, imgTexture->getTextureName());

				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				glEnable(GL_TEXTURE_2D);

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
				glEnable(GL_COLOR_MATERIAL);
			}
			else {
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_COLOR_MATERIAL);
			}
		}
		else {
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_COLOR_MATERIAL);
		}

		// Material
		material = appearance->getMaterialNodes();
		if (material) {
			float	ambientIntesity = material->getAmbientIntensity();

			material->getDiffuseColor(color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

			material->getSpecularColor(color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

			material->getEmissiveColor(color);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);

			material->getDiffuseColor(color);
			color[0] *= ambientIntesity; 
			color[1] *= ambientIntesity; 
			color[2] *= ambientIntesity; 
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

			glMateriali(GL_FRONT, GL_SHININESS, (int)(material->getShininess()*128.0));
		}

	}
	
	if (!appearance || !material) {
		color[0] = 0.8f; color[1] = 0.8f; color[2] = 0.8f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
		color[0] = 0.8f*0.2f; color[1] = 0.8f*0.2f; color[2] = 0.8f*0.2f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, (int)(0.2*128.0));
	}

	if (!appearance || !imgTexture || drawMode != OGL_RENDERING_TEXTURE) {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	/////////////////////////////////
	//	Transform 
	/////////////////////////////////

	float	m4[4][4];
	shape->getTransformMatrix(m4);
	glMultMatrixf((float *)m4);

	glColor3f(1.0f, 1.0f, 1.0f);

	/////////////////////////////////
	//	Geometry
	/////////////////////////////////

	GeometryNode *gnode = shape->getGeometry();
	if (gnode) {
		if (0 < gnode->getDisplayList())
			gnode->draw();
	}

	ShapeNode *selectedShapeNode = sg->getSelectedShapeNode();
	if (gnode && selectedShapeNode == shape) {
		float	bboxCenter[3];
		float	bboxSize[3];
		gnode->getBoundingBoxCenter(bboxCenter);
		gnode->getBoundingBoxSize(bboxSize);

		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_LIGHTING);
//		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		glBegin(GL_LINES);
		int x, y, z;
		for (x=0; x<2; x++) {
			for (y=0; y<2; y++) {
				float point[3];
				point[0] = (x==0) ? bboxCenter[0] - bboxSize[0] : bboxCenter[0] + bboxSize[0];
				point[1] = (y==0) ? bboxCenter[1] - bboxSize[1] : bboxCenter[1] + bboxSize[1];
				point[2] = bboxCenter[2] - bboxSize[2];
				glVertex3fv(point);
				point[2] = bboxCenter[2] + bboxSize[2];
				glVertex3fv(point);
			}
		}
		for (x=0; x<2; x++) {
			for (z=0; z<2; z++) {
				float point[3];
				point[0] = (x==0) ? bboxCenter[0] - bboxSize[0] : bboxCenter[0] + bboxSize[0];
				point[1] = bboxCenter[1] - bboxSize[1];
				point[2] = (z==0) ? bboxCenter[2] - bboxSize[2] : bboxCenter[2] + bboxSize[2];
				glVertex3fv(point);
				point[1] = bboxCenter[1] + bboxSize[1];
				glVertex3fv(point);
			}
		}
		for (y=0; y<2; y++) {
			for (z=0; z<2; z++) {
				float point[3];
				point[0] = bboxCenter[0] - bboxSize[0];
				point[1] = (y==0) ? bboxCenter[1] - bboxSize[1] : bboxCenter[1] + bboxSize[1];
				point[2] = (z==0) ? bboxCenter[2] - bboxSize[2] : bboxCenter[2] + bboxSize[2];
				glVertex3fv(point);
				point[0] = bboxCenter[0] + bboxSize[0];
				glVertex3fv(point);
			}
		}
		glEnd();

		glEnable(GL_LIGHTING);
//		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	}

	glPopMatrix();
}


void DrawNode(SceneGraph *sceneGraph, Node *firstNode, int drawMode) 
{
	if (!firstNode)
		return;

	Node	*node;

	for (node = firstNode; node; node=node->next()) {
		if (node->isLightNode()) 
			PushLightNode((LightNode *)node);
	}

	for (node = firstNode; node; node=node->next()) {
		if (node->isShapeNode()) 
			DrawShapeNode(sceneGraph, (ShapeNode *)node, drawMode);
		else
			DrawNode(sceneGraph, node->getChildNodes(), drawMode);
	}

	for (node = firstNode; node; node=node->next()) {
		if (node->isLightNode()) 
			PopLightNode((LightNode *)node);
	}
}

void DrawSceneGraph(SceneGraph *sg, int drawMode)
{
	ViewpointNode *view = sceneGraph.getViewpointNode();
	if (view == NULL)
		view = sg->getDefaultViewpointNode();

	if (view) {
		GLint	viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		OpenGLSetSize(sg, viewport[2], viewport[3]);
	}

    glEnable(GL_DEPTH_TEST);
	switch (drawMode) {
	case OGL_RENDERING_WIRE:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	default:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
//	glShadeModel (GL_FLAT);
    glShadeModel (GL_SMOOTH);

	float clearColor[] = {0.0f, 0.0f, 0.0f};
	
	BackgroundNode *bg = sg->getBackgroundNode();
	if (bg != NULL) {
		if (0 < bg->getNSkyColors())
			bg->getSkyColor(0, clearColor);
	}

	glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if (!view)
		return;

	/////////////////////////////////
	//	ViewpointNode 
	/////////////////////////////////

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	float	m4[4][4];
	view->getMatrix(m4);
	glMultMatrixf((float *)m4);

	/////////////////////////////////
	//	Light
	/////////////////////////////////

	GLint	nMaxLightMax;
	glGetIntegerv(GL_MAX_LIGHTS, &nMaxLightMax);
	for (int n = 0; n < nMaxLightMax; n++)
		glDisable(GL_LIGHT0+n);

	/////////////////////////////////
	//	General Node
	/////////////////////////////////

	gnLights = 0;

	DrawNode(sg, sg->getNodes(), drawMode);

    glFlush ();
}

////////////////////////////////////////////////////////// 
//  LoadSceneGraph
////////////////////////////////////////////////////////// 

void LoadSceneGraph(HWND hWnd)
{
    OPENFILENAME ofn;     
	char szFile[256];        
    char szFileTitle[256];   
	char szFilter[256] = "VRML Files(*.wrl)\0*.wrl\0All Files(*.*)\0*.*\0";

	szFile[0] = '\0';

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = (LPSTR)NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = (LPSTR)NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
	            OFN_FILEMUSTEXIST;
	ofn.nFileOffset = (WORD)NULL;
	ofn.nFileExtension = (WORD)NULL;
	ofn.lpstrDefExt = (LPSTR)NULL;
	ofn.lCustData = 0L;
	ofn.lpfnHook = (LPOFNHOOKPROC)NULL;
	ofn.lpTemplateName = (LPSTR)NULL;

	if (GetOpenFileName(&ofn) == 0)
		return;

	sceneGraph.load(ofn.lpstrFile);
	if (!sceneGraph.isOK()) {
		char	msg[1024];
		sprintf(msg, "Loading Error (%d) : %s", sceneGraph.getErrorLineNumber(), sceneGraph.getErrorLineString());
		MessageBox(hWnd, msg, szTitle, MB_ICONEXCLAMATION | MB_OK);
	}
	else {
		sceneGraph.initialize();
		if (sceneGraph.getViewpointNode() == NULL)
			sceneGraph.zoomAllViewpoint();
	}
}

////////////////////////////////////////////////////////// 
//  InitializeJavaEnv
////////////////////////////////////////////////////////// 

void InitializeJavaEnv(SceneGraph *sg) 
{
#ifdef SUPPORT_JSAI
	char classPath[1024];
	sprintf(classPath, ".\\;.\\cv97_102a.jar;%s", getenv("CLASSPATH"));
	sg->setJavaEnv(classPath);
#endif
}

////////////////////////////////////////////////////////// 
//  OnPaint
////////////////////////////////////////////////////////// 

void OnPaint(void) 
{
	NavigationInfoNode *navInfo = sceneGraph.getNavigationInfoNode();
	if (navInfo == NULL)
		navInfo = sceneGraph.getDefaultNavigationInfoNode();

	if (navInfo->getHeadlight()) {
		float	location[3];
		ViewpointNode *view = sceneGraph.getViewpointNode();
		if (view == NULL)
			view = sceneGraph.getDefaultViewpointNode();
		view->getPosition(location);
		defaultLight.setLocation(location);
		defaultLight.setAmbientIntensity(0.3f);
		defaultLight.setIntensity(0.7f);
		sceneGraph.addNode(&defaultLight);
	}

	DrawSceneGraph(&sceneGraph, OGL_RENDERING_TEXTURE);

	glFinish();
	SwapBuffers(wglGetCurrentDC());

	defaultLight.remove();
}

////////////////////////////////////////////////////////// 
//  MoveViewpoint
////////////////////////////////////////////////////////// 

void MoveViewpoint(HWND hWnd, SceneGraph *sg, int mosx, int mosy)
{
	ViewpointNode *view = sg->getViewpointNode();
	if (view == NULL)
		view = sg->getDefaultViewpointNode();

	NavigationInfoNode *navInfo = sg->getNavigationInfoNode();
	if (navInfo == NULL)
		navInfo = sg->getDefaultNavigationInfoNode();

	RECT winRect;
	GetWindowRect(hWnd, &winRect);
	int width = winRect.right - winRect.left;
	int height = winRect.bottom - winRect.top;

	float	trans[3] = {0.0f, 0.0f, 0.0f};
	float	rot[4] = {0.0f, 0.0f, 1.0f, 0.0f};

	static float transScale = 1.0f /10.0f;
	static float rotScale = (3.1415f/120.0f);

	trans[2] = (float)(mosy - height/2) / (float)(height/2) * navInfo->getSpeed() * transScale;
	rot[0] = 0.0f;
	rot[1] = 1.0f;
	rot[2] = 0.0f;
	rot[3] = -(float)(mosx - width/2) / (float)(width/2) * rotScale;

	view->translate(trans);
	view->rotate(rot);
}

////////////////////////////////////////////////////////// 
//  WinMain
////////////////////////////////////////////////////////// 

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{	
	static char szAppName[] = "OpenGL";
	WNDCLASS	wc;
	MSG			msg; 
	HWND		hWnd;

	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)WndProc;	  
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;  
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MENU);  
	wc.lpszClassName	= szAppName; 

	RegisterClass( &wc );
  
	hWnd = CreateWindow(
				szAppName,
				szTitle,
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				CW_USEDEFAULT, CW_USEDEFAULT, 300, 300,
				NULL,	 
				NULL,	 
				hInstance,
				NULL);

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );		

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage( &msg ); 
		DispatchMessage( &msg );
		if (mouseButton != MOUSE_BUTTON_NONE)
			MoveViewpoint(hWnd, &sceneGraph, mxPos, myPos);
		sceneGraph.update();
		InvalidateRect(hWnd, NULL, NULL);
		UpdateWindow(hWnd);
	}

	return( msg.wParam ); 
}

////////////////////////////////////////////////////////// 
//  WndProc
////////////////////////////////////////////////////////// 
 
LONG WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	static HDC		hDC;
    static HGLRC	hRC;
	
	SceneGraph		*sg = &sceneGraph;

	switch (msg){
	case WM_CREATE: 
		InitializeJavaEnv(&sceneGraph);
		hRC = OpenGLSetup( hWnd );
		hDC = GetDC (hWnd);
		wglMakeCurrent (hDC, hRC);
		return 0;

	case WM_SIZE:
		OpenGLSetSize(&sceneGraph, (GLsizei)LOWORD(lParam), (GLsizei)HIWORD(lParam));
	     return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case MENU_FILEOPEN:
			LoadSceneGraph(hWnd);
			InvalidateRect(hWnd, NULL, NULL);
			break;
		case MENU_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		return 0;

	case WM_LBUTTONDOWN:
		mouseButton = MOUSE_BUTTON_LEFT;
		return 0;

	case WM_LBUTTONUP:
		mouseButton = MOUSE_BUTTON_NONE;
		return 0;

	case WM_MOUSEMOVE:
		mxPos = LOWORD(lParam); 
		myPos = HIWORD(lParam); 
		return 0;

	case WM_DESTROY:
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( hRC );
		ReleaseDC( hWnd, hDC );
		PostQuitMessage( 0 );
		return 0;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

