#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>

#include "shader.h"
#include "camera.h"
#include "sphere.h"
#include "cube.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

/*
 * @DECLARATIONS_AND_PARAMETERS
 */

// custom functions declarations
void drawScene();
void drawGui();
void propagateMaterial(unsigned int);
void propagateAlbedo();
void propagateMetallic(unsigned int);
void propagateRoughness(unsigned int);

// glfw and input functions
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings
const unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
int glfw_width, glfw_height;

// variables used for rendering
Shader* PBRshader;
Shader* debugLightShader;
PBRCube* lightCube;
std::vector<PBRMaterial*> materials;
std::vector<PBRSphere> spheres;
std::vector<PBRCube> cubes;
std::vector<glm::vec3> lightPositions;
std::vector<glm::vec3> lightColors;

// global variables used for movements control
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // used to stop the camera movement when the GUI is open
Camera camera(glm::vec3(0.0f, 0.0f, 25.0f));

// parameters that can be set in the GUI
struct Config {
    // constants & helpers
    const int NR_ROWS = 5;
    const int NR_COLS = 5;
    const float SPACING = 3.0f;
    const unsigned int NR_LIGHTS = 15;
    const float MAX_LIGHT_DIST = 15.f;
    float rotation = 0.0f;

    // Basic settings
    bool drawCubes = false;
    bool useMaterials = false;
    // Uniforms
    glm::vec3 albedoBL = glm::vec3(1.0f, 0.0f, 0.0f); // albedo of the bottom left object
    glm::vec3 albedoBR = glm::vec3(0.0f, 1.0f, 0.0f); // albedo of the bottom right object
    glm::vec3 albedoTL = glm::vec3(0.0f, 0.0f, 1.0f); // albedo of the top left object
    glm::vec3 albedoTR = glm::vec3(0.0f, 0.0f, 1.0f); // albedo of the top right object
    std::vector<float> rowsMetallic;
    std::vector<float> colsRoughness;
    // Materials
    std::vector<const char*> materials = {"animal-fur","black-white-tile","gold-scuffed","marble","grass",
                                          "bricks","damp-rocky-ground","wood","yoga-mat","iron-scuffed",
                                          "old-padded-leather", "rusted-iron", "dented-metal", "layered-fungus"};
    std::vector<unsigned int> activeMaterial;
    float heightScale = 0.1f;

    // PBR Lightining variables
    bool useBeckmannDist = false;
    bool useGGXDistApprox = false;
    bool useGGXCorrelated = true;
    bool useGGXCorrelateApprox = false;
    bool useFresnelRoughness = false;
    bool useDisneyDiffuse = false;
    bool useOrenNayarDiffuse = false;
    bool burleyAnisotropic = false;
    bool kullaAnisotropicRoughness = false;
    float anisotropy = 0.f;
    bool HDRtoneMapping = true;
    bool hdrACES = false;

    // Lights
    bool drawLights = true;
    float rotationSpeed = 5.f;
    float lightIntensity = 200.f;
} config;




/**
 * @MAIN_LOOP
 */

