/*
* Copyright (c) 2016-2019 Irlan Robson https://irlanrobson.github.io
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

#ifndef MODEL_H
#define MODEL_H

#include <testbed/framework/draw.h>
#include <bounce/common/profiler.h>

extern b3FrameAllocator* g_frameAllocator;

extern b3Profiler* g_profiler;

// Set to 1 to write profile events into a .json file. Set to 0 otherwise.
#define PROFILE_JSON 0

#if (PROFILE_JSON == 1)
#include <testbed/framework/json_profiler.h>
#endif

class Test;

class ViewModel;

class Model
{
public:
	Model();

	~Model();

	void Action_SaveTest();
	void Action_SetTest();
	void Action_PlayPause();
	void Action_SinglePlay();
	void Action_ResetCamera();

	void Command_Press_Key(int button);
	void Command_Release_Key(int button);
	void Command_Press_Mouse_Left(const b3Vec2& ps);
	void Command_Release_Mouse_Left(const b3Vec2& ps);
	void Command_Move_Cursor(const b3Vec2& ps);

	void Command_ResizeCamera(scalar w, scalar h);
	void Command_RotateCameraX(scalar angle);
	void Command_RotateCameraY(scalar angle);
	void Command_TranslateCameraX(scalar d);
	void Command_TranslateCameraY(scalar d);
	void Command_ZoomCamera(scalar d);

	void Update();

#if (PROFILE_JSON == 1)
	void UpdateJson();
#endif

	bool IsPaused() const { return m_pause; }
private:
	friend class ViewModel;

	ViewModel* m_viewModel;

	Draw m_draw;
	Camera m_camera;
	b3FrameAllocator m_frame;
	b3Profiler m_profiler;

#if (PROFILE_JSON == 1)
	JsonProfiler m_jsonProfiler;
#endif

	Test* m_test;
	bool m_setTest;
	bool m_pause;
	bool m_singlePlay;
};

inline void Model::Action_SetTest()
{
	m_setTest = true;
}

inline void Model::Action_PlayPause()
{
	m_pause = !m_pause;
}

inline void Model::Action_SinglePlay()
{
	m_pause = true;
	m_singlePlay = true;
}

inline void Model::Action_ResetCamera()
{
	m_camera.m_q = b3QuatRotationX(-0.125f * B3_PI);
	
	b3Quat d = b3QuatRotationY(0.125f * B3_PI);
	
	m_camera.m_q = d * m_camera.m_q;
	m_camera.m_q.Normalize();
	m_camera.m_center.SetZero();
	m_camera.m_zoom = 50.0f;
}

inline void Model::Command_ResizeCamera(scalar w, scalar h)
{
	m_camera.m_width = w;
	m_camera.m_height = h;
}

inline void Model::Command_RotateCameraX(scalar angle)
{
	b3Quat d = b3QuatRotationX(angle);

	m_camera.m_q = m_camera.m_q * d;
	m_camera.m_q.Normalize();
}

inline void Model::Command_RotateCameraY(scalar angle)
{
	b3Quat d = b3QuatRotationY(angle);

	m_camera.m_q = d * m_camera.m_q;
	m_camera.m_q.Normalize();
}

inline void Model::Command_TranslateCameraX(scalar d)
{
	b3Transform transform = m_camera.BuildWorldTransform();

	m_camera.m_center += d * transform.rotation.GetXAxis();
}

inline void Model::Command_TranslateCameraY(scalar d)
{
	b3Transform transform = m_camera.BuildWorldTransform();

	m_camera.m_center += d * transform.rotation.GetYAxis();
}

inline void Model::Command_ZoomCamera(scalar d)
{
	m_camera.m_zoom += d;
}

#endif