//
// Created by Alberto Giudice.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_PBRCUBE_H
#define ITU_GRAPHICS_PROGRAMMING_PBRCUBE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <shader.h>
#include "pbrmaterial.h"

const float back_face[]  = {
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-left
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-left
};
const float front_face[] = {
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
};
const float left_face[] = {
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // top-right
};
const float right_face[] = {
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
};
const float bottom_face[] = {
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // top-right
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // top-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // bottom-left
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // top-right
};
const float top_face[] = {
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,  // bottom-left
};

class PBRCube{
public:
    // constructor for a plain colored sphere
    PBRCube(glm::vec3 offset = glm::vec3(0.0f), float metallic = 0.f, float roughness = 0.f) {
        this->offset = offset;
        this->roughness = roughness;
        this->metallic = metallic;

        if(vao == 0)
            SetupCube();
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

        // draw cube
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void DebugDraw() {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
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
    void SetupCube(){
        unsigned int vbo;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;

        readCubeFace(back_face, positions, normals, uv);
        readCubeFace(front_face, positions, normals, uv);
        readCubeFace(left_face, positions, normals, uv);
        readCubeFace(right_face, positions, normals, uv);
        readCubeFace(bottom_face, positions, normals, uv);
        readCubeFace(top_face, positions, normals, uv);

        for(unsigned int i=0; i< positions.size(); i+=3) {
            calcTangents(positions[i], positions[i+1], positions[i+2], uv[i], uv[i+1], uv[i+2], tangents, bitangents);
        }

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

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
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

    void readCubeFace(const float *input, std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uv) const {
        for(unsigned int i=0; i<6; i++) {
            unsigned int v = i * 8;
            positions.emplace_back(input[v], input[v+1], input[v+2]);
            normals.emplace_back(input[v+3], input[v+4], input[v+5]);
            uv.emplace_back(input[v+6], input[v+7]);
        }
    }

    void calcTangents(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3,
                      glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3,
                      std::vector<glm::vec3> &tangents, std::vector<glm::vec3> &bitangents) const {
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = glm::normalize(f * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
        glm::vec3 bitangent = glm::normalize(f * (deltaUV1.x * edge2 - deltaUV2.x * edge1));
        for(int i = 0; i < 3; ++i) {
            tangents.push_back(tangent);
            bitangents.push_back(bitangent);
        }
    }
};

unsigned int PBRCube::vao = 0;
std::vector<unsigned int> PBRCube::indices = std::vector<unsigned int>();


#endif //ITU_GRAPHICS_PROGRAMMING_PBRCUBE_H
