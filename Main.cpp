#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "Model.h"
#include "Camera.h"
#include "GenericObject.h"
#include "Light.h"
#include "ScreenPainter.h"
#include "Raytracer.h"
#include "Scene.h"
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
	double posx, posy, posz, cr, cg, cb, intensity;
	Camera cam;
	double cam_x, cam_y, cam_z;
	double near, far, view_angle;
	Scene sceneInfo;

	inf >> display_window.width >> display_window.height;
	
	inf >> posx >> posy >> posz >> cr >> cg >> cb >> intensity;
	PointLight light = PointLight(posx, posy, posz, cr, cg, cb, intensity);
	sceneInfo.SetPointLight(&light);

	inf >> posx >> posy >> posz >> cr >> cg >> cb >> intensity;
	DirectedLight light_inf = DirectedLight(posx, posy, posz, cr, cg, cb, intensity);

	sceneInfo.SetInfiniteLight(&light_inf);

	inf >> cam_x >> cam_y >> cam_z;
	cam.setEyeWorld(cam_x, cam_y, cam_z);

	inf >> cam_x >> cam_y >> cam_z;
	cam.setGazeWorld(cam_x, cam_y, cam_z);

	inf >> near >> far >> view_angle;
	cam.SetViewFrustum(near, far, view_angle, display_window.height, display_window.width);

	inf >> cam_x >> cam_y >> cam_z;
	cam.setUPWorld(cam_x, cam_y, cam_z);

	// remaining camera setup
	cam.buildCamera();
	cam.setRotationAngle(TANGLE);
	
	sceneInfo.SetCamera(&cam);

	// Initialize objects
	unsigned obj_num;
	inf >> obj_num;

	for(unsigned obj_idx = 0; obj_idx < obj_num; ++obj_idx)
	{
		unsigned obj_type;
		inf >> obj_type;

		sceneInfo.AddObject(GenericObject::makeObject(obj_type));

		double color_r, color_g, color_b, coeff;
		inf >> color_r >> color_g >> color_b >> coeff;
		sceneInfo.GetObject(obj_idx)->setColor(color_r, color_g, color_b);

		sceneInfo.GetObject(obj_idx)->setColorCoeff(COLOR_AMBIENT, coeff);

		inf >> coeff;
		sceneInfo.GetObject(obj_idx)->setColorCoeff(COLOR_DIFFUSE, coeff);

		inf >> coeff;
		sceneInfo.GetObject(obj_idx)->setColorCoeff(COLOR_SPEC, coeff);

		
		double fallout, reflect, refract, transparency;
		inf >> fallout >> reflect >> refract >> transparency;
		sceneInfo.GetObject(obj_idx)->setFallout(fallout);
		sceneInfo.GetObject(obj_idx)->setReflect(reflect);
		sceneInfo.GetObject(obj_idx)->setRefract(refract);
		sceneInfo.GetObject(obj_idx)->setTransparency(transparency);

		Matrix<double> mat(4, 4);
		for (unsigned row = 1; row <= 4; ++row)
		{
			for (unsigned col = 1; col <= 4; ++col)
				inf >> mat(row, col);
		}
		sceneInfo.GetObject(obj_idx)->setAffineTransMat(mat);
		
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
	ScreenPainter screen(display_window);

	screen.draw(&sceneInfo);

	exit(EXIT_SUCCESS);
}
