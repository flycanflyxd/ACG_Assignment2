#include "draw.h"

using namespace std;

bool init(Camera &camera, Viewport &viewport, Light &light, vector<Sphere> &spheres, vector<Triangle> &triangles, vector<Plane> &planes)
{
	char type;
	float input[10];
	string trash;
	Triangle triangle;
	Plane plane;
	Material material;

	ifstream fin("hw2_input.txt");
	if (!fin)
		return false;
	while (!fin.eof())
	{
		fin >> type;
		switch (type)
		{
		case 'E':
			fin >> camera.position[0] >> camera.position[1] >> camera.position[2];
			break;
		case 'V':
			fin >> camera.direction[0] >> camera.direction[1] >> camera.direction[2];
			break;
		case 'F':
			fin >> camera.FOV;
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
					viewport.pixel[i][j][0] = 0.3;
					viewport.pixel[i][j][1] = 0.3;
					viewport.pixel[i][j][2] = 0.3;
				}
			break;
		case 'S':
			for (int i = 0; i < 4; i++)
				fin >> input[i];
			spheres.push_back(Sphere(input[0], input[1], input[2], input[3], material));
			break;
		case 'T':
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					fin >> input[j];
				triangle.vertices[i].set(input[0], input[1], input[2]);
			}
			triangle.material = material;
			triangles.push_back(triangle);
			break;
		case 'P':
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 3; j++)
					fin >> input[j];
				plane.vertices[i].set(input[0], input[1], input[2]);
			}
			plane.material = material;
			planes.push_back(plane);
			break;
		case 'L':
			for (int i = 0; i < 6; i++)
				fin >> input[i];
			light.setLight(vec3(input[0] , input[1], input[2])/*position*/, vec3(input[3], input[4], input[5])/*color*/);
			break;
		case 'M':
			for (int i = 0; i < 10; i++)
				fin >> input[i];
			material.setMaterial(vec3(input[0], input[1], input[2]), input[3], input[4], input[5], input[6], input[7], input[8], input[9]);
			break;
		default:
			//return false;
			getline(fin, trash);
			break;
		}
	}
	return true;
}

void PhongShading(Camera cmaera, Intersection &intersection, vec3 &pixel, Light light)
{
	Material material = intersection.material;
	float Ka = material.Ka, Kd = material.Kd, Ks = material.Ks;
	// Ambient
	vec3 ambient = Ka * light.color;
	ambient = prod(ambient / 255, material.color / 255);

	// Diffuse
	vec3 lightDirection = (light.position - intersection.position).normalize();
	vec3 diffuse = MAX(intersection.normal * lightDirection, 0.0) * light.color;
	diffuse = Kd * prod(diffuse / 255, material.color / 255);

	// Specular
	vec3 specular;
	int exp = material.exp;
	vec3 viewDirection = (cmaera.position - intersection.position).normalize();
	vec3 H = (lightDirection + viewDirection).normalize();
	specular = Ks * light.color / 255 * pow(MAX(intersection.normal * H, 0.0), exp);
	pixel = ambient + diffuse + specular;
}

