#include "draw.h"
#include <iostream>
using namespace std;

int main()
{
	Camera camera;
	Viewport viewport;
	Light light;
	vector<Sphere> spheres;
	vector<Triangle> triangles;
	vector<Plane> planes;
	CheckerBoard checkerboard;
	if (!init(camera, viewport, light, spheres, triangles, planes, checkerboard))
	{
		cerr << "Cannot read input file" << endl;
		system("pause");
		return 1;
	}
	rayTracing(camera, viewport, light, spheres, triangles, planes, checkerboard);
	output(viewport);
	return 0;
}