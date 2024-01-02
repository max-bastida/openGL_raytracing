/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Pyramid class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_PYRAMID
#define H_PYRAMID

#include <glm/glm.hpp>
#include "SceneObject.h"
#include "Plane.h"

class Pyramid
{
private:
	Plane* plane_a = new Plane();
	Plane* plane_b = new Plane();
	Plane* plane_c = new Plane();
	Plane* plane_d = new Plane();

public:
	Pyramid() = default;

	Pyramid(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd) {
		plane_a = new Plane(pa, pb, pc);
		plane_b = new Plane(pa, pc, pd);
		plane_c = new Plane(pa, pd, pb);
		plane_d = new Plane(pb, pd, pc);
	}

	void performFunction(void (*func)(Plane* plane));

};

#endif //!H_PYRAMID