bool shadow(Intersection point, Light light, vec3 &pixel, vector<Sphere> &spheres, vector<Triangle> &triangles, vector<Plane> &planes)
{
	float a = 0, b = 0, c = 0;
	vec3 ray = (light.position - point.position).normalize();
	vec3 v1, v2, normal, intersection;
	float t, d;
	// Sphere
	for (int nSphere = 0; nSphere < spheres.size(); nSphere++)
	{
		a = 1; //a = nx^2 + ny^2 + nz^2 = 1 cuz it's normalized
		for (int k = 0; k < 3; k++)
		{
			b += 2 * ray[k] * (point.position[k] - spheres[nSphere].center[k]);
			c += pow(point.position[k] - spheres[nSphere].center[k], 2);
		}
		c -= pow(spheres[nSphere].radius, 2);
		if ((-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a) > 0 && !(point.type == 's' && point.index == nSphere))
		{
			pixel = vec3(0, 0, 0);
			return false;
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
		t = (d - (normal[0] * point.position[0] + normal[1] * point.position[1] + normal[2] * point.position[2]))
			/ (normal[0] * ray[0] + normal[1] * ray[1] + normal[2] * ray[2]);
		intersection = point.position + ray * t;
		if (((triangles[nTriangle].vertices[1] - triangles[nTriangle].vertices[0]) ^ (intersection - triangles[nTriangle].vertices[0])) * normal >= 0
			&& ((triangles[nTriangle].vertices[2] - triangles[nTriangle].vertices[1]) ^ (intersection - triangles[nTriangle].vertices[1])) * normal >= 0
			&& ((triangles[nTriangle].vertices[0] - triangles[nTriangle].vertices[2]) ^ (intersection - triangles[nTriangle].vertices[2])) * normal >= 0)
		{
			/*
			(p1- p0) x (intersection - p0) * normal >= 0
			&& (p2- p1) x (intersection - p1) * normal >= 0
			&& (p0- p2) x (intersection - p2) * normal >= 0
			the intersection is at the same side of each line
			*/
			if (t > 0)
			{
				pixel = vec3(0, 0, 0);
				return false;
			}
		}
	}
	// Plane
	for (int nPlane = 0; nPlane < planes.size(); nPlane++)
	{
		v1 = planes[nPlane].vertices[1] - planes[nPlane].vertices[0];
		v2 = planes[nPlane].vertices[2] - planes[nPlane].vertices[0];
		normal = (v1 ^ v2).normalize();
		d = normal[0] * planes[nPlane].vertices[0][0] + normal[1] * planes[nPlane].vertices[0][1] + normal[2] * planes[nPlane].vertices[0][2];
		t = (d - (normal[0] * point.position[0] + normal[1] * point.position[1] + normal[2] * point.position[2]))
			/ (normal[0] * ray[0] + normal[1] * ray[1] + normal[2] * ray[2]);
		intersection = point.position + ray * t;
		if (((planes[nPlane].vertices[1] - planes[nPlane].vertices[0]) ^ (intersection - planes[nPlane].vertices[0])) * normal >= 0
			&& ((planes[nPlane].vertices[2] - planes[nPlane].vertices[1]) ^ (intersection - planes[nPlane].vertices[1])) * normal >= 0
			&& ((planes[nPlane].vertices[3] - planes[nPlane].vertices[2]) ^ (intersection - planes[nPlane].vertices[2])) * normal >= 0
			&& ((planes[nPlane].vertices[0] - planes[nPlane].vertices[3]) ^ (intersection - planes[nPlane].vertices[3])) * normal >= 0)
		{
			if (t > 0)
			{
				pixel = vec3(0, 0, 0);
				return false;
			}
		}
	}
	return true;
}

void rayTracing(Camera &camera, Viewport &viewport, Light light, vector<Sphere> &spheres, vector<Triangle> &triangles, vector<Plane> &planes)
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
					t = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
					if (t < intersection.t && t > 0)
					{
						intersection.type = 's';
						intersection.index = nSphere;
						intersection.t = t;
						intersection.position = camera.position + ray * t;
						intersection.normal = (intersection.position - spheres[nSphere].center).normalize();
						intersection.material = spheres[nSphere].material;
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
					if (t < intersection.t && t > 0)
					{
						intersection.type = 't';
						intersection.index = nTriangle;
						intersection.t = t;
						intersection.position = point;
						intersection.normal = normal;
						intersection.material = triangles[nTriangle].material;
					}
				}
			}
			// Plane
			for (int nPlane = 0; nPlane < planes.size(); nPlane++)
			{
				v1 = planes[nPlane].vertices[1] - planes[nPlane].vertices[0];
				v2 = planes[nPlane].vertices[2] - planes[nPlane].vertices[0];
				normal = (v1 ^ v2).normalize();
				d = normal[0] * planes[nPlane].vertices[0][0] + normal[1] * planes[nPlane].vertices[0][1] + normal[2] * planes[nPlane].vertices[0][2];
				t = (d - (normal[0] * camera.position[0] + normal[1] * camera.position[1] + normal[2] * camera.position[2]))
					/ (normal[0] * ray[0] + normal[1] * ray[1] + normal[2] * ray[2]);
				point = camera.position + ray * t;
				if (((planes[nPlane].vertices[1] - planes[nPlane].vertices[0]) ^ (point - planes[nPlane].vertices[0])) * normal >= 0
					&& ((planes[nPlane].vertices[2] - planes[nPlane].vertices[1]) ^ (point - planes[nPlane].vertices[1])) * normal >= 0
					&& ((planes[nPlane].vertices[3] - planes[nPlane].vertices[2]) ^ (point - planes[nPlane].vertices[2])) * normal >= 0
					&& ((planes[nPlane].vertices[0] - planes[nPlane].vertices[3]) ^ (point - planes[nPlane].vertices[3])) * normal >= 0)
				{
					if (t < intersection.t)
					{
						intersection.type = 'p';
						intersection.index = nPlane;
						intersection.t = t;
						intersection.position = point;
						intersection.normal = normal;
						intersection.material = planes[nPlane].material;
					}
				}
			}
			if (intersection.t != numeric_limits<float>::max() && shadow(intersection, light, viewport.pixel[i][j], spheres, triangles, planes))
			{
				PhongShading(camera, intersection, viewport.pixel[i][j], light);
			}
			intersection.t = numeric_limits<float>::max();
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
			viewport.pixel[x][y] *= 255;
			for (int i = 0; i < 3; i++)
				if (viewport.pixel[x][y][i] > 255)
					viewport.pixel[x][y][i] = 255;
			p.R = viewport.pixel[x][y][0];
			p.G = viewport.pixel[x][y][1];
			p.B = viewport.pixel[x][y][2];
			image.writePixel(y, x, p);
		}
	}
	image.outputPPM("rayTracing.ppm");
}