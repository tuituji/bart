#ifndef _MESH_H
#define _MESH_H

#include <vector>
#include <cstdio>
#include <cstdlib>
#include "shape.h"
#include "ray.h"
#include "texture.h"
#include "transform.h"

//typedef float Vec2f[2];
//typedef float Vec3f[3];
//typedef float Vec4f[4];

class Triangle;
class Mesh {
public:	
	Mesh(Vec2f *t, int nt, Vec3f *n, int nn, Vec3f *v, int nv) : 
		isStatic(true) 
	{
		txts = t;   num_txts = nt;
		norms = n;  num_norms = nn;
		verts = v;  num_verts = nv;
		if(norms) {
			norms_back = (Vec3f*)malloc(num_norms * sizeof(Vec3f)) ;
			memcpy(norms_back, norms, num_norms * sizeof(Vec3f));
		}
		verts_back = (Vec3f*)malloc(num_verts * sizeof(Vec3f));
		memcpy(verts_back, verts, num_verts * sizeof(Vec3f));
	
		texture = NULL;
	}
	~Mesh() {
		free(verts);
		free(verts_back);
		free(norms);
		free(norms_back);
		free(txts);
	}
	Texture *texture;
	Vec3f *verts;
	Vec3f *verts_back;

	Vec2f *txts;
	Vec3f *norms;
	Vec3f *norms_back;
	vector<Triangle *> tris;
	bool isStatic;  // calculating it 
	size_t num_verts;
	size_t num_norms;
	size_t num_txts;
	Matrix4x4 transMatrixWorld; //when initialize, shoule be identity matrix 
	Matrix4x4 transMatrixWorldInvT;
	Material material;
	TransformHierarchy *meshTrans;
	void addTriangle(Triangle *tri) {
		tris.push_back(tri);
	}

	void setTransformHierarchy(TransformHierarchy *h);

	Vector getNormal(int idx0, int idx1, int idx2, float beta, float gamma) {
        Vector norm;
        float alpha = 1 - beta - gamma;
        norm.x = norms[idx0][0] * alpha + norms[idx1][0] * beta +
                norms[idx2][0] * gamma;

        norm.y = norms[idx0][1] * alpha + norms[idx1][1] * beta +
                norms[idx2][1] * gamma;
        norm.z = norms[idx0][2] * alpha + norms[idx1][2] * beta +
                norms[idx2][2] * gamma;
        return norm;
    }
    Color sampleTexture(int idx0, int idx1, int idx2,
                        float beta, float gamma) {
        if(texture == NULL || txts == NULL || num_txts == 0) {
            return Color(1.0, 1.0, 1.0);
        }

        float alpha = 1 - beta - gamma;
        float u = txts[idx0][0] * alpha + txts[idx1][0] * beta +
                txts[idx2][0] * gamma;
        float v = txts[idx0][1] * alpha + txts[idx1][1] * beta +
                txts[idx2][1] * gamma;

        return texture->sample(u, v);
    }

	void meshAnimate(double trans[3], double rot[4], double scal[4]);
	void updateVertex();

};

class Triangle :public Shape {
public:
	Triangle(Mesh *m) {
		int i = 0;
		for(i = 0 ; i < 3; ++i) {
			idx_vert[i] = idx_txt[i] = idx_norm[i] = -1;
		}
		meshPtr = m;
	}
	bool intersect(const Ray &ray, HitRecord &r);
	void setVertIdx(unsigned short id0, unsigned short id1,
				unsigned short id2) {
		idx_vert[0] = id0;
		idx_vert[1] = id1;
		idx_vert[2] = id2;
	}
	void setNormIdx(unsigned short id0, unsigned short id1, 
				unsigned short id2) {
		idx_norm[0] = id0;
		idx_norm[1] = id1;
		idx_norm[2] = id2;
	}
	void setTxtIdx(unsigned short id0, unsigned short id1,
				unsigned short id2) {
		idx_txt[0] = id0;
		idx_txt[1] = id1;
		idx_txt[2] = id2;
	}
	Material getMaterial() {
		return meshPtr->material;
	}
	Vector getNormal(int idx0, int idx1, int idx2, float beta, float gamma) {
		Vector vec;
		Vec3f *norms = meshPtr->norms;
	    float alpha = 1 - beta - gamma;
	    vec.x = norms[idx0][0] * alpha + norms[idx1][0] * beta + 
				norms[idx2][0] * gamma;
	    vec.y = norms[idx0][1] * alpha + norms[idx1][1] * beta + 
				norms[idx2][1] * gamma;
	    vec.z = norms[idx0][2] * alpha + norms[idx1][2] * beta + 
				norms[idx2][2] * gamma;
		return vec;	
	}
	Color sampleTexture(int idx0, int idx1, int idx2,
						float beta, float gamma) {
		if(meshPtr->texture == NULL || meshPtr->txts == NULL || 
			meshPtr->num_txts == 0) {
			return Color(1.0, 1.0, 1.0);
		}
		Vec2f* txts = meshPtr->txts;
		Texture *texture = meshPtr->texture;
	    float alpha = 1 - beta - gamma;
	    float u = txts[idx0][0] * alpha + txts[idx1][0] * beta + 
				txts[idx2][0] * gamma;
	    float v = txts[idx0][1] * alpha + txts[idx1][1] * beta + 
				txts[idx2][1] * gamma;
		return texture->sample(u, v);	
	}
	BBox boundingBox(){ return bbox; }
	void updateBoundingBox();
	unsigned short idx_vert[3];
	unsigned short idx_txt[3];
	unsigned short idx_norm[3];
	Mesh *meshPtr;
};

class Triangle_TPA : public Shape {
public:

	Triangle_TPA(int n, float *ts, Vec3f* vs, Vec3f *ns, Material &mat):
		num_times(n), times(ts), verts_bak(vs), norms_bak(ns) {	
		material = mat; 	
	};
	~Triangle_TPA() {
		free(verts_bak);
		free(times);
		free(norms_bak);
		for(int i = 0; i < 3; ++i) {
			free(aniNorm[i]);
			free(aniPos[i]);
		}
	}

	void updateBoundingBox();
	void updateVertex(float time); 
	bool intersect(const Ray &ray, HitRecord &r);
	Vector getNormal(float beta, float gamma) {
		Vector normal;
		float alpha = 1 - beta - gamma;
		normal = alpha * norms[0] + beta * norms[1] + gamma * norms[2];
		return normal;
	}
	Material getMaterial() {
		return material;
	}
	Vec3f *verts_bak;
	Vec3f *norms_bak;
	int num_times;
	float* times;
	Point points[3];
	Vector norms[3];
	float time;
	void *aniPos[3];
	void *aniNorm[3];
	Material material;

};

#endif
