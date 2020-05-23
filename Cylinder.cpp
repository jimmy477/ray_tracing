/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>
#include <iostream>
using namespace std;

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float a = (dir[0] * dir[0]) + (dir[2] * dir[2]);
    float b = 2 * (dir[0] * (p0[0] - center[0]) + dir[2] * (p0[2] - center[2]));
    float c = (p0[0] - center[0]) * (p0[0] - center[0]) + (p0[2] - center[2]) * (p0[2] - center[2]) - (radius * radius);
    float delta = b * b - (4 * a * c);  //Discriminant

    if(fabs(delta) < 0.001) return -1.0; //When delta is very close to 0, the intersection is near tangental so count as non existent
    if(delta < 0.0) return -1.0;
    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);
    if (fabs(t1) < 0.001) return -1;
    if (fabs(t2) < 0.001) return -1;
    glm::vec3 p1 = p0 + (t1 * dir);
    glm::vec3 p2 = p0 + (t2 * dir);
    if (p1[1] > (center[1] + height) || p1[1] < center[1]) {
        if (p2[1] > (center[1] + height) || p2[1] < center[1]) return -1;
        else return t2;

    }
    return (t1 < t2)? t1: t2;

}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the Cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = { p[0] - center[0], 0, p[2] - center[2] };
    n = glm::normalize(n);
    return n;
}
