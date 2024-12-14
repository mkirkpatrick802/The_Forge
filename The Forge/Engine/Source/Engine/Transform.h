//
// Created by mKirkpatrick on 1/30/2024.
//

#pragma once
#include "Data.h"

namespace Engine
{
    class Transform {

    public:

        Transform()
        {
            position = Vector2D();
        }

        Transform(float x, float y)
        {
            position = Vector2D(x, y);
        }

    public:

        Vector2D position = Vector2D();
        float rotation = 0; // degrees

    };
}