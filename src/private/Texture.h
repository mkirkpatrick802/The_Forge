#pragma once


class Texture
{
public:

    Texture();

    void Generate(int width, int height, unsigned char* data);
    void Bind() const;

public:

    unsigned int ID;

private:

    int Width, Height;

    int Internal_Format;
    int Image_Format;

    int Wrap_S;
    int Wrap_T;
    int Filter_Min;
    int Filter_Max;

};