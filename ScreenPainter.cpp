#include <X11/Xutil.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "Model.h"
#include "Raytracer.h"
#include "Scene.h"
#include "ScreenPainter.h"

Display *initX(Display *d, Window *w, int *s, int w_width, int w_height)
{
	d = XOpenDisplay(NULL);
	if (d == NULL)
	{
		std::cerr << "Cannot open display" << std::endl;
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
	XSetForeground(d, *gc, r << 16 | g << 8 | b);
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

void ScreenPainter::drawRow(Display *d, Window w, int s, unsigned rowIdx)
{
	for (int col = 0; col < mWidth; ++col)
	{

		XSetForeground(d, DefaultGC(d, s), (int)mFrameBuffer[rowIdx][col].r << 16
										 | (int)mFrameBuffer[rowIdx][col].g << 8 
										 | (int)mFrameBuffer[rowIdx][col].b);
		XDrawPoint(d, w, DefaultGC(d, s), col, rowIdx);
	}
}

void ScreenPainter::drawRows(bool retrace, Display& d, Window w, int s, unsigned beginRow, unsigned interval)
{
	for (int row = beginRow; row < mHeight; row+=interval)
	{
		if (retrace)
		{
			for (int col = 0; col < mWidth; ++col)
			{
				mFrameBuffer[row][col] = mRaytracer->calculatePixelColor(mHeight, mWidth, mHeight-row-1, col);
				/*color_t color = mRaytracer->calculatePixelColor(mHeight, mWidth, mHeight-row-1, col);
				XLockDisplay(&d);
				setCurrentColorX(&d, &DefaultGC(&d, s), color.r, color.g, color.b);
				setPixelX(&d, w, s, col, row);
				XUnlockDisplay(&d);*/
			}
		}
		XLockDisplay(&d);
		drawRow(&d, w, s, row);
		XUnlockDisplay(&d);
	}
}


void drawObj(Display *d, Window w, int s, unsigned w_width, unsigned w_height, int*** framebuffer)
{
	for (unsigned row = 0; row < w_height; ++row)
	{
		for (unsigned col = 0; col < w_width; ++col)
		{
			setCurrentColorX(d, &(DefaultGC(d, s)), framebuffer[COLOR_R][row][col], framebuffer[COLOR_G][row][col], framebuffer[COLOR_B][row][col]);
			setPixelX(d, w, s, col, w_height-row-1);
		}
	}
}

ScreenPainter::ScreenPainter(window_t window)
{
	mHeight = window.height;
	mWidth = window.width;
	mFrameBuffer = std::vector<std::vector<color_t>>(mHeight, std::vector<color_t>(mWidth, color_t()));
}

ScreenPainter::~ScreenPainter()
{ 
	if (mRaytracer != nullptr) 
		delete mRaytracer; 
}

void ScreenPainter::test(/*bool retrace, int s, unsigned beginRow, unsigned interval*/){}

void ScreenPainter::draw(Scene* sceneInfo)
{
	Display *d = NULL;
	Window w;
	XEvent e;
	int s;
	mRaytracer = new Raytracer(sceneInfo);

	if (XInitThreads() ==0)
	{
		std::cout << "XinitThreads returned 0." << std::endl;
		return;
	}

	d = initX(d, &w, &s, mWidth, mHeight);

	bool camUpdate = true;

	std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();

	do
	{
		XNextEvent(d, &e);
		if (e.type == ClientMessage)
		{
			break;
		}
		
		if (e.type == Expose || e.type == KeyPress)
		{
			if (e.type == KeyPress && (
				XLookupKeysym(&e.xkey, 0) == XK_Left ||
				XLookupKeysym(&e.xkey, 0) == XK_Right ||
				XLookupKeysym(&e.xkey, 0) == XK_Up ||
				XLookupKeysym(&e.xkey, 0) == XK_Down ||
				XLookupKeysym(&e.xkey, 0) == XK_f ||
				XLookupKeysym(&e.xkey, 0) == XK_b))
			{
				camUpdate = true;
				switch (XLookupKeysym(&e.xkey, 0))
				{
				case XK_Left:
					sceneInfo->GetCamera()->moveCamera(CAM_L); break;
				case XK_Right:
					sceneInfo->GetCamera()->moveCamera(CAM_R); break;
				case XK_Up:
					sceneInfo->GetCamera()->moveCamera(CAM_U); break;
				case XK_Down:
					sceneInfo->GetCamera()->moveCamera(CAM_D); break;
				case XK_f:
					sceneInfo->GetCamera()->moveCamera(CAM_F); break;
				case XK_b:
					sceneInfo->GetCamera()->moveCamera(CAM_N); break;
				}

				XClearWindow(d, w);
			}

			/// Single thread
			//drawRows(camUpdate, *d,w,s, 0, 1);

			/// Multithread
			unsigned workerCount = std::thread::hardware_concurrency();
			if (workerCount <= 0) 
			{
				workerCount = 1;
			}

			std::vector<std::thread> workers;
			for(unsigned workerIdx = 0; workerIdx < workerCount; ++workerIdx)
			{
				workers.push_back(std::thread(&ScreenPainter::drawRows, this, camUpdate, std::ref(*d), w, s, workerIdx, workerCount));
			}
			for(auto& worker : workers)
			{
				worker.join();
			}
		}

		if (camUpdate)
		{
			std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::now();
			std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(timeEnd-timeStart);
			std::cout << "time spent: " << timeSpan.count() << " seconds." << std::endl;

			camUpdate = false;
		}
	} while (true);
//cam.moveCamera(CAM_N);
//raytrace(screen, &cam, framebuffer, objects, light, light_inf, near, near_h);
//XClearWindow(d, w);
//drawObj(d, w, s, screen.width, screen.height, framebuffer);

	quitX(d, w);
}

