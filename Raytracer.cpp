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

void calculateIntensityInfinite(GenericObject &obj, Matrix<double> &intersection, Matrix<double> &cam, light_t &light, unsigned hit_type, double *buffer_array)
{
	Matrix<double> surface_to_cam = *cam.subtract(intersection)->normalize();
	Matrix<double> surface_normal = *obj.calculateSurfaceNormal(intersection, hit_type);

	Matrix<double> light_negative = *light.position.multiplyDot(-1);
	double surface_normal_mag = surface_normal.normal();
	Matrix<double> spec_r_inf = *light_negative.add(*surface_normal.multiplyDot(2 * light.position.multiplyDot(surface_normal) / (surface_normal_mag*surface_normal_mag)));

	light_negative.Erase();

	// calculate diffuse and specular intensities
	double Id_inf = light.position.multiplyDot(surface_normal) / (surface_normal_mag*light.position.normal());
	double Is_inf = pow(spec_r_inf.multiplyDot(surface_to_cam) / (spec_r_inf.normal()*surface_to_cam.normal()), obj.getFallout());

	if (Id_inf < 0) Id_inf = 0;
	if (Is_inf < 0) Is_inf = 0;

	buffer_array[COLOR_R] += light.color.r*LI_INF*(Id_inf*obj.getDiffuse(COLOR_R) + Id_inf*obj.getSpecular(COLOR_R));
	buffer_array[COLOR_G] += light.color.g*LI_INF*(Id_inf*obj.getDiffuse(COLOR_G) + Id_inf*obj.getSpecular(COLOR_G));
	buffer_array[COLOR_B] += light.color.b*LI_INF*(Id_inf*obj.getDiffuse(COLOR_B) + Id_inf*obj.getSpecular(COLOR_B));
}

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer, 
	std::vector<GenericObject> &objects,
	light_t &light, light_t &light_inf,
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
				Matrix<double> surface_to_light = *light.position.subtract(rayOnObj)->normalize();

				double super_res_buffer[SUPER_RES][N_CHANNELS];
				super_res_buffer[m][COLOR_R] = objects[tmin_idx].getAbmient(COLOR_R);
				super_res_buffer[m][COLOR_G] = objects[tmin_idx].getAbmient(COLOR_G);
				super_res_buffer[m][COLOR_B] = objects[tmin_idx].getAbmient(COLOR_B);

				unsigned rayhit_type = objects[tmin_idx].getRayHit().enter_type;
				
				unsigned idx = 0;
				for (idx = 0; idx < objects.size(); ++idx)
				{
					if (idx == tmin_idx) continue;

					objects[idx].setRayHit(rayOnObj, light_inf.position);

					if (!isinf(objects[idx].getRayHit().enter))
						break;
				}

				if (idx >= objects.size())
					calculateIntensityInfinite(objects[tmin_idx], rayOnObj, *cam->getE(), light_inf, rayhit_type, super_res_buffer[m]);

			}

		}
	}
}