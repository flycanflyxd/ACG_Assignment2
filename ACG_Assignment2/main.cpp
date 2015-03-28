#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "algebra3.h"
#include "imageIO.cpp"

using namespace std;

class Camera
{
public:
	vec3 position;
	vec3 direction;
	float FOV;
};

class Viewport
{
public:
	int width, height;
	float distance = 0.1;
	bool **pixel;
	vec3 startPosition;
	vec3 vectorWidth, vectorHeight;
};

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

bool init(Camera &camera, Viewport &viewport, vector<Sphere> &spheres, vector<Triangle> &triangles)
{
	char type;
	float input[4];
	Triangle triangle;
	ifstream fin("hw2_input.txt");
	if (!fin)
		return false;
	while (!fin.eof())
	{
		fin >> type;
		switch (type)
		{
		case 'E':
			for (int i = 0; i < 3; i++)
				fin >> input[i];
			camera.position.set(input[0], input[1], input[2]);
			break;
		case 'V':
			for (int i = 0; i < 3; i++)
				fin >> input[i];
			camera.direction.set(input[0], input[1], input[2]);
			break;
		case 'F':
			fin >> input[0];
			camera.FOV = input[0];
			break;
		case 'R':
			fin >> viewport.width;
			fin >> viewport.height;
			viewport.pixel = new bool*[viewport.height];
			for (int i = 0; i < viewport.height; i++)
				viewport.pixel[i] = new bool[viewport.width];
			for (int i = 0; i < viewport.height; i++)
				for (int j = 0; j < viewport.width; j++)
					viewport.pixel[i][j] = false;
			break;
		case 'S':
			for (int i = 0; i < 4; i++)
				fin >> input[i];
			spheres.push_back(Sphere(input[0], input[1], input[2], input[3]));
			break;
		case 'T':
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					fin >> input[j];
				triangle.vertices[i].set(input[0], input[1], input[2]);
			}
			triangles.push_back(triangle);
			break;
		default:
			return false;
			break;
		}
	}
	return true;
}

void rayTracing(Camera &camera, Viewport &viewport, vector<Sphere> &spheres, vector<Triangle> &triangles)
{
	//calculate the center position of the viewport
	float t;
	t = sqrt(pow(viewport.distance, 2) / (pow(camera.direction[0], 2) + pow(camera.direction[1], 2) + pow(camera.direction[2], 2)));
	vec3 planeCenter = camera.position + camera.direction * t;
	//viewport plane nx * x + ny * y + nz * z = d
	float d = camera.direction[0] * planeCenter[0] + camera.direction[1] * planeCenter[1] + camera.direction[2] * planeCenter[2];
	vec3 vDownCenter = camera.direction, vLeftCenter = camera.direction;
	//find the vectors of camera to down center edge and left center edge
	vDownCenter = rotation3D(vec3(0.0, 1.0, 0.0) ^ camera.direction, camera.FOV / 2) * vDownCenter;
	vLeftCenter = rotation3D(vec3(0.0, 1.0, 0.0), camera.FOV / 2) * vLeftCenter;
	vec3 downCenter;
	t = (d - (camera.direction[0] * camera.position[0] + camera.direction[1] * camera.position[1] + camera.direction[2] * camera.position[2]))
		/ (camera.direction[0] * vDownCenter[0] + camera.direction[1] * vDownCenter[1] + camera.direction[2] * vDownCenter[2]);
	downCenter = camera.position + vDownCenter * t;
	vec3 leftCenter;
	t = (d - (camera.direction[0] * camera.position[0] + camera.direction[1] * camera.position[1] + camera.direction[2] * camera.position[2]))
		/ (camera.direction[0] * vLeftCenter[0] + camera.direction[1] * vLeftCenter[1] + camera.direction[2] * vLeftCenter[2]);
	leftCenter = camera.position + vLeftCenter * t;
	viewport.startPosition = planeCenter - downCenter + leftCenter;
	viewport.vectorWidth = (planeCenter - leftCenter) / (viewport.width * 0.5);
	viewport.vectorHeight = (downCenter - planeCenter) / (viewport.height * 0.5);
	vec3 ray;
	vec3 v1, v2, normal;
	vec3 intersection;
	float a = 0, b = 0, c = 0;
	for (int i = 0; i < viewport.height; i++)
	{
		for (int j = 0; j < viewport.width; j++)
		{
			ray = viewport.startPosition + i * viewport.vectorHeight + j * viewport.vectorWidth - camera.position;
			ray.normalize(); //normalize the vector
			//sphere
			for (int nSphere = 0; nSphere < spheres.size(); nSphere++)
			{
				a = 1; //a = nx^2 + ny^2 + nz^2 = 1 cuz it's normalized
				for (int k = 0; k < 3; k++)
				{
					b += 2 * ray[k] * (camera.position[k] - spheres[nSphere].center[k]);
					c += pow(camera.position[k] - spheres[nSphere].center[k], 2);
				}
				c -= pow(spheres[nSphere].radius, 2);
				if (pow(b, 2) - 4 * a * c >= 0)
					viewport.pixel[i][j] = true;
				a = b = c = 0;
			}
			//triangle
			for (int nTriangle = 0; nTriangle < triangles.size(); nTriangle++)
			{
				v1 = triangles[nTriangle].vertices[1] - triangles[nTriangle].vertices[0];
				v2 = triangles[nTriangle].vertices[2] - triangles[nTriangle].vertices[0];
				normal = v1 ^ v2;
				d = normal[0] * triangles[nTriangle].vertices[0][0] + normal[1] * triangles[nTriangle].vertices[0][1] + normal[2] * triangles[nTriangle].vertices[0][2];
				t = (d - (normal[0] * camera.position[0] + normal[1] * camera.position[1] + normal[2] * camera.position[2]))
					/ (normal[0] * ray[0] + normal[1] * ray[1] + normal[2] * ray[2]);
				intersection = camera.position + ray * t;
				if (((triangles[nTriangle].vertices[1] - triangles[nTriangle].vertices[0]) ^ (intersection - triangles[nTriangle].vertices[0])) * normal >= 0
					&& ((triangles[nTriangle].vertices[2] - triangles[nTriangle].vertices[1]) ^ (intersection - triangles[nTriangle].vertices[1])) * normal >= 0
					&& ((triangles[nTriangle].vertices[0] - triangles[nTriangle].vertices[2]) ^ (intersection - triangles[nTriangle].vertices[2])) * normal >= 0)
					/*
					(p1- p0) x (intersection - p0) * normal >= 0
					&& (p2- p1) x (intersection - p1) * normal >= 0
					&& (p0- p2) x (intersection - p2) * normal >= 0
					the intersection is at the same side of each line
					*/
					viewport.pixel[i][j] = true;
			}
		}
	}
}

void draw(Viewport &viewport)
{
	ColorImage image;
	int x, y;
	Pixel p = { 0, 0, 0 };

	image.init(viewport.width, viewport.height);
	for (x = 0; x < viewport.width; x++) {
		for (y = 0; y < viewport.height; y++) {
			if (viewport.pixel[x][y])
				p.R = p.G = p.B = 255;
			else
				p.R = p.G = p.B = 0;
			image.writePixel(y, x, p);
		}
	}
	image.outputPPM("rayTracing.ppm");
}

int main()
{
	Camera camera;
	Viewport viewport;
	vector<Sphere> spheres;
	vector<Triangle> triangles;
	if (!init(camera, viewport, spheres, triangles))
	{
		cerr << "Cannot read input file" << endl;
		system("pause");
		return 1;
	}
	rayTracing(camera, viewport, spheres, triangles);
	draw(viewport);
	return 0;
}