﻿
// seulement si glew32s.lib
#define GLEW_STATIC 1

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <tuple>

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include "../common/GLShader.h"

#include "Vertex.h"
#include "vec3.h"
#include "vec2.h"
#include "Color.h"

#include "Models/DragonData.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// attention, ce define ne doit etre specifie que dans 1 seul fichier cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string ObjectModel = "Models/penguin.obj";
const char * ObjectTexture = "Models/Textures/penguin.png";

GLShader g_TransformShader;

GLuint VBO;
GLuint IBO;
GLuint VAO;

GLuint TexID;

float scale= 1.0f, x_translation = 1.0f, y_translation = 1.0f, z_translation = 1.0f;

float deltaTime = 1.0f;
float movementSpeed = 1.0f;
float currentTime = 0;
double lastTime = 0;

std::vector<Vertex> Vertices;
std::vector<uint16_t> Indices;

float * CreateViewMatrix(vec3 position, vec3 target, vec3 up) {
    vec3 forward = -(target-position);
    normalize(&forward);
    vec3 right = forward*up;
    static float ViewMatrix[16];
    ViewMatrix[0] = forward.x;
    ViewMatrix[1] = right.x;
    ViewMatrix[2] = up.x;
    ViewMatrix[3] = 0.0f;
    ViewMatrix[4] = forward.y;
    ViewMatrix[5] = right.y;
    ViewMatrix[6] = up.y;
    ViewMatrix[7] = 0.0f;
    ViewMatrix[8] = forward.z;
    ViewMatrix[9] = right.z;
    ViewMatrix[10] = up.z;
    ViewMatrix[11] = 0.0f;
    ViewMatrix[12] = -dot(forward,position);
    ViewMatrix[13] = -dot(right, position);
    ViewMatrix[14] = -dot(up, position);;
    ViewMatrix[15] = 1.0f;

    return ViewMatrix;
}

float * Multiply4DMatrices(float * _m1, float * _m2) {
    static float result[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += _m1[i * 4 + k] * _m2[j + 4 * k];
            }
            //printf("%f\n", sum);
        }
    }
    
    return result;
}

void UpdateScale(GLFWwindow* window, double xoffset, double yoffset) {
    scale += yoffset * deltaTime * movementSpeed;
}

void UpdateTranslation(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        x_translation += deltaTime * movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        x_translation -= deltaTime * movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        z_translation += deltaTime * movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        z_translation -= deltaTime * movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        y_translation += deltaTime * movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        y_translation -= deltaTime * movementSpeed;
    }
}

std::tuple<std::vector<Vertex>, std::vector<uint16_t>> LoadObj(std::string inputfile) {
    std::vector<Vertex> Vertices;
    std::vector<uint16_t> Indices;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./Models/"; // Path to material files

    tinyobj::ObjReader reader;


    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                tinyobj::real_t nx = 0, ny = 0, nz = 0;
                tinyobj::real_t tx = 0, ty = 0;

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                Vertex Vj = { { vx, vy, vz }, { nx, ny, nz }, { tx, ty } };

                uint16_t index = Vertices.size();
                auto it = std::find(Vertices.begin(), Vertices.end(), Vj);
                if (it != Vertices.end()) {
                    index = it - Vertices.begin();
                }
                else {
                    Vertices.push_back(Vj);
                }
                Indices.push_back(index);
            }
            index_offset += fv;
        }
    }
    return {Vertices, Indices};
}

bool Initialise()
{
    tie(Vertices, Indices) = LoadObj(ObjectModel);
    GLenum ret = glewInit();

    g_TransformShader.LoadVertexShader("transform.vs");
    g_TransformShader.LoadFragmentShader("transform.fs");
    g_TransformShader.Create();

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), &Vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size()*sizeof(uint16_t), &Indices[0], GL_STATIC_DRAW);


    constexpr size_t stride = sizeof(Vertex);// sizeof(float) * 5;

    // 
    auto program = g_TransformShader.GetProgram();

    // VAO ---
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    int loc_position = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(loc_position);
    glVertexAttribPointer(loc_position, 3, GL_FLOAT, false, stride, (void*)offsetof(Vertex, position));

    int loc_uv = glGetAttribLocation(program, "a_texcoords");
    glEnableVertexAttribArray(loc_uv);
    glVertexAttribPointer(loc_uv, 2, GL_FLOAT, false, stride, (void*)offsetof(Vertex, uv));

    // La bonne pratique est de reinit a zero
    // MAIS ATTENTION, toujours le VAO en premier
    // sinon le VAO risque d'enregistrer les modifications
    // de VertexAttrib et VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(1, &TexID);
    glBindTexture(GL_TEXTURE_2D, TexID);
    int w, h;
    uint8_t* data = stbi_load(ObjectTexture, &w, &h, nullptr, STBI_rgb_alpha);
    if (data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    // filtre bilineaire
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // filtre trilineaire (necessite mipmap)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return true;
}

