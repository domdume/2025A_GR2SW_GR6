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
// glm::vec3 performWallRaycast(glm::vec3 origin, glm::vec3 direction, float maxDistance);

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

// raycast variables - Variables para el sistema de detección de colisiones por rayos (DESACTIVADO)
// glm::vec3 raycastHitPoint = glm::vec3(0.0f);    // Punto donde el rayo golpea una pared/objeto
// bool raycastHit = false;                        // Indica si el rayo ha golpeado algo
// float raycastMaxDistance = 10.0f;               // Distancia máxima del rayo para detección

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
    Shader slendermanShader("shaders/slenderman.vs", "shaders/slenderman.fs");
    //Shader emissiveShader("shaders/luzemissive.vs", "shaders/luzemissive.fs");
    // load models
    // -----------
    Model ourModel("model/partyroom/partyroom.obj");
    Model slendermanModel("model/slenderman/slenderman.obj");
    // Cargar shaders para los espejos
    Shader mirrorShader("shaders/mirror.vs", "shaders/mirror.fs");

    // Cargar modelos de espejos
    Model mirrorModel("model/espejo/espejo.obj");
    Model mirrorModel1("model/espejo1/espejo.obj");
    Model mirrorModel2("model/espejo2/espejo3.obj");

    // Estructura para almacenar posición, rotación y modelo de cada espejo
    struct MirrorData {
        glm::vec3 position;
        float rotation;
        int modelType; // 0 = espejo, 1 = espejo1, 2 = espejo2
    };

    // Espejos colocados en coordenadas específicas obtenidas del sistema de raycast
    // NOTA: Estas coordenadas se obtuvieron usando el sistema de raycast para encontrar
    // las posiciones exactas de las paredes donde colocar los espejos
    // Rotaciones ajustadas según la orientación de las zonas caminables para que la superficie reflectante mire hacia el interior
    std::vector<MirrorData> mirrors = {
        // Zona 2: Primer segmento del pasillo (4.52f, 12.25f, -51.10f, -46.88f) - pasillo horizontal
        {glm::vec3(12.297f, -7.5f, -49.1318f), 180.0f, 0},    // Espejo 1 - pared derecha, mira hacia la izquierda    // Espejo 2 - pared superior, mira hacia abajo
        {glm::vec3(6.26015f, -8.75f, -46.835f), 180.0f, 2},      // Espejo 3 - pared superior, mira hacia abajo
        {glm::vec3(6.29512f, -7.5f, -51.1213f), 270.0f, 0},   // Espejo 4 - pared inferior, mira hacia arriba  // Espejo 5 - pared derecha, mira hacia la izquierda

        // Zona 4: Tercer segmento del pasillo (11.84f, 15.92f, -54.13f, -51.47f) - pasillo horizontal
        {glm::vec3(10.3214f, -8.75f, -54.1734f), 0.0f, 2},   // Espejo 6 - pared inferior, mira hacia arriba
        {glm::vec3(9.31519f, -7.5f, -52.265f), 0.0f, 0},    // Espejo 7 - pared izquierda, mira hacia la derecha
        {glm::vec3(15.949f, -7.75f, -53.1732f), 270.0f, 1},     // Espejo 8 - pared derecha, mira hacia la izquierda

        // Zona: (13.48f, 15.94f, -55.23f, -53.70f) y (5.05f, 15.81f, -60.53f, -55.18f) - pasillos horizontales
        {glm::vec3(15.8235f, -8.75f, -57.1734f), 270.0f, 2},    // Espejo 9 - pared derecha, mira hacia la izquierda
        {glm::vec3(14.7786f, -7.5f, -60.5661f), 270.0f, 0},   // Espejo 10 - pared inferior, mira hacia arriba
        {glm::vec3(5.78436f, -7.75f, -60.5531f), 360.0f, 1},   // Espejo 11 - pared inferior, mira hacia arriba
        {glm::vec3(5.02137f, -8.75f, -57.678f), 90.0f, 2},    // Espejo 12 - pared izquierda, mira hacia la derecha

        // Zona 3: Segundo segmento del pasillo (9.06f, 11.70f, -47.15f, -43.56f) - pasillo horizontal
        {glm::vec3(10.8152f, -7.5f, -43.5293f), 90.0f, 0},     // Espejo 13 - pared superior, mira hacia abajo
        {glm::vec3(14.6936f, -7.75f, -46.0318f), 360.0f, 1},   // Espejo 14 - pared inferior, mira hacia arriba
        {glm::vec3(15.7578f, -8.75f, -45.0963f), 270.0f, 2},    // Espejo 15 - pared derecha, mira hacia la izquierda

        // Zona: (5.18f, 15.74f, -43.2f, -34.72f) - pasillo vertical largo
        {glm::vec3(15.90f, -7.5f, -36.7161f), 180.0f, 0},     // Espejo 16 - pared derecha, mira hacia la izquierda
        {glm::vec3(14.339f, -7.75f, -34.6716f), 180.0f, 1},      // Espejo 17 - pared superior, mira hacia abajo
        {glm::vec3(5.15202f, -8.75f, -36.4961f), 90.0f, 2},   // Espejo 18 - pared izquierda, mira hacia la derecha
        {glm::vec3(5.13132f, -7.80f, -41.491f), 360.0f, 0}     // Espejo 19 - pared izquierda, mira hacia la derecha
    };


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

        // SISTEMA DE RAYCAST - Detecta paredes/objetos del mundo (DESACTIVADO)
        // Realizar raycast desde la posición de la cámara en la dirección que mira
        // para encontrar el punto exacto donde hay una pared u objeto
        // raycastHitPoint = performWallRaycast(camera.Position, camera.Front, raycastMaxDistance);
        // raycastHit = true;  // Siempre marcamos como hit porque siempre encontramos algo

        // Actualizar movimiento de Slenderman para perseguir al jugador
        slendermanMovementTimer += deltaTime;

        // Cambiar dirección cada 3-5 segundos de forma aleatoria
        if (slendermanMovementTimer > 3.0f + (sin(currentFrame * 0.3f) * 2.0f)) {
            slendermanDirection += glm::radians(45.0f + (sin(currentFrame * 0.7f) * 90.0f));
            slendermanMovementTimer = 0.0f;
        }

        // Calcular nueva posición de Slenderman
        float moveX = sin(slendermanDirection) * slendermanSpeed * deltaTime;
        float moveZ = cos(slendermanDirection) * slendermanSpeed * deltaTime;
        glm::vec3 newPosition = slendermanPosition;
        newPosition.x += moveX;
        newPosition.z += moveZ;

        // Mantener Slenderman dentro de los límites de la habitación
        // Límites basados en las dimensiones del party room
        if (isPlayerInAllowedZone(newPosition, walkableZones)) {
            slendermanPosition = newPosition; // Actualizar posición solo si está dentro de las zonas permitidas
        }
        else {
            // Si se sale de los límites, cambiar dirección
            slendermanDirection += glm::radians(180.0f);
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

        // Renderizar los espejos
        for (int i = 0; i < static_cast<int>(mirrors.size()); ++i) {
            const auto& mirror = mirrors[i];
            mirrorShader.use();
            mirrorShader.setMat4("projection", projection);
            mirrorShader.setMat4("view", view);
            mirrorShader.setVec3("viewPos", camera.Position);

            // Configurar uniforms de la linterna para los espejos
            mirrorShader.setBool("flashlightOn", flashlightOn);
            if (flashlightOn) {
                mirrorShader.setVec3("flashlight.position", camera.Position);
                mirrorShader.setVec3("flashlight.direction", camera.Front);
                mirrorShader.setFloat("flashlight.cutOff", glm::cos(glm::radians(15.0f)));
                mirrorShader.setFloat("flashlight.outerCutOff", glm::cos(glm::radians(25.0f)));
                mirrorShader.setVec3("flashlight.ambient", 0.0f, 0.0f, 0.0f);
                mirrorShader.setVec3("flashlight.diffuse", 1.0f, 1.0f, 0.9f);
                mirrorShader.setVec3("flashlight.specular", 1.0f, 1.0f, 1.0f);
                mirrorShader.setFloat("flashlight.constant", 1.0f);
                mirrorShader.setFloat("flashlight.linear", 0.022f);
                mirrorShader.setFloat("flashlight.quadratic", 0.0019f);
            }

            glm::mat4 mirrorModelMatrix = glm::mat4(1.0f);
            mirrorModelMatrix = glm::translate(mirrorModelMatrix, mirror.position);
            mirrorModelMatrix = glm::rotate(mirrorModelMatrix, glm::radians(mirror.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            mirrorModelMatrix = glm::scale(mirrorModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f)); // Espejos más grandes (era 0.5f)
            mirrorShader.setMat4("model", mirrorModelMatrix);

            // Seleccionar el modelo según el tipo
            switch (mirror.modelType) {
            case 0:
                mirrorModel.Draw(mirrorShader);
                break;
            case 1:
                mirrorModel1.Draw(mirrorShader);
                break;
            case 2:
                mirrorModel2.Draw(mirrorShader);
                break;
            }
        }

        // SISTEMA DE DEBUG PARA RAYCAST (DESACTIVADO)
        // Mostrar coordenadas de donde apunta el jugador en tiempo real
        // Presiona TAB para ver las coordenadas de la pared/objeto al que apuntas
        /*
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            std::cout << "Pared/Objeto en: X=" << raycastHitPoint.x
                      << ", Y=" << raycastHitPoint.y
                      << ", Z=" << raycastHitPoint.z << std::endl;
        }
        */

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
    /*
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // RAYCAST DEBUG: Click derecho para mostrar coordenadas exactas (DESACTIVADO)
        // Útil para obtener posiciones precisas de paredes/objetos donde colocar elementos
        std::cout << "Coordenadas de pared/objeto: X=" << raycastHitPoint.x
                  << ", Y=" << raycastHitPoint.y
                  << ", Z=" << raycastHitPoint.z << std::endl;
    }
    */
}

/*
// FUNCIÓN DE RAYCAST - Detecta paredes/objetos del mundo mediante raycasting (DESACTIVADA)
// Esta función simula un rayo que sale desde el origen en una dirección específica
// y detecta cuando encuentra una pared (zona no caminable)
glm::vec3 performWallRaycast(glm::vec3 origin, glm::vec3 direction, float maxDistance)
{
    // Normalizar la dirección del rayo para que tenga longitud 1
    direction = glm::normalize(direction);

    // Configurar el paso pequeño para mayor precisión en la detección
    // Un paso más pequeño = mayor precisión pero más cálculos
    float stepSize = 0.05f;
    float currentDistance = 0.0f;

    // Recorrer el rayo paso a paso hasta encontrar una pared o alcanzar la distancia máxima
    while (currentDistance < maxDistance) {
        // Calcular el punto actual en el rayo basado en la distancia recorrida
        glm::vec3 currentPoint = origin + direction * currentDistance;

        // DETECCIÓN DE COLISIÓN:
        // Si el punto actual está fuera de las zonas caminables,
        // significa que hemos encontrado una pared u objeto sólido
        if (!isPlayerInAllowedZone(currentPoint, walkableZones)) {
            // ¡ENCONTRAMOS UNA PARED! Devolver las coordenadas exactas del impacto
            return currentPoint;
        }

        // Avanzar al siguiente punto del rayo
        currentDistance += stepSize;
    }

    // Si llegamos aquí, no encontramos ninguna pared dentro del rango
    // Devolver el punto final del rayo (máxima distancia alcanzada)
    return origin + direction * maxDistance;
}
*/