int main()
{
    // GLFW: Initialize and Configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: Window Creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Physically Based Rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
	glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    /**
     * GLAD: load all OpenGL function pointers
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // IMGUI init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders and materials
    PBRshader = new Shader("shaders/PBRshader.vert", "shaders/PBRshader.frag");
    debugLightShader = new Shader("shaders/debugLightShader.vert", "shaders/debugLightShader.frag");

    PBRshader->use();
    PBRshader->setInt("albedoMap", 0);
    PBRshader->setInt("normalMap", 1);
    PBRshader->setInt("metallicMap", 2);
    PBRshader->setInt("roughnessMap", 3);
    PBRshader->setInt("aoMap", 4);
    PBRshader->setInt("heightMap", 5);

    for(int i=0; i<config.materials.size(); i++)
        materials.push_back(new PBRMaterial(config.materials[i]));

    // lighting info
    srand(0);
    for (unsigned int i = 0; i < config.NR_LIGHTS; i++)
    {
        bool valid = false;
        glm::vec3 pos, col;
        while(!valid){ // so the lights are in a circular arrangement (instead of squared)
            pos.x = ((rand() % 100) / 100.f) * config.MAX_LIGHT_DIST * 2 - config.MAX_LIGHT_DIST;
            pos.z = ((rand() % 100) / 100.f) * config.MAX_LIGHT_DIST * 2 - config.MAX_LIGHT_DIST;
            pos.y = ((rand() % 100) / 100.f) * config.MAX_LIGHT_DIST * 2 - config.MAX_LIGHT_DIST;
            if (glm::dot(pos, pos) < config.MAX_LIGHT_DIST * config.MAX_LIGHT_DIST * 2)
                valid = true;
        }
        lightPositions.push_back(pos);
        // calculate random color between 0.2 and 1.0
        col.r = ((rand() % 80) / 100.f) + .2f;
        col.g = ((rand() % 80) / 100.f) + .2f;
        col.b = ((rand() % 80) / 100.f) + .2f;
        lightColors.push_back(col);
    }
    lightCube = new PBRCube();

    /**
     * Create the spheres and cubes at startup
     */
    float metallic, roughness;
    bool isFirstRow = true;
    for (int row = 0; row < config.NR_ROWS; ++row)
    {
        metallic = (float)row / (float)config.NR_ROWS;
        config.rowsMetallic.push_back(metallic);
        for (int col = 0; col < config.NR_COLS; ++col)
        {
            roughness = (float)col / (float)config.NR_COLS;
            if(isFirstRow) {
                config.colsRoughness.push_back(roughness);
                config.activeMaterial.push_back(col % config.materials.size());
            }

            glm::vec3 offset = glm::vec3((col - (config.NR_COLS / 2)) * config.SPACING, (row - (config.NR_ROWS / 2)) * config.SPACING, 0.0f);
            spheres.emplace_back(offset, metallic, roughness);
            cubes.emplace_back(offset, metallic, roughness);
        }
        isFirstRow = false;
    }
    propagateAlbedo();
    for(int i=0; i<config.NR_COLS; i++) propagateMaterial(i);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        config.rotation = glm::mod(config.rotation + deltaTime * config.rotationSpeed, 360.0f);

        processInput(window);

        // clear buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawScene();

        // Render GUI (if paused)
        if (isPaused) {
			drawGui();
		}

        // show the frame buffer
        glfwSwapBuffers(window);
        glfwPollEvents();

        glfwSetWindowTitle(window, ("PBR Rendering [FPS: " + std::to_string(int(1.0f/deltaTime + .5f)) + "]").c_str());
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete lightCube;
    delete(PBRshader);
    delete(debugLightShader);
    for(int i=0; i<materials.size(); ++i) {
        delete(materials[i]);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}





/**
 * @CUSTOM_FUNCTIONS
 */

void drawScene() {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 lightRotation(1.0f);
    lightRotation = glm::mat3(glm::rotate(lightRotation, glm::radians(config.rotation), glm::vec3(1.0f , 1.0f, 0.0f)));
    glm::mat3 lightRotationM3 = glm::mat3(lightRotation);

    // setup the correct uniforms in the shaders
    PBRshader->use();
    PBRshader->setMat4("projection", projection);
    PBRshader->setMat4("view", view);
    PBRshader->setVec3("viewPosition", camera.Position);

    for(int i = 0; i<config.NR_LIGHTS; i++) {
        PBRshader->setVec3("lightPositions[" + std::to_string(i) + "]", lightRotationM3 * lightPositions[i]);
        PBRshader->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i] * config.lightIntensity);
    }

    PBRshader->setBool("useMaterials", config.useMaterials);

    PBRshader->setBool("useBeckmannDist", config.useBeckmannDist);
    PBRshader->setBool("useGGXDistApprox", config.useGGXDistApprox);
    PBRshader->setBool("useGGXCorrelated", config.useGGXCorrelated);
    PBRshader->setBool("useGGXCorrelateApprox", config.useGGXCorrelateApprox);
    PBRshader->setBool("useFresnelRoughness", config.useFresnelRoughness);
    PBRshader->setBool("useDisneyDiffuse", config.useDisneyDiffuse);
    PBRshader->setBool("useOrenNayarDiffuse", config.useOrenNayarDiffuse);

    PBRshader->setBool("burleyAnisotropic", config.burleyAnisotropic);
    PBRshader->setBool("kullaAnisotropicRoughness", config.kullaAnisotropicRoughness);
    PBRshader->setFloat("anisotropy", config.anisotropy);

    PBRshader->setBool("HDRtoneMapping", config.HDRtoneMapping);
    PBRshader->setBool("hdrACES", config.hdrACES);

    unsigned int startIndex = config.useMaterials ? ((config.NR_ROWS / 2) * config.NR_COLS) : 0;
    unsigned int endIndex = startIndex + (config.useMaterials ? config.NR_COLS : (config.NR_ROWS * config.NR_COLS));
    PBRshader->setFloat("heightScale", config.drawCubes ? config.heightScale : 0.0f);
    for(int i=startIndex; i<endIndex; i++) {
        if(config.drawCubes)    cubes[i].Draw(PBRshader, config.useMaterials);
        else                    spheres[i].Draw(PBRshader, config.useMaterials);
    }

    // Draw a debug cube to visualize lights
    if(config.drawLights) {
        debugLightShader->use();
        debugLightShader->setMat4("projection", projection);
        debugLightShader->setMat4("view", view);

        for(int i=0; i<config.NR_LIGHTS; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.05f));
            debugLightShader->setMat4("model", lightRotation * model);
            debugLightShader->setVec3("lightColor", lightColors[i] * (config.HDRtoneMapping ? 1.f : config.lightIntensity));
            lightCube->DebugDraw();
        }
    }
}

