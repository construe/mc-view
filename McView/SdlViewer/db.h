#pragma once

#include <vector>

struct point
{
	float color[3];
	float position[3];
	char value;
};

std::vector<point> load_points();