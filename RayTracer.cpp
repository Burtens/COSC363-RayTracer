/*==================================================================================
* COSC 363  Computer Graphics (2021)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf  for details.
*===================================================================================
*/
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Ray.h"
#include "Cone.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>


using namespace std;

const float WIDTH = 100.0;
const float HEIGHT = 100.0;
const float EDIST = 100.0;
const int NUMDIV = 600;
const int MAX_STEPS = 5;
const float MAX_FOG = -200;
const float MIN_FOG = -20;

const int MAX_ALIAS_STEPS = 5;
const float COL_DIFF = 0.2f;

const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const int ANTI_ALIASING = true;
const int FOG = true;

TextureBMP texture;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0.8, 0.8, 0.8);
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

        if (ray.hit.x < 0) l = abs((ix + 1) % 2);

        glm::vec3 floor_color(0);
	    if (k == 0 && l == 0) floor_color = glm::vec3(1, 0, 0);
	    else if (k == 0 && l == 1) floor_color = glm::vec3(0, 1, 0);
	    else if (k == 1 && l == 0) floor_color = glm::vec3(0, 1, 0);
        else if (k == 1 && l == 1) floor_color = glm::vec3(1, 0, 0);

        obj->setColor(floor_color);
    }

	if (ray.index == 1) {

	    glm::vec3 n = obj->normal(ray.hit);

	    float u = 0.5 + atan2(n.x, n.z)/(2*M_PI);
	    float v = 0.5 - asin(n.y)/M_PI;

	    glm::vec3 texture_colour = texture.getColorAt(u, v);
	    obj->setColor(texture_colour);
	}

    glm::vec3 surface_color = obj->lighting(lightPos, -ray.dir, ray.hit);
    glm::vec3 lightVec = lightPos - ray.hit;

    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);

    if(shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {

        SceneObject* shadowHitObj = sceneObjects[shadowRay.index];

        if (shadowHitObj->isTransparent())
        {
            surface_color = 0.2f * shadowHitObj->getColor() * (1-shadowHitObj->getTransparencyCoeff()) + ((shadowHitObj->getTransparencyCoeff()) * surface_color);
        }
        else if (shadowHitObj->isRefractive())
        {
            surface_color = 0.8f * (shadowHitObj->getColor() * (1-shadowHitObj->getRefractionCoeff()) + ((shadowHitObj->getRefractionCoeff()) * surface_color));
        }
        else {
            surface_color = 0.1f * obj->getColor();
        }
    }

	if(obj->isReflective() && step < MAX_STEPS)
	{
	    float rho = obj->getReflectionCoeff();
	    glm::vec3 normalVec = obj->normal(ray.hit);
	    glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
	    Ray reflectedRay(ray.hit, reflectedDir);
	    glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
	    surface_color = (1-rho)*surface_color + (rho * reflectedColor);
	}

	if(obj->isRefractive() && step < MAX_STEPS)
	{
	    float rho = obj->getRefractionCoeff();
	    float eta = obj->getRefractiveIndex();

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
        surface_color = (1-rho) * surface_color + (rho * refractedColor);
	}

	if(obj->isTransparent() && step < MAX_STEPS)
    {
	    float rho = obj->getTransparencyCoeff();
	    Ray transparentRay(ray.hit, ray.dir);
	    transparentRay.closestPt(sceneObjects);
	    Ray exitRay(transparentRay.hit, ray.dir);
        glm::vec3 transparentColor = trace(exitRay, step + 1);
        surface_color = (1-rho)*surface_color + (rho * transparentColor);
    }


	if (FOG)
    {
        float fog = (ray.hit.z-MIN_FOG)/(MAX_FOG-MIN_FOG);
        color += (1-fog)*surface_color + fog*glm::vec3(0.8, 0.8, 0.8);
    }
	else
    {
	    color += surface_color;
    }


    return color;
}

int isDistinct(glm::vec3 color1, glm::vec3 ave) {
    return (abs(color1.x - ave.x) > COL_DIFF) ||
    (abs(color1.y - ave.y) > COL_DIFF) ||
    (abs(color1.z - ave.z) > COL_DIFF);
}