// draw imGUI
void drawGui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("PBR Settings");

        ImGui::Text("Draw: "); ImGui::SameLine();
        if(ImGui::RadioButton("Cubes", config.drawCubes)) {config.drawCubes = true;} ImGui::SameLine();
        if(ImGui::RadioButton("Spheres", !config.drawCubes)) {config.drawCubes = false;}
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("PBR Parameters: ");
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Draw with: "); ImGui::SameLine();
        if(ImGui::RadioButton("Uniforms##MaterialsOff", !config.useMaterials)) {config.useMaterials = false;} ImGui::SameLine();
        if(ImGui::RadioButton("Materials##MaterialstOn", config.useMaterials)) {config.useMaterials = true;}
        ImGui::Separator();
        if(config.useMaterials) {
            if(config.drawCubes)
                ImGui::SliderFloat("Heightmap Scale ", (float*)&config.heightScale, 0.0f, 0.2f);
            if(ImGui::CollapsingHeader("Materials")) {
                for(int i=0; i<config.NR_COLS; i++) {
                    if(ImGui::ListBox(("Obj["+std::to_string(i)+"]").c_str(), (int*)&config.activeMaterial[i], config.materials.data(), config.materials.size())) {
                        propagateMaterial(i);
                    };
                }
            }
        }
        else {
            if(ImGui::CollapsingHeader("Uniforms")) {
                ImGui::Text("Albedo colors: ");
                if(ImGui::ColorEdit3("Albedo[BottomLeft]", (float*)&config.albedoBL)) {propagateAlbedo();};
                if(ImGui::ColorEdit3("Albedo[BottomRight]", (float*)&config.albedoBR)) {propagateAlbedo();};
                if(ImGui::ColorEdit3("Albedo[TopRight]", (float*)&config.albedoTR)) {propagateAlbedo();};
                if(ImGui::ColorEdit3("Albedo[TopLeft]", (float*)&config.albedoTL)) {propagateAlbedo();};
                ImGui::Separator();
                ImGui::Text("Metallic (bottom to top): ");
                for(unsigned int i=0; i<config.NR_ROWS; i++) {
                    if(ImGui::SliderFloat(("Row[" + std::to_string(i) + "]").c_str(), (float*)&config.rowsMetallic[i], 0.f, 1.f)){propagateMetallic(i);};
                }
                ImGui::Separator();
                ImGui::Text("Roughness (left to right): ");
                for(unsigned int i=0; i<config.NR_COLS; i++) {
                    if(ImGui::SliderFloat(("Col[" + std::to_string(i) + "]").c_str(), (float*)&config.colsRoughness[i], 0.f, 1.f)) { propagateRoughness(i);};
                }
            }
        }
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("BRDF Parameters");
        ImGui::Separator();
        ImGui::Text("Anisotropic BRDF "); ImGui::SameLine();
        if(ImGui::RadioButton("ON##Anison", config.burleyAnisotropic)) {config.burleyAnisotropic = true;} ImGui::SameLine();
        if(ImGui::RadioButton("OFF##Anisoff", !config.burleyAnisotropic)) {config.burleyAnisotropic = false;}
        if(config.burleyAnisotropic) {
            ImGui::Text("Calculate roughness with: "); ImGui::SameLine();
            if(ImGui::RadioButton("Kulla", config.kullaAnisotropicRoughness)) {config.kullaAnisotropicRoughness = true;} ImGui::SameLine();
            if(ImGui::RadioButton("Burley", !config.kullaAnisotropicRoughness)) {config.kullaAnisotropicRoughness = false;}
            ImGui::SliderFloat("Anisotropy ", (float*)&config.anisotropy, -1.f, 1.f);
        }
        else {
            ImGui::Text("Distribution "); ImGui::SameLine();
            if(ImGui::RadioButton("Beckmann", config.useBeckmannDist)) {config.useBeckmannDist = true; config.useGGXDistApprox = false;} ImGui::SameLine();
            if(ImGui::RadioButton("GGX", (!config.useBeckmannDist) && !config.useGGXDistApprox)) {config.useBeckmannDist = false; config.useGGXDistApprox = false;} ImGui::SameLine();
            if(ImGui::RadioButton("GGX16pf", (!config.useBeckmannDist) && config.useGGXDistApprox)) {config.useBeckmannDist = false; config.useGGXDistApprox = true;}
            ImGui::Separator();
            ImGui::Text("Visibility "); ImGui::SameLine();
            if(ImGui::RadioButton("SchlickGGX", !config.useGGXCorrelated)) {config.useGGXCorrelated = false; config.useGGXCorrelateApprox = false;} ImGui::SameLine();
            if(ImGui::RadioButton("SmithGGXCorrelated", config.useGGXCorrelated && !config.useGGXCorrelateApprox)) {config.useGGXCorrelated = true; config.useGGXCorrelateApprox = false;}
            if(ImGui::RadioButton("SmithGGXCorrelated-Fast", config.useGGXCorrelated && config.useGGXCorrelateApprox)) {config.useGGXCorrelated = true; config.useGGXCorrelateApprox = true;}
        }
        ImGui::Separator();
        ImGui::Text("Fresnel "); ImGui::SameLine();
        if(ImGui::RadioButton("Base", !config.useFresnelRoughness)) {config.useFresnelRoughness = false;} ImGui::SameLine();
        if(ImGui::RadioButton("With Roughness", config.useFresnelRoughness)) {config.useFresnelRoughness = true;}
        ImGui::Separator();
        ImGui::Text("Diffuse "); ImGui::SameLine();
        if(ImGui::RadioButton("Lambertian", !config.useDisneyDiffuse && !config.useOrenNayarDiffuse)) {config.useDisneyDiffuse = false; config.useOrenNayarDiffuse = false;} ImGui::SameLine();
        if(ImGui::RadioButton("Oren-Nayar", config.useOrenNayarDiffuse)) {config.useDisneyDiffuse = false; config.useOrenNayarDiffuse = true;} ImGui::SameLine();
        if(ImGui::RadioButton("Disney(Burley)", config.useDisneyDiffuse)) {config.useDisneyDiffuse = true; config.useOrenNayarDiffuse = false;}
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("HDR Tonemapping "); ImGui::SameLine();
        if(ImGui::RadioButton("Reinhard", config.HDRtoneMapping && !config.hdrACES)) {config.HDRtoneMapping = true; config.hdrACES = false;} ImGui::SameLine();
        if(ImGui::RadioButton("ACES", config.HDRtoneMapping && config.hdrACES)) {config.HDRtoneMapping = true; config.hdrACES = true;} ImGui::SameLine();
        if(ImGui::RadioButton("OFF##HDRoff", !config.HDRtoneMapping)) {config.HDRtoneMapping = false;}
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Light Sources: ");
        ImGui::Text("Debug Draw "); ImGui::SameLine();
        if(ImGui::RadioButton("ON##DrawLightOn", config.drawLights)) {config.drawLights = true;} ImGui::SameLine();
        if(ImGui::RadioButton("OFF##DrawLightOff", !config.drawLights)) {config.drawLights = false;}
        ImGui::SliderFloat("Rotation speed", (float*)&config.rotationSpeed, 0.0f, 50.f);
        ImGui::SliderFloat("Light Intensity", (float*)&config.lightIntensity, 0.5f, 300.f);
        ImGui::Separator();
        if(ImGui::CollapsingHeader("Lights position + color")) {
            for(int i=0; i<config.NR_LIGHTS; i++) {
                ImGui::Text("Light %d", i);
                ImGui::SliderFloat3(("LightPosition[" + std::to_string(i) + "]").c_str(), (float*)&lightPositions[i], -config.MAX_LIGHT_DIST, config.MAX_LIGHT_DIST);
                ImGui::ColorEdit3(("LightColor[" + std::to_string(i) + "]").c_str(), (float*)&lightColors[i]);
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}





/**
 * @HELPER_FUNCTIONS
 */

void propagateMaterial(unsigned int col) {
    for(int r=0; r<config.NR_ROWS; r++) {
        cubes[r*config.NR_COLS + col].SetMaterial(materials[config.activeMaterial[col]]);
        spheres[r*config.NR_COLS + col].SetMaterial(materials[config.activeMaterial[col]]);
    }
}

void propagateAlbedo() {
    float rowPerc, colPerc;
    for(unsigned int r=0; r<config.NR_ROWS; r++) {
        rowPerc = (float)r / (float)config.NR_ROWS;
        for(unsigned int c=0; c<config.NR_COLS; c++) {
            colPerc = (float)c / (float)config.NR_COLS;
            glm::vec3 color = glm::normalize(
                            (1.0f - rowPerc)* (1.0f - colPerc) * config.albedoBL +
                            (1.0f - rowPerc) * colPerc * config.albedoBR +
                            rowPerc * (1.0f - colPerc) * config.albedoTL +
                            rowPerc * colPerc * config.albedoTR
                    );
            cubes[r*config.NR_COLS + c].SetAlbedo(color);
            spheres[r*config.NR_COLS + c].SetAlbedo(color);
        }
    }
}

void propagateMetallic(unsigned int row) {
    for(int c=0; c<config.NR_COLS; c++) {
        cubes[row*config.NR_COLS + c].SetMetallic(config.rowsMetallic[row]);
        spheres[row*config.NR_COLS + c].SetMetallic(config.rowsMetallic[row]);
    }
}

void propagateRoughness(unsigned int col) {
    for(int r=0; r<config.NR_ROWS; r++) {
        cubes[r*config.NR_COLS + col].SetRoughness(config.colsRoughness[col]);
        spheres[r*config.NR_COLS + col].SetRoughness(config.colsRoughness[col]);
    }
}

// Process key inputs
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

	if (isPaused)
		return;

	// movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Update the cursor position for camera movements
void cursor_input_callback(GLFWwindow* window, double posX, double posY){
	// camera rotation
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;

	if (isPaused)
		return;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW: whenever a key is pressed, this callback is called
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods){
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(!isPaused)
        camera.ProcessMouseScroll(yoffset);
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}