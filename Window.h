#pragma once
#include <vector>
#include "model.h"
#include "GenericObject.h"

void draw(window_t screen, Camera &cam, 
	std::vector<GenericObject*> &objects, 
	light_t &light, light_t light_inf, 
	double near, double view_angle);