//
// Created by Alberto Giudice.
// Sphere setup algorithm by https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.1.lighting/lighting.cpp
//

#ifndef ITU_GRAPHICS_PROGRAMMING_PBRSPHERE_H
#define ITU_GRAPHICS_PROGRAMMING_PBRSPHERE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <shader.h>
#include "pbrmaterial.h"

const unsigned int X_SEGMENTS = 64;
const unsigned int Y_SEGMENTS = 64;
const float PI = 3.14159265359f;

class PBRSphere{
public:
    // constructor for a plain colored sphere
    PBRSphere(glm::vec3 offset, float metallic = 0.f, float roughness = 0.f) {
        this->offset = offset;
        this->roughness = roughness;
        this->metallic = metallic;

        if(vao == 0)
            SetupSphere();
    }

    void SetMaterial(PBRMaterial* material) {
        this->material = material;
    }

    void SetAlbedo(const glm::vec3& albedo) {
        this->albedo = albedo;
    }

    void SetMetallic(const float& metallic) {
        this->metallic = metallic;
    }

    void SetRoughness(const float& roughness) {
        this->roughness = roughness;
    }

    void Draw(Shader* shader, bool withMaterial = false) const
    {
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, offset);
        shader->setMat4("model", model);
        shader->setMat3("modelInvTra", glm::inverse(glm::transpose(glm::mat3(model))));

        if(withMaterial) {
            if(material != nullptr) material->use();
        }
        else {
            shader->setVec3("albedoUniform", albedo);
            shader->setFloat("metallicUniform", metallic);
            shader->setFloat("roughnessUniform", roughness);
        }

        // draw sphere
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    // render variables
    static unsigned int vao;
    static std::vector<unsigned int> indices;
    glm::vec3 offset;

    // material properties
    glm::vec3 albedo;
    float roughness;
    float metallic;
    PBRMaterial* material;

    // initialize buffer object & arrays
    void SetupSphere(){
        unsigned int vbo, ebo;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;
        std::vector<unsigned int> stripIndices;

        glGenVertexArrays(1, &vao);

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.emplace_back(xPos, yPos, zPos);
                uv.emplace_back(xSegment, ySegment);
                normals.push_back(glm::normalize(glm::vec3(xPos,yPos,zPos)));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    stripIndices.push_back(y       * (X_SEGMENTS + 1) + x);
                    stripIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    stripIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    stripIndices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        // calculate tangents and bitangents
        unsigned int index0, index1, index2;
        bool even = true;
        for (unsigned int i = 0; i < stripIndices.size()-2; i++) {
            index0 = even ? stripIndices[i] : stripIndices[i+1];
            index1 = even ? stripIndices[i+1] : stripIndices[i];
            index2 = stripIndices[i+2];

            if(index0 != index1 && index1 != index2 && index2 != index0) {
                indices.push_back(index0);
                indices.push_back(index1);
                indices.push_back(index2);
            }
            even = !even;
        }

        // sphere tangent and bitangent simple calculation for a unit sphere.
        // Inspired by https://computergraphics.stackexchange.com/questions/5498/compute-sphere-tangent-for-normal-mapping
        for(int i=0; i<positions.size(); i++) {
            tangents.push_back(glm::normalize(glm::cross(glm::vec3(1.f), positions[i])));
            bitangents.push_back(glm::normalize(glm::cross(normals[i], tangents[i])));
        }

        // push all the sphere data to the buffers
        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            // positions
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            // normals
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
            // uv
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
            // tangents
            data.push_back(tangents[i].x);
            data.push_back(tangents[i].y);
            data.push_back(tangents[i].z);
            // bitangents
            data.push_back(bitangents[i].x);
            data.push_back(bitangents[i].y);
            data.push_back(bitangents[i].z);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 3 + 2 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
    }
};

unsigned int PBRSphere::vao = 0;
std::vector<unsigned int> PBRSphere::indices = std::vector<unsigned int>();


#endif //ITU_GRAPHICS_PROGRAMMING_PBRSPHERE_H
