
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
#include "Models/DragonData.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// attention, ce define ne doit etre specifie que dans 1 seul fichier cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLShader g_TransformShader;

GLuint VBO;
GLuint IBO;
GLuint VAO;

GLuint TexID;

std::vector<Vertex> Vertices;
std::vector<uint16_t> Indices;

std::tuple<std::vector<Vertex>, std::vector<uint16_t>> LoadObj(std::string inputfile) {
    std::vector<Vertex> Vertices;
    std::vector<uint16_t> Indices;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

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

                Vertex Vj = { { vx, vy, vz }, { nx, ny, nz}, {tx, ty} };

                uint16_t index = Vertices.size();
                auto it = std::find(Vertices.begin(), Vertices.end(), Vj);
                if (it != Vertices.end()) {
                    index = it - Vertices.begin();
                }
                else {
                    Vertices.push_back(Vj);
                }

                Indices.push_back(index);


                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }
    return {Vertices, Indices};
}

bool Initialise()
{
    tie(Vertices, Indices) = LoadObj("Models/teapot.obj");
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
    uint8_t* data = stbi_load("Models/Textures/dragon.png", &w, &h, nullptr, STBI_rgb_alpha);
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

    // etape a. A vous de recuperer/passer les variables width/height
    glViewport(0, 0, width, height);
    // etape b. Notez que glClearColor est un etat, donc persistant
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
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
    float rotation2D_homogene3D[] = { cosf(time),     sinf(time),     0.0f,
                                        - sinf(time),    cosf(time),     0.0f,
                                        0.0f,           0.0f,           1.0f };

    float rotation2D_homogene4D[] = {   cosf(time),         sinf(time),     0.0f,       0.0f,
                                        -sinf(time),        cosf(time),     0.0f,       0.0f,
                                        0.0f,               0.0f,           1.0f,       0.0f,
                                        0.0f,               0.0f,           -50.0f,     1.0f };

    GLint rot2D_location = glGetUniformLocation(program, "u_rotation4D");
    glUniformMatrix4fv(rot2D_location, 1, false, rotation2D_homogene4D);

    const float zNear = 0.1f;
    const float zFar = 100.0f;
    const float aspect = float(width) / float(height); //important de cast en float
    const float fov = 45.0f * M_PI / 180.0; //en radian
    const float f = 1.0f / tanf(fov / 2.0f); //cotan = 1/tan
    const float projection[] = {
        f / aspect, 0.f, 0.f, 0.f,
        0.f, f, 0.f, 0.f,
        0.f, 0.f, ((zFar + zNear) / (zNear - zFar)), -1.f,
        0.f, 0.f, ((2 * zNear * zFar) / (zNear - zFar)), 0.f
    };

    GLint rot2D_proj = glGetUniformLocation(program, "u_projection");
    glUniformMatrix4fv(rot2D_proj, 1, false, projection);

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