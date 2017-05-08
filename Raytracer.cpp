#include <iostream>
#include <cmath>
#include "Raytracer.h"

void addColor(color_t &c1, color_t c2)
{
	c1.r += c2.r;
	c1.g += c2.g;
	c1.b += c2.b;
}
color_t multiplyColor(double coeff, color_t c)
{
	color_t result;
	result.r = c.r * coeff;
	result.g = c.g * coeff;
	result.b = c.b * coeff;
	return result;
}

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

void calculateIntensity(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &surf_normal, Matrix<double> &surf_to_cam, Light &light, Matrix<double> &surf_to_light, color_t &shading)
{
	Matrix<double>* surf_light_negative = -surf_to_light;
	double surf_normal_mag = surf_normal.normal();

	Matrix<double>* temp = surf_normal.multiplyDot(2 * surf_to_light.multiplyDot(surf_normal) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection = *surf_light_negative+*temp;
	temp->Erase(); delete temp;

	double i_diffuse = surf_to_light.multiplyDot(surf_normal) / (surf_normal_mag*surf_to_light.normal());
	double i_spec = pow(reflection->multiplyDot(surf_to_cam) / (reflection->normal() * surf_to_cam.normal()), obj->getFallout());

	if (i_diffuse < 0 || isnan(i_diffuse)) i_diffuse = 0;
	if (i_spec < 0 || isnan(i_spec)) i_spec = 0;

	double lightI = light.calculateIntensity(intersection);
	shading.r += light.getColor().r*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));
	shading.g += light.getColor().g*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));
	shading.b += light.getColor().b*lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));

	surf_light_negative->Erase(); delete surf_light_negative;
	reflection->Erase(); delete reflection;
}

void calculateIntensityInfinite(GenericObject *obj, Matrix<double> &surf_norm, Matrix<double> &surf_to_cam, Light &light, color_t &shading)
{
	Matrix<double>* light_negative = -*light.getPosition();
	double surf_normal_mag = surf_norm.normal();

	Matrix<double>* temp = surf_norm.multiplyDot(2 * light.getPosition()->multiplyDot(surf_norm) / (surf_normal_mag*surf_normal_mag));
	Matrix<double>* reflection_inf = *light_negative+(*temp);
	temp->Erase(); delete temp;

	// calculate diffuse and specular intensities
	double Id_inf = light.getPosition()->multiplyDot(surf_norm) / (surf_normal_mag*light.getPosition()->normal());
	double Is_inf = pow(reflection_inf->multiplyDot(surf_to_cam) / (reflection_inf->normal()*surf_to_cam.normal()), obj->getFallout());

	if (Id_inf < 0 || isnan(Id_inf)) Id_inf = 0;
	if (Is_inf < 0 || isnan(Is_inf)) Is_inf = 0;

	shading.r += light.getColor().r*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));
	shading.g += light.getColor().g*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));
	shading.b += light.getColor().b*light.getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));

	light_negative->Erase(); delete light_negative;
	reflection_inf->Erase(); delete reflection_inf;
}

