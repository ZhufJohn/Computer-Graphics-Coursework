#include <iostream>
#include <cmath>
#include <random>

#include <common/common.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>
#include <common/terrain.hpp>
#include <common/skyBox.hpp>
#include <common/sphere.hpp>

const int windowWidth = 1024;
const int windowHeight = 768;
Camera g_Camera(windowWidth, windowHeight);

float g_deltaFrame = 0;
float g_lastFrame = 0;

glm::mat4 g_rockTransform0;
glm::mat4 g_rockTransform1;
glm::mat4 g_rockTransform2;
glm::mat4 g_rockTransform3;

glm::mat4 g_manTransform;

glm::mat4 g_terrainTransform;

glm::mat4 g_phongSphereTransform;

// 随机数种子 用于生成随机颜色
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 1.0);

float pointLightRotate0 = 0.0f;
glm::vec3 pointLightPosition0 = glm::vec3(5, 25, 0);
bool isPointLightMoving = true;
glm::mat4 pointLightTransform0 = glm::mat4();
glm::vec3 pointLightColor0 = glm::vec3(1, 0, 0);

float dirLightRotate0 = 0.0f;
glm::vec3 dirLightInitDirection = glm::vec3(0, 1, 0);

// Function prototypes
void keyboardInput(GLFWwindow *window);
void mouseMove(GLFWwindow* window, double x, double y);
void mouseClick(GLFWwindow* window, int button, int action, int mods);
void keyClick(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseScroll(GLFWwindow* window, double xOffset, double yOffset);

void printHelp() {
	std::cout << "\npress 'h' to print this message again.\n"
		<< "press w/a/s/d to move camera\n"
		<< "hold left mouse to turn around\n"
		<< "press '+' or '-' to change the direction of the dir light.\n"
		<< "press 'p' to pause or start point light moving.\n"
		<< "press 'c' to change point light color to a random.\n"
		<< "press 'm' to change the fly mode of the free camera.\n"
		<< "press ESC to quit.\n";
}

int main( void )
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(windowWidth, windowHeight, "Computer Graphics Coursework", NULL, NULL);
    
    if( window == NULL ){
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================
    
    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetMouseButtonCallback(window, mouseClick);
    glfwSetKeyCallback(window, keyClick);
    glfwSetScrollCallback(window, mouseScroll);

    Model rock("../assets/models/rock/rock.obj");
	rock.addTexture("../assets/models/rock/Rock-Texture-Surface.jpg", "diffuse");
	rock.addTexture("../assets/textures/gray.jpg", "specular");
    unsigned int modelShader = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
	g_rockTransform0 = glm::translate(glm::mat4(), glm::vec3(0, 23, -5));
	g_rockTransform1 = glm::translate(glm::mat4(), glm::vec3(0, 22, 5));
	g_rockTransform2 = glm::translate(glm::mat4(), glm::vec3(5, 22, 0));
	g_rockTransform3 = glm::translate(glm::mat4(), glm::vec3(-5, 22, 0));

	g_phongSphereTransform = glm::translate(glm::mat4(), glm::vec3(0, 25, 5));

    Model man("../assets/models/cyborg/cyborg.obj");
    man.addTexture("../assets/models/cyborg/cyborg_diffuse.png", "diffuse");
    man.addTexture("../assets/models/cyborg/cyborg_specular.png", "specular");
    g_manTransform = glm::translate(glm::mat4(), glm::vec3(5, 22, 5));

    Terrain terrain(30.0f, 2.0f);
    unsigned int terrainShader = LoadShaders("terrainVS.glsl", "terrainFS.glsl");
	g_Camera.terrain = &terrain;

	SkyBox skyBox;
    unsigned int skyBoxShader = LoadShaders("skyBoxVS.glsl", "skyBoxFS.glsl");

	Sphere sphere;
	sphere.initTextures("../assets/textures/sphere_diffuse.png", "../assets/textures/sphere_specular.png", "../assets/textures/sphere_normal.png");
	unsigned int phongShader = LoadShaders("phongVS.glsl", "phongFS.glsl");

    // Point Lights
    PointLight pointLight0;
    pointLight0.diffuseIntensity = 0.5;
    unsigned int lightShader = LoadShaders("lightVS.glsl", "lightFS.glsl");
    // Directional Light
    Light dirLight0;
    dirLight0.lightColor = glm::vec3(1);

	glEnable(GL_DEPTH_TEST);

	printHelp();

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
		float currentFrame = glfwGetTime();
        g_deltaFrame = currentFrame - g_lastFrame;
		g_lastFrame = currentFrame;
		glfwPollEvents();

        // Get inputs
        keyboardInput(window);

        g_Camera.update(currentFrame, g_deltaFrame);

		pointLight0.lightColor = pointLightColor0;
		if (isPointLightMoving)
		{
			pointLightRotate0++;
			glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.1));
			glm::mat3 rotate = maths::rotate(-pointLightRotate0, glm::vec3(0, 1, 0));
			glm::mat4 translate = glm::translate(glm::mat4(), pointLightPosition0);
			pointLightTransform0 = glm::mat4(rotate) * translate * scale;
			pointLight0.lightPosition = pointLightPosition0 * maths::rotate(pointLightRotate0, glm::vec3(0, 1, 0));
		}

		glm::mat3 rotateDirLight = maths::rotate(dirLightRotate0, glm::vec3(0, 0, 1));
		dirLight0.lightPosition = glm::normalize(dirLightInitDirection * rotateDirLight);
        
        // Clear the window
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render rocks
        glUseProgram(modelShader);
        // Set Light parameters
		glUniform3f(glGetUniformLocation(modelShader, "viewPos"), g_Camera.position.x, g_Camera.position.y, g_Camera.position.z);
		glUniform3f(glGetUniformLocation(modelShader, "pointLights[0].color"), pointLight0.lightColor.x, pointLight0.lightColor.y, pointLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(modelShader, "pointLights[0].position"), pointLight0.lightPosition.x, pointLight0.lightPosition.y, pointLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(modelShader, "pointLights[0].ambientIntensity"), pointLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(modelShader, "pointLights[0].diffuseIntensity"), pointLight0.diffuseIntensity);
		glUniform1f(glGetUniformLocation(modelShader, "pointLights[0].constant"), pointLight0.constantFactor);
		glUniform1f(glGetUniformLocation(modelShader, "pointLights[0].linear"), pointLight0.linearFactor);
		glUniform1f(glGetUniformLocation(modelShader, "pointLights[0].exp"), pointLight0.expFactor);

		glUniform3f(glGetUniformLocation(modelShader, "dirLight.color"), dirLight0.lightColor.x, dirLight0.lightColor.y, dirLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(modelShader, "dirLight.direction"), dirLight0.lightPosition.x, dirLight0.lightPosition.y, dirLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(modelShader, "dirLight.ambientIntensity"), dirLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(modelShader, "dirLight.diffuseIntensity"), dirLight0.diffuseIntensity);

		glUniformMatrix4fv(glGetUniformLocation(modelShader, "view"), 1, GL_FALSE, (float*)glm::value_ptr(g_Camera.getViewTransform()));
		glUniformMatrix4fv(glGetUniformLocation(modelShader, "projection"), 1, GL_FALSE, g_Camera.projTransform);
        glUniformMatrix4fv(glGetUniformLocation(modelShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_rockTransform0));
        rock.draw(modelShader);
		glUniformMatrix4fv(glGetUniformLocation(modelShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_rockTransform1));
        rock.draw(modelShader);
		glUniformMatrix4fv(glGetUniformLocation(modelShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_rockTransform2));
        rock.draw(modelShader);
		glUniformMatrix4fv(glGetUniformLocation(modelShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_rockTransform3));
        rock.draw(modelShader);

        // Render man
		glUniformMatrix4fv(glGetUniformLocation(modelShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_manTransform));
        man.draw(modelShader);

        // Render terrain
		glUseProgram(terrainShader);
		// Set Light parameters
		glUniform3f(glGetUniformLocation(terrainShader, "viewPos"), g_Camera.position.x, g_Camera.position.y, g_Camera.position.z);
		glUniform3f(glGetUniformLocation(terrainShader, "pointLights[0].color"), pointLight0.lightColor.x, pointLight0.lightColor.y, pointLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(terrainShader, "pointLights[0].position"), pointLight0.lightPosition.x, pointLight0.lightPosition.y, pointLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(terrainShader, "pointLights[0].ambientIntensity"), pointLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(terrainShader, "pointLights[0].diffuseIntensity"), pointLight0.diffuseIntensity);
		glUniform1f(glGetUniformLocation(terrainShader, "pointLights[0].constant"), pointLight0.constantFactor);
		glUniform1f(glGetUniformLocation(terrainShader, "pointLights[0].linear"), pointLight0.linearFactor);
		glUniform1f(glGetUniformLocation(terrainShader, "pointLights[0].exp"), pointLight0.expFactor);

		glUniform3f(glGetUniformLocation(terrainShader, "dirLight.color"), dirLight0.lightColor.x, dirLight0.lightColor.y, dirLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(terrainShader, "dirLight.direction"), dirLight0.lightPosition.x, dirLight0.lightPosition.y, dirLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(terrainShader, "dirLight.ambientIntensity"), dirLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(terrainShader, "dirLight.diffuseIntensity"), dirLight0.diffuseIntensity);

		glUniformMatrix4fv(glGetUniformLocation(terrainShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_terrainTransform));
		glUniformMatrix4fv(glGetUniformLocation(terrainShader, "view"), 1, GL_FALSE, (float*)glm::value_ptr(g_Camera.getViewTransform()));
		glUniformMatrix4fv(glGetUniformLocation(terrainShader, "projection"), 1, GL_FALSE, g_Camera.projTransform);
		terrain.draw(terrainShader);

		// Render Sphere using phong lighting
		glUseProgram(phongShader);
		// Set Light parameters
		glUniform3f(glGetUniformLocation(phongShader, "viewPos"), g_Camera.position.x, g_Camera.position.y, g_Camera.position.z);
		glUniform3f(glGetUniformLocation(phongShader, "pointLights[0].color"), pointLight0.lightColor.x, pointLight0.lightColor.y, pointLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(phongShader, "pointLights[0].position"), pointLight0.lightPosition.x, pointLight0.lightPosition.y, pointLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(phongShader, "pointLights[0].ambientIntensity"), pointLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(phongShader, "pointLights[0].diffuseIntensity"), pointLight0.diffuseIntensity);
		glUniform1f(glGetUniformLocation(phongShader, "pointLights[0].constant"), pointLight0.constantFactor);
		glUniform1f(glGetUniformLocation(phongShader, "pointLights[0].linear"), pointLight0.linearFactor);
		glUniform1f(glGetUniformLocation(phongShader, "pointLights[0].exp"), pointLight0.expFactor);

		glUniform3f(glGetUniformLocation(phongShader, "dirLight.color"), dirLight0.lightColor.x, dirLight0.lightColor.y, dirLight0.lightColor.z);
		glUniform3f(glGetUniformLocation(phongShader, "dirLight.direction"), dirLight0.lightPosition.x, dirLight0.lightPosition.y, dirLight0.lightPosition.z);
		glUniform1f(glGetUniformLocation(phongShader, "dirLight.ambientIntensity"), dirLight0.ambientIntensity);
		glUniform1f(glGetUniformLocation(phongShader, "dirLight.diffuseIntensity"), dirLight0.diffuseIntensity);

		glUniformMatrix4fv(glGetUniformLocation(phongShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(g_phongSphereTransform));
		glUniformMatrix4fv(glGetUniformLocation(phongShader, "view"), 1, GL_FALSE, (float*)glm::value_ptr(g_Camera.getViewTransform()));
		glUniformMatrix4fv(glGetUniformLocation(phongShader, "projection"), 1, GL_FALSE, g_Camera.projTransform);
		sphere.drawPhong(phongShader);

		//Render lights
		glUseProgram(lightShader);
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(pointLightTransform0));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "view"), 1, GL_FALSE, (float*)glm::value_ptr(g_Camera.getViewTransform()));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "projection"), 1, GL_FALSE, g_Camera.projTransform);
		pointLight0.draw(lightShader);

        //Render skybox
		GLint oldDepthFuncMode;
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
        glDepthFunc(GL_LEQUAL);
		glUseProgram(skyBoxShader);
        glm::mat4 skyboxTransform = glm::translate(glm::mat4(), g_Camera.position);
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader, "model"), 1, GL_FALSE, (float*)glm::value_ptr(skyboxTransform));
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader, "view"), 1, GL_FALSE, (float*)glm::value_ptr(g_Camera.getViewTransform()));
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader, "projection"), 1, GL_FALSE, g_Camera.projTransform);
        skyBox.draw(skyBoxShader);
        glDepthFunc(oldDepthFuncMode);
        
        // Swap buffers
        glfwSwapBuffers(window);
    }
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_UP);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_DOWN);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_RIGHT);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_W);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_A);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_S);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_Camera.onKeyboard(GLFW_KEY_D);
	}
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
	{
		dirLightRotate0++;
	}
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
	{
		dirLightRotate0--;
	}
}

void mouseMove(GLFWwindow* window, double x, double y)
{
    g_Camera.onMouseMove(x, y);
}

void mouseClick(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (action == GLFW_PRESS)
        {
			g_Camera.onMouseDown();
		}
		else if (action == GLFW_RELEASE)
        {
			g_Camera.onMouseUp();
		}
	}
}

void keyClick(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool pressed = (action == GLFW_PRESS);

	g_Camera.onKeyboard(key);

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		printHelp();
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		isPointLightMoving = !isPointLightMoving;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		g_Camera.useConstraints = !g_Camera.useConstraints;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		pointLightColor0 = glm::vec3(dis(gen), dis(gen), dis(gen));
	}
}

void mouseScroll(GLFWwindow* window, double xOffset, double yOffset)
{
    g_Camera.onMouseScroll(xOffset, yOffset);
}