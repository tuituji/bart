LINKFLAGS = -lm 
CFLAGS    =  -g3  #-DDEBUG# -Wall
CC        = g++  -pthread
SRCS      = $(wildcard *.cpp)
OBJS      = $(SRCS:.cpp=.o)
PROG      = raytracer

all: $(SRCS) $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(INCFLAGS) $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

depend:
	makedepend $(INCFLAGS) -Y $(SRCS)

clean:
	rm $(OBJS) $(PROG)

# DO NOT DELETE

mesh.o: mesh.h shape.h material.h geometry.h ray.h bbox.h scene.h camera.h
mesh.o: light.h primitive.h texture.h animation.h kbsplpos.h kbsplrot.h
mesh.o: transform.h bvh.h
sphere.o: shape.h material.h geometry.h ray.h bbox.h
cone.o: cone.h shape.h material.h geometry.h ray.h bbox.h
scene.o: scene.h camera.h ray.h geometry.h light.h material.h shape.h bbox.h
scene.o: primitive.h texture.h mesh.h transform.h animation.h kbsplpos.h
scene.o: kbsplrot.h bvh.h
raytracer.o: raytracer.h scene.h camera.h ray.h geometry.h light.h material.h
raytracer.o: shape.h bbox.h primitive.h texture.h mesh.h transform.h
raytracer.o: animation.h kbsplpos.h kbsplrot.h bvh.h
kbsplpos.o: kbsplpos.h
transform.o: transform.h geometry.h
quat.o: quat.h
texture.o: texture.h material.h
bbox.o: bbox.h geometry.h ray.h
bvh.o: bvh.h shape.h material.h geometry.h ray.h bbox.h
geometry.o: geometry.h
animation.o: animation.h kbsplpos.h kbsplrot.h quat.h
camera.o: camera.h ray.h geometry.h transform.h
main.o: parse.h raytracer.h scene.h camera.h ray.h geometry.h light.h
main.o: material.h shape.h bbox.h primitive.h texture.h mesh.h transform.h
main.o: animation.h kbsplpos.h kbsplrot.h bvh.h
parse.o: parse.h animation.h kbsplpos.h kbsplrot.h texture.h material.h
parse.o: raytracer.h scene.h camera.h ray.h geometry.h light.h shape.h bbox.h
parse.o: primitive.h mesh.h transform.h bvh.h
kbsplrot.o: kbsplrot.h quat.h
