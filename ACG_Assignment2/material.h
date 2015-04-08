#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "algebra3.h"

class Material
{
public:
	vec3 color;
	float Ka, Kd, Ks;
	int exp;
	float Reflact, Refract, Nr;
	void setMaterial(vec3 color, float Ka, float Kd, float Ks, int exp, float Reflact, float Refract, float Nr)
	{
		this->color = color;
		this->Ka = Ka;
		this->Kd = Kd;
		this->Ks = Ks;
		this->exp = exp;
		this->Reflact = Reflact;
		this->Refract = Refract;
		this->Nr = Nr;
	}
};

#endif