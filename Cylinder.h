//
// Created by sgb79 on 1/06/21.
//

#ifndef H_CYLINDER
#define H_CYLINDER
#include <glm/glm.hpp>
#include "SceneObject.h"


/**
 * Defines a simple cylinder located at 'center'
 * with the specified height and radius
 */
class Cylinder : public SceneObject
{

private:

    glm::vec3 center = glm::vec3(0);
    int hasCap = false;
    float radius = 3;
    float height = 5;

public:
    Cylinder() {};

    Cylinder(glm::vec3 c, float r, float h, int hasCap) : center(c), radius(r), height(h), hasCap(hasCap) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};


#endif //!H_CYLINDER
