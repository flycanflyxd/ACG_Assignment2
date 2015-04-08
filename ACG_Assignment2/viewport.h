#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "algebra3.h"

class Viewport
{
public:
	int width, height;
	float distance = 0.1;
	vec3 **pixel;
	vec3 startPosition;
	vec3 vectorWidth, vectorHeight;
};

#endif