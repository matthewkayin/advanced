#include "transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

Transform::Transform() {
    origin = glm::vec3(0.0f);
    basis = glm::mat4(1.0f);
}

glm::vec3 Transform::get_xbasis() const {
    return glm::vec3(basis[0]);
}

glm::vec3 Transform::get_ybasis() const {
    return glm::vec3(basis[1]);
}

glm::vec3 Transform::get_zbasis() const {
    return glm::vec3(basis[2]);
}

glm::mat4 Transform::to_model() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, origin) * basis;

    return model;
}

void Transform::rotate(float angle, glm::vec3 axis) {
    basis = glm::rotate(glm::mat4(1.0f), angle, axis) * basis;
}

void Transform::scale(glm::vec3 factors) {
    basis = glm::scale(basis, factors);
    origin *= factors;
}