/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Pyramid class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Pyramid.h"
#include <math.h>

/**
* Pyramid's intersection method.  The input is a ray (p0, dir).
* See slides Lec08-Slide 29
*/
void Pyramid::performFunction(void (*func)(Plane* plane)) {
	func(plane_a);
	func(plane_b);
	func(plane_c);
	func(plane_d);
}