color_t shade(std::vector<GenericObject*> &objects, Matrix<double> &e, Matrix<double> &d, Light &li, Light &li_inf, int k)
{
	color_t shadeC;

	for (unsigned obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
		objects[obj_idx]->setRayHit(e, d);

	unsigned tmin_idx = findMinHitIdx(objects);

	if (!isinf(objects[tmin_idx]->getRayHit().enter))
	{
		// calculate local shading components
		Matrix<double>* temp = d.multiplyDot(objects[tmin_idx]->getRayHit().enter);
		Matrix<double>* rayOnObj = e + *temp;
		temp->Erase(); delete temp;

		temp = *li.getPosition() - *rayOnObj;
		Matrix<double>* surface_to_light = temp->normalize();
		temp->Erase(); delete temp;

		double ambient_coeff = objects[tmin_idx]->getCoeff(COLOR_AMBIENT);
		color_t obj_color = objects[tmin_idx]->getColor();
		shadeC.r = ambient_coeff * obj_color.r;
		shadeC.g = ambient_coeff * obj_color.g;
		shadeC.b = ambient_coeff * obj_color.b;

		unsigned rayhit_type = objects[tmin_idx]->getRayHit().enter_type;

		temp = e - *rayOnObj;
		Matrix<double>* surf_to_cam = temp->normalize();
		temp->Erase(); delete temp;

		Matrix<double>* surf_norm = objects[tmin_idx]->calculateSurfaceNormal(*rayOnObj, rayhit_type);

		// trace a 2nd ray from intersection to infinite light source and see if it hits an object
		// i.e. whether current object is in shadow
		unsigned idx = 0;
		for (idx = 0; idx < objects.size(); ++idx)
		{
			//if (idx == tmin_idx) continue;

			objects[idx]->setRayHit(*rayOnObj, *li_inf.getPosition());

			if (objects[idx]->getRayHit().enter >= -BIAS && objects[idx]->getRayHit().enter <= 1)
				//if (!isinf(objects[idx]->getRayHit().enter))
				break;
		}

		if (idx >= objects.size())
			calculateIntensityInfinite(objects[tmin_idx], *surf_norm, *surf_to_cam, li_inf, shadeC);

		// trace a 2nd ray from intersection to point light source and see if it hits an object
		// i.e. whether current object is in shadow
		for (idx = 0; idx < objects.size(); ++idx)
		{
			//if (idx == tmin_idx) continue;

			objects[idx]->setRayHit(*rayOnObj, *surface_to_light);

			if (objects[idx]->getRayHit().enter >= -BIAS && objects[idx]->getRayHit().enter <= 1)
				//if (!isinf(objects[idx]->getRayHit().enter))
				break;
		}

		if (idx >= objects.size())
			calculateIntensity(objects[tmin_idx], *rayOnObj, *surf_norm, *surf_to_cam, li, *surface_to_light, shadeC);
		
		if (k > 0 && objects[tmin_idx]->getReflect() != 0)
		{
			Matrix<double>* reflection_dir = surf_norm->multiplyDot(2 * d.multiplyDot(*surf_norm));
			Matrix<double>* temp = d - *reflection_dir;
			reflection_dir->Erase(); delete reflection_dir;
			reflection_dir = temp->normalize();
			temp->Erase(); delete temp;

			addColor(shadeC, multiplyColor(objects[tmin_idx]->getReflect(), shade(objects, *rayOnObj, *reflection_dir, li, li_inf, k - 1)));

			reflection_dir->Erase(); delete reflection_dir;
		}

		surf_to_cam->Erase(); delete surf_to_cam;
		surf_norm->Erase(); delete surf_norm;
		rayOnObj->Erase(); delete rayOnObj;
		surface_to_light->Erase(); delete surface_to_light;
	}
	else
	{
		//color=0 0 0
		shadeC.r = 0; shadeC.g = 0; shadeC.b = 0;
	}

	return shadeC;
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
				
				color_t shading = shade(objects, *cam->getE(), *direction, light, light_inf, 2);
				
				direction->Erase(); delete direction;

				if (shading.r > 1) shading.r = 1;
				if (shading.g > 1) shading.g = 1;
				if (shading.b > 1) shading.b = 1;

				total_color[COLOR_R] += shading.r * SATURATION;
				total_color[COLOR_G] += shading.g * SATURATION;
				total_color[COLOR_B] += shading.b * SATURATION;
			}

			framebuffer[COLOR_R][row][col] = total_color[COLOR_R] / SUPER_RES;
			framebuffer[COLOR_G][row][col] = total_color[COLOR_G] / SUPER_RES;
			framebuffer[COLOR_B][row][col] = total_color[COLOR_B] / SUPER_RES;
		}
	}
}
