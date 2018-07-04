//  ========================================================================
//  Author: Thomas Bingham
//  COSC363 Assignment 2
//  Date last Edited: 07/06/2018
//
//  ========================================================================
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cylinder.h"
#include "Cone.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/glut.h>
#include "Plane.h"
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

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene


//---The most important function in a ray tracer! ----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 backgroundCol(0);
    glm::vec3 light(50, 40, -3);   //Light vector
    glm::vec3 light2(-80, 40, -3);   //Second light vector
    glm::vec3 ambientCol(0.2);   //Ambient color of light
    glm::vec3 colorSum; //Total colour at each point

    ray.closestPt(sceneObjects);        //Compute the closest point of intersetion of objects with the ray

    if(ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour

    glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor();
    glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);
    glm::vec3 lightVector = glm::normalize(light - ray.xpt);
    glm::vec3 lightVector2 = glm::normalize(light2 - ray.xpt);
    float lDotn = glm::dot(lightVector, normalVector);
    float l2Dotn = glm::dot(lightVector2, normalVector);

    glm::vec3 reflVector = glm::reflect(-lightVector, normalVector);
    glm::vec3 reflVector2 = glm::reflect(-lightVector2, normalVector);
    float rDotv = glm::dot(reflVector, -ray.dir);
    float r2Dotv = glm::dot(reflVector2, -ray.dir);

    glm::vec3 oneVec(1, 1, 1);
    glm::vec3 zeroVec(0, 0, 0);
    float specularTerm = pow(rDotv, 10);
    float specularTerm2 = pow(rDotv, 10);
    //glm::vec3 specularRef = zeroVec;

    Ray shadow(ray.xpt, light - ray.xpt); //Creates a shadow ray at every point towards light source
    Ray shadow2(ray.xpt, light2 - ray.xpt);
    shadow.normalize();  //to not make the whole thing shadowed
    shadow2.normalize();
    shadow.closestPt(sceneObjects);  //finds closest side of object at every location on 2d viewing plane
    shadow2.closestPt(sceneObjects);


//-----------Floor------------------------------------------------------

    if (ray.xindex == 3) {
        if ((int((ray.xpt.x + 40)/4) - int(ray.xpt.z/4)) % 2 == true) {
            colorSum = glm::vec3 (0, 0.5, 0);
        } else {
            colorSum = glm::vec3(0.5, 0, 0);
        }
    }

    //Colour parts with shadows
    //shadow 1
    if((ray.xindex != 2) && (lDotn <= 0 || ((shadow.xindex > -1) && (shadow.xdist < glm::distance(ray.xpt, light))))) {
        colorSum += ambientCol* materialCol;
    } else {

            if (rDotv >= 0) {
                colorSum += (ambientCol * materialCol + lDotn * materialCol + specularTerm * oneVec);
            }

            if (rDotv < 0) {
                colorSum += (ambientCol * materialCol + lDotn * materialCol);
            }
    }
    //shadow 2
    if((ray.xindex != 2) && (l2Dotn <= 0 || ((shadow2.xindex > -1) && (shadow2.xdist < glm::distance(ray.xpt, light2))))) {
        colorSum += ambientCol* materialCol;
    } else {

            if (r2Dotv >= 0) {
                colorSum += (ambientCol * materialCol + l2Dotn * materialCol + specularTerm2 * oneVec);
            }

            if (r2Dotv < 0) {
                colorSum += (ambientCol * materialCol + l2Dotn * materialCol);
            }
    }


//-----------Reflection-------------------------------------------------

    if (ray.xindex == 0 && step < MAX_STEPS)
    {
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, reflectedDir);
        glm::vec3 reflectedCol = trace(reflectedRay, step+1);
        colorSum = colorSum + (0.8f*reflectedCol);
    }

//-----------Transparency-----------------------------------------------

    if (ray.xindex == 2 && step < MAX_STEPS)
    {
        Ray transray1(ray.xpt, ray.dir);
        transray1.closestPt(sceneObjects);
        Ray transray2(transray1.xpt, transray1.dir);
        colorSum += glm::vec3(0, 0.1, 0) + trace(transray2, step+1);

        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, reflectedDir);
        glm::vec3 reflectedCol = trace(reflectedRay, step+1);
        colorSum = colorSum + (0.05f*reflectedCol);
    }

    return colorSum;

