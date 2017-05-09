#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "model.h"
#include "Camera.h"
#include "GenericObject.h"
#include "Light.h"
#include "Window.h"
#include "Raytracer.h"
using namespace std;

#define ARG_NUM 2

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cerr << "Usage: raytrace [input_file_name]" << endl;
		exit(1);
	}

	char* filename = argv[1];

	ifstream inf(filename, ifstream::in);

	if (!inf)
	{
		cerr << "Cannot find the input file:" << filename << endl;
		exit(1);
	}

	window_t display_window;
	double aspect_ratio;
	inf >> display_window.height >> aspect_ratio;
	display_window.width = aspect_ratio * display_window.height;
	
	double posx, posy, posz, cr, cg, cb, intensity;
	inf >> posx >> posy >> posz >> cr >> cg >> cb >> intensity;

	PointLight light;
	light.setPosition(posx, posy, posz);
	light.setColor(cr, cg, cb);
	light.setIntensity(intensity);

	DirectedLight light_inf;
	inf >> posx >> posy >> posz >> cr >> cg >> cb >> intensity;

	light_inf.setPosition(posx, posy, posz);
	light_inf.setColor(cr, cg, cb);
	light_inf.setIntensity(intensity);

	Camera cam;
	double cam_x, cam_y, cam_z;
	inf >> cam_x >> cam_y >> cam_z;
	cam.setEyeWorld(cam_x, cam_y, cam_z);

	inf >> cam_x >> cam_y >> cam_z;
	cam.setGazeWorld(cam_x, cam_y, cam_z);

	double near, far;
	inf >> near >> far;
	cam.setMperspective(near, far);

	double view_angle;
	inf >> view_angle;
	cam.setTransformMatrices(near, view_angle, display_window);

	inf >> cam_x >> cam_y >> cam_z;
	cam.setUPWorld(cam_x, cam_y, cam_z);

	// remaining of camera setup
	cam.buildCamera();
	cam.setRotationAngle(TANGLE);
	
	// Initialize objects
	unsigned obj_num;
	inf >> obj_num;

	vector<GenericObject*> objects;

	for(unsigned obj_idx = 0; obj_idx < obj_num; ++obj_idx)
	{
		unsigned obj_type;
		inf >> obj_type;

		objects.push_back(GenericObject::makeObject(obj_type));

		double color_r, color_g, color_b, coeff;
		inf >> color_r >> color_g >> color_b >> coeff;
		objects[obj_idx]->setColor(color_r, color_g, color_b);

		objects[obj_idx]->setColorCoeff(COLOR_AMBIENT, coeff);

		inf >> coeff;
		objects[obj_idx]->setColorCoeff(COLOR_DIFFUSE, coeff);

		inf >> coeff;
		objects[obj_idx]->setColorCoeff(COLOR_SPEC, coeff);

		
		double fallout, reflect, refract, transparency;
		inf >> fallout >> reflect >> refract >> transparency;
		objects[obj_idx]->setFallout(fallout);
		objects[obj_idx]->setReflect(reflect);
		objects[obj_idx]->setRefract(refract);
		objects[obj_idx]->setTransparency(transparency);

		Matrix<double> mat(4, 4);
		for (unsigned row = 1; row <= 4; ++row)
		{
			for (unsigned col = 1; col <= 4; ++col)
				inf >> mat(row, col);
		}
		objects[obj_idx]->setAffineTransMat(mat);
		
		mat.Erase();
	}

	inf.close();
	/*
	Matrix<double>* Pw = new Matrix<double>(4, 1, 0);
	(*Pw)(4, 1) = 1;
	temp = cam.getM()->multiply(*Pw);
	Pw->Erase(); delete Pw;
	Matrix<double>* Pv = projTrans(*temp);
	temp->Erase(); delete temp;
	std::cout << *Pv;
	Pv->Erase(); delete Pv;*/
	draw(display_window, cam, objects, light, light_inf, near, view_angle);

	exit(EXIT_SUCCESS);
}
