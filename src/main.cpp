#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const * path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    bool spotlight = false;
    glm::vec3 princePosition = glm::vec3(0.0f);
    float princeScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader objShader("resources/shaders/object_lighting.vs", "resources/shaders/object_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader sourceShader("resources/shaders/light_source.vs", "resources/shaders/light_source.fs");

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,1.0f
    };


    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);


    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    // load models
    // -----------
    Model princ("resources/objects/princ/9848.obj");
    princ.SetShaderTextureNamePrefix("material.");
    Model rose("resources/objects/ruza/rose.obj");
    rose.SetShaderTextureNamePrefix("material");

    vector<std::string> skyboxSides = {
            FileSystem::getPath("resources/textures/space1/px.png"),
            FileSystem::getPath("resources/textures/space1/nx.png"),
            FileSystem::getPath("resources/textures/space1/py.png"),
            FileSystem::getPath("resources/textures/space1/ny.png"),
            FileSystem::getPath("resources/textures/space1/pz.png"),
            FileSystem::getPath("resources/textures/space1/nz.png")
    };

    unsigned int cubemapTexture = loadCubemap(skyboxSides);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyboxShader.setBool("celShading", true);
        sourceShader.setBool("celShading", true);

        // don't forget to enable shader before setting uniforms
        objShader.use();
        objShader.setVec3("viewPos", programState->camera.Position);
        objShader.setFloat("material.shininess", 32.0f);
        objShader.setBool("celShading", true);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        objShader.setMat4("projection", projection);
        objShader.setMat4("view", view);

        // transformation matrices for the lanterns and the lights

        glm::mat4 transMat1 = glm::mat4(1.0f);
        transMat1 = glm::translate(transMat1, glm::vec3(1.05f, 1.95f, -0.46f));
        transMat1 = glm::scale(transMat1, glm::vec3(0.08f, 0.08f, 0.08f));
        transMat1 = glm::rotate(transMat1, sin(currentFrame*2)*glm::radians(60.0f), glm::vec3(0,0,1));
        transMat1 = glm::translate(transMat1, glm::vec3(0.0f, -3.0f, 0.0f));

        glm::mat4 transMat2 = glm::mat4(1.0f);
        transMat2 = glm::translate(transMat2, glm::vec3(-1.85f, 1.95f, 0.56f));
        transMat2 = glm::scale(transMat2, glm::vec3(0.08f, 0.08f, 0.08f));
        transMat2 = glm::rotate(transMat2, sin(0.6f+currentFrame*2)*glm::radians(60.0f), glm::vec3(0,0,1));
        transMat2 = glm::translate(transMat2, glm::vec3(0.0f, -3.0f, 0.0f));

        glm::mat4 baseMat1 = glm::mat4(1.0f);
        baseMat1 = glm::translate(transMat1, glm::vec3(1.05f, 1.95f, 0.06f));
        baseMat1 = glm::scale(baseMat1, glm::vec3(0.08f, 0.08f, 0.08f));
        glm::mat4 baseMat2 = glm::mat4(1.0f);
        baseMat2 = glm::translate(transMat2, glm::vec3(-1.85f, 1.95f, 1.16f));
        baseMat2 = glm::scale(baseMat2, glm::vec3(0.08f, 0.08f, 0.08f));


        glm::vec3 pos0 = transMat1 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 pos1 = transMat2 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 basePos0 = baseMat1 *  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 basePos1 = baseMat2 *  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 spotlight_vector1 = normalize(pos0 - basePos0);
        glm::vec3 spotlight_vector2 = normalize(pos1 - basePos1);

        // directional light

        objShader.setVec3("dirLight.direction", -1.0f, -0.2f, 0.0f);
        objShader.setVec3("dirLight.ambient", 0.15f, 0.05f, 0.20f);
        objShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.6f);
        objShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.7f);

        // point light 1

        objShader.setVec3("pointLights[0].position", pos0);
        objShader.setVec3("pointLights[0].ambient", 0.10f, 0.05f, 0.05f);
        objShader.setVec3("pointLights[0].diffuse", 0.8f, 0.6f, 0.6f);
        objShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 0.0f);
        objShader.setFloat("pointLights[0].constant", 1.0f);
        objShader.setFloat("pointLights[0].linear", 0.09);
        objShader.setFloat("pointLights[0].quadratic", 0.032);


        // point light 2

        objShader.setVec3("pointLights[1].position", pos1);
        objShader.setVec3("pointLights[1].ambient", 0.10f, 0.05f, 0.05f);
        objShader.setVec3("pointLights[1].diffuse", 0.8f, 0.6f, 0.6f);
        objShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 0.0f);
        objShader.setFloat("pointLights[1].constant", 1.0f);
        objShader.setFloat("pointLights[1].linear", 0.09);
        objShader.setFloat("pointLights[1].quadratic", 0.032);

        // spotLight 1

        objShader.setVec3("spotLights[0].position", basePos0);
        objShader.setVec3("spotLights[0].direction", spotlight_vector1);
        objShader.setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        if(programState->spotlight) {
            objShader.setVec3("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
            objShader.setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        }
        else{
            objShader.setVec3("spotLights[0].diffuse", 0.0f, 0.0f, 0.0f);
            objShader.setVec3("spotLights[0].specular", 0.0f, 0.0f, 0.0f);
        }
        objShader.setFloat("spotLights[0].constant", 1.0f);
        objShader.setFloat("spotLights[0].linear", 0.09);
        objShader.setFloat("spotLights[0].quadratic", 0.032);
        objShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(2.5f)));
        objShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(5.0f)));

        // spotLight 2

        objShader.setVec3("spotLights[1].position", basePos1);
        objShader.setVec3("spotLights[1].direction", spotlight_vector2);
        objShader.setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
        if(programState->spotlight) {
            objShader.setVec3("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
            objShader.setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
        }
        else{
            objShader.setVec3("spotLights[1].diffuse", 0.0f, 0.0f, 0.0f);
            objShader.setVec3("spotLights[1].specular", 0.0f, 0.0f, 0.0f);
        }
        objShader.setFloat("spotLights[1].constant", 1.0f);
        objShader.setFloat("spotLights[1].linear", 0.09);
        objShader.setFloat("spotLights[1].quadratic", 0.032);
        objShader.setFloat("spotLights[1].cutOff", glm::cos(glm::radians(2.5f)));
        objShader.setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(5.0f)));

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->princePosition);
        model = glm::rotate(model,sin(0.6f+currentFrame)*glm::radians(180.0f), glm::vec3(0,0.5f,0));
        model = glm::scale(model, glm::vec3(programState->princeScale));
        objShader.setMat4("model", model);
        princ.Draw(objShader);

        //object rendering end, start of light source rendering

        sourceShader.use();
        sourceShader.setMat4("projection", projection);
        sourceShader.setMat4("view", view);
        sourceShader.setBool("celShading", false);

        //using the transformation matrices from earlier
        sourceShader.setMat4("model", transMat1);
        rose.Draw(sourceShader);
        sourceShader.setMat4("model", transMat2);
        rose.Draw(sourceShader);


        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // render skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->princePosition);
        ImGui::DragFloat("Backpack scale", &programState->princeScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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