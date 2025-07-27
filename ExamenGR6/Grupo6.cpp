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
float detectionRange = 8.0f; // Distancia a la que Slenderman detecta la cámara
bool isFollowing = false;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _APPLE_
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
    Model skullModel("model/skull/skull1.obj");
    Model bloodModel("model/blood/blood.obj");


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

        // Actualizar movimiento de Slenderman
        slendermanMovementTimer += deltaTime;

        // Calcular distancia entre Slenderman y la cámara
        float distanceToCamera = glm::distance(slendermanPosition, camera.Position);
        
        // Determinar si Slenderman debe seguir a la cámara
        if (distanceToCamera <= detectionRange) {
            isFollowing = true;
        } else if (distanceToCamera > detectionRange * 1.5f) {
            isFollowing = false;
        }

        float moveX, moveZ;
        glm::vec3 newPosition = slendermanPosition;

        if (isFollowing) {
            // Seguir a la cámara
            glm::vec3 directionToCamera = glm::normalize(camera.Position - slendermanPosition);
            moveX = directionToCamera.x * slendermanSpeed * deltaTime * 1.5f; // Más rápido cuando sigue
            moveZ = directionToCamera.z * slendermanSpeed * deltaTime * 1.5f;
            
            // Actualizar la dirección hacia la cámara para orientar el modelo
            slendermanDirection = atan2(directionToCamera.x, directionToCamera.z);
        } else {
            // Movimiento aleatorio como antes
            // Cambiar dirección cada 3-5 segundos de forma aleatoria
            if (slendermanMovementTimer > 3.0f + (sin(currentFrame * 0.3f) * 2.0f)) {
                slendermanDirection += glm::radians(45.0f + (sin(currentFrame * 0.7f) * 90.0f));
                slendermanMovementTimer = 0.0f;
            }

            // Calcular nueva posición de Slenderman
            moveX = sin(slendermanDirection) * slendermanSpeed * deltaTime;
            moveZ = cos(slendermanDirection) * slendermanSpeed * deltaTime;
        }

        newPosition.x += moveX;
        newPosition.z += moveZ;

        // Mantener Slenderman dentro de los límites de la habitación
        // Límites basados en las dimensiones del party room
        float roomMinX = -15.0f;
        float roomMaxX = 15.0f;
        float roomMinZ = -45.0f;
        float roomMaxZ = -15.0f;

        if (newPosition.x >= roomMinX && newPosition.x <= roomMaxX &&
            newPosition.z >= roomMinZ && newPosition.z <= roomMaxZ) {
            slendermanPosition.x = newPosition.x;
            slendermanPosition.z = newPosition.z;
        }
        else {
            // Si se sale de los límites, cambiar dirección
            slendermanDirection += glm::radians(180.0f);
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

        // Renderizar el escenario principal (sin iluminación especial)
        ourShader.setVec3("light.position", 0.0f, -5.0f, -28.0f); // Luz tenue general
        ourShader.setVec3("light.ambient", 0.05f, 0.05f, 0.1f);
        ourShader.setVec3("light.diffuse", 0.2f, 0.1f, 0.3f);
        ourShader.setVec3("light.specular", 0.3f, 0.2f, 0.4f);
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, -30.0f));
        ourShader.setMat4("model", model);
        ourShader.setBool("hasEmissiveMap", false); // El escenario no brilla
        ourModel.Draw(ourShader);

        // Renderizar Slenderman (sin iluminación especial)
        glm::mat4 slendermanModelMatrix = glm::mat4(1.0f);
        slendermanModelMatrix = glm::translate(slendermanModelMatrix, slendermanPosition);
        slendermanModelMatrix = glm::rotate(slendermanModelMatrix, slendermanDirection, glm::vec3(0.0f, 1.0f, 0.0f));
        slendermanModelMatrix = glm::scale(slendermanModelMatrix, glm::vec3(0.005f, 0.005f, 0.005f)); // Mucho más pequeño, tamaño humano
        ourShader.setMat4("model", slendermanModelMatrix);
        ourShader.setBool("hasEmissiveMap", false); // Slenderman no brilla
        slendermanModel.Draw(ourShader);

        // Renderizar múltiples skulls con luces individuales desde su centro
        float flickerIntensity = 0.7f + 0.4f * sin(currentFrame * 8.0f) * cos(currentFrame * 12.0f);
        float warmFlicker = 0.8f + 0.3f * sin(currentFrame * 6.0f + 1.0f);

        // Skull 1 - posición inicial de la cámara con luz propia
        ourShader.setVec3("light.position", 0.0f, -11.0f, -32.0f); // Luz en el centro del skull
        ourShader.setVec3("light.ambient", 0.2f * flickerIntensity, 0.1f * flickerIntensity, 0.05f * flickerIntensity);
        ourShader.setVec3("light.diffuse", 1.5f * warmFlicker, 0.9f * warmFlicker, 0.4f * warmFlicker);
        ourShader.setVec3("light.specular", 1.2f * flickerIntensity, 1.0f * flickerIntensity, 0.5f * flickerIntensity);
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.35f);
        ourShader.setFloat("light.quadratic", 0.44f);
        
        glm::mat4 skullModelMatrix1 = glm::mat4(1.0f);
        skullModelMatrix1 = glm::translate(skullModelMatrix1, glm::vec3(0.0f, -10.0f, -32.0f)); // Pegado al piso
        skullModelMatrix1 = glm::rotate(skullModelMatrix1, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix1 = glm::scale(skullModelMatrix1, glm::vec3(0.6f, 0.6f, 0.6f)); 
        ourShader.setMat4("model", skullModelMatrix1);
        ourShader.setBool("hasEmissiveMap", true);
        skullModel.Draw(ourShader);

        // Skull 2 - esquina izquierda con luz propia
        float flicker2 = 0.8f + 0.3f * sin(currentFrame * 7.0f + 1.5f) * cos(currentFrame * 11.0f);
        ourShader.setVec3("light.position", -10.0f, -11.0f, -25.0f); // Luz en el centro del skull
        ourShader.setVec3("light.ambient", 0.2f * flicker2, 0.1f * flicker2, 0.05f * flicker2);
        ourShader.setVec3("light.diffuse", 1.5f * flicker2, 0.9f * flicker2, 0.4f * flicker2);
        ourShader.setVec3("light.specular", 1.2f * flicker2, 1.0f * flicker2, 0.5f * flicker2);
        
        glm::mat4 skullModelMatrix2 = glm::mat4(1.0f);
        skullModelMatrix2 = glm::translate(skullModelMatrix2, glm::vec3(-10.0f, -10.0f, -25.0f)); 
        skullModelMatrix2 = glm::rotate(skullModelMatrix2, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix2 = glm::scale(skullModelMatrix2, glm::vec3(0.5f, 0.5f, 0.5f)); 
        ourShader.setMat4("model", skullModelMatrix2);
        ourShader.setBool("hasEmissiveMap", true);
        skullModel.Draw(ourShader);

        // Skull 3 - esquina derecha con luz propia
        float flicker3 = 0.8f + 0.3f * sin(currentFrame * 9.0f + 3.0f) * cos(currentFrame * 13.0f);
        ourShader.setVec3("light.position", 8.0f, -11.0f, -28.0f); // Luz en el centro del skull
        ourShader.setVec3("light.ambient", 0.2f * flicker3, 0.1f * flicker3, 0.05f * flicker3);
        ourShader.setVec3("light.diffuse", 1.5f * flicker3, 0.9f * flicker3, 0.4f * flicker3);
        ourShader.setVec3("light.specular", 1.2f * flicker3, 1.0f * flicker3, 0.5f * flicker3);
        
        glm::mat4 skullModelMatrix3 = glm::mat4(1.0f);
        skullModelMatrix3 = glm::translate(skullModelMatrix3, glm::vec3(8.0f, -10.0f, -28.0f)); 
        skullModelMatrix3 = glm::rotate(skullModelMatrix3, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix3 = glm::scale(skullModelMatrix3, glm::vec3(0.7f, 0.7f, 0.7f)); 
        ourShader.setMat4("model", skullModelMatrix3);
        ourShader.setBool("hasEmissiveMap", true);
        skullModel.Draw(ourShader);

        // Skull 4 - centro-fondo con luz propia
        float flicker4 = 0.8f + 0.3f * sin(currentFrame * 6.5f + 4.5f) * cos(currentFrame * 10.0f);
        ourShader.setVec3("light.position", -2.0f, -11.0f, -20.0f); // Luz en el centro del skull
        ourShader.setVec3("light.ambient", 0.2f * flicker4, 0.1f * flicker4, 0.05f * flicker4);
        ourShader.setVec3("light.diffuse", 1.5f * flicker4, 0.9f * flicker4, 0.4f * flicker4);
        ourShader.setVec3("light.specular", 1.2f * flicker4, 1.0f * flicker4, 0.5f * flicker4);
        
        glm::mat4 skullModelMatrix4 = glm::mat4(1.0f);
        skullModelMatrix4 = glm::translate(skullModelMatrix4, glm::vec3(-2.0f, -10.0f, -20.0f)); 
        skullModelMatrix4 = glm::rotate(skullModelMatrix4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix4 = glm::scale(skullModelMatrix4, glm::vec3(0.4f, 0.4f, 0.4f)); 
        ourShader.setMat4("model", skullModelMatrix4);
        ourShader.setBool("hasEmissiveMap", true);
        skullModel.Draw(ourShader);

        // Skull 5 - rotando en tiempo real con luz propia
        float flicker5 = 0.8f + 0.3f * sin(currentFrame * 8.5f + 2.0f) * cos(currentFrame * 12.5f);
        ourShader.setVec3("light.position", 5.0f, -10.0f, -35.0f); // Luz en el centro del skull
        ourShader.setVec3("light.ambient", 0.2f * flicker5, 0.1f * flicker5, 0.05f * flicker5);
        ourShader.setVec3("light.diffuse", 1.5f * flicker5, 0.9f * flicker5, 0.4f * flicker5);
        ourShader.setVec3("light.specular", 1.2f * flicker5, 1.0f * flicker5, 0.5f * flicker5);
        
        glm::mat4 skullModelMatrix5 = glm::mat4(1.0f);
        skullModelMatrix5 = glm::translate(skullModelMatrix5, glm::vec3(5.0f, -10.0f, -35.0f)); 
        skullModelMatrix5 = glm::rotate(skullModelMatrix5, currentFrame * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación continua
        skullModelMatrix5 = glm::scale(skullModelMatrix5, glm::vec3(0.8f, 0.8f, 0.8f)); 
        ourShader.setMat4("model", skullModelMatrix5);
        ourShader.setBool("hasEmissiveMap", true);
        skullModel.Draw(ourShader);

        // Renderizar múltiples charcos de sangre por el escenario
        // Configurar luz ambiente tenue para la sangre
        ourShader.setVec3("light.position", 0.0f, -5.0f, -28.0f);
        ourShader.setVec3("light.ambient", 0.1f, 0.02f, 0.02f); // Luz rojiza tenue
        ourShader.setVec3("light.diffuse", 0.3f, 0.05f, 0.05f);
        ourShader.setVec3("light.specular", 0.2f, 0.02f, 0.02f);
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.linear", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);
        ourShader.setBool("hasEmissiveMap", false); // La sangre no brilla

        // Charco de sangre 1 - cerca del skull central
        glm::mat4 bloodMatrix1 = glm::mat4(1.0f);
        bloodMatrix1 = glm::translate(bloodMatrix1, glm::vec3(2.0f, -9.65f, -30.0f)); // Más subido del piso
        bloodMatrix1 = glm::rotate(bloodMatrix1, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix1 = glm::scale(bloodMatrix1, glm::vec3(0.4f, 0.1f, 0.4f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix1);
        bloodModel.Draw(ourShader);

        // Charco de sangre 2 - esquina izquierda
        glm::mat4 bloodMatrix2 = glm::mat4(1.0f);
        bloodMatrix2 = glm::translate(bloodMatrix2, glm::vec3(-8.0f, -9.65f, -23.0f));
        bloodMatrix2 = glm::rotate(bloodMatrix2, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix2 = glm::scale(bloodMatrix2, glm::vec3(0.3f, 0.1f, 0.5f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix2);
        bloodModel.Draw(ourShader);

        // Charco de sangre 3 - esquina derecha
        glm::mat4 bloodMatrix3 = glm::mat4(1.0f);
        bloodMatrix3 = glm::translate(bloodMatrix3, glm::vec3(6.0f, -9.65f, -26.0f));
        bloodMatrix3 = glm::rotate(bloodMatrix3, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix3 = glm::scale(bloodMatrix3, glm::vec3(0.4f, 0.1f, 0.3f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix3);
        bloodModel.Draw(ourShader);

        // Charco de sangre 4 - centro-fondo
        glm::mat4 bloodMatrix4 = glm::mat4(1.0f);
        bloodMatrix4 = glm::translate(bloodMatrix4, glm::vec3(-4.0f, -9.65f, -18.0f));
        bloodMatrix4 = glm::rotate(bloodMatrix4, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix4 = glm::scale(bloodMatrix4, glm::vec3(0.5f, 0.1f, 0.3f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix4);
        bloodModel.Draw(ourShader);

        // Charco de sangre 5 - área posterior
        glm::mat4 bloodMatrix5 = glm::mat4(1.0f);
        bloodMatrix5 = glm::translate(bloodMatrix5, glm::vec3(3.0f, -9.65f, -37.0f));
        bloodMatrix5 = glm::rotate(bloodMatrix5, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix5 = glm::scale(bloodMatrix5, glm::vec3(0.3f, 0.1f, 0.2f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix5);
        bloodModel.Draw(ourShader);

        // Charco de sangre 6 - área central-lateral
        glm::mat4 bloodMatrix6 = glm::mat4(1.0f);
        bloodMatrix6 = glm::translate(bloodMatrix6, glm::vec3(-1.0f, -9.65f, -33.0f));
        bloodMatrix6 = glm::rotate(bloodMatrix6, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix6 = glm::scale(bloodMatrix6, glm::vec3(0.4f, 0.1f, 0.1f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix6);
        bloodModel.Draw(ourShader);

        // Charco de sangre 7 - área frontal
        glm::mat4 bloodMatrix7 = glm::mat4(1.0f);
        bloodMatrix7 = glm::translate(bloodMatrix7, glm::vec3(7.0f, -9.85f, -22.0f));
        bloodMatrix7 = glm::rotate(bloodMatrix7, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix7 = glm::scale(bloodMatrix7, glm::vec3(0.3f, 0.1f, 0.1f)); // Mucho más pequeño
        ourShader.setMat4("model", bloodMatrix7);
        bloodModel.Draw(ourShader);

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