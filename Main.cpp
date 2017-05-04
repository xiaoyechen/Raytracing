#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "model.h"
#include "Camera.h"
#include "GenericObject.h"
#include "Window.h"
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

	light_t light;
	light.position = new Matrix<double>(4, 1, 1);
	inf >> (*light.position)(X, 1) >> (*light.position)(Y, 1) >> (*light.position)(Z, 1)
		>> light.color.r >> light.color.g >> light.color.b;

	light_t light_inf;
	light_inf.position = new Matrix<double>(4, 1, 0);
	inf >> (*light_inf.position)(X, 1) >> (*light_inf.position)(Y, 1) >> (*light_inf.position)(Z, 1)
		>> light_inf.color.r >> light_inf.color.g >> light_inf.color.b;
	Matrix<double>* temp = light_inf.position->normalize();
	light_inf.position->Erase();
	light_inf.position = temp;

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

		double color_r, color_g, color_b;
		inf >> color_r >> color_g >> color_b;
		objects[obj_idx]->setColor(COLOR_ORIGIN, color_r, color_g, color_b);

		double coeff;
		inf >> color_r >> color_g >> color_b >> coeff;
		objects[obj_idx]->setColor(COLOR_DIFFUSE, color_r, color_g, color_b);
		objects[obj_idx]->setColorCoeff(COLOR_DIFFUSE, coeff);

		inf >> color_r >> color_g >> color_b >> coeff;
		objects[obj_idx]->setColor(COLOR_SPEC, color_r, color_g, color_b);
		objects[obj_idx]->setColorCoeff(COLOR_SPEC, coeff);

		inf >> color_r >> color_g >> color_b >> coeff;
		objects[obj_idx]->setColor(COLOR_AMBIENT, color_r, color_g, color_b);
		objects[obj_idx]->setColorCoeff(COLOR_AMBIENT, coeff);

		double fallout;
		inf >> fallout;
		objects[obj_idx]->setFallout(fallout);

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
	
	draw(display_window, cam, objects, light, light_inf, near, view_angle);

	exit(EXIT_SUCCESS);
}
