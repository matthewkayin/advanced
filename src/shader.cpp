#include "shader.hpp"

#include <fstream>
#include <cstdio>
#include <fstream>
#include <map>
#include <vector>

GLuint shader;
GLuint text_shader;
GLuint screen_shader;

const std::map<std::string, GLenum> SHADER_TYPE = {
    { "vertex", GL_VERTEX_SHADER },
    { "fragment", GL_FRAGMENT_SHADER }
};

bool shader_compile(unsigned int* id, const char* path);

bool shader_init() {
    if (!shader_compile(&shader, "./shader/shader.glsl")) {
        return false;
    }
    if (!shader_compile(&text_shader, "./shader/text.glsl")) {
        return false;
    }
    if (!shader_compile(&screen_shader, "./shader/screen.glsl")) {
        return false;
    }

    return true;
}

bool shader_compile(unsigned int* id, const char* path) {
    std::ifstream shader_file;
    std::string line;
    std::string version_string;

    struct ShaderData {
        std::string type_name;
        std::string source;
        GLint id;
    };
    std::map<GLenum, ShaderData> shaders;
    GLenum current_shader;

    // open shader file
    shader_file.open(path);
    if (!shader_file.is_open()) {
        printf("Unable to open shader %s\n", path);
        return false;
    }

    // read shader file
    while (std::getline(shader_file, line)) {
        if (line.rfind("#version") == 0) {
            version_string = line;
        } else if (line.rfind("#begin") == 0) {
            std::string shader_type = line.substr(line.find(" ") + 1);
            GLenum gl_shader_type = SHADER_TYPE.at(shader_type);
            shaders[gl_shader_type].source = version_string + "\n";
            current_shader = gl_shader_type;
        } else {
            shaders[current_shader].source += line + "\n";
        }
    }
    shader_file.close();

    // compile shaders
    int success;
    char info_log[512];

    for (std::map<GLenum, ShaderData>::iterator itr = shaders.begin(); itr != shaders.end(); ++itr) {
        itr->second.id = glCreateShader(itr->first);
        const char* shader_source = itr->second.source.c_str();
        glShaderSource(itr->second.id, 1, &shader_source, NULL);
        glCompileShader(itr->second.id);
        glGetShaderiv(itr->second.id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(itr->second.id, 512, NULL, info_log);
            printf("Error: shader with path %s failed to compile %s shader.\n%s\n", path, itr->second.type_name.c_str(), info_log);
            return false;
        }

    }

    // link program
    GLuint program = glCreateProgram();
    for (std::map<GLenum, ShaderData>::iterator itr = shaders.begin(); itr != shaders.end(); ++itr) {
        glAttachShader(program, itr->second.id);
    }

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Error linking shader program %s.\n%s\n", path, info_log);
        return false;
    }

    for (std::map<GLenum, ShaderData>::iterator itr = shaders.begin(); itr != shaders.end(); ++itr) {
        glDeleteShader(itr->second.id);
    }

    *id = program;
    return true;
}
