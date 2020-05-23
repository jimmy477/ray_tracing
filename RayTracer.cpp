/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
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
#include "Cylinder.h"
#include "TextureBMP.h"
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const float FOG_RANGE[2] = {-75, -150};
TextureBMP texture;
vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 backgroundCol(1);						//Background colour = (0,0,0)
    glm::vec3 ambientCol(0.2);
	glm::vec3 lightPos(10, 40, -3);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

    if (ray.index == 2)  //Hitting the plane
    {
        //Stripe pattern
        int stripeWidth = 5;
        int iz = (ray.hit.z) / stripeWidth;
        int ix = (ray.hit.x) / stripeWidth;
        int kx = ix % 2;
        int kz = iz % 2;
        if (ray.hit.x < 0) kx++;  //Swaps the colour when x is negative
        if (((kx + kz) % 2) == 0) color = glm::vec3(1, 1, 0.5);
        else color = glm::vec3(0, 1, 0);
        obj->setColor(color);

        //Texture mapping
//        float x1 = -15; float x2 = 5;
//        float z1 = -60; float z2 = -90;
//        float texcoords = (ray.hit.x - x1) / (x2 - x1);
//        float texcoordt = (ray.hit.z - z1) / (z2 - z1);
//        if (texcoords > 0 && texcoords < 1 &&
//            texcoordt > 0 && texcoordt < 1)
//        {
//            color = texture.getColorAt(texcoords, texcoordt);
//            obj->setColor(color);
//        }
    }

    color = obj->lighting(lightPos, -ray.dir, ray.hit);						//Object's colour
    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit, lightVec);
    shadowRay.closestPt(sceneObjects);
    float lightDist = glm::length(lightVec);
    if (shadowRay.index > -1 && shadowRay.dist < lightDist) color = ambientCol * obj->getColor();

    if (obj->isReflective() && step < MAX_STEPS)
    {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }
    if (obj->isRefractive() && step < MAX_STEPS)
    {
        float eta = 1 / obj->getRefractionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, normalVec, eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f / eta);
        Ray finalRay(refrRay.hit, h);
        glm::vec3 refractedColor = trace(finalRay, step + 1);
//        color = color + refractedColor;
        return refractedColor;
    }
    if (obj->isTransparent() && step < MAX_STEPS)
    {
        float transCoeff = obj->getTransparencyCoeff();
        glm::vec3 transparentCol = trace(ray, step + 1);
        color = color + (transCoeff * transparentCol);
    }
    float t = (ray.hit.z - FOG_RANGE[0]) / (FOG_RANGE[1] - FOG_RANGE[0]);
    glm::vec3 white(1);
    color = (1 - t) * color + t * white;
    return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
    float xp, yp, temp_xp, temp_yp;  //grid point
    float red, green, blue;
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
            red = 0;
            green = 0;
            blue = 0;

            //Anti-aliasing
            for (int s = 0; s < 4; s++)
            {
                if (s == 0)
                {
                    temp_xp = xp - (cellX * 0.25);
                    temp_yp = yp - (cellY * 0.25);
                }
                if (s == 1)
                {
                    temp_xp = xp + (cellX * 0.25);
                    temp_yp = yp - (cellY * 0.25);
                }
                if (s == 2)
                {
                    temp_xp = xp + (cellX * 0.25);
                    temp_yp = yp + (cellY * 0.25);
                }
                if (s == 3)
                {
                    temp_xp = xp - (cellX * 0.25);
                    temp_yp = yp + (cellY * 0.25);
                }
                glm::vec3 dir(temp_xp+0.5*cellX, temp_yp+0.5*cellY, -EDIST);	//direction of the primary ray
                Ray ray = Ray(eye, dir);
                glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
                red += col.r;
                green += col.g;
                blue += col.b;
            }
            glColor3f(red / 4, green / 4, blue / 4);

            //UNCOMMENT BELOW FOR NO ANTI-ALIASING
