/*==================================================================================
* COSC 363  Computer Graphics (2022)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab06.pdf  for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "Pyramid.h"
#include "Cylinder.h"

using namespace std;

const float EDIST = 40.0;	//distance of image plane from camera
const int NUMDIV = 500;		//number of cells along x and y directions
const int MAX_STEPS = 5;	//number of levels of recursion

//boundary values of image plane
const float XMIN = -10.0; 
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject*> sceneObjects;

glm::vec3 calculateLighting(SceneObject* sourceObj, glm::vec3 lightPos, Ray ray) {

	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	glm::vec3 color;

	if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {
		SceneObject* shadowObj = sceneObjects[shadowRay.index];
		if (shadowObj->isTransparent() || shadowObj->isRefractive())
		{
			color = shadowObj->getTransparencyCoeff() * sourceObj->lighting(lightPos, -ray.dir, ray.hit);
		}
		else {
			color = { 0,0,0 };
		}
	}
	else {
		color = sourceObj->lighting(lightPos, -ray.dir, ray.hit);
	}
	return color;
}


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0.1, 0.1, 0.4);						//Background colour = (0,0,0)
	glm::vec3 lightPos1(10, 40, -3);					//Light's position
	glm::vec3 lightPos2(-20, 90, -100);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	if (ray.index == 1 || ray.index == 2)
	{
		//Checker pattern
		int checkerWidth = 5;
		int iz = (ray.hit.z) / checkerWidth;
		int iy = (ray.hit.y) / checkerWidth;
		int k = abs(iz) % 2; //2 colors
		int l = abs(iy) % 2;
		if (ray.hit.y < 0) { l = (l + 1) % 2; }
		if (k == l) color = glm::vec3(0.8, 0.4, 0.2);
		else color = glm::vec3(0.5, 0.2, 0);
		obj->setColor(color);
	}

	glm::vec3 lighting1 = calculateLighting(obj, lightPos1, ray);
	glm::vec3 lighting2 = calculateLighting(obj, lightPos2, ray);
	color = 0.2f * obj->getColor() + lighting1 + lighting2;

	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	if (obj->isTransparent() && step < MAX_STEPS)
	{
		float transparencyCoeff = obj->getTransparencyCoeff();
		Ray innerRay(ray.hit, ray.dir);
		innerRay.closestPt(sceneObjects);
		Ray continueRay(innerRay.hit, ray.dir);
		glm::vec3 backgroundColour = trace(continueRay, step + 1);
		color = transparencyCoeff * backgroundColour + color;
	}

	if (obj->isRefractive() && step < MAX_STEPS)
	{
		float refractionCoeff = obj->getRefractionCoeff();
		float refractionIndex = obj->getRefractiveIndex();
		float eta = 1.0 / refractionIndex;
		glm::vec3 n = obj->normal(ray.hit);
		glm::vec3 g = glm::refract(ray.dir, n, eta);
		Ray innerRay(ray.hit, g);
		innerRay.closestPt(sceneObjects);
		glm::vec3 m = obj->normal(innerRay.hit);
		glm::vec3 h = glm::refract(innerRay.dir, -m, 0.1f/eta);
		Ray continueRay(innerRay.hit, h);
		glm::vec3 backgroundColour = trace(continueRay, step + 1);
		color = refractionCoeff * backgroundColour + color;
	}
	
	//fog
	float fogz1 = -50;
	float fogz2 = -250;
	if (ray.hit.z < fogz2) {
		color = backgroundCol;
	}
	else if (ray.hit.z < fogz1) {
		float t = (ray.hit.z - fogz1) / (fogz2 - fogz1);
		color = ((1.0f - t) * color) + (t * backgroundCol);
	}

	return color;
}

void display_loop(float cellX, float cellY, glm::vec3 eye) {
	float xp, yp;  //grid point
	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + (i + 0.5) * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + (j + 0.5) * cellY;

			glm::vec3 dir(xp, yp, -EDIST);	//direction of the primary ray

			Ray ray = Ray(eye, dir);

			glm::vec3 col = trace(ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}
}

void display_loop_anti_alias(float cellX, float cellY, glm::vec3 eye) {
	float xp, yp;  //grid point
	float xgap = cellX / 4;
	float ygap = cellY / 4;
	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + (i + 0.5) * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + (j + 0.5) * cellY;

			glm::vec3 dir1(xp + xgap, yp + ygap, -EDIST);
			glm::vec3 dir2(xp + xgap, yp + ygap * 3, -EDIST);
			glm::vec3 dir3(xp + xgap * 3, yp + ygap, -EDIST);
			glm::vec3 dir4(xp + xgap * 3, yp + ygap * 3, -EDIST);

			Ray ray1 = Ray(eye, dir1);
			Ray ray2 = Ray(eye, dir2);
			Ray ray3 = Ray(eye, dir3);
			Ray ray4 = Ray(eye, dir4);

			glm::vec3 col1 = trace(ray1, 1);
			glm::vec3 col2 = trace(ray2, 1);
			glm::vec3 col3 = trace(ray3, 1);
			glm::vec3 col4 = trace(ray4, 1);

			glColor3f((col1.r + col2.r + col3.r + col4.r) / 4,
					(col1.g + col2.g + col3.g + col4.g) / 4, 
					(col1.b + col2.b + col3.b + col4.b) / 4);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	//display_loop(cellX, cellY, eye);
	display_loop_anti_alias(cellX, cellY, eye);

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

	Plane* plane1 = new Plane(glm::vec3(-30., -15, -40), //Point A
		glm::vec3(30., -15, -40), //Point B
		glm::vec3(30., -15, -300), //Point C
		glm::vec3(-30., -15, -300)); //Point D
	plane1->setColor(glm::vec3(0.7, 0.3, 0));
	plane1->setSpecularity(false);
	plane1->setReflectivity(true, 1);
	sceneObjects.push_back(plane1);

	Plane* plane2 = new Plane(glm::vec3(-30., -15, -40), //Point A
		glm::vec3(-30., -15, -300), //Point B
		glm::vec3(-30., 15, -300), //Point C
		glm::vec3(-30., 15, -40)); //Point D
	plane2->setColor(glm::vec3(0.8, 0.0, 0));
	plane2->setSpecularity(false);
	plane2->setReflectivity(true, 0.1);
	sceneObjects.push_back(plane2);

	Plane* plane3 = new Plane(glm::vec3(30., -15, -40), //Point A
		glm::vec3(30., 15, -40), //Point B
		glm::vec3(30., 15, -300), //Point C
		glm::vec3(30., -15, -300)); //Point D
	plane3->setColor(glm::vec3(0.8, 0.0, 0));
	plane3->setSpecularity(false);
	plane3->setRefractivity(true, 1, 1.5);
	sceneObjects.push_back(plane3);

	Cylinder* cylinder1 = new Cylinder(glm::vec3(-20, -15, -180), 1, 30);
	cylinder1->setColor(glm::vec3(.4, 0.1, 0.2));
	cylinder1->setTransparency(true, .3);
	sceneObjects.push_back(cylinder1);

	Cylinder* cylinder2 = new Cylinder(glm::vec3(-20, -15, -150), 1, 30);
	cylinder2->setColor(glm::vec3(.4, 0.1, 0.2));
	cylinder2->setTransparency(true, .3);
	sceneObjects.push_back(cylinder2);

	Cylinder* cylinder3 = new Cylinder(glm::vec3(-20, -15, -120), 1, 30);
	cylinder3->setColor(glm::vec3(.4, 0.1, 0.2));
	cylinder3->setTransparency(true, .3);
	sceneObjects.push_back(cylinder3);

	Cylinder* cylinder4 = new Cylinder(glm::vec3(-20, -15, -90), 1, 30);
	cylinder4->setColor(glm::vec3(.4, 0.1, 0.2));
	cylinder4->setTransparency(true, .3);
	sceneObjects.push_back(cylinder4);

	Pyramid* pyramid = new Pyramid(
		glm::vec3(0, 10, -140),
		glm::vec3(0, -15, -120),
		glm::vec3(17.2, -15, -150),
		glm::vec3(-17.2, -15, -150));
	pyramid->performFunction([](Plane* obj) -> void {
		obj->setColor(glm::vec3(1, 0, 1));
		obj->setReflectivity(true, 1);
		obj->setTransparency(true, .6);
		sceneObjects.push_back(obj);
	});

	Sphere* sphere1 = new Sphere(glm::vec3(0, 30, -200), 25);
	sphere1->setColor(glm::vec3(0.2, 0.3, 1));
	sphere1->setReflectivity(true, 0.8);
	sceneObjects.push_back(sphere1);

	Sphere* sphere2 = new Sphere(glm::vec3(10, 5, -120), 5);
	sphere2->setColor(glm::vec3(0., 0.5, 0.));
	sceneObjects.push_back(sphere2);

	Sphere* sphere3 = new Sphere(glm::vec3(-5, -9, -70), 3);
	sphere3->setColor(glm::vec3(0.2, 0.4, 0.));
	sphere3->setRefractivity(true, .8, 1.5);
	sceneObjects.push_back(sphere3);

	Sphere* sphere4 = new Sphere(glm::vec3(-7, -2, -50), 5);
	sphere4->setColor(glm::vec3(0.5, 0.1, 0.1));
	sphere4->setTransparency(true, .6);
	sceneObjects.push_back(sphere4);

	Sphere* sphere5 = new Sphere(glm::vec3(8, -5, -70), 7);
	sphere5->setColor(glm::vec3(0, 0.5, 0.1));
	sphere5->setRefractivity(true, 1, 1.5);
	sceneObjects.push_back(sphere5);

	Sphere* sphere6 = new Sphere(glm::vec3(10, 18, -100), 4);
	sphere6->setColor(glm::vec3(1, 0, 0));
	sphere6->setReflectivity(true, 1);
	sceneObjects.push_back(sphere6);

}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
