/*#include "shader.h"

Shader::Shader()
{
    ID = 0;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    int succcess;
    char infoLog[1024];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &succcess);
    if (!succcess)
    {
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &succcess);
    if (!succcess)
    {
        glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::Fragment::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &succcess);
    if (!succcess)
    {
        glGetProgramInfoLog(ID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use()
{
	glUseProgram(ID);
}

// Uniform setting functions:
void Shader::set1Bool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::set1Int(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set1Float(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set2Float(const std::string& name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()),
        x, y);
}

void Shader::set2Float(const std::string& name, glm::vec2 value) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()),
        value.x, value.y);
}

void Shader::set3Float(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()),
        x, y, z);
}

void Shader::set3Float(const std::string& name, glm::vec3 value) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()),
        value.x, value.y, value.z);
}

void Shader::set4Float(const std::string& name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()),
        x, y, z, w);
}

void Shader::set4Float(const std::string& name, glm::vec4 value) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()),
        value.x, value.y, value.z, value.w);
}

void Shader::setM4(const std::string& name, glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()),
        1, GL_FALSE, &value[0][0]);
}*/