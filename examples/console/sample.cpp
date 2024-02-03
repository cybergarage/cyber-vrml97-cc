/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-1997
*
*	File:	sample.cpp
*
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "CyberVRML97.h"

void main(int argc, char **argv)
{
    if (argc < 2){
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(0);
    }

	SceneGraph	sceneGraph;

	sceneGraph.load(argv[1], false);
	if (sceneGraph.isOK())
		sceneGraph.print();
	else
		printf("Error(%d) : %s\n", sceneGraph.getErrorLineNumber(), sceneGraph.getErrorLineString());
}
