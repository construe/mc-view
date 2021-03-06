// SdlViewer.cpp : Defines the entry point for the console application.
//

#include "db.h"
#include "sdlpp.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <gl/glew.h>

#include <SDL.h>

#include <iostream>
#include <string>
#include <memory>
#include <random>
#include <iterator>
#include <algorithm>

#include <Windows.h>
#include <gl/GL.h>

using namespace std;
using namespace sdl;

class camera
{
	glm::lowp_vec3 position_;
	float orientation_x_;
	float orientation_y_;
	float x_near_corner_;
	float y_near_corner_;
	float z_near_;
	float z_far_;
	float speed_;
public:
	camera(float x_near_corner, float y_near_corner)
		: position_{ 0.f, 10.f, 0 }
		, orientation_x_(0.f)
		, orientation_y_(180.f)
		, x_near_corner_(x_near_corner)
		, y_near_corner_(y_near_corner)
		, z_near_(1)
		, z_far_(500)
		, speed_(1.f)
	{
	}

	void setup_projection_matrix() const
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-x_near_corner_, x_near_corner_, -y_near_corner_, y_near_corner_, z_near_, z_far_);
		glRotatef(orientation_x_, -1.f, 0.f, 0.f);
		glRotatef(orientation_y_, 0.f, -1.f, 0.f);
		glTranslatef(-position_.x, -position_.y, -position_.z);
	}

	void set_near_size(float x, float y)
	{
		x_near_corner_ = x;
		y_near_corner_ = y;
	}

	void set_near_distance(float distance)
	{
		z_near_ = distance;
	}

	void process_event(const SDL_Event& event)
	{
		if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_LMASK)
		{
			orientation_x_ += -static_cast<float>(event.motion.yrel) * 0.1f;
			orientation_y_ += -static_cast<float>(event.motion.xrel) * 0.1f;
		}

		if (event.type == SDL_KEYDOWN)
		{
			bool direction_set = true;
			glm::lowp_vec3 direction;
			switch (event.key.keysym.sym)
			{
			case SDLK_w: direction = { 0.f, 0.f, -1.f }; break;
			case SDLK_s: direction = { 0.f, 0.f, 1.f }; break;
			case SDLK_a: direction = { -1.f, 0.f, 0.f }; break;
			case SDLK_d: direction = { 1.f, 0.f, 0.f }; break;
			default: direction_set = false; break;
			}

			if (direction_set)
			{
				direction = glm::rotateX(direction, glm::radians(orientation_x_));
				direction = glm::rotateY(direction, glm::radians(orientation_y_));
				position_ += direction;
			}
		}
	}
};

class my_app
{
	Window window_;
	camera camera_;
	SDL_GLContext gl_context_;
	std::vector<point> points_;
	std::vector<point> highlighted_points_;

public:
	my_app() : window_("My app"
		, SDL_WINDOWPOS_UNDEFINED
		, SDL_WINDOWPOS_UNDEFINED
		, 1200, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
	, camera_(2.f, 1.f)
	{
		gl_context_ = window_.glCreateContext();
		GLenum glew_result = glewInit();
		if (glew_result != GLEW_OK)
			throw runtime_error(reinterpret_cast<const char*>(glewGetErrorString(glew_result)));
		
		points_ = load_points();

		copy_if(points_.begin(), points_.end(), back_inserter(highlighted_points_), [](auto p) { return p.value == 8; });
		
		cout << "number of points: " << points_.size() << endl;
	}

	void draw()
	{
		window_.glMakeCurrent(gl_context_);

		camera_.setup_projection_matrix();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glScalef(0.1f, 0.1f, 0.1f);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glPointSize(1.f);
		glColorPointer(3, GL_FLOAT, sizeof(points_[0]), &points_[0].color);
		glVertexPointer(3, GL_FLOAT, sizeof(points_[0]), &points_[0].position);
		glDrawArrays(GL_POINTS, 0, points_.size());

		glPointSize(45.f);
		glColorPointer(3, GL_FLOAT, sizeof(highlighted_points_[0]), &highlighted_points_[0].color);
		glVertexPointer(3, GL_FLOAT, sizeof(highlighted_points_[0]), &highlighted_points_[0].position);
		glDrawArrays(GL_POINTS, 0, highlighted_points_.size());

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glBegin(GL_LINES);
		glColor3f(0.f, 0.f, 0.f);
		glVertex3f(-100.f, 0.f, 0.f);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(100.f, 0.f, 0.f);
		glColor3f(0.f, 0.f, 0.f);
		glVertex3f(0.f, -100.f, 0.f);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(0.f, 100.f, 0.f);
		glColor3f(0.f, 0.f, 0.f);
		glVertex3f(0.f, 0.f, -100.f);
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(0.f, 0.f, 100.f);
		glEnd();

		window_.glSwap();
	}

	void process_event(const SDL_Event& event)
	{
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			float x_size = static_cast<float>(event.window.data1) / 800.f;
			float y_size = static_cast<float>(event.window.data2) / 800.f;
			camera_.set_near_size(x_size, y_size);

			window_.glMakeCurrent(gl_context_);
			glViewport(0, 0, event.window.data1, event.window.data2);
		}

		camera_.process_event(event);
	}
};

int my_main()
{
	Init init(SDL_INIT_VIDEO);
	
	my_app app;

	bool stopping = false;

	while (!stopping)
	{
		if (SDL_WaitEvent(nullptr) == 0)
			stopping = true;

		SDL_Event event;
		while(!stopping && SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
				stopping = true;
			app.process_event(event);
		}

		if (!stopping)
			app.draw();
	}

	return 0;
}

int SDL_main(int argc, char* argv[])
{
	try
	{
		return my_main();
	}
	catch(const std::runtime_error& error)
	{
		std::cerr << error.what() << std::endl;
		cin.get();
		return -1;
	}
}