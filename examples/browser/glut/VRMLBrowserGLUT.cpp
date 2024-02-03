/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2000
*
*	File:	VRMLBrowserGLUT.cpp
*
******************************************************************/
#if defined(WIN32)
#include <windows.h>   
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "CyberVRML97.h"

enum {
MOUSE_BUTTON_NONE,
MOUSE_BUTTON_LEFT,
};

enum {
OGL_RENDERING_WIRE,
OGL_RENDERING_SHADE,
OGL_RENDERING_TEXTURE,
};

static SceneGraph sceneGraph;
static PointLightNode defaultLight;

static int mouseButton = MOUSE_BUTTON_NONE;
static int mousePos[2];

////////////////////////////////////////////////////////// 
//  GLUT Callback Functions
////////////////////////////////////////////////////////// 

void SetSize(int width, int height) 
{
	GLdouble aspect = (GLdouble)width/(GLdouble)height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ViewpointNode *view = sceneGraph.getViewpointNode();
	if (view == NULL)
		view = sceneGraph.getDefaultViewpointNode();

	float fov = (view->getFieldOfView() / 3.14f) * 180.0f;

	gluPerspective(fov, aspect, 0.1f, 10000.0f);

	glViewport( 0, 0, width, height );
}

void MouseMotion(int x, int y)
{
	mousePos[0] = x;
	mousePos[1] = y;
}

void MouseButton(int button, int state, int x, int y)
{
	mouseButton = MOUSE_BUTTON_NONE;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		mouseButton = MOUSE_BUTTON_LEFT;
	mousePos[0] = x;
	mousePos[1] = y;
}

void MoveViewpoint(SceneGraph *sg, int mosx, int mosy)
{
	ViewpointNode *view = sg->getViewpointNode();
	if (view == NULL)
		view = sg->getDefaultViewpointNode();

	NavigationInfoNode *navInfo = sg->getNavigationInfoNode();
	if (navInfo == NULL)
		navInfo = sg->getDefaultNavigationInfoNode();

	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

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

void UpdateSceneGraph()
{
	if (mouseButton == MOUSE_BUTTON_LEFT)
		MoveViewpoint(&sceneGraph, mousePos[0], mousePos[1]);
	sceneGraph.update();
	glutPostRedisplay();
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
	TextureNode				*textureNode = NULL;
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
		textureNode = appearance->getTextureNode();
		if (textureNode && drawMode == OGL_RENDERING_TEXTURE) {

			int width = textureNode->getWidth();
			int height = textureNode->getHeight();
			RGBAColor32 *color = textureNode->getImage();

			if (0 < width && 0 < height && color != NULL) 
				bEnableTexture = true;

			if (bEnableTexture == true) {
				if (textureNode->hasTransparencyColor() == true) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				else
					glDisable(GL_BLEND);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glBindTexture(GL_TEXTURE_2D, textureNode->getTextureName());

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

	if (!appearance || !textureNode || drawMode != OGL_RENDERING_TEXTURE) {
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
		SetSize(viewport[2], viewport[3]);
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

void DrawSceneGraph(void) 
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
	glutSwapBuffers();

	defaultLight.remove();
}

////////////////////////////////////////////////////////// 
//  main
////////////////////////////////////////////////////////// 

void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("VRML Browser for GLUT");

	glutReshapeFunc(SetSize);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	glutMouseFunc(MouseButton);
	glutDisplayFunc(DrawSceneGraph);
	glutIdleFunc(UpdateSceneGraph);

	if (2 <= argc) {
		char *filename = argv[1];
		sceneGraph.load(filename);
		sceneGraph.initialize();
		if (sceneGraph.getViewpointNode() == NULL)
			sceneGraph.zoomAllViewpoint();
	}
	
	glutMainLoop();
}
