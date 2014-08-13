#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include <cmath>
#include <cstring>
#include <stack>

using namespace std;

#define PI 3.141592653587

#define deg2rad(x) ((x)*PI/180)
#define rad2deg(x) ((x)*180/PI)

#define INF 1e20

#define EPSILON 1e-6 

//class MatrixStack;

//extern MatrixStack *transStack;

typedef float Vec2f[2];
typedef float Vec3f[3];
typedef float Vec4f[4];


class Point;

class Vector {
public:
	Vector() { x = y = z = 0.0; }
	Vector(float xi, float yi, float zi) : x(xi), y(yi), z(zi) { };
	Vector(const Vector &v) { x = v.x; y = v.y; z = v.z; }
	Vector(const float v[3]) {x = v[0]; y = v[1]; z = v[2]; }

	explicit Vector(const Point &p);  //?? what for

	
	Vector operator+(const Vector &v) const {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

	Vector operator+(const Point &p) const;

	Vector operator-(const Vector &v) const {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
	
	Vector operator-() const {
		return Vector(-x, -y, -z);
	}	

	Vector operator* (const float f) const {
		return Vector(x*f, y*f, z*f);
	}

	float operator* (const Vector v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	Vector operator/ (const float f) const {
		float inv = 1.0/f;
		return Vector(x*inv, y*inv, z*inv);
	}

	Vector& operator*= (const float f) {
		x *= f; y *= f; z *= f;
		return *this;
	}

	Vector& operator /= (const float f) {
		float inv = 1.0/f;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}
	friend Vector operator* (float s, const Vector &v) {
		return Vector(v.x * s, v.y *s , v.z * s);
	}
	// dir and normal must be normalized vector
	friend Vector reflect(const Vector& dir, const Vector& normal){
		return dir - 2*(normal * dir) * normal;
	}

	void normalize() {
		*this /= Length();
	}

	float LengthSquared() const { return x*x + y*y + z*z; }
	float Length() const { return sqrtf(LengthSquared()); }

	float x , y, z;
};

inline Vector cross (const Vector &v1, const Vector& v2)
{
	Vector v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}


class Normal {
public:

	Normal () { x = y = z = 0.0; }
	Normal (float xi, float yi, float zi)	 {
		x = xi; y = yi; z = zi;
	}

	Normal operator + (const Normal &n ) const {
		return Normal(x + n.x , y + n.y, z + n.z);
	}

	Normal operator - (const Normal &n) const {
		return Normal(x - n.x, y - n.y, z - n.z);
	}
	
	void normalize() {
		float len = length();
		float len_inv = (len == 0? 0:(1.0/len));
		x *= len_inv; y *= len_inv; z *= len_inv;
	}

	float lengthSquared() const { return x*x + y*y + z*z; }
	float length() const { return sqrtf(lengthSquared()); }
	float x, y , z;
};

class Point2 {
public:
	float x, y;
};

class Point {
public:

	Point() { x = y = z = 0.0;}
	Point(float xi, float yi, float zi) : x(xi), y(yi), z(zi) { };
	Point(const Point &p) : x(p.x), y(p.y), z(p.z) { };
	Point(const Vector &v): x(v.x), y(v.y), z(v.z) { };
	Point(const float vec3[3]): x(vec3[0]), y(vec3[1]), z(vec3[2]) { };
	Point operator + (const Point &p) const {
		return Point(x + p.x, y + p.y, z + p.z );
	}
	Vector operator - (const Point &p) const {
		return Vector(x- p.x, y - p.y, z - p.z);
	}

	float x, y, z;
};


class ONB {
public:
	ONB(): U(), V(), W() { };
	ONB(const Vector &a, const Vector &b, const Vector &c)
		: U(a), V(b), W(c) { };
	void initFromUV(const Vector& u, const Vector& v);
	void initFromWV(const Vector& w, const Vector& v);

	Vector u() { return U; }
	Vector v() { return V; }
	Vector w() { return W; }

	Vector U, V, W;
};



inline Vector::Vector(const Point &p)
	:x(p.x), y (p.y), z(p.z) { }

inline Vector Vector::operator+(const Point &p) const {
	return Vector(x + p.x, y + p.y , z + p.z);
}


class Matrix4x4{
public:
	Matrix4x4(const float f = 1.0) {
		mat[0][0] = f; mat[0][1] = 0.0; mat[0][2] = 0.0; mat[0][3] = 0.0; 
		mat[1][0] = 0.0; mat[1][1] = f; mat[1][2] = 0.0; mat[1][3] = 0.0; 
		mat[2][0] = 0.0; mat[2][1] = 0.0; mat[2][2] = f; mat[2][3] = 0.0; 
		mat[3][0] = 0.0; mat[3][1] = 0.0; mat[3][2] = 0.0; mat[3][3] = f; 
	}
	
	Matrix4x4(float *value) {
		memcpy(mat, value, sizeof(float) * 16);
	}

	Matrix4x4(const Matrix4x4 &m) {
		memcpy(mat, m.mat, sizeof(float) * 16);
	}
	
	void Identity() {
		mat[0][0] = 1.0; mat[0][1] = 0.0; mat[0][2] = 0.0; mat[0][3] = 0.0; 
		mat[1][0] = 0.0; mat[1][1] = 1.0; mat[1][2] = 0.0; mat[1][3] = 0.0; 
		mat[2][0] = 0.0; mat[2][1] = 0.0; mat[2][2] = 1.0; mat[2][3] = 0.0; 
		mat[3][0] = 0.0; mat[3][1] = 0.0; mat[3][2] = 0.0; mat[3][3] = 1.0; 
	}
	float* operator[](unsigned int i) {
		//assert(i < 4);
		return mat[i];
	}
#if 0  // do not deep copy here
	Matrix4x4 operator = (Matrix4x4 &m){
		memcpy(mat, m.mat, sizeof(float) * 16);
		return *this;
	}
#endif
	Point operator* (const Point& p) {
		Point ret;
		ret.x = p.x*mat[0][0] + p.y*mat[0][1] + p.z*mat[0][2] + mat[0][3];
		ret.y = p.x*mat[1][0] + p.y*mat[1][1] + p.z*mat[1][2] + mat[1][3];
		ret.z = p.x*mat[2][0] + p.y*mat[2][1] + p.z*mat[2][2] + mat[2][3];
		return ret;
	}

	Vector operator* (const Vector& v) {
		Vector ret;
		ret.x = v.x*mat[0][0] + v.y*mat[0][1] + v.z*mat[0][2];
		ret.y = v.x*mat[1][0] + v.y*mat[1][1] + v.z*mat[1][2];
		ret.z = v.x*mat[2][0] + v.y*mat[2][1] + v.z*mat[2][2];
		return ret;
	}
	
	
	Matrix4x4 operator* (const Matrix4x4& m) {
		Matrix4x4 ret;
		for(int i = 0; i < 4; ++i) {
			for(int j= 0; j < 4; ++j) {
				ret[i][j] = mat[i][0]*m.mat[0][j] + mat[i][1]*m.mat[1][j]
							+ mat[i][2]*m.mat[2][j] + mat[i][3]*m.mat[3][j];
			}
		}
		return ret;
	}

	Matrix4x4& operator*= (const Matrix4x4& m) {
		Matrix4x4 tmp = *this;
		for(int i = 0; i < 4; ++i) {
			for(int j= 0; j < 4; ++j) {
				mat[i][j] = tmp.mat[i][0]* m.mat[0][j] 
							+ tmp.mat[i][1] * m.mat[1][j] 
							+ tmp.mat[i][2] * m.mat[2][j] 
							+ tmp.mat[i][3] * m.mat[3][j];
			}
		}
		return *this;
	}
	Matrix4x4 Inverse() const;
	Matrix4x4 Transpose() const;
	float mat[4][4];
private:
	float determinate() const;
};

inline float det3(float a, float b, float c,
					float d, float e, float f,
					float g, float h, float i)
{
	return a*e*i + d*h*c + g*b*f - g*e*c - d*b*i - a*h*f;
}

inline float Matrix4x4::determinate() const
{
	float det;

	det  = mat[0][0] * det3(mat[1][1], mat[1][2], mat[1][3],
						mat[2][1], mat[2][2], mat[2][3],
						mat[3][1], mat[3][2], mat[3][3]);

	det -= mat[0][1] * det3(mat[1][0], mat[1][2], mat[1][3],
						mat[2][0], mat[2][2], mat[2][3],
						mat[3][0], mat[3][2], mat[3][3]);

	det += mat[0][2] * det3(mat[1][0], mat[1][1], mat[1][3],
						mat[2][0], mat[2][1], mat[2][3],
						mat[3][0], mat[3][1], mat[3][3]);
	
	det -= mat[0][3] * det3(mat[1][0], mat[1][1], mat[1][2],
						mat[2][0], mat[2][1], mat[2][2],
						mat[3][0], mat[3][1], mat[3][2]);

	return det;
}

inline Matrix4x4 Matrix4x4::Transpose() const 
{
	Matrix4x4 trans;
	for(int i = 0; i < 4; ++i){
		for(int j = 0; j < 4; ++j) {
			trans.mat[i][j] = mat[j][i];
		}
	}
	return trans;
}

inline Matrix4x4 Matrix4x4::Inverse() const
{
	Matrix4x4 inverse;
	float det = determinate();
	inverse.mat[0][0] = det3(mat[1][1], mat[1][2], mat[1][3],
							mat[2][1], mat[2][2], mat[2][3],
							mat[3][1], mat[3][2], mat[3][3])/det;
	inverse.mat[0][1] = -det3(mat[0][1], mat[0][2], mat[0][3],
							mat[2][1], mat[2][2], mat[2][3],
							mat[3][1], mat[3][2], mat[3][3])/det;
	inverse.mat[0][2] = det3(mat[0][1], mat[0][2], mat[0][3],
							mat[1][1], mat[1][2], mat[1][3],
							mat[3][1], mat[3][2], mat[3][3])/det;
	inverse.mat[0][3] = -det3(mat[0][1], mat[0][2], mat[0][3],
							mat[1][1], mat[1][2], mat[1][3],
							mat[2][1], mat[2][2], mat[2][3])/det;
	
	inverse.mat[1][0] = -det3(mat[1][0], mat[1][2], mat[1][3],
							mat[2][0], mat[2][2], mat[2][3],
							mat[3][0], mat[3][2], mat[3][3])/det;
	inverse.mat[1][1] =  det3(mat[0][0], mat[0][2], mat[0][3],
							mat[2][0], mat[2][2], mat[2][3],
							mat[3][0], mat[3][2], mat[3][3])/det;
	inverse.mat[1][2] = -det3(mat[0][0], mat[0][2], mat[0][3],
							mat[1][0], mat[1][2], mat[1][3],
							mat[3][0], mat[3][2], mat[3][3])/det;
	inverse.mat[1][3] = det3(mat[0][0], mat[0][2], mat[0][3],
							mat[1][0], mat[1][2], mat[1][3],
							mat[2][0], mat[2][2], mat[2][3])/det;
	
	inverse.mat[2][0] = det3(mat[1][0], mat[1][1], mat[1][3],
							mat[2][0], mat[2][1], mat[2][3],
							mat[3][0], mat[3][1], mat[3][3])/det;
	inverse.mat[2][1] = -det3(mat[0][0], mat[0][1], mat[0][3],
							mat[2][0], mat[2][1], mat[2][3],
							mat[3][0], mat[3][1], mat[3][3])/det;
	inverse.mat[2][2] = det3(mat[0][0], mat[0][1], mat[0][3],
							mat[1][0], mat[1][1], mat[1][3],
							mat[3][0], mat[3][1], mat[3][3])/det;
	inverse.mat[2][3] = -det3(mat[0][0], mat[0][1], mat[0][3],
							mat[1][0], mat[1][1], mat[1][3],
							mat[2][0], mat[2][1], mat[2][3])/det;

	inverse.mat[3][0] = -det3(mat[1][0], mat[1][1], mat[1][2],
							mat[2][0], mat[2][1], mat[2][2],
							mat[3][0], mat[3][1], mat[3][2])/det;
	inverse.mat[3][1] = det3(mat[0][0], mat[0][1], mat[0][2],
							mat[2][0], mat[2][1], mat[2][2],
							mat[3][0], mat[3][1], mat[3][2])/det;
	inverse.mat[3][2] = -det3(mat[0][0], mat[0][1], mat[0][2],
							mat[1][0], mat[1][1], mat[1][2],
							mat[3][0], mat[3][1], mat[3][2])/det;
	inverse.mat[3][3] = det3(mat[0][0], mat[0][1], mat[0][2],
							mat[1][0], mat[1][1], mat[1][2],
							mat[2][0], mat[2][1], mat[2][2])/det;
	
	return inverse;		
}



/*
class MatrixStack {
public:
	stack<Matrix4x4> mStack;
	MatrixStack() {
		LoadIdentity();
	}

	void Push();
	void Pop(void) {  mStack.pop(); }
	
	Matrix4x4& Top() { return  mStack.top(); }
	void Translate(float x, float y, float z);
	void Scale(float x, float y, float z);
	void Rotate(float x, float y, float z, float angle);
	void LoadIdentity() {
		Matrix4x4 m(1.0);
		mStack.push(m);
	}
};
*/
#endif
