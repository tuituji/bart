#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <cstring>
#include "camera.h"
#include "light.h"
#include "shape.h"
#include "primitive.h"
#include "texture.h"

#include "mesh.h"
#include "animation.h"
#include "transform.h"
#include "sample.h"
#include "bvh.h"

using namespace std;

inline unsigned char max(unsigned char a, unsigned char b)
{
	return (a>b? a:b);
}

inline unsigned char min(unsigned char a, unsigned char b)
{
	return (a<b? a:b);
}

class Film {
public:
	Film() : width(400), height(300) { } ;
	Film(int w, int h): width(w), height(h) { 
		bitmap = new unsigned char[width * height * 3];
	};
	~Film() {
		delete[] bitmap;	
	}
	int width, height;
	unsigned char *bitmap;
	inline void setPixel(int i, int j, Color col) {
		unsigned char r = min((unsigned char)(col.r * 255), 255);
		unsigned char g = min((unsigned char)(col.g * 255), 255);
		unsigned char b = min((unsigned char)(col.b * 255), 255);
		int idx = (i* width + j) * 3;
		bitmap[idx++] = r;
		bitmap[idx++] = g;
		bitmap[idx++] = b;
	}
	void outFilm(const char *file_name) {
		FILE *fp = fopen(file_name, "w");
//		char head[32];
//		sprintf(head, "P6\n%d %d\n255\n", width, height);
//		fwrite(head, strlen(head), 1, fp);
		fprintf(fp, "P6\n%d %d\n255\n", width, height);
		fwrite(bitmap, width * 3, height, fp);
		fclose(fp);
	}
	
	Sampler *sampler;
};

//class Triangle_TPA;

class Scene {
public:
	Scene() {
		mAnimations = NULL;
		mBVH = NULL;
	};
//	Scene(const Film & f) : film(f) { };
	~Scene()  {
		AnimationList *anil;
		for(anil = mAnimations; anil != NULL; anil = anil->next) {
			free(anil);
		}
		vector<Shape*>::iterator sitr;
		for(sitr = shapes.begin(); sitr != shapes.end(); ++sitr) {
			delete (*sitr);
		}
		freeBVH();
		vector<Light*>::iterator litr;
		for(litr = lights.begin(); litr != lights.end(); ++litr) {
			delete(*litr);
		}
		delete camera;
		map<string, TransformHierarchy*>::iterator mitr;
		for(mitr = XFormMap.begin(); mitr != XFormMap.end(); ++mitr) {
			delete mitr->second;
		}
		map<string, Texture*>::iterator titr;
		for(titr = textures.begin(); titr != textures.end(); ++titr) {
			free (titr->second);
		}
		//if(vertexs) delete[] vertexs;
		// and free primitive
		// and free lights
	}
	bool Intersect(const Ray &ray,  HitRecord& record);
	Texture *addTexture(string name);
	void addTriangleTPA(Triangle_TPA * tri) {
		triTPA.push_back(tri);
		shapes.push_back(tri);
	}
	void addShape(Shape* s) {  
		shapes.push_back(s);
//		shapesMap.insert(pair<string, Shape*>(name, s));
	}
	void addXForm(const char* name, TransformHierarchy *child) {
		XFormMap.insert(pair<string, TransformHierarchy*>(string(name), child));
//		TransformHierarchy * tmp;
//		for(tmp = child; tmp->transParent != NULL; tmp = tmp->transParent) { ;}
//		XFormMapRoot.insert(pair<TransformHierarchy*, bool>(tmp, true));
	}
	void addLight(Light *light) { lights.push_back(light); }
	void doAnimate(float time);
	void buildBVH();
	void freeBVH();
	void addAniMesh(Mesh *mesh) {
		aniMesh.push_back(mesh);
	}
#if 0
	void setTime(float start, float end, int num) {
		start_time = start;
		end_time = end;
		num_frames = num;
		step = (end_time - start_time) /num_frames;
	}
#endif
	
	AnimationList* mAnimations;
	vector<Shape*> shapes;  // all shapes  
	map <string, TransformHierarchy*> XFormMap;  // used for animation
	vector<Mesh*> aniMesh;
	vector<Triangle_TPA*> triTPA;
	vector<Light*> lights;
	Color backgroundColor;
	Color Radiance(Ray &r, int depth, float tIndex);
	map<string, Texture*> textures;	
	Camera *camera;
	BVH *mBVH;
//	float start_time;
//	float end_time;
//	float step;
//	int num_frames};
};

#endif