glm::vec3 aliasing(glm::vec3 eye, float xp, float yp, float cellX, float cellY, int step)
{
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
            col1 = aliasing(eye, xp, yp, cellX*0.5f, cellY*0.5f, step+1);
        }

        if (isDistinct(col2, ave))
        {
            col2 = aliasing(eye, xp + 0.5f*cellX, yp, cellX*0.5f, cellY*0.5f, step+1);
        }

        if (isDistinct(col3, ave))
        {
            col3 = aliasing(eye, xp, yp + 0.5f*cellY, cellX*0.5f, cellY*0.5f, step+1);
        }

        if (isDistinct(col4, ave))
        {
            col4 = aliasing(eye, xp + 0.5f*cellX, yp + 0.5f*cellY, cellX*0.5f, cellY*0.5f, step+1);
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

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glm::vec3 eye(0., 0., 0.);

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

            glm::vec3 col;


			if (ANTI_ALIASING) {

                col = aliasing(eye, xp, yp, cellX, cellY, 1);

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

    // Textured Sphere
    Sphere *texturedSphere = new Sphere(glm::vec3(6, -4, -55), 3.0);
    //texturedSphere->setShininess(5);
    sceneObjects.push_back(texturedSphere);


    glm::vec3 A(-10, -15, -45);
    glm::vec3 B(0, -15, -35);
    glm::vec3 C(0, -5, -37.5);
    glm::vec3 D(10, -15, -45);
    glm::vec3 E(0, -15, -55);


    Plane *triangle1 = new Plane(A, B, C);
    Plane *triangle2 = new Plane(B, D, C);
    Plane *triangle3 = new Plane(D, E, C);
    Plane *triangle4 = new Plane(E, A, C);


    triangle1->setColor(glm::vec3(0, 0, 1));
    triangle2->setColor(glm::vec3(0, 0, 1));
    triangle3->setColor(glm::vec3(0, 0, 1));
    triangle4->setColor(glm::vec3(0, 0, 1));
    sceneObjects.push_back(triangle1);
    sceneObjects.push_back(triangle2);
    sceneObjects.push_back(triangle3);
    sceneObjects.push_back(triangle4);


    //Refractive Sphere
	Sphere *sphere1 = new Sphere(glm::vec3(0, 0, -37), 5.0);
	sphere1->setColor(glm::vec3(1, 1, 0));
	sphere1->setRefractivity(true, 0.76, 1.01);
	sphere1->setReflectivity(true, 0.2);

    sphere1->setShininess(20);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

	//Red Sphere
	Sphere *sphere2 = new Sphere(glm::vec3(5, 10, -100), 4.0);
	sphere2->setColor(glm::vec3(1, 0, 0));
	sphere2->setShininess(5);
	sceneObjects.push_back(sphere2);

	//Reflective Sphere
    Sphere *sphere3 = new Sphere(glm::vec3(-5, 0, -60), 5.0);
    sphere3->setColor(glm::vec3(0, 0, 0));
    sphere3->setShininess(5);
    sphere3->setReflectivity(true, 0.8);
    sceneObjects.push_back(sphere3);

    Sphere *sphere4 = new Sphere(glm::vec3(20, -5, -50), 4.25);
    sphere4->setTransparency(true, 0.5);
    sphere4->setColor(glm::vec3(0.4, 0.4, 0.8));
    sphere4->setReflectivity(true, 0.2);
    sceneObjects.push_back(sphere4);

	Cylinder *cylinder1 = new Cylinder(glm::vec3(20, -15, -50), 2.5, 5, true);
	cylinder1->setColor(glm::vec3(0, 0, 1));
	sceneObjects.push_back(cylinder1);

    Sphere *sphere6 = new Sphere(glm::vec3(-20, -5, -50), 4.25);
    sphere6->setTransparency(true, 0.5);
    sphere6->setColor(glm::vec3(0.4, 0.4, 0.8));
    sphere6->setReflectivity(true, 0.2);
    sceneObjects.push_back(sphere6);

    Cylinder *cylinder2 = new Cylinder(glm::vec3(-20, -15, -50), 2.5, 5, true);
    cylinder2->setColor(glm::vec3(0, 0, 1));
    sceneObjects.push_back(cylinder2);

    Cone *cone1 = new Cone(glm::vec3(40, -15, -100), 2, 10);
    cone1->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(cone1);

    Cone *cone2 = new Cone(glm::vec3(-40, -15, -100), 2, 10);
    cone2->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(cone2);

}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
