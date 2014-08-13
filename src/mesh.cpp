#include "mesh.h"
#include <cassert>
#include <math.h>

#define X 0
#define Y 1
#define Z 2

void Mesh::updateVertex()
{
	int i;
	assert(verts != NULL && num_verts >= 3);
	for(i = 0; i <  num_verts; ++i) {
		Point p(verts_back[i]);
		p =  transMatrixWorld * p;
		verts[i][0] = p.x; verts[i][1] = p.y; verts[i][2] = p.z;
	}
	if(norms){
		assert(num_norms > 0);
		for(i = 0; i < num_norms; ++i) {
			Vector n(norms_back[i]);
			n = transMatrixWorldInvT * n;
			n.normalize();  // need ??
			norms[i][0] = n.x; norms[i][1] = n.y; norms[i][2] = n.z;
		}
	}
}


void Mesh::setTransformHierarchy(TransformHierarchy *h)
{
	meshTrans = h;

	if(h == NULL) return;

	TransformHierarchy *tmp;
	for(tmp = h; tmp != NULL; tmp = tmp->transParent) {
		if(! tmp->isStatic) {
			isStatic = false;
			transMatrixWorld.Identity();
			return;
		}
		transMatrixWorld = tmp->transformMatrix * transMatrixWorld;
	}  // if there is no animation
	transMatrixWorldInvT = transMatrixWorld.Inverse();
	transMatrixWorldInvT = transMatrixWorldInvT.Transpose(); 
	updateVertex();
}

bool Triangle::intersect(const Ray &ray, HitRecord &record)
{
	
//	if(!bbox.intersect(ray)) return false;  // need ??
	
	float tval;
	float eps = 1e-4;
	//Point *p = //mesh_ptr->scene_ptr->vertexs;
	Vec3f *p = meshPtr->verts;
	int idx0 = idx_vert[0];
	int idx1 = idx_vert[1];
	int idx2 = idx_vert[2];

	float A = p[idx0][X] - p[idx1][X];
	float B = p[idx0][Y] - p[idx1][Y];
	float C = p[idx0][Z] - p[idx1][Z];

	float D = p[idx0][X] - p[idx2][X];
	float E = p[idx0][Y] - p[idx2][Y];
	float F = p[idx0][Z] - p[idx2][Z];
	
	float G = ray.dir.x;
	float H = ray.dir.y;
	float I = ray.dir.z;
	
	float J = p[idx0][X] - ray.pos.x;
	float K = p[idx0][Y] - ray.pos.y;
	float L = p[idx0][Z] - ray.pos.z;

	float EIHF = E*I - H*F;
	float GFDI = G*F - D*I;
	float DHEG = D*H - E*G;	
	
	float denom = A * EIHF + B * GFDI + C * DHEG;
	if(denom == 0.0) {
#if DEBUG
		fprintf(stderr, "error denorm is 0\n");
#endif
		return false;
	}
	assert(denom != 0.0);
	float beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
	if( isnan(beta)) { return false; }
//	assert(! isnan(beta));
	if(beta <= 0.0f || beta >= 1.0f) return false;
//	if(beta < 0.0f || beta > 1.0f) return false;

	float AKJB = A*K - J*B;
	float JCAL = J*C - A*L;
	float BLKC = B*L - K*C;
	
	float gamma = (I*AKJB + H *JCAL + G*BLKC)/denom;
	if(gamma <= 0.0f || (beta + gamma) >= 1.0f) return false;
//	if(gamma < 0.0f || (beta + gamma) > 1.0f) return false;

	tval = -(F*AKJB + E*JCAL + D*BLKC)/denom;
	if(tval < eps) return false;

	record.t = tval;
	
	Vector normal;
	if(meshPtr->norms == NULL || meshPtr->num_norms == 0) {
		normal = -cross(
			Vector(p[idx2][X]- p[idx0][X], p[idx2][Y]- p[idx0][Y],
				p[idx2][Z]- p[idx0][Z]), 
			Vector(p[idx1][X]- p[idx0][X], p[idx1][Y]- p[idx0][Y],
				p[idx1][Z]- p[idx0][Z])); 
		normal.normalize();
	}
	else {
		idx0 = idx_norm[0];
		idx1 = idx_norm[1];
		idx2 = idx_norm[2];
		normal = meshPtr->getNormal(idx0, idx1, idx2, beta, gamma);
		normal.normalize();
	}
	record.normal = normal;
	record.obj = this;  // please check it
	record.pos = Point(ray.pos + record.t * ray.dir);
// To set the texture color
	idx0 = idx_txt[0];
	idx1 = idx_txt[1];
	idx2 = idx_txt[2];	
	record.tex_col = meshPtr->sampleTexture(idx0, idx1, idx2, beta, gamma);
	return true;
}

