#pragma once

#include <glm/glm.hpp>

struct Transform {
    glm::vec3 origin;
    glm::mat4 basis;

    Transform();
    glm::vec3 get_xbasis() const;
    glm::vec3 get_ybasis() const;
    glm::vec3 get_zbasis() const;
    glm::mat4 to_model() const;
    void rotate(float angle, glm::vec3 axis);
    void scale(glm::vec3 factors);
};