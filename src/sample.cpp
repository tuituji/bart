#include "sample.h"
#include <cstdlib>
#include <cstdio>

Point2 Sampler::sampleNext()
{
	return samples[idx++ % num_samples];
}

Point2 Sampler::sample(int i)
{
	return samples[i];
}

void RandomSampler::generateSamples()
{
	Point2 p;
	for(int i = 0; i < num_samples; ++i) {
		p.x = drand48();
		p.y = drand48();
		samples[i] = p;
	}
}

void JitterSampler::generateSamples()
{
	Point2 p;
	for(int i = 0; i < sqrt_num; ++i) {
		for(int j = 0; j < sqrt_num; ++j) {
			p.x = (i + drand48()) / (double)sqrt_num;
			p.y = (j + drand48()) / (double)sqrt_num;
			samples[i * sqrt_num + j]= p;
		}
	}
}

void MultiJitterSampler::generateSamples()
{
	printf( "error\n");
	float width = 1.0/sqrt_num;
	Point2 p;
	for(int i = 0; i < sqrt_num; ++i) {
		for(int j = 0; j < sqrt_num; ++j) {
			p.x = j * sqrt_num * width + j * width;
			
		}
	}


}
