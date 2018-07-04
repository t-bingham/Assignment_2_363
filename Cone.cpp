//  ========================================================================
//  Author: Thomas Bingham
//  COSC363 Assignment 2 Cone
//  Date last Edited: 07/06/2018
//  Using Sphere structure
//  ========================================================================

#include "Cone.h"
#include <math.h>

/**
* Sphere's intersection method.  The input is a ray (pos, dir).
*/
float Cone::intersect(glm::vec3 posn, glm::vec3 dir)
{
    float n = (radius / height) * (radius / height);
    float a = dir.x * dir.x + dir.z * dir.z - n * dir.y * dir.y;
    float b = 2 * dir.x * (posn.x - center.x) + 2 * (height - posn.y + center.y) * n * dir.y + 2 * dir.z * (posn.z - center.z);
    float c = (posn.x - center.x) * (posn.x - center.x) - n * (center.y - posn.y + height) * (center.y - posn.y + height) + (posn.z - center.z) * (posn.z - center.z);
    float delta = (b * b) - (4 * a * c);

    if(fabs(delta) < 0.001) return -1.0;
    if(delta < 0.0) return -1.0;

    float t1 = (-b - sqrt(delta)) / (2*a);
    float t2 = (-b + sqrt(delta)) / (2*a);

    if(fabs(t1) < 0.001 )
    {
        if (t2 > 0) return t2;
        else t1 = -1.0;
    }
    if(fabs(t2) < 0.001 ) t2 = -1.0;

    float maxT;
    float minT;

    if(t2 > t1){
        maxT = t2;
        minT = t1;
    }
    else {
        maxT = t1;
        minT = t2;
    }

    float h1 = (posn.y + minT * dir.y) - center.y;
    float h2 = (posn.y + maxT * dir.y) - center.y;
    if(not (h1 < 0 or h1 > height) && not (minT == -1.0)) return minT;
    else if(not (h2 < 0 or h2 > height) && not (maxT == -1.0)) return maxT;
    else return -1.0;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3(p.x - center.x, sqrt((p.x - center.x) * (p.x - center.x)) + (p.z - center.z) * (p.z - center.z) * (radius / height), p.z - center.z);
    n = glm::normalize(n);
    return n;
}
