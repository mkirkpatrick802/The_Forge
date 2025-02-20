#include <fstream>

#include "shader.h"

#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

Shader& Shader::Use()
{
    glUseProgram(ID);
    return *this;
}

void Shader::Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource)
{
    Reset();
    
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;

    try
    {
        // open files
        std::ifstream vertexShaderFile(vertexSource);
        std::ifstream fragmentShaderFile(fragmentSource);
        std::stringstream vShaderStream, fShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // if geometry shader path is present, also load a geometry shader
        if (geometrySource != nullptr)
        {
            std::ifstream geometryShaderFile(geometrySource);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files \n";
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();

    // vertex Shader
    GLuint sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vShaderCode, nullptr);
    glCompileShader(sVertex);
    CheckCompileErrors(sVertex, "VERTEX");

    // fragment Shader
    GLuint sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fShaderCode, nullptr);
    glCompileShader(sFragment);
    CheckCompileErrors(sFragment, "FRAGMENT");

    // if geometry shader source code is given, also compile geometry shader
    GLuint gShader = NULL;
    if (geometrySource != nullptr)
    {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &gShaderCode, nullptr);
        glCompileShader(gShader);
        CheckCompileErrors(gShader, "GEOMETRY");
    }

    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, sVertex);
    glAttachShader(ID, sFragment);
    if (geometrySource != nullptr)
        glAttachShader(ID, gShader);

    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (geometrySource != nullptr)
        glDeleteShader(gShader);
}

void Shader::Reset() const
{
    glDeleteProgram(ID);
}

void Shader::SetFloat(const char* name, float value, bool useShader)
{
    if (useShader) Use();
    glUniform1f(glGetUniformLocation(ID, name), value);
}
void Shader::SetInteger(const char* name, int value, bool useShader)
{
    if (useShader) Use();
    glUniform1i(glGetUniformLocation(ID, name), value);
}
void Shader::SetVector2f(const char* name, float x, float y, bool useShader)
{
    if (useShader) Use();
    glUniform2f(glGetUniformLocation(ID, name), x, y);
}
void Shader::SetVector2f(const char* name, const glm::vec2& value, bool useShader)
{
    if (useShader) Use();
    glUniform2f(glGetUniformLocation(ID, name), value.x, value.y);
}
void Shader::SetVector3f(const char* name, float x, float y, float z, bool useShader)
{
    if (useShader) Use();
    glUniform3f(glGetUniformLocation(ID, name), x, y, z);
}
void Shader::SetVector3f(const char* name, const glm::vec3& value, bool useShader)
{
    if (useShader) Use();
    glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}
void Shader::SetVector4f(const char* name, float x, float y, float z, float w, bool useShader)
{
    if (useShader) Use();
    glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
}
void Shader::SetVector4f(const char* name, const glm::vec4& value, bool useShader)
{
    if (useShader) Use();
    glUniform4f(glGetUniformLocation(ID, name), value.x, value.y, value.z, value.w);
}
void Shader::SetMatrix4(const char* name, const glm::mat4& matrix, bool useShader)
{
    if (useShader) Use();
    glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(matrix));
}

void Shader::CheckCompileErrors(const GLuint object, const std::string& type) const
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, nullptr, infoLog);
            std::cout << "\nERROR::SHADER: Compile-time error: Type: " << type << "\n"
                << infoLog << "\n-- --------------------------------------------------- --\n";
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, nullptr, infoLog);
            std::cout << "\nERROR::Shader: Link-time error: Type: " << type << "\n"
                << infoLog << "\n-- --------------------------------------------------- --\n";
        }
    }
}
