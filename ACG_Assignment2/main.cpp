#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
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
	vec3 **pixel;
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

class Intersection
{
public:
	float t;
	vec3 position;
	vec3 normal;
	vec3 color;
	Intersection()
	{
		t = numeric_limits<float>::max();
	}

};

bool init(Camera &camera, Viewport &viewport, Light &light, vector<Sphere> &spheres, vector<Triangle> &triangles)
{
	char type;
	float input[4];
	Triangle triangle;
	// init light
	light.setLight(vec3(2.0, 2.0, -2.0)/*position*/, vec3(255, 255, 255)/*color*/);
	
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
			viewport.pixel = new vec3*[viewport.height];
			for (int i = 0; i < viewport.height; i++)
				viewport.pixel[i] = new vec3[viewport.width];
			for (int i = 0; i < viewport.height; i++)
				for (int j = 0; j < viewport.width; j++)
				{
					viewport.pixel[i][j][0] = 100;
					viewport.pixel[i][j][1] = 100;
					viewport.pixel[i][j][2] = 100;
				}
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

void PhongShading(Camera cmaera, Intersection &intersection, vec3 &pixel, Light light/*, float Ka, float Kd, float Ks*/)
{
	float Ka = 0.1, Kd, Ks;
	// Ambient
	vec3 ambient = Ka * light.color;
	vec3 objectColor(100, 100, 255);
	ambient = prod(ambient / 255, objectColor / 255);
	//pixel = ambient;

	// Diffuse
	vec3 lightDirection = (light.position - intersection.position).normalize();
	vec3 diffuse = MAX(intersection.normal * lightDirection, 0.0) * light.color;
	diffuse = prod(diffuse / 255, objectColor / 255);
	/*pixel = (ambient + diffuse) * 255;
	for (int i = 0; i < 3; i++)
		if (pixel[i] > 255)
			pixel[i] = 255;*/

	// Specular
	vec3 specular;
	int exp = 32;
	Ks = 0.5;
	vec3 viewDirection = (cmaera.position - intersection.position).normalize();
	vec3 H = (lightDirection + viewDirection).normalize();
	specular = Ks * light.color / 255 * pow(MAX(intersection.normal * H, 0), exp);
	pixel = (ambient + diffuse + specular) * 255;
	for (int i = 0; i < 3; i++)
		if (pixel[i] > 255)
			pixel[i] = 255;
}

void rayTracing(Camera &camera, Viewport &viewport, Light light, vector<Sphere> &spheres, vector<Triangle> &triangles)
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
	vec3 point;
	Intersection intersection;
	float a = 0, b = 0, c = 0;
	for (int i = 0; i < viewport.height; i++)
	{
		for (int j = 0; j < viewport.width; j++)
		{
			ray = viewport.startPosition + i * viewport.vectorHeight + j * viewport.vectorWidth - camera.position;
			ray.normalize(); //normalize the vector
			// Sphere
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
				{
					t = (-b - sqrt(pow(b, 2) - 4 * a * c)) / 2 * a;
					if (t < intersection.t)
					{
						intersection.t = t;
						intersection.position = camera.position + ray * t;
						intersection.normal = (intersection.position - spheres[nSphere].center).normalize();
					}
				}
				a = b = c = 0;
			}
			// Triangle
			for (int nTriangle = 0; nTriangle < triangles.size(); nTriangle++)
			{
				v1 = triangles[nTriangle].vertices[1] - triangles[nTriangle].vertices[0];
				v2 = triangles[nTriangle].vertices[2] - triangles[nTriangle].vertices[0];
				normal = (v1 ^ v2).normalize();
				d = normal[0] * triangles[nTriangle].vertices[0][0] + normal[1] * triangles[nTriangle].vertices[0][1] + normal[2] * triangles[nTriangle].vertices[0][2];
				t = (d - (normal[0] * camera.position[0] + normal[1] * camera.position[1] + normal[2] * camera.position[2]))
					/ (normal[0] * ray[0] + normal[1] * ray[1] + normal[2] * ray[2]);
				point = camera.position + ray * t;
				if (((triangles[nTriangle].vertices[1] - triangles[nTriangle].vertices[0]) ^ (point - triangles[nTriangle].vertices[0])) * normal >= 0
					&& ((triangles[nTriangle].vertices[2] - triangles[nTriangle].vertices[1]) ^ (point - triangles[nTriangle].vertices[1])) * normal >= 0
					&& ((triangles[nTriangle].vertices[0] - triangles[nTriangle].vertices[2]) ^ (point - triangles[nTriangle].vertices[2])) * normal >= 0)
				{
					/*
					(p1- p0) x (intersection - p0) * normal >= 0
					&& (p2- p1) x (intersection - p1) * normal >= 0
					&& (p0- p2) x (intersection - p2) * normal >= 0
					the intersection is at the same side of each line
					*/
					/*viewport.pixel[i][j][0] = 255;
					viewport.pixel[i][j][1] = 255;
					viewport.pixel[i][j][2] = 255;*/
					if (t < intersection.t)
					{
						intersection.t = t;
						intersection.position = point;
						intersection.normal = normal;
					}
				}
			}
			if (intersection.t != numeric_limits<float>::max())
			{
				PhongShading(camera, intersection, viewport.pixel[i][j], light);
				intersection.t = numeric_limits<float>::max();
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
			p.R = viewport.pixel[x][y][0];
			p.G = viewport.pixel[x][y][1];
			p.B = viewport.pixel[x][y][2];
			/*for (int i = 0; i < 3; i++)
				cout << viewport.pixel[x][y][i] << " ";
			cout << endl;
			system("pause");*/
			image.writePixel(y, x, p);
		}
	}
	image.outputPPM("rayTracing.ppm");
}

int main()
{
	Camera camera;
	Viewport viewport;
	Light light;
	vector<Sphere> spheres;
	vector<Triangle> triangles;
	if (!init(camera, viewport, light, spheres, triangles))
	{
		cerr << "Cannot read input file" << endl;
		system("pause");
		return 1;
	}
	rayTracing(camera, viewport, light, spheres, triangles);
	draw(viewport);
	return 0;
}