#undef X
#undef Y
#undef Z


BBox TriangleBoundingBox(const Point &p0, const Point &p1, const Point &p2)
{

	const float epsilon = 0.00001f;
	Vector min, max;

	min.x = p0.x < p1.x ? p0.x : p1.x;
	min.y = p0.y < p1.y ? p0.y : p1.y;
	min.z = p0.z < p1.z ? p0.z : p1.z;
	min.x = min.x < p2.x ? min.x : p2.x;
	min.y = min.y < p2.y ? min.y : p2.y;
	min.z = min.z < p2.z ? min.z : p2.z;
		
	max.x = p0.x > p1.x ? p0.x : p1.x;
	max.y = p0.y > p1.y ? p0.y : p1.y;
	max.z = p0.z > p1.z ? p0.z : p1.z;
	max.x = max.x > p2.x ? max.x : p2.x;
	max.y = max.y > p2.y ? max.y : p2.y;
	max.z = max.z > p2.z ? max.z : p2.z;
	
	min.x -= epsilon;
	min.y -= epsilon;
	min.z -= epsilon;

	max.x += epsilon;
	max.y += epsilon;
	max.z += epsilon;

	return BBox(min, max);
}

void Triangle::updateBoundingBox()
{
	Point p0(meshPtr->verts[idx_vert[0]]);
	Point p1(meshPtr->verts[idx_vert[1]]);
	Point p2(meshPtr->verts[idx_vert[2]]);
	bbox = TriangleBoundingBox(p0, p1, p2);
}

void Triangle_TPA::updateBoundingBox()
{
	bbox = TriangleBoundingBox(points[0], points[1], points[2]);
}


#include "kbsplpos.h"
void Triangle_TPA::updateVertex(float time)
{
	Point3 p;
	for(int i = 0; i < 3; ++i) {
		KB_PosInterpolate0(aniPos[i], time, &p);
		points[i].x = p.x; 	
		points[i].y = p.y; 	
		points[i].z = p.z;
		 	
		KB_PosInterpolate0(aniNorm[i], time, &p);
		norms[i].x = p.x;
		norms[i].y = p.y;
		norms[i].z = p.z;
	}	
}

bool Triangle_TPA::intersect(const Ray &ray, HitRecord &record)
{
	float tval;
	float eps = 1e-4;
	
	float A = points[0].x - points[1].x;
	float B = points[0].y - points[1].y;
	float C = points[0].z - points[1].z;
	
	float D = points[0].x - points[2].x;
	float E = points[0].y - points[2].y;
	float F = points[0].z - points[2].z;

	float G = ray.dir.x;
	float H = ray.dir.y;
	float I = ray.dir.z;
	
	float J = points[0].x - ray.pos.x;
	float K = points[0].y - ray.pos.y;
	float L = points[0].z - ray.pos.z;

	float EIHF = E*I - H*F;
	float GFDI = G*F - D*I;
	float DHEG = D*H - E*G;

    float denom = A * EIHF + B * GFDI + C * DHEG;
    if (denom == 0.0) { return false; }

	float beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
	if (isnan(beta)) { return false;  }
	if(beta <= 0.0f || beta >=1.0f) { return false; }
	
	float AKJB = A*K - J*B;
	float JCAL = J*C - A*L;
	float BLKC = B*L - K*C;

	float gamma = (I*AKJB + H *JCAL + G*BLKC)/denom;
	if(gamma <= 0.0f || (beta + gamma) >= 1.0f) return false;

	tval = -(F*AKJB + E*JCAL + D*BLKC)/denom;
	if(tval < eps)  return false;

	record.t = tval;
	Vector normal = getNormal(beta, gamma);
	normal.normalize();
	record.normal = normal;
	record.obj = this;
	record.pos = Point(ray.pos + record.t * ray.dir);
	record.tex_col = Color(1.0, 1.0 , 1.0);
	return true;	
}

void Mesh::meshAnimate(double trans[3], double rot[4], double scal[4])
{	
	fprintf(stderr, "shapeAnimate not implemented yet\n");
}

