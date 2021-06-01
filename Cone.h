//
// Created by sgb79 on 1/06/21.
//

#ifndef H_CONE
#define H_CONE
#include <glm/glm.hpp>
#include "SceneObject.h"


/**
 * Defines a simple cylinder located at 'center'
 * with the specified height and radius
 */
class Cone : public SceneObject
{

private:
    glm::vec3 center = glm::vec3(0);
    float radius = 3;
    float height = 5;
    float theta = 15;

public:
    Cone() {};

    Cone(glm::vec3 c, float r, float h, float theta) : center(c), radius(r), height(h), theta(theta) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);
};
#endif //!H_CONE
