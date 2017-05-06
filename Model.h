#pragma once

#include "Matrix.h"

#define OBJ_PLANE 1
#define OBJ_SPHERE 2
#define OBJ_CYLINDER 3
#define CYLINDER_TOP 4
#define CYLINDER_BASE 5
#define OBJ_CONE 6
#define CONE_BASE 7

#define X 1
#define Y 2
#define Z 3

#define N_CHANNELS 3 // # RGB channels
#define COLOR_R 0
#define COLOR_G 1
#define COLOR_B 2

/* Default */
#define LI (1.0) // light intensity
#define LI_INF (1.0)
#define SUPER_RES 4 // super-resolution uses 4 corners of pixel

#define CAM_L 0
#define CAM_R 1
#define CAM_U 2
#define CAM_D 3
#define CAM_N 4
#define CAM_F 5

#define TANGLE (20)
#define SATURATION 255

struct window_t {
	unsigned width, height;
};

struct color_t {
	double r, g, b;
};

struct hit_t {
	unsigned enter_type, exit_type;
	double enter, exit;
};

struct light_t {
	Matrix<double>* position;
	color_t color;
};
