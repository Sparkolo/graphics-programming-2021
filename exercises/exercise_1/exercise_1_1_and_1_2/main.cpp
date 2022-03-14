#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char* vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "\n"
                                   "void main()\n"
                                   "{\n"
                                   "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                   "}\0";

// Callback to adjust the viewport if the window gets resized
void framebuffer_size_callback(GLFWwindow* w, int width, int height) {
    glViewport(0, 0, width, height);
}

// Function to handle key inputs
void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(){
    srand(time(NULL));

    // Initialize GLFW for OpenGL 3.3 in Core Profile
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Initialize the GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Exercise 1.1", NULL, NULL);
    if(window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) { // get the OpenGL functions pointer address through the GLFW method
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport dimensions and a callback to adjust it in case the window gets resized
    glViewport(0, 0, 800, 600); // lower-left corner in 0,0 - window dimensions 800x600 px
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Variables for checking errors while compiling shaders
    int success;
    char infoLog[512];

    /** VERTEX SHADER **/
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for errors in the vertex shaders compiling
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    /** FRAGMENT SHADER **/
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for errors in the fragment shaders compiling
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    /** SHADER PROGRAM **/
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    // Link the shaders to the program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for errors in linking the shaders to the program
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // Delete shaders since they already are linked in the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /** VERTICES AND INDICES **/
    // Define vertices of our triangle(s)
    float vertices[] = {
             0.5f,  0.5f, 0.0f,  // top right
             0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left
    };
    // Define the indices of the vertices for the EBO
    unsigned int indices[] = {
            0, 1, 3,  // first triangle
            1, 2, 3   // second triangle
    };


    /** VAO, VBO & EBO **/
    // Define the VertexArrayObject and bind it, to store vertices and EBO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Define the VertexBufferObject and copy the previously defined vertices in there
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Define the ElementBufferObject
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // This also binds it to the current VAO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Link the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind the VAO
    glBindVertexArray(0);

    // Useless stuff to randomize bg color
    unsigned int colorChangerCycles = 0;
    float curColor[] = {(rand() % 101) / 100.0f, (rand() % 101) / 100.0f, (rand() % 101) / 100.0f};

    /** RENDER LOOP **/
    while(!glfwWindowShouldClose(window)) {
        // Useless stuff to randomize bg color
        if(colorChangerCycles > 50) {
            curColor[0] = (rand() % 101) / 100.0f;
            curColor[1] = (rand() % 101) / 100.0f;
            curColor[2] = (rand() % 101) / 100.0f;
            colorChangerCycles = 0;
        }
        colorChangerCycles++;

        // Input
        processInput(window);

        /**
         * START OF RENDERING COMMANDS
         **/

        glClearColor(curColor[0], curColor[1], curColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Wireframe Mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Put GL_FILL in the second field to switch back to normal

        // Render the vertices with the shaders program
        glUseProgram(shaderProgram);
        /** VAO ONLY RENDERING **/
        //glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        /** EBO ONLY RENDERING **/
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        /** VAO + EBO RENDERING **/
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        /**
         * END OF RENDERING COMMANDS
         **/

        // Check and call events and swap the buffers
        glfwSwapBuffers(window); // swap the color buffer for each pixel in the window and render it
        glfwPollEvents(); // check if there are any events to be called
    }

    // Clear all GLFW resources when window closes
    glfwTerminate();
	return 0;
}