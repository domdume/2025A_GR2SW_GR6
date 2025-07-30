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
bool isPlayerNearSkull(glm::vec3 playerPos, glm::vec3 skullPos, float radius = 2.0f);
void checkSkullCollisions(glm::vec3 playerPos);
void checkSlendermanDamage(glm::vec3 playerPos, glm::vec3 slendermanPos, float currentTime);
void displayGameOver();
void resetGame();
void renderGameOverScreen(Shader& shader, glm::mat4 projection, glm::mat4 view);
void renderGameOverText(Shader& shader, glm::mat4 projection, glm::mat4 view);
void renderGameOverOverlay(Shader& shader, unsigned int gameOverTexture);
unsigned int loadTexture(char const * path);
void setupGameOverQuad();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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
float flashlightBattery = 5.0f; // Duración de la linterna en segundos
const float maxBattery = 5.0f;
const float batteryRecharge = 5.0f; // Segundos que se añaden al pisar calavera
bool flashlightBatteryEmpty = false;

// skull collision detection
std::vector<bool> skullCollected(7, false); // Para rastrear qué calaveras ya fueron pisadas

// player life system
int playerLives = 3; // Vidas del jugador
bool gameOver = false;
bool showGameOverScreen = false;
float gameOverTime = 0.0f;
float lastDamageTime = 0.0f; // Para evitar pérdida múltiple de vidas muy rápido
const float damageInterval = 2.0f; // Tiempo mínimo entre daños (segundos)
const float dangerDistance = 3.0f; // Distancia a la que Slenderman causa daño

// slenderman variables
glm::vec3 slendermanPosition = glm::vec3(0.0f, -7.5f, -30.0f); // Posicionado dentro de la habitación
float slendermanSpeed = 2.0f;
float slendermanDirection = 0.0f;
float slendermanMovementTimer = 0.0f;
bool slendermanIsIlluminated = false; // Estado de iluminación de Slenderman

