#include <cmath>
#include "Raytracer.h"
#include "Scene.h"
#include "GenericObject.h"

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

unsigned Raytracer::findMinHitIdx()
{
	std::vector<GenericObject*> objects = mpScene->GetObjects();
	size_t objCount = objects.size();
	if (objCount == 1) return 0;

	unsigned min_idx = 0;
	for (unsigned i = 1; i < objCount; ++i)
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

void calculateIntensity(GenericObject *obj, Matrix<double> &intersection, Matrix<double> &surf_normal, Matrix<double> &surf_to_cam, const Light* light, Matrix<double> &surf_to_light, color_t &shading)
{
	Matrix<double>* surf_light_negative = -surf_to_light;
	Matrix<double>* temp = surf_normal.multiplyDot(2 * surf_to_light.multiplyDot(surf_normal));
	Matrix<double>* reflection = *surf_light_negative+*temp;
	temp->Erase(); delete temp;

	double i_diffuse = surf_to_light.multiplyDot(surf_normal);
	double i_spec = pow(reflection->multiplyDot(surf_to_cam), obj->getFallout());

	if (std::isnan(i_diffuse) || i_diffuse < 0) i_diffuse = 0;
	if (std::isnan(i_spec) || i_spec < 0) i_spec = 0;

	double lightI = light->calculateIntensity(intersection);
  	double total_coeff = lightI *(obj->getCoeff(COLOR_DIFFUSE) * i_diffuse + i_spec*obj->getCoeff(COLOR_SPEC));
  	shading.r += light->getColor().r*total_coeff;
	shading.g += light->getColor().g*total_coeff;
	shading.b += light->getColor().b*total_coeff;

	surf_light_negative->Erase(); delete surf_light_negative;
	reflection->Erase(); delete reflection;
}

void calculateIntensityInfinite(GenericObject *obj, Matrix<double> &surf_norm, Matrix<double> &surf_to_cam, const Light* light, color_t &shading)
{
	Matrix<double>* temp = surf_norm.multiplyDot(2 * light->getDirOpposite()->multiplyDot(surf_norm));
	Matrix<double>* reflection_inf = *(light->getPosition()) + *temp;
	temp->Erase(); delete temp;

	// calculate diffuse and specular intensities
	double Id_inf = light->getDirOpposite()->multiplyDot(surf_norm);
	double Is_inf = pow(reflection_inf->multiplyDot(surf_to_cam), obj->getFallout());

	if (std::isnan(Id_inf) || Id_inf < 0) Id_inf = 0;
	if (std::isnan(Is_inf) || Is_inf < 0) Is_inf = 0;

  double total_coeff = light->getIntensity()*(Id_inf*obj->getCoeff(COLOR_DIFFUSE) + Is_inf*obj->getCoeff(COLOR_SPEC));
  shading.r += light->getColor().r*total_coeff;
  shading.g += light->getColor().g*total_coeff;
  shading.b += light->getColor().b*total_coeff;

	reflection_inf->Erase(); delete reflection_inf;
}

color_t Raytracer::shade(Matrix<double> &e, Matrix<double> &d, int k)
{
	color_t shadeC;
	std::vector<GenericObject*>& objects = mpScene->GetObjects();
	const Light* li = mpScene->GetPointLight();
	const Light* li_inf = mpScene->GetInfiniteLight();

  // trace incident ray from e and check if it intersects any object
	for (unsigned obj_idx = 0; obj_idx < objects.size(); ++obj_idx)
		objects[obj_idx]->setRayHit(e, d);

	unsigned tmin_idx = findMinHitIdx();

	if (!std::isinf(objects[tmin_idx]->getRayHit().enter) && objects[tmin_idx]->getRayHit().enter >= -BIAS)
	{
		// calculate intersection point
		Matrix<double>* temp = d.multiplyDot(objects[tmin_idx]->getRayHit().enter);
		Matrix<double>* rayOnObj = e + *temp;
		temp->Erase(); delete temp;
	    unsigned rayhit_type = objects[tmin_idx]->getRayHit().enter_type;

	    // direction vector from intersection to point light source
		temp = *(li->getPosition()) - *rayOnObj;
		Matrix<double>* surface_to_light = temp->normalize();
		temp->Erase(); delete temp;

	    // direction vector from intersection to camera
	    temp = e - *rayOnObj;
	    Matrix<double>* surf_to_cam = temp->normalize();
	    temp->Erase(); delete temp;

	    // surface normal at intersection
	    Matrix<double>* surf_norm = objects[tmin_idx]->calculateSurfaceNormal(*rayOnObj, rayhit_type);

	    bool is_inside = (d.multiplyDot(*surf_norm) > 0) ? true : false;
	    if (is_inside)
	    {
	      //shadeC.r = 0; shadeC.g = 0; shadeC.b = 0;
	      temp = -*surf_norm;
	      surf_norm->Erase(); delete surf_norm;
	      surf_norm = temp;
	    }

	    Matrix<double>* bias_offset = surf_norm->multiplyDot(BIAS);
	    Matrix<double>* rayOnObjPlus = *rayOnObj + *bias_offset;
	    Matrix<double>* rayOnObjMinus = *rayOnObj - *bias_offset;
	    bias_offset->Erase(); delete bias_offset;

	    // calculate local shading components

	    // ambient
		double ambient_coeff = objects[tmin_idx]->getCoeff(COLOR_AMBIENT);
		color_t obj_color = objects[tmin_idx]->getColor();
		shadeC.r = ambient_coeff * obj_color.r;
		shadeC.g = ambient_coeff * obj_color.g;
		shadeC.b = ambient_coeff * obj_color.b;

	    // diffuse + specular

		// trace a 2nd ray from intersection to infinite light source and see if it hits an object
		// i.e. whether current object is in shadow
		unsigned idx = 0;
		for (idx = 0; idx < objects.size(); ++idx)
		{
			objects[idx]->setRayHit(*rayOnObjPlus, *(li_inf->getDirOpposite()));
			if (!std::isinf(objects[idx]->getRayHit().enter) && objects[idx]->getRayHit().enter >= -BIAS)
				break;
		}
		if (idx >= objects.size())
			calculateIntensityInfinite(objects[tmin_idx], *surf_norm, *surf_to_cam, li_inf, shadeC);

		// trace a 2nd ray from intersection to point light source and see if it hits an object
		// i.e. whether current object is in shadow
		for (idx = 0; idx < objects.size(); ++idx)
		{
			objects[idx]->setRayHit(*rayOnObjPlus, *surface_to_light);
			if (!std::isinf(objects[idx]->getRayHit().enter) && objects[idx]->getRayHit().enter >= -BIAS)
				break;
		}
		if (idx >= objects.size())
			calculateIntensity(objects[tmin_idx], *rayOnObj, *surf_norm, *surf_to_cam, li, *surface_to_light, shadeC);
		
		if (k > 0)
		{
			// reflection
			if (objects[tmin_idx]->getReflect() != 0)
			{
				Matrix<double>* reflection_dir = surf_norm->multiplyDot(2 * d.multiplyDot(*surf_norm));
				Matrix<double>* temp = d - *reflection_dir;
				reflection_dir->Erase(); delete reflection_dir;
        		(*temp)(4, 1) = 0;
				reflection_dir = temp->normalize();
				temp->Erase(); delete temp;

				addColor(shadeC, multiplyColor(objects[tmin_idx]->getReflect(), shade(*rayOnObjPlus, *reflection_dir, k - 1)));
				reflection_dir->Erase(); delete reflection_dir;
			}
			
			// refraction
			if (objects[tmin_idx]->getTransparency() != 0)
			{
				double median_ratio = is_inside?objects[tmin_idx]->getRefract():(1/objects[tmin_idx]->getRefract());

				// determine whether we are entering/exiting the median
				double n_dot_d = -surf_norm->multiplyDot(d);
				Matrix<double>* refract_norm = new Matrix<double>(*surf_norm);

		        if (is_inside)
		        {
		          shadeC.r = 0; shadeC.g = 0; shadeC.b = 0;
		        }
        
				double refract_rate = 1 - median_ratio*median_ratio * (1 - n_dot_d*n_dot_d);

				// check if refraction above threshold (90 degree)
				// i.e. whether light is reflected internally and no refraction
				if (refract_rate >= 0)
				{
					double coeff = median_ratio * n_dot_d - sqrt(refract_rate);
					temp = refract_norm->multiplyDot(coeff);
					Matrix<double>* temp2 = d.multiplyDot(median_ratio);
					Matrix<double>* refract_dir = *temp2 + *temp;
					temp->Erase(); delete temp;
					temp2->Erase(); delete temp2;

			        temp = refract_dir->normalize();
		         	refract_dir->Erase(); delete refract_dir;
		        	refract_dir = temp;

					addColor(shadeC, multiplyColor(objects[tmin_idx]->getTransparency(), shade(*rayOnObjMinus, *refract_dir, k - 1)));

					refract_dir->Erase(); delete refract_dir;
				}
				refract_norm->Erase(); delete refract_norm;
			}
		}

		surf_to_cam->Erase(); delete surf_to_cam;
		surf_norm->Erase(); delete surf_norm;
		rayOnObj->Erase(); delete rayOnObj;
	    rayOnObjPlus->Erase(); delete rayOnObjPlus;
	    rayOnObjMinus->Erase(); delete rayOnObjMinus;
		surface_to_light->Erase(); delete surface_to_light;
	}
	else
	{
		// the ray didn't intersect anything, return black
		shadeC.r = 0; shadeC.g = 0; shadeC.b = 0;
	}

	return shadeC;
}

color_t Raytracer::calculatePixelColor(int wHeight, int wWidth, int rowIdx, int colIdx)
{
	color_t pixelColor;

	double near_w = mpScene->GetNearWidth();
	double near_h = mpScene->GetNearHeight();
	Camera* cam = mpScene->GetCamera();
	for (unsigned m = 0; m < SUPER_RES; ++m)
	{
		// coordinates of columns & rows on near plane
		double uc, vr;
		if (m == 0 || m == 1)
			uc = near_w*(2.0 * (2 * (double)colIdx - 1) / (2.0*wWidth) - 1);
		else 
			uc = near_w*(2.0 * (2 * (double)colIdx + 1) / (2.0*wWidth) - 1);
		if (m == 1 || m == 3)
			vr = near_h*(2.0*(2 * (double)rowIdx + 1) / (2.0*wHeight) - 1);
		else
			vr = near_h*(2.0*(2 * (double)rowIdx - 1) / (2.0*wHeight) - 1);

		//direction from camera to current pixel
		Matrix<double>* direction = cam->getN()->multiplyDot(-cam->GetNear());
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
		
		color_t shading = shade(*cam->getE(), *direction, RECURSIVE_LV);
		
		direction->Erase(); delete direction;

		pixelColor.r += (shading.r > 1 ? 1 : shading.r) * SATURATION;
		pixelColor.g += (shading.g > 1 ? 1 : shading.g) * SATURATION;
		pixelColor.b += (shading.b > 1 ? 1 : shading.b) * SATURATION;
	}

	pixelColor.r /= SUPER_RES;
	pixelColor.g /= SUPER_RES;
	pixelColor.b /= SUPER_RES;
	return pixelColor;
}