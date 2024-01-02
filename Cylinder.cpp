/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinders's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float a = dir.x * dir.x + dir.z * dir.z;
    float xdiff = (p0.x - center.x);
    float zdiff = (p0.z - center.z);
    float b = 2 * (dir.x * xdiff + dir.z * zdiff);
    float c = xdiff * xdiff + zdiff * zdiff - radius * radius;
    float delta = b * b - 4 * a * c;

    if (delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = ( - b - sqrt(delta)) / (2 * a);
    float t2 = ( - b + sqrt(delta)) / (2 * a);

    if (t1 > 0) {
        if ((p0.y + dir.y * t1) > center.y + height) {
            return ((p0.y + dir.y * t2) < center.y + height) ? (center.y + height - p0.y) / dir.y : -1;
        }
        else if ((p0.y + dir.y * t1) < center.y) {
            return ((p0.y + dir.y * t2) > center.y) ? (center.y - p0.y) / dir.y : -1;
        }
        return t1;
    } else return (t2 > 0 && (p0.y + dir.y * t2) > center.y && (p0.y + dir.y * t2) < center.y + height) ? t2 : -1;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    if (p.y <= center.y) {
        return { 0, -1, 0 };
    }
    else if (p.y >= center.y + height) {
        return { 0, 1, 0 };
    }
    glm::vec3 n = p - center;
    n.y = 0;
    n = glm::normalize(n);
    return n;
}
