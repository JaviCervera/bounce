/*
* Copyright (c) 2016-2016 Irlan Robson http://www.irlan.net
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <testbed/framework/view.h>
#include <testbed/framework/model.h>

#if defined (U_OPENGL_2)
#include <imgui/imgui_impl_glfw_gl2.h>
#elif defined (U_OPENGL_4)
#include <imgui/imgui_impl_glfw_gl3.h>
#else
#endif

#include <glfw/glfw3.h>

static bool GetTestName(void* userData, int idx, const char** name)
{
	assert(u32(idx) < g_testCount);
	*name = g_tests[idx].name;
	return true;
}

static inline bool ImGui_GLFW_GL_Init(GLFWwindow* w, bool install_callbacks)
{

#if defined(U_OPENGL_2)

	return ImGui_ImplGlfwGL2_Init(w, install_callbacks);

#elif defined(U_OPENGL_4)

	return ImGui_ImplGlfwGL3_Init(w, install_callbacks);

#else

	// error

#endif

}

static inline void ImGui_GLFW_GL_Shutdown()
{

#if defined(U_OPENGL_2)

	ImGui_ImplGlfwGL2_Shutdown();

#elif defined(U_OPENGL_4)

	ImGui_ImplGlfwGL3_Shutdown();

#else

	// error

#endif

}

static inline void ImGui_GLFW_GL_NewFrame()
{

#if defined(U_OPENGL_2)

	ImGui_ImplGlfwGL2_NewFrame();

#elif defined(U_OPENGL_4)

	ImGui_ImplGlfwGL3_NewFrame();

#else

	// error

#endif

}

static inline void ImGui_GLFW_GL_RenderDrawData(ImDrawData* draw_data)
{

#if defined(U_OPENGL_2)

	ImGui_ImplGlfwGL2_RenderDrawData(draw_data);

#elif defined(U_OPENGL_4)

	ImGui_ImplGlfwGL3_RenderDrawData(draw_data);

#else

	// error

#endif

}

View::View(GLFWwindow* window, Model* model) : m_presenter(model, this)
{
	m_window = window;

	// Create UI
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = NULL;
	io.Fonts[0].AddFontDefault();

	ImGui_GLFW_GL_Init(m_window, false);

	ImGui::StyleColorsDark();

	m_leftDown = false;
	m_rightDown = false;
	m_shiftDown = false;
	m_ps0.SetZero();
}

View::~View()
{
	// Destroy UI
	ImGui_GLFW_GL_Shutdown();

	ImGui::DestroyContext();
}

void View::Event_SetWindowSize(int w, int h)
{
	m_presenter.Event_SetWindowSize(float32(w), float32(h));
}

void View::Event_Press_Key(int button)
{
	if (button == GLFW_KEY_LEFT_SHIFT)
	{
		m_shiftDown = true;
	}

	m_presenter.Event_Press_Key(button);
}

void View::Event_Release_Key(int button)
{
	if (button == GLFW_KEY_LEFT_SHIFT)
	{
		m_shiftDown = false;
	}

	m_presenter.Event_Release_Key(button);
}

void View::Event_Press_Mouse(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_leftDown = true;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		m_rightDown = true;
	}
	
	m_presenter.Event_Press_Mouse(button);
}

void View::Event_Release_Mouse(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_leftDown = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		m_rightDown = false;
	}

	m_presenter.Event_Release_Mouse(button);
}

void View::Event_Move_Cursor(float x, float y)
{
	b3Vec2 ps(x, y);

	m_presenter.Event_Move_Cursor(ps.x, ps.y);
	
	m_ps0 = ps;
}

void View::Event_Scroll(float dx, float dy)
{
	m_presenter.Event_Scroll(dx, dy);
}

void View::Command_PreDraw()
{
	ImGui_GLFW_GL_NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
}

void View::Command_Draw()
{
	Model* model = m_presenter.m_model;
	Settings& settings = model->m_settings;
	TestSettings& testSettings = model->m_testSettings;
	Camera& camera = model->m_camera;

	bool openControls = false;
	bool openAbout = false;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				model->Action_SaveTest();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				glfwSetWindowShouldClose(m_window, true);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Profile", "", &settings.drawProfile);
			ImGui::MenuItem("Statistics", "", &settings.drawStats);

			ImGui::Separator();

			ImGui::MenuItem("Points", "", &settings.drawPoints);
			ImGui::MenuItem("Lines", "", &settings.drawLines);
			ImGui::MenuItem("Triangles", "", &settings.drawTriangles);

			ImGui::Separator();
			
			ImGui::MenuItem("Reference Grid", "", &settings.drawGrid);

			ImGui::Separator();

			ImGui::MenuItem("Center of Masses", "", &testSettings.drawCenterOfMasses);
			ImGui::MenuItem("Bounding Boxes", "", &testSettings.drawBounds);
			ImGui::MenuItem("Shapes", "", &testSettings.drawShapes);
			ImGui::MenuItem("Joints", "", &testSettings.drawJoints);
			ImGui::MenuItem("Contact Points", "", &testSettings.drawContactPoints);
			ImGui::MenuItem("Contact Normals", "", &testSettings.drawContactNormals);
			ImGui::MenuItem("Contact Tangents", "", &testSettings.drawContactTangents);
			ImGui::MenuItem("Contact Polygons", "", &testSettings.drawContactPolygons);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Controls"))
			{
				openControls = true;
			}
			
			if (ImGui::MenuItem("About"))
			{
				openAbout = true;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	
	if (openControls)
	{
		ImGui::OpenPopup("Controls");
	}

	if (openAbout)
	{
		ImGui::OpenPopup("About Bounce Testbed");
	}

	ImVec2 buttonSize(-1.0f, 0.0f);
	
	if (ImGui::BeginPopupModal("Controls", NULL, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Rotate the scene using LSHIFT + LMB");
		ImGui::Text("Translate the scene using LSHIFT + RMB");
		ImGui::Text("Zoom in / out the scene using LSHIFT + Mouse Wheel");

		if (ImGui::Button("OK", buttonSize))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("About Bounce Testbed", NULL, ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoResize))
	{
		extern b3Version b3_version;

		ImGui::Text("Bounce Testbed");
		ImGui::Text("Version %d.%d.%d", b3_version.major, b3_version.minor, b3_version.revision);
		ImGui::Text("Copyright (c) Irlan Robson");
		ImGui::Text("https://github.com/irlanrobson/bounce");

		if (ImGui::Button("OK", buttonSize))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(g_camera->m_width, 20.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

	ImGui::Begin("##ToolBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		ImGui::PushItemWidth(250.0f);

		ImGui::Separator();
		
		if (ImGui::Combo("##Test", &settings.testID, GetTestName, NULL, g_testCount, g_testCount))
		{
			model->Action_SelectTest(settings.testID);
		}

		ImGui::PopItemWidth();

		ImVec2 menuButtonSize(100.0f, 0.0f);

		ImGui::Separator();

		if (ImGui::Button("Previous", menuButtonSize))
		{
			model->Action_PreviousTest();
		}

		if (ImGui::Button("Next", menuButtonSize))
		{
			model->Action_NextTest();
		}

		ImGui::Separator();

		if (ImGui::Button("Play/Pause", menuButtonSize))
		{
			model->Action_PlayPause();
		}

		if (ImGui::Button("Single Step", menuButtonSize))
		{
			model->Action_SingleStep();
		}

		ImGui::Separator();

		if (ImGui::Button("Restart", menuButtonSize))
		{
			model->Action_RestartTest();
		}

		ImGui::Separator();

		if (ImGui::Button("Reset Camera", menuButtonSize))
		{
			model->Action_DefaultCamera();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
	
	ImGui::PopStyleVar();

	ImGui::SetNextWindowPos(ImVec2(camera.m_width - 250.0f, 40.0f));
	ImGui::SetNextWindowSize(ImVec2(250.0f, camera.m_height - 40.0f));
	ImGui::Begin("Test Settings", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::PushItemWidth(-1.0f);

	ImGui::Text("Hertz");
	ImGui::SliderFloat("##Hertz", &testSettings.hertz, 0.0f, 240.0f, "%.1f");

	ImGui::Text("Velocity Iterations");
	ImGui::SliderInt("##Velocity Iterations", &testSettings.velocityIterations, 0, 50);

	ImGui::Text("Position Iterations");
	ImGui::SliderInt("##Position Iterations", &testSettings.positionIterations, 0, 50);

	ImGui::Checkbox("Sleep", &testSettings.sleep);
	ImGui::Checkbox("Convex Cache", &testSettings.convexCache);
	ImGui::Checkbox("Warm Start", &testSettings.warmStart);

	ImGui::PopItemWidth();

	ImGui::End();
}

void View::Command_PostDraw()
{
	ImGui::PopStyleVar();

	ImGui::Render();

	ImGui_GLFW_GL_RenderDrawData(ImGui::GetDrawData());
}