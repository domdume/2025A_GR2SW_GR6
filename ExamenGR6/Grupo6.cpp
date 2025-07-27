#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <cmath>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, -7.5f, -32.0f)); // Posición dentro de la habitación
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// flashlight
bool flashlightOn = false;

// slenderman variables
glm::vec3 slendermanPosition = glm::vec3(0.0f, -7.5f, -30.0f); // Posicionado dentro de la habitación
float slendermanSpeed = 2.0f;
float slendermanDirection = 0.0f;
float slendermanMovementTimer = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 16 Task 3", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
   //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("shaders/shader.vs", "shaders/shader.fs");
    //Shader emissiveShader("shaders/luzemissive.vs", "shaders/luzemissive.fs");
    // load models
    // -----------
    Model ourModel("model/partyroom/partyroom.obj");
    Model slendermanModel("model/slenderman/slenderman.obj");


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    camera.MovementSpeed = 10; //Optional. Modify the speed of the camera

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        // Actualizar movimiento de Slenderman para perseguir al jugador
        slendermanMovementTimer += deltaTime;

        // Calcular la dirección hacia la cámara (jugador)
        glm::vec3 directionToPlayer = camera.Position - slendermanPosition;
        directionToPlayer.y = 0.0f; // Solo movimiento horizontal
        
        // Calcular la distancia al jugador
        float distanceToPlayer = glm::length(directionToPlayer);
        
        // Si está muy cerca, moverse más lento para crear tensión
        float currentSpeed = slendermanSpeed;
        if (distanceToPlayer < 5.0f) {
            currentSpeed = slendermanSpeed * 0.3f; // Más lento cuando está cerca
        } else if (distanceToPlayer > 20.0f) {
            currentSpeed = slendermanSpeed * 1.5f; // Más rápido cuando está lejos
        }
        
        // Normalizar la dirección y calcular el ángulo
        if (distanceToPlayer > 0.1f) { // Evitar división por cero
            directionToPlayer = glm::normalize(directionToPlayer);
            slendermanDirection = atan2(directionToPlayer.x, directionToPlayer.z);
            
            // Agregar un poco de movimiento errático ocasionalmente
            if (slendermanMovementTimer > 2.0f + (sin(currentFrame * 0.5f) * 1.0f)) {
                slendermanDirection += glm::radians((sin(currentFrame * 1.2f) * 30.0f)); // Movimiento errático sutil
                slendermanMovementTimer = 0.0f;
            }
        }

        // Calcular nueva posición de Slenderman
        float moveX = sin(slendermanDirection) * currentSpeed * deltaTime;
        float moveZ = cos(slendermanDirection) * currentSpeed * deltaTime;
        glm::vec3 newPosition = slendermanPosition;
        newPosition.x += moveX;
        newPosition.z += moveZ;

        // Mantener Slenderman dentro de los límites de la habitación
        float roomMinX = -15.0f;
        float roomMaxX = 15.0f;
        float roomMinZ = -45.0f;
        float roomMaxZ = -15.0f;
        
        if (newPosition.x >= roomMinX && newPosition.x <= roomMaxX &&
            newPosition.z >= roomMinZ && newPosition.z <= roomMaxZ) {
            slendermanPosition.x = newPosition.x;
            slendermanPosition.z = newPosition.z;
        }

        // Mantener Slenderman siempre a la altura correcta (completamente visible sobre el piso)
        slendermanPosition.y = -7.5f;

        camera.Position.y = -7.5f; // Mantener la cámara a una altura fija
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // Configura todos los uniforms
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setFloat("material.shininess", 16.0f);

        // Luz principal tenue para ambiente de discoteca
        ourShader.setVec3("light.position", 0.0f, 5.0f, -15.0f);
        ourShader.setVec3("light.ambient", 0.05f, 0.05f, 0.1f);  // Muy tenue y azulado
        ourShader.setVec3("light.diffuse", 0.4f, 0.2f, 0.6f);    // Púrpura para ambiente disco
        ourShader.setVec3("light.specular", 0.6f, 0.4f, 0.8f);
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.045f);
        ourShader.setFloat("light.quadratic", 0.0075f);

        // Configuración de la linterna (más brillante para contraste)
        ourShader.setBool("flashlightOn", flashlightOn);
        if (flashlightOn) {
            ourShader.setVec3("flashlight.position", camera.Position);
            ourShader.setVec3("flashlight.direction", camera.Front);
            ourShader.setFloat("flashlight.cutOff", glm::cos(glm::radians(15.0f)));
            ourShader.setFloat("flashlight.outerCutOff", glm::cos(glm::radians(25.0f)));
            ourShader.setVec3("flashlight.ambient", 0.0f, 0.0f, 0.0f);
            ourShader.setVec3("flashlight.diffuse", 1.0f, 1.0f, 0.9f);  // Luz cálida
            ourShader.setVec3("flashlight.specular", 1.0f, 1.0f, 1.0f);
            ourShader.setFloat("flashlight.constant", 1.0f);
            ourShader.setFloat("flashlight.linear", 0.022f);
            ourShader.setFloat("flashlight.quadratic", 0.0019f);
        }

        ourShader.setFloat("time", currentFrame);

        // Renderizar el escenario principal
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, -30.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // Renderizar Slenderman
        glm::mat4 slendermanModelMatrix = glm::mat4(1.0f);
        slendermanModelMatrix = glm::translate(slendermanModelMatrix, slendermanPosition);
        
        // Hacer que Slenderman siempre mire hacia el jugador
        glm::vec3 lookDirection = camera.Position - slendermanPosition;
        lookDirection.y = 0.0f; // Solo rotación horizontal
        if (glm::length(lookDirection) > 0.1f) {
            float lookAngle = atan2(lookDirection.x, lookDirection.z);
            slendermanModelMatrix = glm::rotate(slendermanModelMatrix, lookAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        
        slendermanModelMatrix = glm::scale(slendermanModelMatrix, glm::vec3(0.005f, 0.005f, 0.005f)); // Mucho más pequeño, tamaño humano
        ourShader.setMat4("model", slendermanModelMatrix);
        slendermanModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Alternar linterna con tecla F
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
        flashlightOn = !flashlightOn;
        fKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// glfw: whenever a mouse button is pressed/released, this callback is called
// --------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        flashlightOn = !flashlightOn;
    }
}