//-----------Refraction-------------------------------------------------

    if (ray.xindex == 2 && step < MAX_STEPS)
    {
        float eta = 1/1.01;
        glm::vec3 refract1 = glm::refract (ray.dir, normalVector, eta);
        Ray refray1(ray.xpt, refract1);
        refray1.closestPt(sceneObjects);
        glm::vec3 m = sceneObjects[refray1.xindex]->normal(refray1.xpt);
        glm::vec3 refract2 = glm::refract (refract1, m, 1/eta);
        Ray refray2(refray1.xpt, refract2);
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

    glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a quad.

    for(int i = 0; i < NUMDIV; i++)     //For each grid point xp, yp
    {
        xp = XMIN + i*cellX;
        for(int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j*cellY;

            glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);  //direction of the primary ray

            Ray ray = Ray(eye, dir);        //Create a ray originating from the camera in the direction 'dir'
            ray.normalize();                //Normalize the direction of the ray to a unit vector
            glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);             //Draw each cell with its color value
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

    //-- Create a pointer to a sphere object
    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, -2.0, -125.0), 15.0, glm::vec3(0, 0, 1));
    sceneObjects.push_back(sphere1);//0

    Sphere *sphere2 = new Sphere(glm::vec3(-10.0, -5.0, -65.0), 4.0, glm::vec3(1, 0, 0));
    sceneObjects.push_back(sphere2);//1https://www.startpage.com/

    //-- Adding sphere to be transparent
    Sphere *sphere3 = new Sphere(glm::vec3(-8.0, 10.0, -80.0), 6.0, glm::vec3(0.1, 0, 0));
    sceneObjects.push_back(sphere3);//2

    //--Add plane to scene.
    Plane *plane = new Plane (glm::vec3(-20, -20, -40),
                                glm::vec3(20, -20, -40),
                                glm::vec3(20, -20, -200),
                                glm::vec3(-20, -20, -200), //Points
                                    glm::vec3(0.2, 0.2, 0.2)); //Colour
    sceneObjects.push_back(plane);//3

    //--Adding a box/warped cube to the scene with front left wall commented out at end of initialize()
    //8 Corner points (A - H)

    glm::vec3 cornerA = glm::vec3(12, 4, -80);
    glm::vec3 cornerB = glm::vec3(8, 6, -84);
    glm::vec3 cornerC = glm::vec3(16, 6, -84);
    glm::vec3 cornerD = glm::vec3(12, 8, -88);

    glm::vec3 cornerE = glm::vec3(12, 0, -80);
    glm::vec3 cornerF = glm::vec3(8, 2, -84);
    glm::vec3 cornerG = glm::vec3(16, 2, -84);
    glm::vec3 cornerH = glm::vec3(12, 4, -88);

    Plane *top = new Plane (cornerA,
                                cornerC,
                                cornerD,
                                cornerB, //Points
                                glm::vec3(0, 1, 0)); //Colour
    sceneObjects.push_back(top);//3

    Plane *bottom = new Plane (cornerE,
                                cornerG,
                                cornerH,
                                cornerF, //Points
                                glm::vec3(0, 1, 0)); //Colour
    sceneObjects.push_back(bottom);//4https://www.startpage.com/

    Plane *frontRight = new Plane (cornerA,
                                cornerE,
                                cornerG,
                                cornerC, //Points
                                glm::vec3(0, 1, 0)); //Colour
    sceneObjects.push_back(frontRight);//5

    Plane *backRight = new Plane (cornerG,
                                cornerC,
                                cornerD,
                                cornerH, //Points
                                glm::vec3(0, 1, 0)); //Colour
    sceneObjects.push_back(backRight);//6

    Plane *backLeft = new Plane (cornerH,
                                cornerD,
                                cornerB,
                                cornerF, //Points
                                glm::vec3(0, 1, 0)); //Colour
    sceneObjects.push_back(backLeft);//7

    //Plane *frontLeft = new Plane (cornerA,
                                //cornerB,
                                //cornerF,
                                //cornerE, //Points
                                //glm::vec3(0, 1, 0)); //Colour
    //sceneObjects.push_back(frontLeft);

    Cylinder *cylinder = new Cylinder (glm::vec3(0, -20, -90),
                                        2,
                                        4,
                                        glm::vec3(0.5, 0.5, 0));
    sceneObjects.push_back(cylinder);
    Cone *cone = new Cone (glm::vec3(10, -15, -80),
                                        2,
                                        3,
                                        glm::vec3(0.1, 0.4, 0.1));
    sceneObjects.push_back(cone);

}



int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
