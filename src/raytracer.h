#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <stack>
#include <string>
#include <pthread.h>
#include "scene.h"

using namespace std;

class RayTracer {
public:
	RayTracer(char *filename, char *dir, int depth, int num_t = 1):
			input(filename), outdir(dir), num_threads(num_t) { 
		scene = new Scene; 
		maxdepth = depth;
		num_frames = 1;
		// load identity
//		transStack.Push(Matrix4x4(1.0));
	}
	~RayTracer() {
		delete scene;
		delete film;
		free(threads);
		free(threads_id);
	}
	void initThreads() {
		threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
		threads_id = (int*) malloc(num_threads * sizeof(int));
		for(int i = 0; i< num_threads; ++i) threads_id[i] = i;
		pixel_per_thread = ceil(width * height / (float)num_threads);
	}
	void trace();
	void setResXY(int w, int h) {
		width = w; height = h;
		film = new Film(w, h);
	 }
	void setTime(float start, float end, int num) {
		start_time = start;
		end_time = end;
		num_frames = num;
		step = (end_time - start_time) /num_frames;
	}
	void traceOneFrame(float time);

	Scene *scene;	
	Film* film;

	float start_time;
	float end_time;
	float step;
	int num_frames;

	int width;
	int height;
	int maxdepth;
//private:
	string input;
	string outdir;
	pthread_t *threads;
	int* threads_id;
	int pixel_per_thread;
	int num_threads;
};

extern RayTracer *rayTracer;
//void* traceThread(void *idx);

#endif
