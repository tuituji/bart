#ifndef _MATERIAL_H
#define _MATERIAL_H

class Color {
public:
	Color() { r = g = b = 0.0; }
	Color(float rr, float gg, float bb): r(rr), g(gg), b(bb) { };
	Color(float c[3]): r(c[0]), g(c[1]), b(c[2]) { };
	float r, g, b;

	Color operator* (const Color& col) const {
		return Color(r * col.r, g * col.g, b * col.b);
	}
	Color operator* (const float s) const {
		return Color(r * s, g * s, b * s);
	}
	Color operator+ (const Color &col) {
		return Color(r + col.r, g + col.g, b + col.b);
	}
	Color& operator+= (const Color& col) {
		r += col.r; g += col.g; b += col.b;
		return *this;
	}
	Color& operator*= (const Color& col) {
		r *= col.r; g *= col.g; b *= col.b;
		return *this;
	}
	Color& operator*= (const float s) {
		r *= s; g *= s; b *= s;
		return *this;
	}
	void clamp() {
		r = (r >1.0 ? 1.0: r);
		g = (g >1.0 ? 1.0: g);
		b = (b >1.0 ? 1.0: b);
	}
};

class Material {
public:
	Color ambient;
	Color diffuse;
	Color specular;
	Color reflect;
	Color emission; // we do not use it now
	float shiness;
	float T;
	float ior;
	Material(): ambient(), diffuse(), specular(), 
		reflect(0.1, 0.1, 0.1),emission(), shiness(0.0), T(1.0), ior(0.0) {  }
	void setAmbient(const Color a) {
		ambient = a;
	}
	void setDiffuse(const Color d) {
		diffuse = d;
	}
	void setSpecular(const Color s) {
		specular = s;
	}
	void setReflect(const Color r) {
		reflect = r;
	}
};

#endif
