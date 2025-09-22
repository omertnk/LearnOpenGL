#define STB_IMAGE_IMPLEMENTATION

#include <iostream>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <stb_image.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


#include "Shader.h"
#include "Camera.h"
#include "Model.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void EditObjectColor(glm::vec3& inColor);
void UpdateLightPos(glm::vec3& lightPos);
void MaterialProperties(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular, float& shininess);
void LightProperties(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool isCameraActive = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("res/shaders/shader.vs", "res/shaders/shader.fs");
	//Shader lightCubeShader("res/shaders/lightingShader.vs", "res/shaders/lightingShader.fs");

	stbi_set_flip_vertically_on_load(true);

	Model ourModel("assets/3d_models/backpack.obj");


#pragma region Setup ImGui
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Klavye
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Docking (opsiyonel)
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Multi-viewport (opsiyonel)

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

#pragma endregion



	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);


		ImGui::ShowDemoWindow();


		// Rendering ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	static bool cursorEnabled = false;
	static bool iKeyPressedLastFrame = false;

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

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		if (!iKeyPressedLastFrame) 
		{
			if (!cursorEnabled)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				isCameraActive = false;
				cursorEnabled = true;
			}
			else
			{
				glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				isCameraActive = true;
				cursorEnabled = false;
			}
		}
		iKeyPressedLastFrame = true;
	}
	else
	{
		iKeyPressedLastFrame = false;
	}

}

void EditObjectColor(glm::vec3& inColor)
{
	ImGui::Begin("Debug Window");

	ImGui::Text("Object Color");
	ImGui::ColorEdit3("Color", (float*)&inColor, ImGuiColorEditFlags_Float);

	ImGui::End();
}

void UpdateLightPos(glm::vec3& lightPos)
{
	ImGui::Begin("Debug Window");
	ImGui::NewLine();
	ImGui::Text("Light Position");

	float sliderWidth = 100.0f;
	ImGui::PushItemWidth(sliderWidth);

	ImGui::SliderFloat("##X", &lightPos.x, -10.0f, 100.0f, "X: %.1f", ImGuiSliderFlags_Logarithmic);
	ImGui::SameLine();
	ImGui::SliderFloat("##Y", &lightPos.y, -10.0f, 100.0f, "Y: %.1f", ImGuiSliderFlags_Logarithmic);
	ImGui::SameLine();
	ImGui::SliderFloat("##Z", &lightPos.z, -10.0f, 100.0f, "Z: %.1f", ImGuiSliderFlags_Logarithmic);

	ImGui::PopItemWidth();

	ImGui::End();
}

void MaterialProperties(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular, float& shininess)
{
	ImGui::Begin("Debug Window");
	ImGui::NewLine();

	ImGui::Text("Material Properties");

	// ambient
	//ImGui::DragFloat3("##m_ambient", &ambient.x, 0.01f, 0.f, 1.f);
	//ImGui::SameLine();
	//ImGui::Text("Ambient");

	//// diffuse
	//ImGui::DragFloat3("##m_diffuse", &diffuse.x, 0.01f, 0.f, 1.f);
	//ImGui::SameLine();
	//ImGui::Text("Diffuse");

	//// specular
	//ImGui::DragFloat3("##m_specular", &specular.x, 0.01f, 0.f, 1.f);
	//ImGui::SameLine();
	//ImGui::Text("Specular");

	// shininess
	ImGui::DragFloat("##shininess", &shininess, 0.01f, 1.f, 128.f);
	ImGui::SameLine();
	ImGui::Text("Shininess");

	ImGui::End();
}

void LightProperties(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular)
{
	ImGui::Begin("Debug Window");
	ImGui::NewLine();

	ImGui::Text("Light Properties");

	// ambient
	ImGui::DragFloat3("##ambient", &ambient.x, 0.01f, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::Text("Ambient");

	// diffuse
	ImGui::DragFloat3("##diffuse", &diffuse.x, 0.01f, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::Text("Diffuse");

	// specular
	ImGui::DragFloat3("##specular", &specular.x, 0.01f, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::Text("Specular");

	ImGui::End();
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;

	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (!isCameraActive)
	{
		firstMouse = true;
		return;
	}

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

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
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