//            glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

//            Ray ray = Ray(eye, dir);

//            glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
//            glColor3f(col.r, col.g, col.b);
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

    //Blue sphere
    Sphere *sphere1 = new Sphere(glm::vec3(0.0, 0.0, -130.0), 15.0);
    sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
    sphere1->setReflectivity(true, 0.8);
    sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

    //Cyan sphere
    Sphere *sphere2 = new Sphere(glm::vec3(5.0, -11.0, -110.0), 4.0);
    sphere2->setColor(glm::vec3(0, 1, 1));
//    sphere2->setShininess(5);
    sphere2->setRefractivity(true, 1.01, 0);
    sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

    //Floor plane
    Plane *plane = new Plane (glm::vec3(-50, -15, -40),    //Point A
                              glm::vec3(50, -15, -40),     //Point B
                              glm::vec3(50, -15, -200),    //Point C
                              glm::vec3(-50, -15, -200));  //Point D
    plane->setSpecularity(false);
    sceneObjects.push_back(plane);

    //Octahedron
    Plane *pyramid_front = new Plane(glm::vec3(-10.0, 0.0, -60.0),
                                     glm::vec3(0.0, 0.0, -60.0),
                                     glm::vec3(-5.0, 10.0, -65.0));
    pyramid_front->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_front);

    Plane *pyramid_left = new Plane(glm::vec3(-10.0, 0.0, -70.0),
                                     glm::vec3(-10.0, 0.0, -60.0),
                                     glm::vec3(-5.0, 10.0, -65.0));
    pyramid_left->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_left);

    Plane *pyramid_back = new Plane(glm::vec3(0.0, 0.0, -70.0),
                                     glm::vec3(-10.0, 0.0, -70.0),
                                     glm::vec3(-5.0, 10.0, -65.0));
    pyramid_back->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_back);

    Plane *pyramid_right = new Plane(glm::vec3(0.0, 0.0, -60.0),
                                     glm::vec3(0.0, 0.0, -70.0),
                                     glm::vec3(-5.0, 10.0, -65.0));
    pyramid_right->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_right);

    Plane *pyramid_front_bottom = new Plane(glm::vec3(0.0, 0.0, -60.0),
                                     glm::vec3(-10.0, 0.0, -60.0),
                                     glm::vec3(-5.0, -10.0, -65.0));
    pyramid_front_bottom->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_front_bottom);

    Plane *pyramid_left_bottom = new Plane(glm::vec3(-10.0, 0.0, -60.0),
                                     glm::vec3(-10.0, 0.0, -70.0),
                                     glm::vec3(-5.0, -10.0, -65.0));
    pyramid_left_bottom->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_left_bottom);

    Plane *pyramid_back_bottom = new Plane(glm::vec3(-10.0, 0.0, -70.0),
                                     glm::vec3(0.0, 0.0, -70.0),
                                     glm::vec3(-5.0, -10.0, -65.0));
    pyramid_back_bottom->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_back_bottom);

    Plane *pyramid_right_bottom = new Plane(glm::vec3(0.0, 0.0, -70.0),
                                     glm::vec3(0.0, 0.0, -60.0),
                                     glm::vec3(-5.0, -10.0, -65.0));
    pyramid_right_bottom->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(pyramid_right_bottom);

    //Blue cylinder
    Cylinder *cylinder1 = new Cylinder(glm::vec3(15.0, -15.0, -70.0), 5.0, 5.0);
    cylinder1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
//    cylinder1->setReflectivity(true, 0.8);
    sceneObjects.push_back(cylinder1);		 //Add sphere to scene objects

    //Cyan sphere
    Sphere *sphere3 = new Sphere(glm::vec3(0.0, -11.0, -70.0), 4.0);
    sphere3->setColor(glm::vec3(0, 1, 1));
//    sphere2->setShininess(5);
    sphere3->setTransparency(true, 1);
    sceneObjects.push_back(sphere3);
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
