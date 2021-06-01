//
// Created by sgb79 on 1/06/21.
//

#include "Cylinder.h"
#include <math.h>

/**
 * Cylinder intersection method. The input is a ray.
 */
 float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
 {
    float xDif = p0.x - center.x;
    float zDif = p0.z - center.z;

    // Calculate values to generate cylinder;
    float a = dir.x*dir.x + dir.z*dir.z;
    float b = 2*(dir.x*xDif+dir.z*zDif);
    float c = xDif*xDif + zDif*zDif - radius*radius;

    float delta = b*b - 4.0f*a*c;

    if(delta < 0.001) return -1.0;
    float t1 = (-b - sqrt(delta))/(2.0f*a);
    float t2 = (-b + sqrt(delta))/(2.0f*a);

    float closeValue;
    float farValue;
    if (t1 < 0)
    {
        if (t2 > 0) {
            closeValue = t2;
        } else {
            closeValue = -1;
        }
        farValue = t1;
    }
    else {
        closeValue = t1;
        farValue = t2;
    }

    glm::vec3 point = p0 + closeValue*dir;

    if (point.y > height + center.y)
    {
        point = p0 + farValue*dir;
        if (point.y > height + center.y)
        {
            return -1;
        }
        else
        {
            if (hasCap)
            {

            }
            else
            {
                return farValue;
            }
        }

    }
    else
    {
        return closeValue;
    }
 }


 glm::vec3 Cylinder::normal(glm::vec3 p)
 {
     glm::vec3 n((p.x-center.x),0,(p.z-center.z));
     n = glm::normalize(n);
     return n;
 }