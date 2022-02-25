#include <vector>
#include "Model.h"

class Scene;

class Raytracer
{
public:
	Raytracer() = delete;
	Raytracer(Scene* scene) { mpScene = scene; }
	~Raytracer() {}

	color_t calculatePixelColor(int wHeight, int wWidth, int rowIdx, int colIdx);

private:
	unsigned findMinHitIdx(std::vector<hit_t>& rayOnObjects);
	color_t shade(Matrix<double> &e, Matrix<double> &d, int k);

	Scene* mpScene = nullptr;
};
Matrix<double>* projTrans(Matrix<double> &P);