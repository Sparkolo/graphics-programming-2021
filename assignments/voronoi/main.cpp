#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// structure to hold the info necessary to render an object
struct SceneObject {
    unsigned int VAO;           // vertex array object handle
    unsigned int vertexCount;   // number of vertices in the object
    float r, g, b;              // for object color
    float x, y;                 // for position offset
};


void createArrayBuffer(const std::vector<float> &array, unsigned int &VBO);
void createElementsBuffer(const std::vector<unsigned int> &array, unsigned int &EBO);
// declaration of the function you will implement in voronoi 1.1
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY);
// mouse, keyboard and screen reshape glfw callbacks
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables we will use to store our objects, shaders, and active shaders
std::vector<SceneObject> sceneObjects;
std::vector<Shader> shaderPrograms;
Shader* activeShader;


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    // Random initialization
    srand(time(NULL));

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment - Voronoi Diagram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // setup input callbacks
    glfwSetMouseButtonCallback(window, button_input_callback); // NEW!
    glfwSetKeyCallback(window, key_input_callback); // NEW!

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NEW!
    // build and compile the shaders programs
    shaderPrograms.push_back(Shader("shaders/shaders.vert", "shaders/color.frag"));
    shaderPrograms.push_back(Shader("shaders/shaders.vert", "shaders/distance.frag"));
    shaderPrograms.push_back(Shader("shaders/shaders.vert", "shaders/distance_color.frag"));
    activeShader = &shaderPrograms[0];

    // NEW!
    // set up the z-buffer
    glDepthRange(1,-1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // notice that now we are clearing two buffers, the color and the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the cones
        glUseProgram(activeShader->ID);

        // TODO voronoi 1.3
        // Iterate through the scene object, for each object:
        // - bind the VAO; set the uniform variables; and draw.
        for(SceneObject obj : sceneObjects) {
            // bind vertex array object
            glBindVertexArray(obj.VAO);
            // Set the uniforms related to the current obj
            activeShader->setVec2("offset", obj.x, obj.y);
            activeShader->setVec3("color", obj.r, obj.g, obj.b);
            // draw geometry
            glDrawElements(GL_TRIANGLES, obj.vertexCount, GL_UNSIGNED_INT, 0);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// create a vertex buffer object (VBO) from an array of values, return VBO handle (set as reference)
// -------------------------------------------------------------------------------------------------
void createArrayBuffer(const std::vector<float> &array, unsigned int &VBO){
    // create the VBO on OpenGL and get a handle to it
    glGenBuffers(1, &VBO);
    // bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the content of the VBO (type, size, pointer to start, and how it is used)
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);
}

// create a element buffer object (EBO) from an array of indices, return EBO handle (set as reference)
// -------------------------------------------------------------------------------------------------
void createElementsBuffer(const std::vector<unsigned int> &array, unsigned int &EBO){
    // create the VBO on OpenGL and get a handle to it
    glGenBuffers(1, &EBO);
    // bind the VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // set the content of the EBO (type, size, pointer to start, and how it is used)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(GLuint), &array[0], GL_STATIC_DRAW);
}


// creates a cone triangle mesh, uploads it to openGL and returns the VAO associated to the mesh
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY){
    // Create an instance of a SceneObject,
    SceneObject sceneObject{};

    // Store offsetX, offsetY, r, g and b in the object.
    sceneObject.r = r;
    sceneObject.g = g;
    sceneObject.b = b;
    sceneObject.x = offsetX;
    sceneObject.y = offsetY;

    // Build the geometry into an std::vector<float>
    std::vector<float> positions;
    std::vector<unsigned int> indices;

    float radius = sqrt(8.0f);
    float maxError = 2.0f / SCR_WIDTH; // Error of 1 pixel
    float angle = 2 * acosf((radius - maxError) / radius);
    float triangleCount = 2 * M_PI / angle;

    // cone tip vertex position
    positions.push_back(0.0f);
    positions.push_back(0.0f);
    positions.push_back(1.0f);
    // all the other vertices in order with position followed by color
    for (int i=0; i < triangleCount; i++) {
        // positions
        positions.push_back(cos(angle*i) * radius);
        positions.push_back(sin(angle*i) * radius);
        positions.push_back(-1.0f);
    }

    for(int i=0; i<triangleCount; i++) {
        indices.push_back(0);
        indices.push_back(i+1);
        indices.push_back(i+2 > triangleCount ? 1 : i+2);
    }

    // Store the number of vertices in the mesh in the scene object.
    sceneObject.vertexCount = positions.size();

    // Declare and generate a VAO and VBO (and an EBO if you decide the work with indices).
    unsigned int VBO, EBO, VAO;
    createArrayBuffer(positions, VBO);
    createElementsBuffer(indices, EBO);

    // Bind and set the VAO and VBO (and optionally a EBO) in the correct order.
    // create a vertex array object (VAO) on OpenGL and save a handle to it
    glGenVertexArrays(1, &VAO);

    // bind vertex array object
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Set the position attribute pointers in the shaders.
    int posSize = 3;

    // set vertex shaders attribute "pos"
    int posAttributeLocation = glGetAttribLocation(activeShader->ID, "pos");

    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, posSize, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    // Store the VAO handle in the scene object.
    sceneObject.VAO = VAO;

    // 'return' the scene object for the cone instance you just created.
    return sceneObject;
}

// glfw: called whenever a mouse button is pressed
void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    // Test button press, see documentation at:
    //     https://www.glfw.org/docs/latest/input_guide.html#input_mouse_button
    // CODE HERE
    // If a left mouse button press was detected, call instantiateCone:
    // - Push the return value to the back of the global 'vector<SceneObject> sceneObjects'.
    // - The click position should be transformed from screen coordinates to normalized device coordinates,
    //   to obtain the offset values that describe the position of the object in the screen plane.
    // - A random value in the range [0, 1] should be used for the r, g and b variables.
    if (button == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float mouseXNDC, mouseYNDC;
        mouseXNDC = (float) mouseX / SCR_WIDTH * 2 -1;
        mouseYNDC = (float) -mouseY / SCR_HEIGHT * 2 + 1;

        float r,g,b;
        r = (rand() % 101) / 100.0f;
        g = (rand() % 101) / 100.0f;
        b = (rand() % 101) / 100.0f;

        sceneObjects.push_back(instantiateCone(r,g,b,mouseXNDC,mouseYNDC));
    }
}

// glfw: called whenever a keyboard key is pressed
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    // TODO voronoi 1.4

    // Set the activeShader variable by detecting when the keys 1, 2 and 3 were pressed;
    // see documentation at https://www.glfw.org/docs/latest/input_guide.html#input_keyboard
    // Key 1 sets the activeShader to &shaderPrograms[0];
    //   and so on.
    if(button == GLFW_KEY_1 && action == GLFW_PRESS)
        activeShader = &shaderPrograms[0];
    if(button == GLFW_KEY_2 && action == GLFW_PRESS)
        activeShader = &shaderPrograms[1];
    if(button == GLFW_KEY_3 && action == GLFW_PRESS)
        activeShader = &shaderPrograms[2];
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}