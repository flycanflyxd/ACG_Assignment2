#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "algebra3.h"

class Sphere
{
public:
	float radius;
	vec3 center;
	Sphere(float x, float y, float z, float radius)
	{
		center.set(x, y, z);
		this->radius = radius;
	}
};

class Triangle
{
public:
	vec3 vertices[3];
};

class Plane
{
public:
	vec3 vertices[4];
};

#endif