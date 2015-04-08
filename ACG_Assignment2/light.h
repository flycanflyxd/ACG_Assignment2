#ifndef _LIGHT_H
#define _LIGHT_H

#include "algebra3.h"

class Light
{
public:
	vec3 position;
	vec3 color;
	void setLight(vec3 position, vec3 color)
	{
		this->position = position;
		this->color = color;
	}
};

#endif