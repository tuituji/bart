#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include "shape.h"
#include "material.h"

class Primitive {
public:
	Primitive(Shape *s): shape(s) { };
	
	Shape *shape;
	Material *mate; // i put it to Shape and do not use Primitive now
};

#endif
