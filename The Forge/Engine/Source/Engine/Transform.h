//
// Created by mKirkpatrick on 1/30/2024.
//

#pragma once
#include <glm/vec2.hpp>

namespace Engine
{
    class Transform {

    public:

        Transform()
        {
            position = glm::vec2();
        }

        Transform(float x, float y)
        {
            position = glm::vec2(x, y);
        }

    public:

        glm::vec2 position = glm::vec2();
        float rotation = 0; // degrees

    };
}
