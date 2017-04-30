#include <iostream>
#include <vector>
#include "Camera.h"
#include "GenericObject.h"

unsigned finObjIdx(std::vector<GenericObject> &objects, unsigned num, unsigned target_type)
{
	unsigned i = 0;
	while (i < num && objects[i].getType() != target_type)
		++i;

	return i;
}

unsigned findMinHitIdx(std::vector<GenericObject> &objects)
{
	if (objects.size() == 1) return 0;

	unsigned min_idx = 0;
	for (unsigned i = 1; i < objects.size(); ++i)
	{
		if (objects[i].getRayHit().enter < objects[min_idx].getRayHit().enter)
			min_idx = i;
	}

	return min_idx;
}

Matrix<double>* projTrans(Matrix<double> &P)
{
	Matrix<double>* Q = new Matrix<double>(4,1);

	if (P.getLength() == 4 && P.getHeight() == 1)
	{
		(*Q)(1, 1) = P(1, 1) / P(4, 1);
		(*Q)(2, 1) = P(2, 1) / P(4, 1);
		(*Q)(3, 1) = P(3, 1) / P(4, 1);
		(*Q)(4, 1) = 1;
	}

	return Q;
}

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer, std::vector<GenericObject> &objects,
			  double near, double near_h)
{
	double near_w = near_h * (w.width / w.height);

	Matrix<double> ray(4, 1);

	// process each pixel on near plane
	for (unsigned row = 0; row < w.height; ++row)
	{
		for (unsigned col = 0; col < w.width; ++col)
		{
			for (unsigned m = 0; m < SUPER_RES; ++m)
			{
				// coordinates of columns & rows on near plane
				double uc, vr;
				if (m == 0 || m == 1)
					uc = near_w*(2.0 * (2 * col - 1) / (2.0*w.width) - 1);
				else 
					uc = near_w*(2.0 * (2 * col + 1) / (2.0*w.width) - 1);
				if (m == 1 || m == 3)
					vr = near_h*(2.0*(2 * row + 1) / (2.0*w.height) - 1);
				else
					vr = near_h*(2.0*(2 * row - 1) / (2.0*w.height) - 1);

				//direction from camera to current pixel
				Matrix<double> direction = *cam->getN()->multiplyDot(-near)->add(*cam->getU()->multiplyDot(uc)->add(*cam->getV()->multiplyDot(vr)));
				direction(4, 1) = 0;
				Matrix<double>* dir_normalized = direction.normalize();
				direction.Erase();
				
				for (auto obj = objects.begin(); obj != objects.end(); ++obj)
					obj->setRayHit(*cam->getE(), direction);

				unsigned tmin_idx = findMinHitIdx(objects);

				Matrix<double> rayOnObj = *cam->getE()->add(*direction.multiplyDot(objects[tmin_idx].getRayHit().enter));
				Matrix<double> surface_to_light = 
			}

		}
	}
}