//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_TRANSFORM_H
#define THE_FORGE_TRANSFORM_H

#include "GameData.h"

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


#endif //THE_FORGE_TRANSFORM_H
