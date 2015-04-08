#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "algebra3.h"
#include <limits>

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

class Intersection
{
public:
	char type;// what kind of object
	int index;// the index of the object
	float t;
	vec3 position;
	vec3 normal;
	vec3 color;
	Intersection()
	{
		t = std::numeric_limits<float>::max();
	}

};

#endif