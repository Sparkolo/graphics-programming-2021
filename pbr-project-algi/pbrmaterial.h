//
// Created by Alberto Giudice
// Texture loading function by https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.1.lighting/lighting.cpp
//

#ifndef ITU_GRAPHICS_PROGRAMMING_PBRMATERIAL_H
#define ITU_GRAPHICS_PROGRAMMING_PBRMATERIAL_H

#include <glad/glad.h>
#include "textureloader.h"

class PBRMaterial {
public:
    explicit PBRMaterial(const std::string& materialName) {
        albedo = loadTexture(("resources/textures/pbr/" + materialName + "/albedo.png").c_str());
        normal = loadTexture(("resources/textures/pbr/" + materialName + "/normal.png").c_str());
        metallic = loadTexture(("resources/textures/pbr/" + materialName + "/metallic.png").c_str());
        roughness = loadTexture(("resources/textures/pbr/" + materialName + "/roughness.png").c_str());
        ao = loadTexture(("resources/textures/pbr/" + materialName + "/ao.png").c_str());
        height = loadTexture(("resources/textures/pbr/" + materialName + "/height.png").c_str());
    }

    void use() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ao);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, height);
    }

private:
    unsigned int albedo{};
    unsigned int normal{};
    unsigned int metallic{};
    unsigned int roughness{};
    unsigned int ao{};
    unsigned int height{};
};

#endif //ITU_GRAPHICS_PROGRAMMING_PBRMATERIAL_H
