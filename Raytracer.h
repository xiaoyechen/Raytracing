#include <vector>
#include "Camera.h"
#include "GenericObject.h"

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer,
	std::vector<GenericObject> &objects,
	light_t &light, light_t &light_inf,
	double near, double near_h);