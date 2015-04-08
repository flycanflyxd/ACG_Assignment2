#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "algebra3.h"
#include <vector>
#include <limits>
#include "material.h"

class Sphere
{
public:
	float radius;
	vec3 center;
	Material material;
	Sphere(float x, float y, float z, float radius, Material material)
	{
		center.set(x, y, z);
		this->radius = radius;
		this->material = material;
	}
};

class Triangle
{
public:
	vec3 vertices[3];
	Material material;
};

class Plane
{
public:
	vec3 vertices[4];
	Material material;
};

class Intersection
{
public:
	char type;// what kind of object
	int index;// the index of the object
	float t;
	vec3 position;
	vec3 normal;
	Material material;
	Intersection()
	{
		t = std::numeric_limits<float>::max();
	}

};

#endif