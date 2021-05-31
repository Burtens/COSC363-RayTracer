/*==================================================================================
* COSC 363  Computer Graphics (2021)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf  for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Plane.h"
#include "Ray.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float WIDTH = 100.0;
const float HEIGHT = 100.0;
const float EDIST = 100.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;

const int MAX_ALIAS_STEPS = 2;
const float COL_DIFF = 0.5f;

const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const int ANTI_ALIASING = false;

TextureBMP texture;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);
	glm::vec3 lightPos(30, 40, 20);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	if (ray.index == 0)
    {
	    // Checker pattern
	    int stripeWidth = 5;
	    int iz = (ray.hit.z) / stripeWidth;
        int ix = (ray.hit.x) / stripeWidth;

	    int k = abs(iz % 2);
        int l = abs(ix % 2);

	    if (k == 0 && l == 0) color = glm::vec3(1, 0, 0);
	    else if (k == 0 && l == 1) color = glm::vec3(0, 1, 0);
	    else if (k == 1 && l == 0) color = glm::vec3(0, 1, 0);
        else if (k == 1 && l == 1) color = glm::vec3(1, 0, 0);

        obj->setColor(color);
    }

    color = obj->lighting(lightPos, -ray.dir, ray.hit);
    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);

    if(shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {

        SceneObject* shadowHitObj = sceneObjects[shadowRay.index];

        if (shadowHitObj->isTransparent() || shadowHitObj->isRefractive())
        {
            color = 0.5f * obj->getColor();
        }
        else
        {
            color = 0.2f * obj->getColor();
        }

    }


	if(obj->isReflective() && step < MAX_STEPS)
	{
	    float rho = obj->getReflectionCoeff();
	    glm::vec3 normalVec = obj->normal(ray.hit);
	    glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
	    Ray reflectedRay(ray.hit, reflectedDir);
	    glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
	    color = color + (rho * reflectedColor);
	}

	if(obj->isRefractive() && step < MAX_STEPS)
	{
	    float rho = obj->getRefractionCoeff();
	    float eta = 1/obj->getRefractiveIndex();

	    // Initial Hit
	    glm::vec3 normalVec = obj->normal(ray.hit);
	    glm::vec3 refractedDir = glm::refract(ray.dir, normalVec, eta);
	    Ray refractedRay(ray.hit, refractedDir);
	    refractedRay.closestPt(sceneObjects);

	    // Inside Sphere
	    glm::vec3 refNormalVec = obj->normal(refractedRay.hit);
	    glm::vec3 exitRayDir = glm::refract(refractedDir, -refNormalVec, 1.0f/eta);
	    Ray exitRay(refractedRay.hit, exitRayDir);

	    // Recurse for MAX_STEPS
        glm::vec3 refractedColor = trace(exitRay, step + 1);
        color = color + (rho * refractedColor);
	}


	if(obj->isTransparent() && step < MAX_STEPS)
    {
	    float rho = obj->getTransparencyCoeff();
	    Ray transparentRay(ray.hit, ray.dir);
	    transparentRay.closestPt(sceneObjects);
	    Ray exitRay(transparentRay.hit, ray.dir);
        glm::vec3 transparentColor = trace(exitRay, step + 1);
        color = color + (rho * transparentColor);
    }

	return color;
}

int isDistinct(glm::vec3 color1, glm::vec3 ave) {
    // TODO: FIX This
    return (((color1.x - ave.x) / ave.x > COL_DIFF) ||
    ((color1.y - ave.y) / ave.y > COL_DIFF) ||
    ((color1.z - ave.z) / ave.z > COL_DIFF));
}



glm::vec3 aliasing(float xp, float yp, float cellX, float cellY, int step)
{
    glm::vec3 eye(0., 0., 0.);
    Ray ray;
    glm::vec3 dir;

    // Ray 1
    dir = glm::vec3(xp+0.25*cellX, yp+0.25*cellY, -EDIST);
    ray = Ray(eye, dir);
    glm::vec3 col1 = trace(ray, 1);

    // Ray 2
    dir = glm::vec3(xp+0.75*cellX, yp+0.25*cellY, -EDIST);
    ray = Ray(eye, dir);
    glm::vec3 col2 = trace(ray, 1);

    // Ray 3
    dir = glm::vec3(xp+0.75*cellX, yp+0.25*cellY, -EDIST);
    ray = Ray(eye, dir);
    glm::vec3 col3 = trace(ray, 1);

    // Ray 4
    dir = glm::vec3(xp+0.75*cellX, yp+0.75*cellY, -EDIST);
    ray = Ray(eye, dir);
    glm::vec3 col4 = trace(ray, 1);


    glm::vec3 ave = (col1 + col2 + col3 + col4) / 4.0f;

    if (step >= MAX_ALIAS_STEPS) {
        return ave;
    } else {

        if (isDistinct(col1, ave))
        {
            col1 = aliasing(xp, yp, cellX*0.5f, cellY*0.5f, step);
        }

        if (isDistinct(col2, ave))
        {
            col2 = aliasing(xp + 0.5f*cellX, yp, cellX*0.5f, cellY*0.5f, step);
        }

        if (isDistinct(col3, ave))
        {
            col3 = aliasing(xp, yp + 0.5f*cellY, cellX*0.5f, cellY*0.5f, step);
        }

        if (isDistinct(col4, ave))
        {
            col4 = aliasing(xp + 0.5f*cellX, yp + 0.5f*cellY, cellX*0.5f, cellY*0.5f, step);
        }

        return (col1 + col2 + col3 + col4) / 4.0f;
    }
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

            glm::vec3 col;



			if (ANTI_ALIASING) {

                col = aliasing(xp, yp, cellX, cellY, 1);

			} else {
                glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

                Ray ray = Ray(eye, dir);

                col = trace (ray, 1); //Trace the primary ray and get the colour value
			}

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    texture = TextureBMP("Butterfly.bmp");

    Plane *plane = new Plane(glm::vec3(-200., -15, -30),
                             glm::vec3(200., -15, -30),
                             glm::vec3(200., -15, -200),
                             glm::vec3(-200., -15, -200));


    plane->setSpecularity(false);
    sceneObjects.push_back(plane);

    glm::mat4 idMat(1);

    glm::vec3 A = glm::vec3(-10, -15, -45);
    glm::vec3 B = glm::vec3(0, -15, -35);
    glm::vec3 C = glm::vec3(0, -5, -37.5);
    glm::vec3 D = glm::vec3(10, -15, -45);


    Plane *triangle1 = new Plane(A,
                                B,
                                C);

    Plane *triangle2 = new Plane(B, D, C);


    triangle1->setColor(glm::vec3(0, 0, 1));
    triangle2->setColor(glm::vec3(0, 0, 1));
    sceneObjects.push_back(triangle1);
    sceneObjects.push_back(triangle2);


	Sphere *sphere1 = new Sphere(glm::vec3(0, 0, -37.5), 5.0);
	sphere1->setColor(glm::vec3(0, 0, 0.2));   //Set colour to blue
	sphere1->setRefractivity(true, 0.8, 1.01);
	sphere1->setReflectivity(true, 0.3);
	sphere1->setShininess(20);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects


	Sphere *sphere2 = new Sphere(glm::vec3(5, 5, -70), 4.0);
	sphere2->setColor(glm::vec3(1, 0, 0));
	sphere2->setShininess(5);
	sceneObjects.push_back(sphere2);

	Sphere *sphere4 = new Sphere(glm::vec3(10, 10, -40), 3.0);
	sphere4->setColor(glm::vec3(0, 1, 1));
	sceneObjects.push_back(sphere4);
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
