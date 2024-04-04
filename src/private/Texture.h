#pragma once


class Texture
{
public:

    Texture();
    void Generate(int width, int height, unsigned char* data);

public:

    unsigned int ID;

private:

    int Width, Height;
};