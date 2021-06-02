//
// Created by sgb79 on 1/06/21.
//

#include "Cone.h"
#include <math.h>

/**
* Cone intersection method. The input is a ray.
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float trueHeight = height+center.y;

    float xDif = p0.x - center.x;
    float zDif = p0.z - center.z;
    float yDif = height - p0.y + center.y;
    float heightConstant = (radius/height) * (radius/height);


    // Calculate values to generate cone
    float a = dir.x*dir.x + dir.z*dir.z - heightConstant*dir.y*dir.y;
    float b = 2*(dir.x*xDif + dir.z*zDif + heightConstant*dir.y*yDif);
    float c = xDif*xDif + zDif*zDif - heightConstant*yDif*yDif;

    float delta = b*b - 4.0f*a*c;

    if(delta < 0.001) return -1.0;

    float t1 = (-b - sqrt(delta))/(2.0f*a);
    float t2 = (-b + sqrt(delta))/(2.0f*a);


    float closestVal;
    if (t1 < 0)
    {
        closestVal = (t2 > 0) ? t2 : -1;
    }
    else
    {
        closestVal = t1;
    }


    glm::vec3 point = p0 + closestVal*dir;
    if (point.y > trueHeight)
    {
        return -1;
    }
    else
    {
        return closestVal;
    }
}

/**
 * Calculates the normal vector of the cone
 */
glm::vec3 Cone::normal(glm::vec3 p)
{
    float alpha = atan((p.x - center.x)/(p.z-center.z));
    glm::vec3 n(sin(alpha)*cos(theta), sin(theta), cos(alpha)*cos(theta));
    return n;
}
