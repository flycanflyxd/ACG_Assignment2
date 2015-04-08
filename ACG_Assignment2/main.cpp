#include "draw.h"
#include <iostream>

int main()
{
	Camera camera;
	Viewport viewport;
	Light light;
	std::vector<Sphere> spheres;
	std::vector<Triangle> triangles;
	std::vector<Plane> planes;
	if (!init(camera, viewport, light, spheres, triangles, planes))
	{
		std::cerr << "Cannot read input file" << std::endl;
		system("pause");
		return 1;
	}
	rayTracing(camera, viewport, light, spheres, triangles, planes);
	draw(viewport);
	return 0;
}