// Game Over overlay variables
unsigned int gameOverVAO, gameOverVBO;
unsigned int gameOverTexture;

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
    Shader overlayShader("shaders/overlay.vs", "shaders/overlay.fs");
    // Cargar shaders para los espejos
    Shader mirrorShader("shaders/mirror.vs", "shaders/mirror.fs");
    //Shader emissiveShader("shaders/luzemissive.vs", "shaders/luzemissive.fs");
    // load models
    // -----------
    Model ourModel("model/partyroom/partyroom.obj");
    Model slendermanModel("model/slenderman/slenderman.obj");
    Model skullModel("model/skull/skull.obj");
    Model bloodModel("model/blood/blood.obj");

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

    // Configurar quad para Game Over overlay
    setupGameOverQuad();
    
    // Cargar textura de Game Over 
    
    gameOverTexture = loadTexture("textures/over.png"); 
    


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

        // Actualizar batería de la linterna
        if (flashlightOn && flashlightBattery > 0.0f) {
            flashlightBattery -= deltaTime;
            if (flashlightBattery <= 0.0f) {
                flashlightBattery = 0.0f;
                flashlightBatteryEmpty = true;
                flashlightOn = false; // Apagar linterna cuando se agote la batería
            }
        }

        // Verificar colisiones con calaveras para recargar batería
        checkSkullCollisions(camera.Position);

        // Verificar si Slenderman causa daño al jugador
        if (!gameOver) {
            checkSlendermanDamage(camera.Position, slendermanPosition, currentFrame);
        }

        // Si el juego terminó, mostrar game over y salir del loop
        if (gameOver && !showGameOverScreen) {
            showGameOverScreen = true;
            gameOverTime = currentFrame;
            displayGameOver();
        }

        // Mostrar vidas en consola cada 10 segundos (opcional para debug)
        static float lastLifeDisplay = 0.0f;
        if (currentFrame - lastLifeDisplay > 10.0f) {
            std::cout << "Vidas actuales: " << playerLives << " | Batería: " << flashlightBattery << "s" << std::endl;
            lastLifeDisplay = currentFrame;
        }

        // Actualizar movimiento de Slenderman para perseguir al jugador
        slendermanMovementTimer += deltaTime;

        // Calcular si Slenderman está siendo iluminado por la linternas
        slendermanIsIlluminated = false;
        if (flashlightOn && flashlightBattery > 0.0f) {
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
        
        // Si estamos en game over, usar un fondo menos oscuro
        if (showGameOverScreen) {
            glClearColor(0.1f, 0.0f, 0.0f, 1.0f); // Fondo rojo muy oscuro pero no negro
        } else {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fondo negro normal
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Si estamos en game over, mostrar pantalla especial con modelos 3D
        if (showGameOverScreen) {
            ourShader.use();
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
            glm::mat4 view = camera.GetViewMatrix();
            
            // Renderizar la escena de Game Over con efectos especiales
            renderGameOverScreen(ourShader, projection, view);
            
            // Configurar iluminación adicional para mejor visibilidad de los modelos
            float gameOverTime = glfwGetTime();
            float brightPulse = 0.8f + 0.4f * sin(gameOverTime * 2.5f);
            
            // Luz principal más brillante para Game Over
            ourShader.setVec3("light.position", 0.0f, 8.0f, -30.0f);
            ourShader.setVec3("light.ambient", 0.7f * brightPulse, 0.15f, 0.15f);
            ourShader.setVec3("light.diffuse", 2.0f * brightPulse, 0.4f, 0.4f);
            ourShader.setVec3("light.specular", 1.5f, 0.5f, 0.5f);
            
            // Renderizar el modelo central de Slenderman
            glm::mat4 centralSlenderman = glm::mat4(1.0f);
            centralSlenderman = glm::translate(centralSlenderman, glm::vec3(0.0f, -6.0f, -28.0f));
            centralSlenderman = glm::rotate(centralSlenderman, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            float breathEffect = 1.0f + 0.1f * sin(glfwGetTime() * 3.0f);
            centralSlenderman = glm::scale(centralSlenderman, glm::vec3(0.008f * breathEffect, 0.008f * breathEffect, 0.008f * breathEffect));
            ourShader.setMat4("model", centralSlenderman);
            slendermanModel.Draw(ourShader);
            
            // Renderizar círculo de calaveras flotantes
            for (int i = 0; i < 8; i++) {
                float angle = (i / 8.0f) * 2.0f * 3.14159265f;
                float radius = 12.0f + 2.0f * sin(glfwGetTime() + i);
                float x = cos(angle) * radius;
                float z = sin(angle) * radius;
                
                glm::mat4 skullMatrix = glm::mat4(1.0f);
                skullMatrix = glm::translate(skullMatrix, glm::vec3(x, -8.5f, -35.0f + z));
                float rotationAngle = glm::degrees(angle) + glfwGetTime() * 30.0f;
                skullMatrix = glm::rotate(skullMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
                
                float floatHeight = 1.0f * sin(glfwGetTime() * 2.0f + i);
                skullMatrix = glm::translate(skullMatrix, glm::vec3(0.0f, floatHeight, 0.0f));
                
                float scaleEffect = 2.0f + 0.5f * sin(glfwGetTime() * 3.0f + i);
                skullMatrix = glm::scale(skullMatrix, glm::vec3(scaleEffect, scaleEffect, scaleEffect));
                
                ourShader.setMat4("model", skullMatrix);
                skullModel.Draw(ourShader);
            }
            
            // Renderizar charcos de sangre que se expanden
            for (int i = 0; i < 6; i++) {
                float angle = (i / 6.0f) * 2.0f * 3.14159265f;
                float x = cos(angle) * 8.0f;
                float z = sin(angle) * 8.0f;
                
                glm::mat4 bloodMatrix = glm::mat4(1.0f);
                bloodMatrix = glm::translate(bloodMatrix, glm::vec3(x, -9.1f, -35.0f + z));
                float bloodRotation = glm::degrees(angle);
                bloodMatrix = glm::rotate(bloodMatrix, glm::radians(bloodRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                
                float pulseEffect = 0.5f + 0.5f * sin(glfwGetTime() * 2.0f);
                float expandEffect = 0.5f + 0.3f * pulseEffect;
                bloodMatrix = glm::scale(bloodMatrix, glm::vec3(expandEffect, 0.1f, expandEffect));
                
                ourShader.setMat4("model", bloodMatrix);
                bloodModel.Draw(ourShader);
            }
            
            // Renderizar Slenderman en las esquinas
            glm::vec3 cornerPositions[] = {
                glm::vec3(-15.0f, -6.5f, -45.0f),
                glm::vec3(15.0f, -6.5f, -45.0f),
                glm::vec3(-15.0f, -6.5f, -25.0f),
                glm::vec3(15.0f, -6.5f, -25.0f)
            };
            
            for (int i = 0; i < 4; i++) {
                glm::mat4 cornerSlenderman = glm::mat4(1.0f);
                cornerSlenderman = glm::translate(cornerSlenderman, cornerPositions[i]);
                
                glm::vec3 lookDirection = camera.Position - cornerPositions[i];
                float lookAngle = atan2(lookDirection.x, lookDirection.z);
                cornerSlenderman = glm::rotate(cornerSlenderman, lookAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                
                float swayEffect = 0.1f * sin(glfwGetTime() * 1.5f + i);
                float swayDegrees = swayEffect * 10.0f;
                cornerSlenderman = glm::rotate(cornerSlenderman, glm::radians(swayDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
                
                cornerSlenderman = glm::scale(cornerSlenderman, glm::vec3(0.006f, 0.006f, 0.006f));
                ourShader.setMat4("model", cornerSlenderman);
                slendermanModel.Draw(ourShader);
            }
            
            // Renderizar texto "GAME OVER" usando calaveras como letras
            renderGameOverText(ourShader, projection, view);
            
            // Renderizar las letras "GAME" usando calaveras
            std::vector<glm::vec3> gamePositions = {
                glm::vec3(-15.0f, -3.0f, -25.0f), // G
                glm::vec3(-11.0f, -3.0f, -25.0f), // A
                glm::vec3(-7.0f, -3.0f, -25.0f),  // M
                glm::vec3(-3.0f, -3.0f, -25.0f)   // E
            };
            
            std::vector<glm::vec3> overPositions = {
                glm::vec3(3.0f, -3.0f, -25.0f),   // O
                glm::vec3(7.0f, -3.0f, -25.0f),   // V
                glm::vec3(11.0f, -3.0f, -25.0f),  // E
                glm::vec3(15.0f, -3.0f, -25.0f)   // R
            };
            
            float textTime = glfwGetTime();
            float letterFloat = 0.5f * sin(textTime * 1.5f);
            
            // Renderizar "GAME" 
            for (int i = 0; i < gamePositions.size(); i++) {
                glm::mat4 letterMatrix = glm::mat4(1.0f);
                float individualFloat = letterFloat + sin(textTime * 2.0f + i * 0.5f) * 0.3f;
                letterMatrix = glm::translate(letterMatrix, gamePositions[i] + glm::vec3(0.0f, individualFloat, 0.0f));
                
                float rotation = textTime * 30.0f + i * 45.0f;
                letterMatrix = glm::rotate(letterMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
                
                float letterScale = 3.0f + 1.0f * sin(textTime * 3.0f + i * 0.8f);
                letterMatrix = glm::scale(letterMatrix, glm::vec3(letterScale, letterScale, letterScale));
                
                ourShader.setMat4("model", letterMatrix);
                skullModel.Draw(ourShader);
            }
            
            // Renderizar "OVER"
            for (int i = 0; i < overPositions.size(); i++) {
                glm::mat4 letterMatrix = glm::mat4(1.0f);
                float individualFloat = letterFloat + sin(textTime * 2.0f + (i + 4) * 0.5f) * 0.3f;
                letterMatrix = glm::translate(letterMatrix, overPositions[i] + glm::vec3(0.0f, individualFloat, 0.0f));
                
                float rotation = textTime * 30.0f + (i + 4) * 45.0f;
                letterMatrix = glm::rotate(letterMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
                
                float letterScale = 3.0f + 1.0f * sin(textTime * 3.0f + (i + 4) * 0.8f);
                letterMatrix = glm::scale(letterMatrix, glm::vec3(letterScale, letterScale, letterScale));
                
                ourShader.setMat4("model", letterMatrix);
                skullModel.Draw(ourShader);
            }
            
            // Renderizar texto de instrucciones usando charcos de sangre más pequeños
            std::vector<glm::vec3> instructionPositions = {
                glm::vec3(-8.0f, -6.0f, -25.0f),   // "Press"
                glm::vec3(-4.0f, -6.0f, -25.0f),   // "R"
                glm::vec3(0.0f, -6.0f, -25.0f),    // "to"
                glm::vec3(4.0f, -6.0f, -25.0f),    // "Restart"
                glm::vec3(8.0f, -6.0f, -25.0f)     // "..."
            };
            
            for (int i = 0; i < instructionPositions.size(); i++) {
                glm::mat4 instructionMatrix = glm::mat4(1.0f);
                float instructionFloat = 0.2f * sin(textTime * 2.5f + i * 0.3f);
                instructionMatrix = glm::translate(instructionMatrix, instructionPositions[i] + glm::vec3(0.0f, instructionFloat, 0.0f));
                
                float instructionScale = 0.8f + 0.2f * sin(textTime * 4.0f + i * 0.5f);
                instructionMatrix = glm::scale(instructionMatrix, glm::vec3(instructionScale, 0.1f, instructionScale));
                
                ourShader.setMat4("model", instructionMatrix);
                bloodModel.Draw(ourShader);
            }
            
            // Renderizar overlay PNG de Game Over encima de todo
            renderGameOverOverlay(overlayShader, gameOverTexture);
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

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
        ourShader.setBool("flashlightOn", flashlightOn && flashlightBattery > 0.0f);
        if (flashlightOn && flashlightBattery > 0.0f) {
            ourShader.setVec3("flashlight.position", camera.Position);
            ourShader.setVec3("flashlight.direction", camera.Front);
            ourShader.setFloat("flashlight.cutOff", glm::cos(glm::radians(8.0f)));
            ourShader.setFloat("flashlight.outerCutOff", glm::cos(glm::radians(12.0f)));
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

// Función para detectar si el jugador está cerca de una calavera
bool isPlayerNearSkull(glm::vec3 playerPos, glm::vec3 skullPos, float radius) {
    float distance = glm::length(playerPos - skullPos);
    return distance <= radius;
}

// Función para verificar colisiones con todas las calaveras
void checkSkullCollisions(glm::vec3 playerPos) {
    // Posiciones de las calaveras (deben coincidir con las del render)
    std::vector<glm::vec3> skullPositions = {
        glm::vec3(-4.0f, -9.3f, -39.0f),  // Skull 1
        glm::vec3(-10.0f, -9.3f, -45.0f), // Skull 2
        glm::vec3(2.0f, -9.3f, -50.0f),   // Skull 3
        glm::vec3(-6.0f, -9.3f, -25.0f),  // Skull 4
        glm::vec3(1.0f, -9.3f, -55.0f),   // Skull 5
        glm::vec3(-8.0f, -9.3f, -33.0f),  // Skull 6
        glm::vec3(0.0f, -9.3f, -30.0f)    // Skull 7
    };

    for (int i = 0; i < skullPositions.size(); i++) {
        if (!skullCollected[i] && isPlayerNearSkull(playerPos, skullPositions[i], 2.0f)) {
            // Jugador pisó una calavera nueva
            skullCollected[i] = true;
            flashlightBattery = std::min(flashlightBattery + batteryRecharge, maxBattery);
            flashlightBatteryEmpty = false;
            
            // Opcional: Imprimir mensaje en consola para debug
            std::cout << "¡Calavera " << (i + 1) << " pisada! Batería recargada: " << flashlightBattery << "s" << std::endl;
            break; // Solo una calavera por frame
        }
    }
}

// Función para verificar si Slenderman causa daño al jugador
void checkSlendermanDamage(glm::vec3 playerPos, glm::vec3 slendermanPos, float currentTime) {
    float distance = glm::length(playerPos - slendermanPos);
    
    // Si Slenderman está muy cerca y ha pasado suficiente tiempo desde el último daño
    if (distance <= dangerDistance && (currentTime - lastDamageTime) >= damageInterval) {
        playerLives--;
        lastDamageTime = currentTime;
        
        std::cout << "¡Slenderman te atacó! Vidas restantes: " << playerLives << std::endl;
        
        // Verificar si el jugador se quedó sin vidas
        if (playerLives <= 0) {
            gameOver = true;
            std::cout << "GAME OVER - Te quedaste sin vidas!" << std::endl;
        }
    }
}

// Función para mostrar game over
void displayGameOver() {
    std::cout << "===============================================" << std::endl;
    std::cout << "                 GAME OVER                    " << std::endl;
    std::cout << "        Slenderman te ha vencido...           " << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "Presiona R para reiniciar el juego" << std::endl;
    std::cout << "Presiona ESC para salir del juego" << std::endl;
}

// Función para reiniciar el juego
void resetGame() {
    // Reiniciar todas las variables del juego
    playerLives = 3;
    gameOver = false;
    showGameOverScreen = false;
    gameOverTime = 0.0f;
    lastDamageTime = 0.0f;
    
    // Reiniciar batería de linterna
    flashlightBattery = 5.0f;
    flashlightOn = false;
    flashlightBatteryEmpty = false;
    
    // Reiniciar calaveras
    for (int i = 0; i < skullCollected.size(); i++) {
        skullCollected[i] = false;
    }
    
    // Reiniciar posición del jugador
    camera.Position = glm::vec3(0.0f, -7.5f, -32.0f);
    
    // Reiniciar Slenderman
    slendermanPosition = glm::vec3(0.0f, -7.5f, -30.0f);
    slendermanDirection = 0.0f;
    slendermanMovementTimer = 0.0f;
    slendermanIsIlluminated = false;
    
    std::cout << "¡Juego reiniciado! Buena suerte..." << std::endl;
}

// Función para configurar la iluminación de Game Over
void renderGameOverScreen(Shader& shader, glm::mat4 projection, glm::mat4 view) {
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", camera.Position);
    shader.setFloat("material.shininess", 16.0f);
    
    float currentTime = glfwGetTime();
    float flickerEffect = 0.8f + 0.2f * sin(currentTime * 3.0f);
    float pulseEffect = 0.7f + 0.3f * sin(currentTime * 2.0f);
    
    // Configurar iluminación más brillante y dramática para Game Over
    shader.setVec3("light.position", 0.0f, 5.0f, -25.0f);
    shader.setVec3("light.ambient", 0.6f * flickerEffect, 0.1f, 0.1f); // Más luz ambiente
    shader.setVec3("light.diffuse", 1.5f * flickerEffect, 0.3f, 0.3f); // Mucho más brillante
    shader.setVec3("light.specular", 1.2f, 0.4f, 0.4f);
    shader.setFloat("light.constant", 1.0f);
    shader.setFloat("light.linear", 0.022f); // Menor atenuación para más alcance
    shader.setFloat("light.quadratic", 0.0019f);
    
    // Añadir una segunda luz desde arriba para mejor visibilidad
    shader.setVec3("light2.position", 0.0f, 10.0f, -35.0f);
    shader.setVec3("light2.ambient", 0.4f * pulseEffect, 0.05f, 0.05f);
    shader.setVec3("light2.diffuse", 1.0f * pulseEffect, 0.2f, 0.2f);
    shader.setVec3("light2.specular", 0.8f, 0.3f, 0.3f);
    shader.setFloat("light2.constant", 1.0f);
    shader.setFloat("light2.linear", 0.045f);
    shader.setFloat("light2.quadratic", 0.0075f);
    
    // Desactivar linterna durante Game Over
    shader.setBool("flashlightOn", false);
    shader.setFloat("time", currentTime);
    shader.setBool("hasEmissiveMap", false);
}

// Función para renderizar texto de Game Over usando modelos 3D
void renderGameOverText(Shader& shader, glm::mat4 projection, glm::mat4 view) {
    float currentTime = glfwGetTime();
    float textPulse = 0.8f + 0.4f * sin(currentTime * 2.0f);
    float letterFloat = 0.5f * sin(currentTime * 1.5f);
    
    // Configurar iluminación especial para el texto
    shader.setVec3("light.position", 0.0f, 0.0f, -20.0f);
    shader.setVec3("light.ambient", 1.0f * textPulse, 0.2f, 0.2f);
    shader.setVec3("light.diffuse", 2.0f * textPulse, 0.3f, 0.3f);
    shader.setVec3("light.specular", 1.5f, 0.4f, 0.4f);
    
    // Posiciones para las letras "GAME OVER" usando calaveras
    std::vector<glm::vec3> gamePositions = {
        glm::vec3(-15.0f, -3.0f, -25.0f), // G
        glm::vec3(-11.0f, -3.0f, -25.0f), // A
        glm::vec3(-7.0f, -3.0f, -25.0f),  // M
        glm::vec3(-3.0f, -3.0f, -25.0f)   // E
    };
    
    std::vector<glm::vec3> overPositions = {
        glm::vec3(3.0f, -3.0f, -25.0f),   // O
        glm::vec3(7.0f, -3.0f, -25.0f),   // V
        glm::vec3(11.0f, -3.0f, -25.0f),  // E
        glm::vec3(15.0f, -3.0f, -25.0f)   // R
    };
    
    // Renderizar "GAME" 
    for (int i = 0; i < gamePositions.size(); i++) {
        glm::mat4 letterMatrix = glm::mat4(1.0f);
        float individualFloat = letterFloat + sin(currentTime * 2.0f + i * 0.5f) * 0.3f;
        letterMatrix = glm::translate(letterMatrix, gamePositions[i] + glm::vec3(0.0f, individualFloat, 0.0f));
        
        // Rotación individual para cada letra
        float rotation = currentTime * 30.0f + i * 45.0f;
        letterMatrix = glm::rotate(letterMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Escalado pulsante más grande para las letras
        float letterScale = 3.0f + 1.0f * sin(currentTime * 3.0f + i * 0.8f);
        letterMatrix = glm::scale(letterMatrix, glm::vec3(letterScale, letterScale, letterScale));
        
        shader.setMat4("model", letterMatrix);
    }
    
    // Renderizar "OVER"
    for (int i = 0; i < overPositions.size(); i++) {
        glm::mat4 letterMatrix = glm::mat4(1.0f);
        float individualFloat = letterFloat + sin(currentTime * 2.0f + (i + 4) * 0.5f) * 0.3f;
        letterMatrix = glm::translate(letterMatrix, overPositions[i] + glm::vec3(0.0f, individualFloat, 0.0f));
        
        // Rotación individual para cada letra
        float rotation = currentTime * 30.0f + (i + 4) * 45.0f;
        letterMatrix = glm::rotate(letterMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Escalado pulsante más grande para las letras
        float letterScale = 3.0f + 1.0f * sin(currentTime * 3.0f + (i + 4) * 0.8f);
        letterMatrix = glm::scale(letterMatrix, glm::vec3(letterScale, letterScale, letterScale));
        
        shader.setMat4("model", letterMatrix);
    }
    
    // Texto de instrucciones usando charcos de sangre más pequeños
    std::vector<glm::vec3> instructionPositions = {
        glm::vec3(-8.0f, -6.0f, -25.0f),   // "Press"
        glm::vec3(-4.0f, -6.0f, -25.0f),   // "R"
        glm::vec3(0.0f, -6.0f, -25.0f),    // "to"
        glm::vec3(4.0f, -6.0f, -25.0f),    // "Restart"
        glm::vec3(8.0f, -6.0f, -25.0f)     // "..."
    };
}

// Función para cargar textura desde archivo
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Función para configurar el quad de pantalla completa
void setupGameOverQuad() {
    // Vértices para un quad de pantalla completa en coordenadas normalizadas
    float quadVertices[] = {
        // posiciones   // coordenadas de textura (Y invertida para corregir la orientación)
        -1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,

        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &gameOverVAO);
    glGenBuffers(1, &gameOverVBO);
    
    glBindVertexArray(gameOverVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gameOverVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

// Función para renderizar overlay de Game Over con PNG
void renderGameOverOverlay(Shader& shader, unsigned int gameOverTexture) {
    // Desactivar depth test para renderizar encima de todo
    glDisable(GL_DEPTH_TEST);
    
    // Activar blending para transparencia
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader.use();
    
    // Configurar matriz de proyección ortográfica para 2D
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    
    // Efecto de fade in/out
    float currentTime = glfwGetTime();
    float alpha = 0.7f + 0.3f * sin(currentTime * 2.0f); // Pulsación de transparencia
    shader.setFloat("alpha", alpha);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gameOverTexture);
    shader.setInt("gameOverTexture", 0);
    
    // Render quad
    glBindVertexArray(gameOverVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // Reactivar depth test
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Si estamos en game over, solo permitir reinicio
    if (showGameOverScreen) {
        static bool rKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
            resetGame();
            rKeyPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            rKeyPressed = false;
        }
        return;
    }

    // No permitir movimiento durante game over
    if (gameOver) {
        return;
    }

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
    // Alternar linterna con tecla F (solo si hay batería)
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
        if (flashlightBattery > 0.0f) {
            flashlightOn = !flashlightOn;
            flashlightBatteryEmpty = false;
        }
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
        if (flashlightBattery > 0.0f) {
            flashlightOn = !flashlightOn;
            flashlightBatteryEmpty = false;
        }
    }
}