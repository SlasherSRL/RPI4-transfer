//------------------------------------------------------------------------------
//  shaderresource.cc
//  (C) 2019 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "shaderresource.h"
#include <fstream>
#include <string>
#include <sstream>
#include "renderdevice.h"
#include <iostream>

namespace Render
{

//------------------------------------------------------------------------------
/**
*/
ShaderResource::ShaderResource()
{
    // empty
}

//------------------------------------------------------------------------------
/**
 * @todo	Check if shader has already been loaded
*/
ShaderResourceId
ShaderResource::LoadShader(ShaderType type, const char* path)
{
    printf("Loading shader %s... ", path);
    std::ifstream ifs(path);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    if (content.empty())
    {
        printf("FAIL\n[SHADER LOAD ERROR]: Could not load file!");
        return false;
    }

    // Preprocess includes
    // TODO: this can be improved
    auto Preprocess = [](std::string& src, int& numIncludedLines) -> std::string
    {
        std::stringstream ss(src), out;
        std::string line;
        std::string pragma("#include");
        std::string include_extension("#extension GL_GOOGLE_include_directive : enable");
        while (std::getline(ss, line))
        {
            if (size_t pos = line.find(pragma) != std::string::npos)
            {
                std::string include = line.substr(pos + pragma.size() + 1, pos + line.size() - pragma.size() - 4);
                std::ifstream includeFileStream(include);
                std::string content((std::istreambuf_iterator<char>(includeFileStream)), (std::istreambuf_iterator<char>()));
                auto numLines = std::count(content.begin(), content.end(), '\n');
                numIncludedLines += numLines;
                if (content.empty())
                {
                    printf("\n[SHADER PREPROCESSOR ERROR]: Could not find include '%s'!\n", include.c_str());
                    assert(false);
                }
                out << content << std::endl;
            }
            else if (size_t pos = line.find(include_extension) != std::string::npos)
            {
                // do not include this line, since we handle it internally instead, and it'll just generate a warning.
                numIncludedLines--;
            }
            else
            {
                out << line << std::endl;
            }

        }
        return out.str();
    };
    int numIncludedLines = 0;
    content = Preprocess(content, numIncludedLines);

    GLuint shaderType;
    if (type == VERTEXSHADER)
        shaderType = GL_VERTEX_SHADER;
    else if (type == FRAGMENTSHADER)
        shaderType = GL_FRAGMENT_SHADER;
    else if (type == GEOMETRYSHADER)
        shaderType = GL_GEOMETRY_SHADER;
    else if (type == COMPUTESHADER)
        shaderType = GL_COMPUTE_SHADER;

    GLuint shader = glCreateShader(shaderType);
    GLint length = (GLint)content.length();
    const char* shdSrc = content.c_str();
    glShaderSource(shader, 1, &shdSrc, &length);
    glCompileShader(shader);

    // get error log
    GLint shaderLogSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shaderLogSize);
    if (shaderLogSize > 1)
    {
        char* buf = new char[shaderLogSize];
        glGetShaderInfoLog(shader, shaderLogSize, NULL, buf);

        std::string original = buf;
        std::vector<std::string> vstrings;
        std::string token;
        std::stringstream ss(original);
        while (getline(ss, token, '\n')) {
            vstrings.push_back(token);
        }

        for (auto& output : vstrings)
        {
            size_t start = output.find_first_of("0(");
            size_t end = output.find_last_of(")");

            if (start != std::string::npos && end != std::string::npos)
            {
                std::string sub = output.substr(start + 2, end - start - 1);
                int i = atoi(sub.c_str());
                std::string lineNumber = std::to_string(i - numIncludedLines);
                output.replace(start + 2, sub.length() - 1, lineNumber);
            }
        }

        std::string output;
        for (auto const& str : vstrings)
        {
            output += str;
            output += "\n";
        }

        printf("\n[SHADER COMPILE ERROR]: \n%s", output.c_str());
        delete[] buf;
     }

    Instance()->shaderSources.push_back(path);
    Instance()->shaders.push_back(shader);
    Instance()->shaderTypes.push_back(type);
    printf("OK\n");
    return ShaderResourceId(Instance()->shaders.size() - 1);
}

//------------------------------------------------------------------------------
/**
*/
ShaderProgramId
ShaderResource::CompileShaderProgram(std::vector<ShaderResourceId> const& shaders)
{
    printf("Creating shader program... ");
    GLuint program = glCreateProgram();
    for (auto shader : shaders)
    {
        glAttachShader(program, Instance()->shaders[shader]);
    }

    glLinkProgram(program);
    GLint shaderLogSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &shaderLogSize);
    if (shaderLogSize > 1)
    {
        GLchar* buf = new GLchar[shaderLogSize];
        glGetProgramInfoLog(program, shaderLogSize, NULL, buf);
        printf("FAIL\n[PROGRAM LINK ERROR]: %s\n", buf);
        delete[] buf;
    }

    Instance()->programs.push_back(program);
    Instance()->programShaders.push_back(shaders);
    printf("OK\n");
    return ShaderProgramId(Instance()->programs.size() - 1);
}

//------------------------------------------------------------------------------
/**
*/
GLuint
ShaderResource::GetProgramHandle(ShaderProgramId programId)
{
    return Instance()->programs[programId];
}

//------------------------------------------------------------------------------
/**
*/
void
ShaderResource::ReloadShaders()
{
    printf("--- RELOAD SHADERS ---\n");
    // intentional copies
    std::vector<std::string> paths = Instance()->shaderSources;
    std::vector<ShaderType> types = Instance()->shaderTypes;

    for (size_t i = 0; i < Instance()->shaders.size(); i++)
    {
        glDeleteShader(Instance()->shaders[i]);
    }

    Instance()->shaderSources.clear();
    Instance()->shaders.clear();
    Instance()->shaderTypes.clear();

    for (size_t i = 0; i < paths.size(); i++)
    {
        LoadShader(types[i], paths[i].c_str());
    }

    std::vector<std::vector<ShaderResourceId>> progs = Instance()->programShaders;

    for (size_t i = 0; i < Instance()->programs.size(); i++)
    {
        glDeleteProgram(Instance()->programs[i]);
    }

    Instance()->programs.clear();
    Instance()->programShaders.clear();

    for (size_t i = 0; i < progs.size(); i++)
    {
        CompileShaderProgram(progs[i]);
    }
}

} // namespace Render
