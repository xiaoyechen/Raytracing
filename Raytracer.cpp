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

void calculateIntensity(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &cam, Light &light, Matrix<double> &surf_to_light, unsigned hit_type, double* buffer_array)
{
	Matrix<double>* temp = cam-intersection;
	Matrix<double>* surf_to_cam = temp->normalize();
	temp->Erase(); delete temp;

	Matrix<double>* surf_normal = obj->calculateSurfaceNormal(intersection, hit_type);

	Matrix<double>* surf_light_negative = -surf_to_light;
	double surf_normal_mag = surf_normal->normal();

	temp = surf_normal->multiplyDot(2 * surf_to_light.multiplyDot(*surf_normal) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection = *surf_light_negative+*temp;
	temp->Erase(); delete temp;

	double i_diffuse = surf_to_light.multiplyDot(*surf_normal) / (surf_normal_mag*surf_to_light.normal());
	double i_spec = pow(reflection->multiplyDot(*surf_to_cam) / (reflection->normal()*surf_to_cam->normal()), obj->getFallout());

	if (i_diffuse < 0 || isnan(i_diffuse)) i_diffuse = 0;
	if (i_spec < 0 || isnan(i_spec)) i_spec = 0;

	double lightI = light.calculateIntensity(intersection);
	buffer_array[COLOR_R] += light.getColor().r*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));
	buffer_array[COLOR_G] += light.getColor().g*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));
	buffer_array[COLOR_B] += light.getColor().b*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));

	surf_to_cam->Erase(); delete surf_to_cam;
	surf_normal->Erase(); delete surf_normal;
	surf_light_negative->Erase(); delete surf_light_negative;
	reflection->Erase(); delete reflection;
}

void calculateIntensityInfinite(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &cam, Light &light, unsigned hit_type, double *buffer_array)
{
	Matrix<double>* temp = cam-intersection;
	Matrix<double>* surf_to_cam = temp->normalize();
	temp->Erase(); delete temp;

	Matrix<double>* surf_normal = obj->calculateSurfaceNormal(intersection, hit_type);

	Matrix<double>* light_negative = -*light.getPosition();
	double surf_normal_mag = surf_normal->normal();

	temp = surf_normal->multiplyDot(2 * light.getPosition()->multiplyDot(*surf_normal) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection_inf = *light_negative+(*temp);
	temp->Erase(); delete temp;

	// calculate diffuse and specular intensities
	double Id_inf = light.getPosition()->multiplyDot(*surf_normal) / (surf_normal_mag*light.getPosition()->normal());
	double Is_inf = pow(reflection_inf->multiplyDot(*surf_to_cam) / (reflection_inf->normal()*surf_to_cam->normal()), obj->getFallout());

	if (Id_inf < 0 || isnan(Id_inf)) Id_inf = 0;
	if (Is_inf < 0 || isnan(Is_inf)) Is_inf = 0;

	buffer_array[COLOR_R] += light.getColor().r*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));
	buffer_array[COLOR_G] += light.getColor().g*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));
	buffer_array[COLOR_B] += light.getColor().b*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));

	light_negative->Erase(); delete light_negative;
	surf_to_cam->Erase(); delete surf_to_cam;
	surf_normal->Erase(); delete surf_normal;
	reflection_inf->Erase(); delete reflection_inf;
}

// assume framebuffer[w.height][w.width][# of color channels]
void raytrace(window_t w, Camera *cam, int ***framebuffer, 
	std::vector<GenericObject*> &objects,
	Light &light, Light &light_inf,
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
				Matrix<double>* temp3 = (*temp)+(*temp2);
				Matrix<double>* result = *direction+(*temp3);
				direction->Erase(); delete direction;
				
				direction = new Matrix<double>(4,1,0);
				(*direction)(1, 1) = (*result)(1, 1);
				(*direction)(2, 1) = (*result)(2, 1);
				(*direction)(3, 1) = (*result)(3, 1);

				temp->Erase(); delete temp;
				temp2->Erase(); delete temp2;
				temp3->Erase(); delete temp3;
				result->Erase(); delete result;
				
				temp = direction->normalize();
				direction->Erase(); delete direction;
				direction = temp;
				
				for (unsigned obj_idx=0; obj_idx < objects.size(); ++obj_idx)
					objects[obj_idx]->setRayHit(*cam->getE(), *direction);

				unsigned tmin_idx = findMinHitIdx(objects);

				if (isinf(objects[tmin_idx]->getRayHit().enter))
				{
					direction->Erase(); delete direction;
					continue;
				}
				
				temp = direction->multiplyDot(objects[tmin_idx]->getRayHit().enter);
				Matrix<double>* rayOnObj = *cam->getE() + *temp;
				temp->Erase(); delete temp;

				temp = *light.getPosition() - *rayOnObj;
				Matrix<double>* surface_to_light = temp->normalize();
				temp->Erase(); delete temp;

				double super_res_buffer[N_CHANNELS];
				double ambient_coeff = objects[tmin_idx]->getCoeff(COLOR_AMBIENT);
				color_t obj_color = objects[tmin_idx]->getColor();
				super_res_buffer[COLOR_R] = ambient_coeff * obj_color.r;
				super_res_buffer[COLOR_G] = ambient_coeff * obj_color.g;
				super_res_buffer[COLOR_B] = ambient_coeff * obj_color.b;

				unsigned rayhit_type = objects[tmin_idx]->getRayHit().enter_type;
				
				// trace a 2nd ray from intersection to infinite light source and see if it hits an object
				// i.e. whether current object is in shadow
				unsigned idx = 0;
				for (idx = 0; idx < objects.size(); ++idx)
				{
					//if (idx == tmin_idx) continue;

					objects[idx]->setRayHit(*rayOnObj, *light_inf.getPosition());

					if (objects[idx]->getRayHit().enter>=-BIAS && objects[idx]->getRayHit().enter<=1)
					//if (!isinf(objects[idx]->getRayHit().enter))
						break;
				}

				if (idx >= objects.size())
					calculateIntensityInfinite(objects[tmin_idx], *rayOnObj, *cam->getE(), light_inf, rayhit_type, super_res_buffer);

				// trace a 2nd ray from intersection to point light source and see if it hits an object
				// i.e. whether current object is in shadow
				for (idx = 0; idx < objects.size(); ++idx)
				{
					//if (idx == tmin_idx) continue;

					objects[idx]->setRayHit(*rayOnObj, *surface_to_light);

					if (objects[idx]->getRayHit().enter>=-BIAS && objects[idx]->getRayHit().enter<=1)
					//if (!isinf(objects[idx]->getRayHit().enter))
						break;
				}

				if (idx >= objects.size())
					calculateIntensity(objects[tmin_idx], *rayOnObj, *cam->getE(), light, *surface_to_light, rayhit_type, super_res_buffer);

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
