#ifndef _SAMPLER_H
#define _SAMPLER_H

#include <vector>
#include "geometry.h"

using namespace std;

class Sampler {
public:
	~Sampler() {
		delete [] samples;
	}
	virtual void generateSamples() = 0;
	Sampler(int num) : num_samples(num), idx(0), jump(0) { };
	Point2 sampleNext();
	Point2 sample(int i);
	int getNumSamplers() { return num_samples; }
protected:
	int num_samples;
	Point2* samples;
	int idx;	
	int jump ;
};


class RandomSampler : public Sampler {
public:
	RandomSampler(int num) : Sampler(num) {
		samples = new Point2[num_samples];
		generateSamples();
	};
private:
	void generateSamples();
};


class JitterSampler : public Sampler {
public:	
	JitterSampler(int num): Sampler(num) {
		sqrt_num = (int)(sqrt(num));
		num_samples = sqrt_num * sqrt_num;
		samples = new Point2[num_samples];
		generateSamples();
	}
private:
	void generateSamples();
	int sqrt_num;
};

class MultiJitterSampler : public Sampler {
public:
	MultiJitterSampler(int num) : Sampler(num) {
		sqrt_num = (int)(sqrt(num));
		num_samples = sqrt_num * sqrt_num;
		samples = new Point2[num_samples];
		generateSamples();
	}
private:
	void generateSamples();
	int sqrt_num;
};
#endif
