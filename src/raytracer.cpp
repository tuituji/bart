
#include "raytracer.h"
#include "ray.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>


void* traceThread(void *id)
{
	int idx = *(int*)id;
	int width = rayTracer->width;
	int height = rayTracer->height;
	int num_samples = rayTracer->film->sampler->getNumSamplers();	
	int pixel_per_thread = rayTracer->pixel_per_thread;
	int start_pixel = idx * pixel_per_thread;
	Scene *scene = rayTracer->scene;
	Film* film = rayTracer->film;
#if 1 // DEBUG
	printf("Thread %d start pixel: %d, end pixel: %d\n",
		idx, start_pixel, start_pixel + pixel_per_thread);
	fflush(stdout);
#endif
	for(int m = start_pixel; m < start_pixel + pixel_per_thread
		&& m < width * height; ++m) {
		int i = m / width;
		int j = m - i * width;
		Color col;
		for(int k = 0; k < num_samples; ++k){
			Point2 p = film->sampler->sample(k);
			Ray ray = scene->camera->generateRay(
					(j + p.x)/(double)width,
					(i + p.y)/(double)height);
			col += scene->Radiance(ray, rayTracer->maxdepth, 1.0);
		}
		col *= (1.0/num_samples);
		col.clamp();
		film->setPixel( height - i - 1, j ,col);
	}
#if 1// DEBUG
	printf("Thread %d finished\n", idx);
	fflush(stdout);
#endif
}

void RayTracer::traceOneFrame(float time)
{
	scene->doAnimate(time); // update time
	scene->buildBVH();      // update BVH	

	if(num_threads > 1) {
		for(int i = 0; i < num_threads; ++i) {
			pthread_create(&threads[i], NULL,
						traceThread, (void*)&threads_id[i]);
		}
		for(int i = 0; i < num_threads; ++i) {
			pthread_join(threads[i], NULL);
		}
		return;
	}

one_thread:   // only one thread
	int num_samples = film->sampler->getNumSamplers();
	for(int i = 0; i < height; ++i){
		fprintf(stderr,"\rRendering  %5.2f%%", 100.*i/(height));
		for(int j = 0; j < width; ++j){
		// samples here
			
			Color col;
			for(int k = 0; k < num_samples; ++k){
				Point2 p = film->sampler->sampleNext();
				Ray ray = scene->camera->generateRay(
						(j + p.x)/(double)width, 
						(i + p.y)/(double)height);
				col += scene->Radiance(ray, maxdepth, 1.0);
			}
			col *= (1.0/num_samples);
			col.clamp();
				//	if(col.r == 0.0 && col.g == 0.0 && col.b == 0.0) 
				//		col = scene->backgroundColor;
			film->setPixel( height - i - 1, j ,col);
		}
	}
}


void RayTracer::trace()
{
	int i, j, f;
	string name = input.substr(input.find_last_of("/") + 1);
	initThreads();
	for(f =  1; f <= num_frames; ++f) {
		float time = start_time + step * f;
		char output[256];
		printf("Frame: %d, Time : %f\n", f, time);
		mkdir(outdir.c_str(), 0777);
		sprintf(output, "%s/%s-%04d.ppm", outdir.c_str(), name.c_str(), f);
		traceOneFrame(time);
		printf("\n");
		film->outFilm(output);
	}
}



