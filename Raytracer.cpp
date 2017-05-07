#include <iostream>
#include <cmath>
#include "Raytracer.h"

unsigned findMinHitIdx(std::vector<GenericObject*> &objects)
{
	if (objects.size() == 1) return 0;

	unsigned min_idx = 0;
	for (unsigned i = 1; i < objects.size(); ++i)
	{
		if (objects[i]->getRayHit().enter < objects[min_idx]->getRayHit().enter)
			min_idx = i;
	}

	return min_idx;
}

Matrix<double>* projTrans(Matrix<double> &P)
{
	Matrix<double>* Q = new Matrix<double>(4,1);

	(*Q)(1, 1) = P(1, 1) / P(4, 1);
	(*Q)(2, 1) = P(2, 1) / P(4, 1);
	(*Q)(3, 1) = P(3, 1) / P(4, 1);
	(*Q)(4, 1) = 1;

	return Q;
}

void calculateIntensity(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &cam, light_t &light, unsigned hit_type, double* buffer_array)
{
	Matrix<double>* temp = light.position->subtract(intersection);
	Matrix<double>* surf_to_light = temp->normalize();
	temp->Erase(); delete temp;
	
	Matrix<double>* surf_to_cam = cam.subtract(intersection);
	Matrix<double>* surf_normal = obj->calculateSurfaceNormal(intersection, hit_type);

	Matrix<double>* surf_light_negative = surf_to_light->multiplyDot(-1);
	double surf_normal_mag = surf_normal->normal();

	temp = surf_normal->multiplyDot(2 * surf_to_light->multiplyDot(*surf_normal) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection = surf_light_negative->add(*temp);
	temp->Erase(); delete temp;

	double i_diffuse = surf_to_light->multiplyDot(*surf_normal) / (surf_normal_mag*surf_to_light->normal());
	double i_spec = pow(reflection->multiplyDot(*surf_to_cam) / (reflection->normal()*surf_to_cam->normal()), obj->getFallout());

	if (i_diffuse < 0 || isnan(i_diffuse)) i_diffuse = 0;
	if (i_spec < 0 || isnan(i_spec)) i_spec = 0;

	buffer_array[COLOR_R] += light.color.r*LI*obj->getDiffuse(COLOR_R) + i_spec*obj->getSpecular(COLOR_R);
	buffer_array[COLOR_G] += light.color.g*LI*obj->getDiffuse(COLOR_G) + i_spec*obj->getSpecular(COLOR_G);
	buffer_array[COLOR_B] += light.color.b*LI*obj->getDiffuse(COLOR_B) + i_spec*obj->getSpecular(COLOR_B);

	surf_to_light->Erase(); delete surf_to_light;
	surf_to_cam->Erase(); delete surf_to_cam;
	surf_normal->Erase(); delete surf_normal;
	surf_light_negative->Erase(); delete surf_light_negative;
	reflection->Erase(); delete reflection;
}

void calculateIntensityInfinite(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &cam, light_t &light, unsigned hit_type, double *buffer_array)
{
	Matrix<double>* temp = cam.subtract(intersection);
	Matrix<double>* surf_to_cam = temp->normalize();
	temp->Erase(); delete temp;

	Matrix<double>* surf_normal = obj->calculateSurfaceNormal(intersection, hit_type);

	Matrix<double>* light_negative = light.position->multiplyDot(-1);
	double surf_normal_mag = surf_normal->normal();

	temp = surf_normal->multiplyDot(2 * light.position->multiplyDot(*surf_normal) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection_inf = light_negative->add(*temp);
	temp->Erase(); delete temp;

	// calculate diffuse and specular intensities
	double Id_inf = light.position->multiplyDot(*surf_normal) / (surf_normal_mag*light.position->normal());
	double Is_inf = pow(reflection_inf->multiplyDot(*surf_to_cam) / (reflection_inf->normal()*surf_to_cam->normal()), obj->getFallout());

	if (Id_inf < 0 || isnan(Id_inf)) Id_inf = 0;
	if (Is_inf < 0 || isnan(Is_inf)) Is_inf = 0;

	buffer_array[COLOR_R] += light.color.r*LI_INF*(Id_inf*obj->getDiffuse(COLOR_R) + Id_inf*obj->getSpecular(COLOR_R));
	buffer_array[COLOR_G] += light.color.g*LI_INF*(Id_inf*obj->getDiffuse(COLOR_G) + Id_inf*obj->getSpecular(COLOR_G));
	buffer_array[COLOR_B] += light.color.b*LI_INF*(Id_inf*obj->getDiffuse(COLOR_B) + Id_inf*obj->getSpecular(COLOR_B));

	light_negative->Erase(); delete light_negative;
	surf_to_cam->Erase(); delete surf_to_cam;
	surf_normal->Erase(); delete surf_normal;
	reflection_inf->Erase(); delete reflection_inf;
}

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer, 
	std::vector<GenericObject*> &objects,
	light_t &light, light_t &light_inf,
			  double near, double near_h)
{
	double near_w = near_h * (w.width / w.height);

	// process each pixel on near plane
	for (unsigned row = 0; row < w.height; ++row)
	{
		for (unsigned col = 0; col < w.width; ++col)
		{
			double total_color[3] = { 0,0,0 };
			for (unsigned m = 0; m < SUPER_RES; ++m)
			{
				// coordinates of columns & rows on near plane
				double uc, vr;
				if (m == 0 || m == 1)
					uc = near_w*(2.0 * (2 * (int)col - 1) / (2.0*w.width) - 1);
				else 
					uc = near_w*(2.0 * (2 * (int)col + 1) / (2.0*w.width) - 1);
				if (m == 1 || m == 3)
					vr = near_h*(2.0*(2 * (int)row + 1) / (2.0*w.height) - 1);
				else
					vr = near_h*(2.0*(2 * (int)row - 1) / (2.0*w.height) - 1);

				//direction from camera to current pixel
				Matrix<double>* direction = cam->getN()->multiplyDot(-near);
				Matrix<double>* temp = cam->getU()->multiplyDot(uc);
				Matrix<double>* temp2 = cam->getV()->multiplyDot(vr);
				Matrix<double>* temp3 = temp->add(*temp2);
				Matrix<double>* result = direction->add(*temp3);
				direction->Erase(); delete direction;
				
				direction = new Matrix<double>(4,1,0);
				(*direction)(1, 1) = (*result)(1, 1);
				(*direction)(2, 1) = (*result)(2, 1);
				(*direction)(3, 1) = (*result)(3, 1);

				temp->Erase(); delete temp;
				temp2->Erase(); delete temp2;
				temp3->Erase(); delete temp3;
				result->Erase(); delete result;
				
				temp = direction;
				direction = temp->normalize();
				temp->Erase(); delete temp;
				
				for (unsigned obj_idx=0; obj_idx < objects.size(); ++obj_idx)
					objects[obj_idx]->setRayHit(*cam->getE(), *direction);

				unsigned tmin_idx = findMinHitIdx(objects);
	
				temp = direction->multiplyDot(objects[tmin_idx]->getRayHit().enter);
				Matrix<double>* rayOnObj = cam->getE()->add(*temp);
				temp->Erase(); delete temp;
				
				temp = light.position->subtract(*rayOnObj);
				Matrix<double>* surface_to_light = temp->normalize();
				temp->Erase(); delete temp;

				double super_res_buffer[N_CHANNELS];

				if (!isinf(objects[tmin_idx]->getRayHit().enter))
				{
					super_res_buffer[COLOR_R] = objects[tmin_idx]->getAbmient(COLOR_R);
					super_res_buffer[COLOR_G] = objects[tmin_idx]->getAbmient(COLOR_G);
					super_res_buffer[COLOR_B] = objects[tmin_idx]->getAbmient(COLOR_B);
				}
				
				unsigned rayhit_type = objects[tmin_idx]->getRayHit().enter_type;
				
				unsigned idx = 0;
				for (idx = 0; idx < objects.size(); ++idx)
				{
					if (idx == tmin_idx) continue;

					objects[idx]->setRayHit(*rayOnObj, *light_inf.position);

					if (!isinf(objects[idx]->getRayHit().enter))
						break;
				}

				if (idx >= objects.size())
					calculateIntensityInfinite(objects[tmin_idx], *rayOnObj, *cam->getE(), light_inf, rayhit_type, super_res_buffer);

				for (idx = 0; idx < objects.size(); ++idx)
				{
					if (idx == tmin_idx) continue;

					objects[idx]->setRayHit(*rayOnObj, *surface_to_light);

					if (!isinf(objects[idx]->getRayHit().enter))
						break;
				}

				if (idx >= objects.size())
					calculateIntensity(objects[tmin_idx], *rayOnObj, *cam->getE(), light, rayhit_type, super_res_buffer);

				direction->Erase(); delete direction;
				rayOnObj->Erase(); delete rayOnObj;
				surface_to_light->Erase(); delete surface_to_light;

				if (super_res_buffer[COLOR_R] > 1) super_res_buffer[COLOR_R] = 1;
				if (super_res_buffer[COLOR_G] > 1) super_res_buffer[COLOR_G] = 1;
				if (super_res_buffer[COLOR_B] > 1) super_res_buffer[COLOR_B] = 1;

				total_color[COLOR_R] += super_res_buffer[COLOR_R] * SATURATION;
				total_color[COLOR_G] += super_res_buffer[COLOR_G] * SATURATION;
				total_color[COLOR_B] += super_res_buffer[COLOR_B] * SATURATION;
			}

			framebuffer[COLOR_R][row][col] = total_color[COLOR_R] / SUPER_RES;
			framebuffer[COLOR_G][row][col] = total_color[COLOR_G] / SUPER_RES;
			framebuffer[COLOR_B][row][col] = total_color[COLOR_B] / SUPER_RES;
		}
	}
}
