#pragma once
#include <X11/Xlib.h>
#include <vector>
#include "Model.h"

class Scene;
class Raytracer;

class ScreenPainter
{
public:
	ScreenPainter() = delete;
	ScreenPainter(window_t window);
	~ScreenPainter();

	void draw(Scene* sceneInfo);
	
private:
	void drawRow(Display *d, Window w, int s, unsigned rowIdx);
	void drawRows(bool retrace, Display& d, Window w, int s, unsigned beginRow, unsigned interval);
	void test(/*bool retrace, int s, unsigned beginRow, unsigned interval*/);

	Raytracer* mRaytracer = nullptr;
	std::vector<std::vector<color_t>> mFrameBuffer;
	int mHeight, mWidth;
};
