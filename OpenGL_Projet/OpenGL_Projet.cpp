
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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string PenguinModel = "Models/penguin.obj";
const char * PenguinTexture = "Models/Textures/penguin.png";

std::string TeapotModel = "Models/teapot.obj";

GLShader g_TextureShader;
GLShader g_ColorShader;

GLuint VBO_PENGUIN;
GLuint IBO_PENGUIN;
GLuint VAO_PENGUIN;

GLuint VBO_TEAPOT;
GLuint IBO_TEAPOT;
GLuint VAO_TEAPOT;


GLuint TexID;

float x_translation = 0.0f, y_translation = -0.5f, z_translation = -15.0f; // Coordonnées de position du modèle au lancement

float movementSpeed = 0.01f;//Rapidité du mouvement des objets (scroll/flèches)

std::vector<Vertex> VerticesPenguin;
std::vector<uint16_t> IndicesPenguin;

std::vector<Vertex> VerticesTeapot;
std::vector<uint16_t> IndicesTeapot;

/* Fonction retournant une ViewMatrix calculée 
en fonction des vecteurs position, target et up donnés*/
float * LookAt(vec3 position, vec3 target, vec3 up) {
    vec3 forward = -(target-position); // zaxis
    normalize(&forward);
    vec3 right = forward*up; // xaxis
    normalize(&right);
    up = forward * right; //yaxis
    static float ViewMatrix[16];
    ViewMatrix[0] = right.x;
    ViewMatrix[1] = up.x;
    ViewMatrix[2] = forward.x;
    ViewMatrix[3] = 0.0f;
    ViewMatrix[4] = right.y;
    ViewMatrix[5] = up.y;
    ViewMatrix[6] = forward.y;
    ViewMatrix[7] = 0.0f;
    ViewMatrix[8] = right.z;
    ViewMatrix[9] = up.z;
    ViewMatrix[10] = forward.z;
    ViewMatrix[11] = 0.0f;
    ViewMatrix[12] = -dot(right,position);
    ViewMatrix[13] = -dot(up, position);
    ViewMatrix[14] = -dot(forward, position);;
    ViewMatrix[15] = 1.0f;
    return ViewMatrix;
}

/* Fonction permettant de multiplier deux matrices 4D
   représentées par des tableaux de float 1D */
float * Multiply4DMatrices(float * _m1, float * _m2) {
    static float result[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += _m1[i * 4 + k] * _m2[j + 4 * k];
            }
            result[i *4 + j] = sum;
        }
    }
    
    return result;
}

/* 
* Update la position z du modèle (scroll de la souris)
*/
void UpdateScroll(GLFWwindow* window, double xoffset, double yoffset) {
    z_translation = z_translation + yoffset * movementSpeed;
}

