#include <vector>
#include <cassert>
#include "scene.h"
#include "shape.h"
#include "transform.h"
#include "geometry.h"


void Scene::doAnimate(float time) {
	AnimationList *anil;
	double rot[4];
	double trans[3];
	double scal[3];

	// update all the animation matrix
	for(anil = mAnimations; anil != NULL; anil = anil->next) {
		Animation* ani = &anil->animation;
		_GetRotation(ani, time, rot);
		_GetTranslation(ani, time, trans);
		_GetScale(ani, time, scal);
		if(strcmp(ani->name, "camera") == 0) {
		//	printf("we do not care camera now\n");
			camera->cameraAnimate(trans, rot, scal);
		}
		else {
			string str(ani->name);
			map<string, TransformHierarchy*>::iterator itr;
			itr = XFormMap.find(str);
			assert(itr != XFormMap.end());
			TransformHierarchy* th = itr->second;
			th->xFormAnimate(trans, rot, scal);
		}
	}
	// update all those animated triangle
	vector<Mesh*>::iterator itr;
	for (itr = aniMesh.begin(); itr != aniMesh.end(); ++itr) {
		TransformHierarchy *tmp;
		(*itr)->transMatrixWorld.Identity();	
		for(tmp = (*itr)->meshTrans; tmp != NULL; tmp = tmp->transParent) {
			(*itr)->transMatrixWorld = tmp->transformMatrix * 
					(*itr)->transMatrixWorld;
		}
		(*itr)->transMatrixWorldInvT= 
				(*itr)->transMatrixWorld.Inverse().Transpose();
		(*itr)->updateVertex();
	}

	vector<Triangle_TPA*>::iterator itr1;
	for (itr1 = triTPA.begin(); itr1 != triTPA.end(); ++itr1) {
		(*itr1)->updateVertex(time);
	}
}

bool Scene::Intersect(const Ray &ray,  HitRecord& record)
{
	return mBVH->intersect(ray, record);
#if 0
	record.t = INF;
	vector<Shape*>::iterator itr;
	for(itr = shapes.begin(); itr != shapes.end(); ++itr) {
//		Point o = (*itr)->transform_inv * ray.pos;
//		Vector dir = (*itr)->transform_inv * ray.dir;
		Point o(ray.pos);
		Vector dir(ray.dir);
//		dir.normalize();  // notice we still normalize it here
						  // whether not to changed it but to changed the 
						  // shape intersect function ??
		Ray tray(o, dir); // t will not be the same 
		HitRecord record0;
		// here shape->intersect will return everything
		if((*itr)->intersect(tray, record0) && record0.t < record.t) {
			record = record0;
		}
	}
	if(record.t < INF){
		// record.pos = Point(ray.pos + record.t * ray.dir);// the actual pos 
		// well since we have normalized the transformed vector
		// thus t is not the same
//		record.pos = record.obj->transform * record.pos;// the actual pos 
//		record.normal = (record.obj->transform_inv).Transpose()* record.normal;
		record.normal.normalize();// remember to normalize the normal
		return true;
	}
	return false;
#endif
}

Texture* Scene::addTexture(string name)
{
	map<string, Texture*>::iterator t_it;
	t_it = textures.find(name);
	if(t_it != textures.end()) {
		return t_it->second;
	}
	Texture* tex = viReadPPM(name.c_str());
	assert(tex != NULL);
	textures.insert(pair<string, Texture*>(name, tex));
	return tex;
}

Color Scene::Radiance(Ray &ray, int depth, float tIndex)
{
	HitRecord record;
	if(depth -- == 0){
		return Color(0.0, 0.0, 0.0);
	} // depth to max

	if(!Intersect(ray, record)) {
		return Color(0.0, 0.0, 0.0); // if not hit  return black;
	}

	// if hit
	Color col, col1;
	Color diffuse, specular, ambient;
	Vector L, R, H;

	const Material& mat = record.obj->getMaterial();
//	col = mat.ambient;
	col = mat.emission;
	R = reflect(ray.dir, record.normal);// get the reflected ray
	// to get the radiance value
	vector<Light*>::iterator itr;
	HitRecord record1;
	for(itr = lights.begin(); itr != lights.end(); ++itr) {
		// should first check the light type;
		if((*itr)->type == AmbientLIGHT) {
			ambient = (*itr)->col;
			continue;
		}
		float tt = INF;
		L = -((*itr)->getDir(record.pos, tt));  //(*itr).pos - record.pos;

#if 1
		Ray ray1 = Ray(record.pos + EPSILON * L, L);
		if(Intersect(ray1, record1)) {		
			if(record1.t < tt){
//  The following will not happen since we have add a bias to intersect t
//			if(record1.obj == record.obj){ 
//				if(record1.pos != record.pos){
//					printf("may come to error\n");
//				}
//			}	
				continue;// if light is shadowed the pass
			}
		}
#endif
		H = (L -ray.dir)/2;// notice here
		H.normalize();     // remember to normalize, otherwise errors occur
		// else not shadowed
		float nDotL = record.normal * L;
		float nDotH = record.normal * H;
//		if(nDotL > 0.0) {
//			printf("shoule be > 0\n");
//		}
		diffuse += (*itr)->col * ( nDotL >0.0 ? nDotL : 0);
		specular += (*itr)->col * pow((nDotH > 0.0 ? nDotH : 0), mat.shiness);	
	}

	col += ambient * mat.ambient + mat.diffuse * diffuse + mat.specular * specular;

	// recursive call
	Ray ref = Ray(record.pos, R);
	col1 = Radiance(ref, depth, tIndex);
	// add the color
	//col = col  + col1 * mat.reflect;
	float nDotL = record.normal * R;
	col = col  + col1 * mat.specular * (1 -mat.T); // ok 
			//+ col1 * mat.diffuse * (nDotL > 0.0 ? nDotL : 0); // ok ??
// add the refraction 
	if(mat.T != 0.0 ) {
		float ior = mat.ior;
        float cosI = (-1) *(record.normal * ray.dir);
		if(cosI < 0) {  // from inside
			record.normal = (-1) * record.normal;
			ior = 1.0;
		}
		//float n = tIndex / mat.T;
		float n = tIndex / ior;
        float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
		Color col2;
        if (cosT2 > 0.0f) { 
            Vector vecT = (n*ray.dir) + (n*cosI-sqrtf( cosT2 )) * record.normal;
			Ray tray(record.pos + vecT* EPSILON, vecT);
            col2 = Radiance( tray, depth, ior);
		}
		else {   // full relection
			Ray rray(record.pos + R * EPSILON, R);
            col2 = Radiance( rray, depth, ior);
		}
        col += col2 * mat.T;
	}

	col *= record.tex_col;
	// clamp the color
		
	col.clamp();
//	if(col.r == 0.0 && col.g == 0.0 && col.b == 0.0)	{
//		col = backgroundColor;
//	}
	return col;
}

void doFreeBVH(BVH * bvh)
{
	if(bvh== NULL) return;
	BVH *left = bvh->left;
	BVH *right = bvh->right;
	delete bvh;
	doFreeBVH(left);
	doFreeBVH(right);
}

void Scene::freeBVH() 
{
	if(mBVH == NULL)
		return;
	doFreeBVH(mBVH);	
	mBVH = NULL;
}

void Scene::buildBVH() {
	if(mBVH) freeBVH();
	vector<Shape*>::iterator itr;
	for(itr = shapes.begin(); itr != shapes.end(); ++itr) {
		(*itr)->updateBoundingBox();	
	}
	mBVH = new BVH(shapes); 
} 


