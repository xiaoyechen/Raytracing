#include <vector>
#include "Camera.h"
#include "Light.h"
#include "GenericObject.h"

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer,
	std::vector<GenericObject*> &objects,
	Light &light, Light &light_inf,
	double near, double near_h);

Matrix<double>* projTrans(Matrix<double> &P);