/* Update la position du modèle en fonction des input*/
void UpdateInput(GLFWwindow* window) {
    glfwSetScrollCallback(window, UpdateScroll);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        x_translation -= movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        x_translation +=  movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        y_translation -= movementSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        y_translation += movementSpeed;
    }
}
/* Permet de charger un modèle .obj et retourne un tuple contenant
 un tableau de Vertex et un tableau d'indices */
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


                Vertex Vj = { { vx, vy, vz }, { nx, ny, nz }, { tx, ty } , {255,0,0,255}};

                uint16_t index = Vertices.size();

                // Cherche si le Vertex existe déjà dans le tableau, pour économiser de la mémoire
                // et réutiliser un index
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
    tie(VerticesPenguin, IndicesPenguin) = LoadObj(PenguinModel);
    tie(VerticesTeapot, IndicesTeapot) = LoadObj(TeapotModel);
    GLenum ret = glewInit();

    g_TextureShader.LoadVertexShader("TextureShader.vs");
    g_TextureShader.LoadFragmentShader("TextureShader.fs");
    g_TextureShader.Create();

    g_ColorShader.LoadVertexShader("ColorShader.vs");
    g_ColorShader.LoadFragmentShader("ColorShader.fs");
    g_ColorShader.Create();

    glGenBuffers(1, &VBO_PENGUIN);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_PENGUIN);
    glBufferData(GL_ARRAY_BUFFER, VerticesPenguin.size() * sizeof(Vertex), &VerticesPenguin[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IBO_PENGUIN);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_PENGUIN);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesPenguin.size()*sizeof(uint16_t), &IndicesPenguin[0], GL_STATIC_DRAW);

    glGenBuffers(1, &VBO_TEAPOT);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_TEAPOT);
    glBufferData(GL_ARRAY_BUFFER, VerticesTeapot.size() * sizeof(Vertex), &VerticesTeapot[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IBO_TEAPOT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_TEAPOT);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesTeapot.size() * sizeof(uint16_t), &IndicesTeapot[0], GL_STATIC_DRAW);


    constexpr size_t stride = sizeof(Vertex);// sizeof(float) * 5;

    // 
    auto ProgramTextureShader = g_TextureShader.GetProgram();

    // Penguin Object
    glGenVertexArrays(1, &VAO_PENGUIN);
    glBindVertexArray(VAO_PENGUIN);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_PENGUIN);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_PENGUIN);

    int loc_position = glGetAttribLocation(ProgramTextureShader, "a_position");
    glEnableVertexAttribArray(loc_position);
    glVertexAttribPointer(loc_position, 3, GL_FLOAT, false, stride, (void*)offsetof(Vertex, position));


    int loc_uv = glGetAttribLocation(ProgramTextureShader, "a_texcoords");
    glEnableVertexAttribArray(loc_uv);
    glVertexAttribPointer(loc_uv, 2, GL_FLOAT, false, stride, (void*)offsetof(Vertex, uv));

    int loc_normal = glGetAttribLocation(ProgramTextureShader, "a_normal");
    glEnableVertexAttribArray(loc_normal);
    glVertexAttribPointer(loc_normal, 3, GL_FLOAT, false, stride, (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto ProgramColorShader = g_ColorShader.GetProgram();
    // Teapot Object
    glGenVertexArrays(1, &VAO_TEAPOT);
    glBindVertexArray(VAO_TEAPOT);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_TEAPOT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_TEAPOT);

    int loc_position2 = glGetAttribLocation(ProgramColorShader, "a_position");
    glEnableVertexAttribArray(loc_position2);
    glVertexAttribPointer(loc_position2, 3, GL_FLOAT, false, stride, (void*)offsetof(Vertex, position));

    int loc_normal2 = glGetAttribLocation(ProgramColorShader, "a_normal");
    glEnableVertexAttribArray(loc_normal2);
    glVertexAttribPointer(loc_normal2, 3, GL_FLOAT, false, stride, (void*)offsetof(Vertex, normal));

    int loc_color2 = glGetAttribLocation(ProgramColorShader, "a_color");
    glEnableVertexAttribArray(loc_color2);
    glVertexAttribPointer(loc_color2, 4, GL_UNSIGNED_BYTE, true, stride, (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Prise en charge de la texture
    glGenTextures(1, &TexID);
    glBindTexture(GL_TEXTURE_2D, TexID);
    int w, h;
    uint8_t* data = stbi_load(PenguinTexture, &w, &h, nullptr, STBI_rgb_alpha);
    if (data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // filtre trilineaire (necessite mipmap)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return true;
}

void Terminate()
{
    glDeleteTextures(1, &TexID);

    glDeleteVertexArrays(1, &VAO_PENGUIN);
    glDeleteBuffers(1, &VBO_PENGUIN);
    glDeleteBuffers(1, &IBO_PENGUIN);

    g_TextureShader.Destroy();
}

void Render(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    vec3 position = { 0.1f, 0.2f, -1.0f };
    vec3 target = { 0.1f,0.1f,-5.0f };
    vec3 up = { 0.0f,1.0f,0.0f }; 
    float* ViewMatrix = LookAt(position, target, up);

    float lightPos[] = { 18.0f,2.0f,-10.0f };

    const float zNear = 0.1f;
    const float zFar = 800.0f;
    const float aspect = float(width) / float(height); //important de cast en float
    const float fov = 45.0f * M_PI / 180.0; //en radian
    const float f = 1.0f / tanf(fov / 2.0f); //cotan = 1/tan
    float ProjectionMatrix[] = {
        f / aspect, 0.f, 0.f, 0.f,
        0.f, f, 0.f, 0.f,
        0.f, 0.f, ((zFar + zNear) / (zNear - zFar)), -1.f,
        0.f, 0.f, ((2 * zNear * zFar) / (zNear - zFar)), 0.f
    };

    // ###########################################
    //       RENDERING DU TEAPOT OBJECT
    // ###########################################
    auto ProgramColorShader = g_ColorShader.GetProgram();
    glUseProgram(ProgramColorShader);

    // Position de notre lumière pour l'illumination de phong
    GLint lightPosLocationTeapot = glGetUniformLocation(ProgramColorShader, "u_lightPos");
    glUniform3fv(lightPosLocationTeapot, 1, lightPos);

    float scaleObjectMatrixTeapot[] = { .009f,  0.0f,  0.0f,  0.0f,
                                        0.0f,  .009f,  0.0f,  0.0f,
                                        0.0f,  0.0f,  .009f,  0.0f,
                                        0.0f,  0.0f,  0.0f,  1.0f };

    float rotationObjectMatrixTeapot[] = { cosf(0.5),   0.0f,      0.0f,       0.0f,
                                        0.0f,    1.0f,    0.0f,       0.0f,
                                        0.0f,  0.0f,      1.0f,       0.0f,
                                        0.0f,               0.0f,           0.0f,      1.0f };

    float translationObjectMatrixTeapot[] = { 1.0f,  0.0f,  0.0f,  0.0f,
                                         0.0f,  1.0f,  0.0f,  0.0f,
                                         0.0f,  0.0f,  1.0f,  0.0f,
                                         x_translation - 5.0f,  y_translation-2.0f,  z_translation,  1.0f };

    float* WorldMatrix2 = Multiply4DMatrices(Multiply4DMatrices(translationObjectMatrixTeapot, rotationObjectMatrixTeapot), scaleObjectMatrixTeapot);

    GLint projection_teapot = glGetUniformLocation(ProgramColorShader, "u_ProjectionMatrix");
    glUniformMatrix4fv(projection_teapot, 1, false, ProjectionMatrix);

    GLint view_teapot = glGetUniformLocation(ProgramColorShader, "u_ViewMatrix");
    glUniformMatrix4fv(view_teapot, 1, false, ViewMatrix);

    GLint world_teapot = glGetUniformLocation(ProgramColorShader, "u_WorldMatrix");
    glUniformMatrix4fv(world_teapot, 1, false, WorldMatrix2);

    float* MVPMatrixTeapot = Multiply4DMatrices(Multiply4DMatrices(ProjectionMatrix, ViewMatrix), WorldMatrix2);

    GLint MVP_teapot = glGetUniformLocation(ProgramColorShader, "u_MVP");
    glUniformMatrix4fv(MVP_teapot, 1, false, MVPMatrixTeapot);

    glBindVertexArray(VAO_TEAPOT);
    glDrawElements(GL_TRIANGLES, IndicesTeapot.size(), GL_UNSIGNED_SHORT, 0);

    // ###########################################

    // ###########################################
    //       RENDERING DU PENGUIN OBJECT
    // ###########################################

    glClear(GL_DEPTH_BUFFER_BIT);

    auto ProgramTextureShader = g_TextureShader.GetProgram();
    glUseProgram(ProgramTextureShader);
  

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    GLint timeLocationPenguin = glGetUniformLocation(ProgramTextureShader, "u_time");
    const float time = glfwGetTime();
    glUniform1f(timeLocationPenguin, time);

    // Position de notre lumière pour l'illumination de phong
    GLint lightPosLocationPenguin = glGetUniformLocation(ProgramTextureShader, "u_lightPos");
    glUniform3fv(lightPosLocationPenguin, 1 , lightPos);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexID);

    GLint textureLocationPenguin = glGetUniformLocation(ProgramTextureShader, "u_sampler");
    // 0 ici correspond au canal 0 cad GL_TEXTURE0
    glUniform1i(textureLocationPenguin, 0);

    float scaleObjectMatrixPenguin[] = { 20.0f,  0.0f,  0.0f,  0.0f,
                                 0.0f,  20.0f,  0.0f,  0.0f,
                                 0.0f,  0.0f,  20.0f,  0.0f,
                                 0.0f,  0.0f,  0.0f,  1.0f };

    float rotationObjectMatrixPenguin[] = {1.0f,    0.0f,     0.0f,       0.0f,
                                        0.0f,    1.0f,     0.0f,       0.0f,
                                        0.0f,  0.0f,      1.0f,       0.0f,
                                        0.0f,               0.0f,           0.0f,      1.0f };

    float translationObjectMatrixPenguin[] = { 1.0f,  0.0f,  0.0f,  0.0f,
                                         0.0f,  1.0f,  0.0f,  0.0f,
                                         0.0f,  0.0f,  1.0f,  0.0f,
                                         x_translation,  y_translation,  z_translation+12.0f,  1.0f };

    float* WorldMatrixPenguin = Multiply4DMatrices(Multiply4DMatrices(translationObjectMatrixPenguin, rotationObjectMatrixPenguin), scaleObjectMatrixPenguin);


    GLint projection_penguin = glGetUniformLocation(ProgramTextureShader, "u_ProjectionMatrix");
    glUniformMatrix4fv(projection_penguin, 1, false, ProjectionMatrix);

    GLint view_penguin = glGetUniformLocation(ProgramTextureShader, "u_ViewMatrix");
    glUniformMatrix4fv(view_penguin, 1, false, ViewMatrix);

    GLint world_penguin = glGetUniformLocation(ProgramTextureShader, "u_WorldMatrix");
    glUniformMatrix4fv(world_penguin, 1, false, WorldMatrixPenguin);

    float* MVPMatrixPenguin = Multiply4DMatrices(Multiply4DMatrices(ProjectionMatrix, ViewMatrix), WorldMatrixPenguin);

    GLint MVP_penguin = glGetUniformLocation(ProgramTextureShader, "u_MVP");
    glUniformMatrix4fv(MVP_penguin, 1, false, MVPMatrixPenguin);

    glBindVertexArray(VAO_PENGUIN);
    glDrawElements(GL_TRIANGLES, IndicesPenguin.size(), GL_UNSIGNED_SHORT, 0);

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
        /*Update movements input*/
        UpdateInput(window);

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
