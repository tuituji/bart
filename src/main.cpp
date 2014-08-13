#include "parse.h"
#include "raytracer.h"

RayTracer *rayTracer;

stack<TransformHierarchy* > transformHierarchy;

#define MAX_DEPTH 5
#define NUM_THREADS 16
#define NUM_SAMPLES 9

int main(int argc, char ** argv)
{
	if(argc != 3) {
		fprintf(stderr, "%s <file_name> <outdir>\n", argv[0]);
		return -1;
	}

	FILE *fp = fopen(argv[1], "r");

	rayTracer = new RayTracer(argv[1], argv[2], MAX_DEPTH, NUM_THREADS);
	transformHierarchy.push(NULL);
	if(viParseFile(fp) == false) {
		fprintf(stderr, "Parse file false\n");
	}
	rayTracer->film->sampler = new JitterSampler(NUM_SAMPLES);
	rayTracer->trace();
	fclose(fp);
	return 0;
}
