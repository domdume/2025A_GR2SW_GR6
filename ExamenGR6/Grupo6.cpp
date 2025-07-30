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
#include <vector>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
bool isPlayerInAllowedZone(glm::vec3 position, const std::vector<glm::vec4>& zones);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, -7.5f, -32.0f)); // Posición dentro de la habitación
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
std::vector<glm::vec4> walkableZones = {
    // Zona 1: La discoteca principal (un rectángulo grande)
    glm::vec4(-12.05f, 3.8f, -59.07f, -19.08f),  // minX, maxX, minZ, maxZ
    // Zona 2: Primer segmento del pasillo
    glm::vec4(3.77f, 4.53f, -50.08f, -48.96f),
    glm::vec4(4.52f, 12.25f, -51.10f, -46.88f),
    // Zona 3: Segundo segmento del pasillo (ejemplo si dobla a la izquierda)
    glm::vec4(9.06f, 11.70f, -47.15f, -43.56f),
    glm::vec4(11.52f, 15.73f, -46.01f, -43.4f),
    glm::vec4(13.14f, 15.73f, -43.4f, -41.90f),
    glm::vec4(5.18f, 15.74f, -43.2f, -34.72f),
	// Zona 4: Tercer segmento del pasillo (ejemplo si dobla a la derecha)
    glm::vec4(9.35f, 12.14f, -54.13f, -51.47f),
    glm::vec4(9.43f, 11.89f, -51.92f, -50.70f),
    glm::vec4(11.84f, 15.92f, -54.13f, -51.47f),
    glm::vec4(13.48f, 15.94f, -55.23f, -53.70f),
    glm::vec4(5.05f, 15.81f, -60.53f, -55.18f),
    glm::vec4(5.05f, 7.26f, -55.53f, -55.83f)
    
};

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
bool slendermanIsIlluminated = false; // Estado de iluminación de Slenderman

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
    Shader slendermanShader("shaders/slenderman.vs", "shaders/slenderman.fs");
    //Shader emissiveShader("shaders/luzemissive.vs", "shaders/luzemissive.fs");
    // load models
    // -----------
    Model ourModel("model/partyroom/partyroom.obj");
    Model slendermanModel("model/slenderman/slenderman.obj");
    Model skullModel("model/skull/skull.obj");
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

        // Actualizar movimiento de Slenderman para perseguir al jugador
        slendermanMovementTimer += deltaTime;

        // Calcular si Slenderman está siendo iluminado por la linternas
        slendermanIsIlluminated = false;
        if (flashlightOn) {
            // Calcular vector de la cámara hacia Slenderman
            glm::vec3 directionToSlenderman = glm::normalize(slendermanPosition - camera.Position);

            // Calcular el ángulo entre la dirección de la cámara y la dirección hacia Slenderman
            float dotProduct = glm::dot(camera.Front, directionToSlenderman);
            float angleCos = dotProduct;

            // Verificar si Slenderman está dentro del cono de la linterna
            float flashlightAngle = glm::cos(glm::radians(25.0f)); // Ángulo exterior de la linterna
            float distanceToSlenderman = glm::length(slendermanPosition - camera.Position);

            // Slenderman está iluminado si está dentro del cono y a una distancia razonable
            if (angleCos > flashlightAngle && distanceToSlenderman < 50.0f) {
                slendermanIsIlluminated = true;
            }
        }

        // Solo perseguir si NO está siendo iluminado
        if (!slendermanIsIlluminated) {
            // Calcular la dirección hacia la cámara (jugador)
            glm::vec3 directionToPlayer = camera.Position - slendermanPosition;
            directionToPlayer.y = 0.0f; // Solo movimiento horizontal

            // Calcular la distancia al jugador
            float distanceToPlayer = glm::length(directionToPlayer);

            // Si está muy cerca, moverse más lento para crear tensión
            float currentSpeed = slendermanSpeed;
            if (distanceToPlayer < 5.0f) {
                currentSpeed = slendermanSpeed * 0.3f; // Más lento cuando está cerca
            }
            else if (distanceToPlayer > 20.0f) {
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
        }
        // Si está iluminado, Slenderman se queda inmóvil (no actualizar posición)

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

        // Renderizar Slenderman con su shader específico
        slendermanShader.use();

        // Configurar los uniforms para el shader de Slenderman
        slendermanShader.setMat4("projection", projection);
        slendermanShader.setMat4("view", view);
        slendermanShader.setVec3("viewPos", camera.Position);
        slendermanShader.setVec3("lightPos", camera.Position); // La luz sigue al jugador para efectos dramáticos
        slendermanShader.setFloat("time", currentFrame);
        slendermanShader.setBool("isIlluminated", slendermanIsIlluminated); // Estado de iluminación

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
        slendermanShader.setMat4("model", slendermanModelMatrix);
        slendermanModel.Draw(slendermanShader);

        // Volver a usar el shader principal para skulls y blood
        ourShader.use();

        // Renderizar skull
        float flickerIntensity = 0.7f + 0.4f * sin(currentFrame * 8.0f) * cos(currentFrame * 12.0f);
        float warmFlicker = 0.8f + 0.3f * sin(currentFrame * 6.0f + 1.0f);

        // Skull 1 - centro de la habitación principal
        glm::mat4 skullModelMatrix1 = glm::mat4(1.0f);
        skullModelMatrix1 = glm::translate(skullModelMatrix1, glm::vec3(-4.0f, -9.3f, -39.0f)); // Centro de la habitación
        skullModelMatrix1 = glm::rotate(skullModelMatrix1, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix1 = glm::scale(skullModelMatrix1, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix1);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 2 - esquina izquierda de la habitación
        glm::mat4 skullModelMatrix2 = glm::mat4(1.0f);
        skullModelMatrix2 = glm::translate(skullModelMatrix2, glm::vec3(-10.0f, -9.3f, -45.0f)); // Esquina izquierda
        skullModelMatrix2 = glm::rotate(skullModelMatrix2, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix2 = glm::scale(skullModelMatrix2, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix2);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 3 - esquina derecha de la habitación
        glm::mat4 skullModelMatrix3 = glm::mat4(1.0f);
        skullModelMatrix3 = glm::translate(skullModelMatrix3, glm::vec3(2.0f, -9.3f, -50.0f)); // Esquina derecha
        skullModelMatrix3 = glm::rotate(skullModelMatrix3, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix3 = glm::scale(skullModelMatrix3, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix3);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 4 - zona central-frontal de la habitación
        glm::mat4 skullModelMatrix4 = glm::mat4(1.0f);
        skullModelMatrix4 = glm::translate(skullModelMatrix4, glm::vec3(-6.0f, -9.3f, -25.0f)); // Zona frontal
        skullModelMatrix4 = glm::rotate(skullModelMatrix4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix4 = glm::scale(skullModelMatrix4, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix4);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 5 - zona posterior de la habitación
        glm::mat4 skullModelMatrix5 = glm::mat4(1.0f);
        skullModelMatrix5 = glm::translate(skullModelMatrix5, glm::vec3(1.0f, -9.3f, -55.0f)); // Zona posterior
        skullModelMatrix5 = glm::rotate(skullModelMatrix5, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix5 = glm::scale(skullModelMatrix5, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix5);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 6 - zona lateral izquierda
        glm::mat4 skullModelMatrix6 = glm::mat4(1.0f);
        skullModelMatrix6 = glm::translate(skullModelMatrix6, glm::vec3(-8.0f, -9.3f, -33.0f)); // Lateral izquierda
        skullModelMatrix6 = glm::rotate(skullModelMatrix6, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix6 = glm::scale(skullModelMatrix6, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix6);
        ourShader.setBool("hasEmissiveMap", false);
        skullModel.Draw(ourShader);

        // Skull 7 - zona lateral derecha
        glm::mat4 skullModelMatrix7 = glm::mat4(1.0f);
        skullModelMatrix7 = glm::translate(skullModelMatrix7, glm::vec3(0.0f, -9.3f, -30.0f)); // Lateral derecha
        skullModelMatrix7 = glm::rotate(skullModelMatrix7, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        skullModelMatrix7 = glm::scale(skullModelMatrix7, glm::vec3(1.8f, 1.8f, 1.8f)); 
        ourShader.setMat4("model", skullModelMatrix7);
        ourShader.setBool("hasEmissiveMap", false);
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
        bloodMatrix1 = glm::translate(bloodMatrix1, glm::vec3(-2.0f, -9.2f, -40.0f)); // Cerca del skull central
        bloodMatrix1 = glm::rotate(bloodMatrix1, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix1 = glm::scale(bloodMatrix1, glm::vec3(0.4f, 0.1f, 0.4f)); 
        ourShader.setMat4("model", bloodMatrix1);
        bloodModel.Draw(ourShader);

        // Charco de sangre 2 - esquina izquierda de la habitación
        glm::mat4 bloodMatrix2 = glm::mat4(1.0f);
        bloodMatrix2 = glm::translate(bloodMatrix2, glm::vec3(-8.0f, -9.2f, -47.0f)); // Esquina izquierda
        bloodMatrix2 = glm::rotate(bloodMatrix2, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix2 = glm::scale(bloodMatrix2, glm::vec3(0.3f, 0.1f, 0.5f)); 
        ourShader.setMat4("model", bloodMatrix2);
        bloodModel.Draw(ourShader);

        // Charco de sangre 3 - cerca del área derecha
        glm::mat4 bloodMatrix3 = glm::mat4(1.0f);
        bloodMatrix3 = glm::translate(bloodMatrix3, glm::vec3(0.0f, -9.2f, -52.0f)); // Área derecha
        bloodMatrix3 = glm::rotate(bloodMatrix3, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix3 = glm::scale(bloodMatrix3, glm::vec3(0.4f, 0.1f, 0.3f)); 
        ourShader.setMat4("model", bloodMatrix3);
        bloodModel.Draw(ourShader);

        // Charco de sangre 4 - zona central
        glm::mat4 bloodMatrix4 = glm::mat4(1.0f);
        bloodMatrix4 = glm::translate(bloodMatrix4, glm::vec3(-4.0f, -9.2f, -35.0f)); // Zona central
        bloodMatrix4 = glm::rotate(bloodMatrix4, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix4 = glm::scale(bloodMatrix4, glm::vec3(0.5f, 0.1f, 0.3f)); 
        ourShader.setMat4("model", bloodMatrix4);
        bloodModel.Draw(ourShader);

        // Charco de sangre 5 - zona posterior de la habitación
        glm::mat4 bloodMatrix5 = glm::mat4(1.0f);
        bloodMatrix5 = glm::translate(bloodMatrix5, glm::vec3(-1.0f, -9.2f, -57.0f)); // Zona posterior
        bloodMatrix5 = glm::rotate(bloodMatrix5, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix5 = glm::scale(bloodMatrix5, glm::vec3(0.3f, 0.1f, 0.2f)); 
        ourShader.setMat4("model", bloodMatrix5);
        bloodModel.Draw(ourShader);

        // Charco de sangre 6 - zona frontal izquierda
        glm::mat4 bloodMatrix6 = glm::mat4(1.0f);
        bloodMatrix6 = glm::translate(bloodMatrix6, glm::vec3(-9.0f, -9.2f, -30.0f)); // Zona frontal izquierda
        bloodMatrix6 = glm::rotate(bloodMatrix6, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix6 = glm::scale(bloodMatrix6, glm::vec3(0.4f, 0.1f, 0.1f)); 
        ourShader.setMat4("model", bloodMatrix6);
        bloodModel.Draw(ourShader);

        // Charco de sangre 7 - zona frontal derecha
        glm::mat4 bloodMatrix7 = glm::mat4(1.0f);
        bloodMatrix7 = glm::translate(bloodMatrix7, glm::vec3(2.0f, -9.2f, -28.0f)); // Zona frontal derecha
        bloodMatrix7 = glm::rotate(bloodMatrix7, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix7 = glm::scale(bloodMatrix7, glm::vec3(0.3f, 0.1f, 0.1f)); 
        ourShader.setMat4("model", bloodMatrix7);
        bloodModel.Draw(ourShader);

        // Charco de sangre 8 - en el pasillo
        glm::mat4 bloodMatrix8 = glm::mat4(1.0f);
        bloodMatrix8 = glm::translate(bloodMatrix8, glm::vec3(8.0f, -9.2f, -49.0f)); // Zona del pasillo
        bloodMatrix8 = glm::rotate(bloodMatrix8, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bloodMatrix8 = glm::scale(bloodMatrix8, glm::vec3(0.3f, 0.1f, 0.2f)); 
        ourShader.setMat4("model", bloodMatrix8);
        bloodModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

bool isPlayerInAllowedZone(glm::vec3 position, const std::vector<glm::vec4>& zones) {
    for (const auto& zone : zones) {
        // Comprueba si la posición está dentro de los límites de la zona actual
        bool inThisZone = (position.x >= zone.x && position.x <= zone.y &&
            position.z >= zone.z && position.z <= zone.w);
        // Si está dentro de al menos UNA zona, es un lugar válido.
        if (inThisZone) {
            return true;
        }
    }
    return false;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 oldCameraPos = camera.Position;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (!isPlayerInAllowedZone(camera.Position, walkableZones))
    {
        // Si se sale, revierte a la posición segura anterior
        camera.Position = oldCameraPos;
    }
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