void Terminate()
{
    glDeleteTextures(1, &TexID);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);

    g_TransformShader.Destroy();
}

void Render(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    
    // etape a. A vous de recuperer/passer les variables width/height
    glViewport(0, 0, width, height);
    // etape b. Notez que glClearColor est un etat, donc persistant
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // etape c. on specifie le shader program a utiliser
    auto program = g_TransformShader.GetProgram();
    glUseProgram(program);
    // etape d. 

    // etape e. 
    GLint timeLocation = glGetUniformLocation(program, "u_time");
    const float time = glfwGetTime();
    glUniform1f(timeLocation, time);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexID);

    GLint textureLocation = glGetUniformLocation(program, "u_sampler");
    // 0 ici correspond au canal 0 cad GL_TEXTURE0
    glUniform1i(textureLocation, 0);



    //colonnes d'abord
    //multiplication : droite vers la gauche
    //v' = M * v

    lastTime = currentTime;
    currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    UpdateTranslation(window);
    glfwSetScrollCallback(window, UpdateScale);

    float scaleObjectMatrix[] = { scale,  0.0f,  0.0f,  0.0f,
                            0.0f,  scale,  0.0f,  0.0f,
                            0.0f,  0.0f,  scale,  0.0f,
                            0.0f,  0.0f,  0.0f,  1.0f };

    float rotationObjectMatrix[] = {cosf(time),    0.0f,     sinf(time),       0.0f,
                                        0.0f,    1.0f,     0.0f,       0.0f,
                                        -sinf(time),  0.0f,      cosf(time),       0.0f,
                                        0.0f,               -1.0f,           -5.0f,      1.0f };

    /*float rotationObjectMatrix[] = {1.0f,  0.0f,  0.0f,  0.0f,
                                   0.0f,  1.0f,  0.0f,  0.0f,
                                   0.0f,  0.0f, 1.0f,  0.0f,
                                   0.0f,  -1.0f,  -5.0f,  1.0f };*/

    float translationObjectMatrix[] = { 1.0f,  0.0f,  0.0f,  0.0f,
                                         0.0f,  1.0f,  0.0f,  0.0f,
                                         0.0f,  0.0f,  1.0f,  0.0f,
                                         0.0f,  0.0f,  0.0f,  1.0f };
    
    GLint scaleObject = glGetUniformLocation(program, "u_scale");
    glUniformMatrix4fv(scaleObject, 1, false, scaleObjectMatrix);


    GLint rotationObject = glGetUniformLocation(program, "u_rotation");
    glUniformMatrix4fv(rotationObject, 1, false, rotationObjectMatrix);

    GLint translationObject = glGetUniformLocation(program, "u_translation");
    glUniformMatrix4fv(translationObject, 1, false, translationObjectMatrix);


    float* ModelMatrix = Multiply4DMatrices(translationObjectMatrix, Multiply4DMatrices(rotationObjectMatrix, scaleObjectMatrix));

    vec3 position = { x_translation, y_translation, z_translation };
    vec3 target = { 0.0f,0.0f,0.0f};
    vec3 up = { 0.0f,1.0f,0.0f };

    float * ViewMatrix = CreateViewMatrix(position, target, up);

    const float zNear = 0.1f;
    const float zFar = 800.0f;
    const float aspect = float(width) / float(height); //important de cast en float
    const float fov = 45.0f * M_PI / 180.0; //en radian
    const float f = 1.0f / tanf(fov / 2.0f); //cotan = 1/tan
    const float ProjectionMatrix[] = {
        f / aspect, 0.f, 0.f, 0.f,
        0.f, f, 0.f, 0.f,
        0.f, 0.f, ((zFar + zNear) / (zNear - zFar)), -1.f,
        0.f, 0.f, ((2 * zNear * zFar) / (zNear - zFar)), 0.f
    };


    GLint projection = glGetUniformLocation(program, "u_projection");
    glUniformMatrix4fv(projection, 1, false, ProjectionMatrix);

    GLint view = glGetUniformLocation(program, "u_view");
    glUniformMatrix4fv(view, 1, false, ViewMatrix);

    GLint model = glGetUniformLocation(program, "u_model");
    glUniformMatrix4fv(model, 1, false, ModelMatrix);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_SHORT, 0);

    // on suppose que la phase d’echange des buffers front et back
    // le « swap buffers » est effectuee juste apres
}


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(900, 600, "ESIEE Paris - E4FI - Projet OpenGL - DESRIAUX FOUQUOIRE RENARD KUGATHAS", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    Initialise();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        Render(window);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    Terminate();

    glfwTerminate();
    return 0;
}
