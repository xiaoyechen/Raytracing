#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

	string filename = argv[1];

	ifstream inf(filename);

	if (!inf)
	{
		cerr << "Cannot find the input file:" << filename << endl;
		exit(1);
	}

	window_t display_window;
	double aspect_ratio;
	inf >> display_window.height >> aspect_ratio;

	light_t light;
	inf >> light.position(X, 1) >> light.position(Y, 1) >> light.position(Z, 1)
		>> light.color.r >> light.color.g >> light.color.b;

	light_t light_inf;
	inf >> light_inf.position(X, 1) >> light_inf.position(Y, 1) >> light_inf.position(Z, 1)
		>> light_inf.color.r >> light_inf.color.g >> light_inf.color.b;

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
	cam.setRotationAngle(TANGLE);
	cam.buildCamera();

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

		Matrix<double> mat(4, 3);
		for (unsigned row = 1; row <= 4; ++row)
		{
			for (unsigned col = 1; col <= 3; ++col)
				inf >> mat(row, col);
		}
		objects[obj_idx]->setAffineTransMat(mat);
		mat.Erase();
	}

	inf.close();
	
	draw(display_window, cam, objects, light, light_inf, near, view_angle);

	exit(EXIT_SUCCESS);
}