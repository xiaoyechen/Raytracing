#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>
#include "Raytracer.h"
#include "Window.h"

Display *initX(Display *d, Window *w, int *s, int w_width, int w_height)
{
	d = XOpenDisplay(NULL);
	if (d == NULL)
	{
		cerr << "Cannot open display" << endl;
		exit(1);
	}
	*s = DefaultScreen(d);
	*w = XCreateSimpleWindow(d, RootWindow(d, *s), 0, 0, w_width, w_height, 1, BlackPixel(d, *s), WhitePixel(d, *s));
	Atom delWindow = XInternAtom(d, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(d, *w, &delWindow, 1);
	XSelectInput(d, *w, ExposureMask | KeyPressMask);
	XMapWindow(d, *w);
	return d;
}

void setCurrentColorX(Display *d, GC *gc, unsigned r, unsigned g, unsigned b)
{
	XSetForground(d, *gc, r << 16 | g << 8 | b);
}

void setPixelX(Display *d, Window w, int s, int i, int j)
{
	XDrawPoint(d, w, DefaultGC(d, s), i, j);
}

void quitX(Display *d, Window w)
{
	XDestroyWindow(d, w);
	XCloseDisplay(d);
}

void drawObj(Display *d, Window w, int s, int w_width, int w_height, int*** framebuffer)
{
	for (unsigned row = 0; row < w_height; ++row)
	{
		for (unsigned col = 0; col < w_width; ++col)
		{
			setCurrentColorX(d, &(DefaultGC(d, s)), framebuffer[row][col][COLOR_R], framebuffer[row][col][COLOR_G], framebuffer[row][col][COLOR_B]);
			setPixelX(d, w, s, row, col);
		}
	}
}

void draw(window_t screen, Camera &cam, std::vector<GenericObject> objects, light_t &light, light_t light_inf, double near, double view_angle)
{
	Display *d;
	Window w;
	XEvent e;
	int s;

	d = initX(d, &w, &s, screen.width, screen.height);

	int framebuffer[screen.height][screen.width][N_CHANNELS];
	double near_h = near*tan(M_PI / 180 * view_angle / 2.0);
	
	raytrace(screen, framebuffer, objects, light, light_inf, near, near_h);

	while (true)
	{
		XNextEvent(d, &e);
		if (e.type == Expose)
			drawObj(d, w, s, screen.width, screen.height, framebuffer);

		if (e.type == KeyPress && (
			XLookupKeysym(&e.xkey, 0) == XK_Left ||
			XLookupKeysym(&e.xkey, 0) == XK_Right ||
			XLookupKeysym(&e.xkey, 0) == XK_Up ||
			XLookupKeysym(&e.xkey, 0) == XK_Down ||
			XLookupKeysym(&e.xkey, 0) == XK_f ||
			XLookupKeysym(&e.xkey, 0) == XK_b))
		{
			switch (XLookupKeysym(&e.xkey, 0))
			{
			case XK_Left:
				cam.moveCamera(CAM_L); break;
			case XK_Right:
				cam.moveCamera(CAM_R); break;
			case XK_Up:
				cam.moveCamera(CAM_U); break;
			case XK_Down:
				cam.moveCamera(CAM_D); break;
			case XK_f:
				cam.moveCamera(CAM_F); break;
			case XK_b:
				cam.moveCamera(CAM_N); break;
			}

			raytrace(screen, framebuffer, objects, light, light_inf, near, near_h);

			XClearWindow(d, w);

			drawObj(d, w, s, screen.width, screen.height, framebuffer);
		}

		if (e.type == ClientMessage)
			break;
	}

	quitX(d, w);
}