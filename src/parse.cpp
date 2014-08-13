/*----------------------------------------------------------------------
  Previewer for Benchmark for Animated Ray Tracings: BART
  File:    parse.c
           Based on readnff.c (part of SPD by Eric Haines) by Eduard Schwan
  Purpose: simple parser for data files, includeing args parsing

----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "animation.h"
#include "texture.h"
#include <stack>
#include "raytracer.h"
#include "mesh.h"
#include "transform.h"
#include "geometry.h"

using namespace std;

#ifndef  M_PI
#define  M_PI 3.1415926535897932384626433
#endif

//typedef float Vec2f[2];
//typedef float Vec3f[3];
//typedef float Vec4f[4];

#define X 0 
#define Y 1  
#define Z 2  
#define W 3  
             
#define R 0  
#define G 1  
#define B 2  
#define A 3 /* alpha */

static int gDetailLevel=0;  /* zero as default */

char obj_name[128];
Material currentMaterial;

extern stack<TransformHierarchy* > transformHierarchy;

/*----------------------------------------------------------------------
  parseComment()
  Description:
    "#" [ string ]
    or 
    "%" [ string ]

  Format:
    # [ string ]
    or
    % [ string ]

    As soon as a "#" (or "%") character is detected, the rest of the line is
    considered a comment. 
----------------------------------------------------------------------*/
static void parseComment(FILE *f)
{
   char str[1000];
   fgets(str, 1000, f);
}

/*----------------------------------------------------------------------
  parseViewpoint()
  Description:
    "v"
    "from" Fx Fy Fz
    "at" Ax Ay Az
    "up" Ux Uy Uz
    "angle" angle
    "hither" hither
    "resolution" xres yres

  Format:

    v
    from %g %g %g
    at %g %g %g
    up %g %g %g
    angle %g
    hither %g
    resolution %d %d

  The parameters are:

    From:  the eye location in XYZ.
    At:    a position to be at the center of the image, in XYZ world
	   coordinates.  A.k.a. "lookat".
    Up:    a vector defining which direction is up, as an XYZ vector.
    Angle: in degrees, defined as from the center of top pixel row to
	   bottom pixel row and left column to right column.
           In AFF, if the width is different from the height, then
           we interpret the Angle as the FOV in the y-direction (from top to bottom),
           and we set the aspect ratio to width/height.
    Resolution: in pixels, in x and in y.

  Note that no assumptions are made about normalizing the data (e.g. the
  from-at distance does not have to be 1).  Also, vectors are not
  required to be perpendicular to each other.

  For all databases some viewing parameters are always the same:
    Yon is "at infinity."
    Aspect ratio is 1.0.

  A view entity must be defined before any objects are defined (this
  requirement is so that NFF files can be used by hidden surface machines).
----------------------------------------------------------------------*/
// viInitViewpoint(from, at, up, fov_angle, hither, resx, resy);

static void viInitViewpoint(Vec3f from, Vec3f at, Vec3f up, float fov, 
	float hither, int resx, int resy)
{
		
	rayTracer->setResXY(resx, resy);
	fprintf(stderr, "[%s] ignore hither now\n", __func__);
	rayTracer->scene->camera = new Camera(Point(from), Point(at), Vector(up),
			fov, resx, resy);
	
}

static void parseViewpoint(FILE *fp)
{
   Vec3f from;
   Vec3f at;
   Vec3f up;
   float fov_angle;
   float hither;
   int resx;
   int resy;
	
   if(fscanf(fp, " from %f %f %f", &from[X], &from[Y], &from[Z]) != 3)
      goto fmterr;
   
   if(fscanf(fp, " at %f %f %f", &at[X], &at[Y], &at[Z]) != 3)
      goto fmterr;
   
   if(fscanf(fp, " up %f %f %f", &up[X], &up[Y], &up[Z]) != 3)
      goto fmterr;
   
   if(fscanf(fp, " angle %f", &fov_angle) != 1)
      goto fmterr;
   
   if(fscanf(fp, " hither %f", &hither) !=1)
      goto fmterr;
   if(hither<0.0001) hither=1.0f;
      
   if(fscanf(fp, " resolution %d %d", &resx, &resy) !=2)
      goto fmterr;
 
   /* init your view point here: 
    * e.g, viInitViewpoint(from, at, up, fov_angle, hither, resx, resy);
    */
 
	viInitViewpoint(from, at, up, fov_angle, hither, resx, resy);
  return;

 fmterr:
   printf("Parser view syntax error");
   exit(1);
}

/*----------------------------------------------------------------------
  parseLight()
  Positional light.  A light is defined by XYZ position and an optional color.
  For animated lights we need to be able to identify light sources. We do
  this by giving them a name.
  Description:
    "l" X Y Z [R G B]
    "la" name X Y Z [R G B]   #animated light
  Format:
    l %g %g %g [%g %g %g]
    la %s %g %g %g [%g %g %g]

    All light entities must be defined before any objects are defined (this
    requirement is so that NFF files can be used by hidden surface machines).
    Lights have a non-zero intensity of no particular value [this definition
    may change soon, with the addition of an intensity and/or color].
    The name of an animated light must not contain any white spaces.
----------------------------------------------------------------------*/


static void viAddLight(char *name, Vec3f pos, Vec3f col)
{
	rayTracer->scene->addLight(
		new PointLight(Point(pos), Color(col)));
		//new DirectionalLight(Vector(pos), Color(col)));
}

static void parseLight(FILE *fp)
{
   Vec4f pos;
   Vec4f col;
   int num;
   int is_animated;
   char name[100];
   strcpy(name,"");

   is_animated = getc(fp);
   if(is_animated != 'a')
   {
      ungetc(is_animated, fp);
      is_animated=0;
   }

   if(is_animated)  /* if it's an animated light, read its name */
   {
      fscanf(fp,"%s",name);
   }

   if(fscanf(fp, "%f %f %f ",&pos[X], &pos[Y], &pos[Z]) != 3)
   {
      printf("Light source position syntax error");
      exit(1);
   }
   pos[W]=1.0f;

   /* read optional color of light */
   num=fscanf(fp, "%f %f %f ",&col[X], &col[Y], &col[Z]);
   if(num==0)
   {
      //V4SET4(col,1.0f,1.0f,1.0f,1.0f);
		col[X] = col[Y] = col[Z]= col[W]= 1.0;
   }
   else if(num!=3)
   {
      printf("Light source color syntax error");
      exit(1);
   }
   col[A]=1.0f;

   /* add light source here:
    * e.g. viAddLight(name,pos,col);
    */
   viAddLight(name,pos,col);
}

/*----------------------------------------------------------------------
  parseBackground()
  Background color.  A color is simply RGB with values between 0 and 1
  Description:
    "b" R G B

  Format:
    b %g %g %g

    If no background color is set, assume RGB = {0,0,0}.
----------------------------------------------------------------------*/

static void viInitBackgroundColor(Vec3f bgcolor)
{
	Color& col = rayTracer->scene->backgroundColor;
	col.r = bgcolor[0];
	col.g = bgcolor[1];
	col.b = bgcolor[2];
}

static void parseBackground(FILE *fp)
{
   Vec3f bgcolor;
	
   if(fscanf(fp, "%f %f %f",&bgcolor[X], &bgcolor[Y], &bgcolor[Z]) != 3)
   {
      printf("background color syntax error");
      exit(1);
   }

   /* init you background color here
    * e.g.  viInitBackgroundColor(bgcolor);
    */
   viInitBackgroundColor(bgcolor);
}


/*----------------------------------------------------------------------
  parseFill()
  Fill color and shading parameters. 
  Description:
     "f" red green blue Kd Ks Shine T index_of_refraction
     "fm" amb_r amb_g amb_b
          diff_r diff_g diff_b
          spec_r spec_g spec_b
          Shine T index_of_refraction
  Format:
    f %g %g %g %g %g %g %g %g
    fm %g %g %g  %g %g %g  %g %g %g  %g %g %g

    RGB is in terms of 0.0 to 1.0.

    Kd is the diffuse component, Ks the specular, Shine is the Phong cosine
    power for highlights, T is transmittance (fraction of light passed per
    unit).  Usually, 0 <= Kd <= 1 and 0 <= Ks <= 1, though it is not required
    that Kd + Ks == 1.  Note that transmitting objects ( T > 0 ) are considered
    to have two sides for algorithms that need these (normally objects have
    one side).

    The "fm" (fill material) version (not part of NFF) is a simple
    extension of the material description: it involves RGB for
    the ambient, the diffuse, and the specular component (instead of RGB,
    Ks, Ld) plus Shine, T, and index_of_refraction.

    The fill color is used to color the objects following it until a new color
    is assigned.
----------------------------------------------------------------------*/

static void viAddExtendedMaterial( Vec3f amb, Vec3f dif, Vec3f spc,
	float _4xphong_pow, float t, float ior)
// what t and ior used for
{
	currentMaterial.ambient = Color(amb);
	currentMaterial.diffuse = Color(dif);
	currentMaterial.specular = Color(spc);
	currentMaterial.shiness = _4xphong_pow;
	currentMaterial.T = t;
	currentMaterial.ior = ior;
//	if(t !=  0.0 && ior != 0.0) {
//		fprintf(stderr, "Error? t and ior are not 0.0\n");
//	}
}
static void parseFill(FILE *fp)
{
    float kd, ks, phong_pow, t, ior;
    Vec3f col;
    int moreparams;

    moreparams = getc(fp);
    if(moreparams != 'm')
    {
       ungetc(moreparams, fp);
       moreparams = 0;
    }

    if(moreparams)  /* parse the extended material description */
    {
       Vec3f amb,dif,spc;
       if(fscanf(fp,"%f %f %f",&amb[X], &amb[Y], &amb[Z]) != 3)
       {
	  printf("fill material ambient syntax error");
	  exit(1);
       }
       if(fscanf(fp,"%f %f %f",&dif[X], &dif[Y], &dif[Z]) != 3)
       {
	  printf("fill material diffuse syntax error");
	  exit(1);
       }
       if(fscanf(fp,"%f %f %f",&spc[X], &spc[Y], &spc[Z]) != 3)
       {
	  printf("fill material specular syntax error");
	  exit(1);
       }
       if (fscanf(fp, "%f %f %f", &phong_pow, &t, &ior) != 3)
       {
	  printf("fill material (phong, transp, IOR) syntax error");
	  exit(1);
       }
       /* add your extended material here
	* e.g., viAddExtendedMaterial(amb,dif,spc,4.0*phong_pow,t,ior);
        */
		viAddExtendedMaterial(amb,dif,spc,4.0*phong_pow,t,ior);
	// need to implement this myself
    }
    else   /* parse the old NFF description of a material */
    {
       if (fscanf(fp, "%f %f %f",&col[X], &col[Y], &col[Z]) != 3)
       {
	  printf("fill color syntax error");
	  exit(1);
       }
       
       if (fscanf(fp, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5)
       {
	  printf("fill material syntax error");
	  exit(1);
       }
       
       /* add the normal NFF material here 
	* e.g., viAddMaterial(col,kd,ks,4.0*phong_pow,t,ior);  
        */

	
    }
}


/*----------------------------------------------------------------------
  parseCone()
  Cylinder or cone.  A cylinder is defined as having a radius and an axis
    defined by two points, which also define the top and bottom edge of the
    cylinder.  A cone is defined similarly, the difference being that the apex
    and base radii are different.  The apex radius is defined as being smaller
    than the base radius.  Note that the surface exists without endcaps.  The
    cone or cylinder description:

    "c"
    base.x base.y base.z base_radius
    apex.x apex.y apex.z apex_radius

  Format:
    c
    %g %g %g %g
    %g %g %g %g

    A negative value for both radii means that only the inside of the object is
    visible (objects are normally considered one sided, with the outside
    visible).  Note that the base and apex cannot be coincident for a cylinder
    or cone.
----------------------------------------------------------------------*/
static void parseCone(FILE *fp)
{
   Vec3f base_pt;
   Vec3f apex_pt;
   float r0,r1;
       
   if(fscanf(fp, " %f %f %f %f %f %f %f %f", &base_pt[X],&base_pt[Y],&base_pt[Z],&r0,
	     &apex_pt[X],&apex_pt[Y],&apex_pt[Z],&r1) !=8)
    {
       printf("cylinder or cone syntax error\n");
       exit(1);
    }
    if(r0 < 0.0)
    {
       r0 = -r0;
       r1 = -r1;
    }
    /* add a cone here
     * e.g., viAddCone(base_pt,r0,apex_pt,r1);
     */
}


/*----------------------------------------------------------------------
  parseSphere()
  Sphere.  A sphere is defined by a radius and center position:
  
    "s" center.x center.y center.z radius

Format:
    s %g %g %g %g

    If the radius is negative, then only the sphere's inside is visible
    (objects are normally considered one sided, with the outside visible).
----------------------------------------------------------------------*/
static void parseSphere(FILE *fp)
{
    float radius;
    Vec3f center;
	
    if(fscanf(fp, "%f %f %f %f", &center[X], &center[Y], &center[Z], &radius) != 4)
    {
       printf("sphere syntax error");
       exit(1);
    }
    /* add a sphere here
     * e.g., viAddSphere(center,radius);
     */
}	



/*----------------------------------------------------------------------
  parsePoly()
  Polygon.  A polygon is defined by a set of vertices.  With these databases,
    a polygon is defined to have all points coplanar.  A polygon has only
    one side, with the order of the vertices being counterclockwise as you
    face the polygon (right-handed coordinate system).  The first two edges
    must form a non-zero convex angle, so that the normal and side visibility
    can be determined. 
  Description:

    "p" total_vertices
    vert1.x vert1.y vert1.z
    [etc. for total_vertices vertices]

Format:
    p %d
    [ %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------
  Polygonal patch.  A patch is defined by a set of vertices and their normals.
    With these databases, a patch is defined to have all points coplanar.
    A patch has only one side, with the order of the vertices being
    counterclockwise as you face the patch (right-handed coordinate system).
    The first two edges must form a non-zero convex angle, so that the normal
    and side visibility can be determined.  Description:

    "pp" total_vertices
    vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
    [etc. for total_vertices vertices]

Format:
    pp %d
    [ %g %g %g %g %g %g ] <-- for total_vertices vertices
----------------------------------------------------------------------*/

static void viAddMesh(Vec3f *verts, int num_verts, Vec3f *norms, 
		int num_orms, Vec2f *txts, int num_txts, char *texturename,
		unsigned short* indices, int num_tris);


static void viAddPolygon(int nverts, Vec3f *verts)
{
	int i = 0;
	assert(verts != NULL && nverts >= 3);
	int num_tris = nverts - 2;
	unsigned short *indices = new unsigned short[num_tris * 3];
	for(i = 0 ; i <  num_tris; ++i) {
		indices[3 * i] = 0;
		indices[3 * i + 1] = i + 1;
		indices[3 * i + 2] = i + 2;  // the index ??
	}
	viAddMesh(verts, nverts, NULL, 0, NULL, 0, NULL, indices, num_tris);
	delete[] indices;
}

static void parsePoly(FILE *fp)
{
   int ispatch;
   int nverts;
   int q;
   Vec3f *verts=NULL;
   Vec3f *norms=NULL;
	
   ispatch = getc(fp);
   if(ispatch != 'p')
   {
      ungetc(ispatch, fp);
      ispatch = 0;
   }
   
   if(fscanf(fp, "%d", &nverts) != 1)
		goto fmterr;
	
   verts = (Vec3f*)malloc(nverts*sizeof(Vec3f));
   if(verts == NULL)
      goto memerr;
	
    if(ispatch)
    {
       norms=(Vec3f*)malloc(nverts*sizeof(Vec3f));
       if(norms == NULL)
	  goto memerr;
    }
	
    /* read all the vertices */
    for(q=0; q<nverts; q++)
    {
       if(fscanf(fp, " %f %f %f",&verts[q][X], &verts[q][Y], &verts[q][Z]) != 3)
	  goto fmterr;
       
       if(ispatch)
       {
	  if(fscanf(fp, " %f %f %f",&norms[q][X],&norms[q][Y],&norms[q][Z]) != 3)
	     goto fmterr;
       }
    }

    if(ispatch)
    {
       /* add a polygon patch here
	* e.g.,  viAddPolyPatch(nverts,verts,norms);
	*/
    }
    else
    {
       /* add a polygon here
	* e.g., viAddPolygon(nverts,verts);
	*/
		viAddPolygon(nverts,verts);
    }
	
    return;
fmterr:
    printf("polygon or patch syntax error\n");
    exit(1);
memerr:
    printf("can't allocate memory for polygon or patch\n");
    exit(1);
}


/*----------------------------------------------------------------------
  parseInclude()
  Include another file (typically containing geometry)
  Description:  
    "i" detail_level filename

Format:
    i %d %s

    The file name may not include any white spaces.
----------------------------------------------------------------------*/
static void parseInclude(FILE *fp)
{
   char filename[100];
   FILE *ifp;
   int detail_level;
   if(fscanf(fp,"%d %s",&detail_level,filename)!=2)
   {
      printf("Error: could not parse include.\n");
      exit(0);
   }

   if(detail_level<=gDetailLevel) /* skip file if our detail is less than the global detail */
   {
      if(ifp=fopen(filename,"r"))
      {
	 viParseFile(ifp);  /* parse the file recursively */
	 fclose(ifp);
      }
      else
      {
	 printf("Error: could not open include file: <%s>.\n",filename);
	 exit(1);
      }
   }
   else
   {
      printf("Skipping include file: %s\n",filename);
   }
}

/*----------------------------------------------------------------------
  parseDetailLevel()
  Include another file (typically containing geometry)
  Description:  
    "d" detail_level

Format:
    d %d

    The detail level (DL) number is used to exclude objects
    from the scene so that different a scene can have different
    complexities (number of primitives in them).
    The include command (i) is
    the only one that have a detail number.
    If the detail level of an included file
    is less or equal to DL then that object is included, else
    we skip it. 
    Is 0 (zero) by default.
----------------------------------------------------------------------*/
static void parseDetailLevel(FILE *fp)
{
   if(fscanf(fp,"%d",&gDetailLevel)!=1)
   {
      printf("Error: could not parse detail level.\n");
      exit(1);
   }
}


/*----------------------------------------------------------------------
  parseTexturedTriangle()
  A triangle with texture coordinates at each vertex.
  Can also be a textured triangle patch (with normals at each vertex).
  Description:  
    "tt" texturename

         vert0.x vert0.y vert0.z texcoord0.u texcoord0.v
         vert1.x vert1.y vert1.z texcoord1.u texcoord1.v
         vert2.x vert2.y vert2.z texcoord2.u texcoord2.v
    "ttp" texturename
          vert0.x vert0.y vert0.z norm0.x norm0.y norm0.z texcoord0.u texcoord0.v
          vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z texcoord1.u texcoord1.v
          vert2.x vert2.y vert2.z norm2.x norm2.y norm2.z texcoord2.u texcoord2.v


Format:
    tt %s
       %g %g %g %g %g
       %g %g %g %g %g
       %g %g %g %g %g

    ttp %s 
        %g %g %g %g %g %g %g %g
        %g %g %g %g %g %g %g %g
         %g %g %g %g %g %g %g %g

    The texture name may not include any white spaces.
    Note that the texturing works like OpenGL REPEAT mode.
----------------------------------------------------------------------*/

static void viAddTexturedTriangle(char *texturename,Vec3f* verts, 
			float tu[3], float tv[3]) {

	assert( verts != NULL);
	//unsigned short indices[6] =  {0, 1, 2, 0, 1, 2};// = new unsigned short[3];
	unsigned short indices[6] =  {0, 0, 1, 1, 2, 2};// = new unsigned short[3];
	//unsigned short indices[6] =  {0,1, 2};// = new unsigned short[3];
	Vec2f *tex = (Vec2f*)malloc(3 * sizeof(Vec2f));
//	Vec2f tex[3] = { {tu[0], tv[0]}, {tu[1], tv[1]}, {tu[2], tv[2]} };
	tex[0][0] = tu[0];
	tex[0][1] = tv[0];
	tex[1][0] = tu[1];
	tex[1][1] = tv[1];
	tex[2][0] = tu[2];
	tex[2][1] = tv[2];
	
	viAddMesh(verts, 3, NULL, 0, tex, 3, texturename, indices, 1);
	//viAddMesh(verts, 3, NULL, 0, NULL, 0, NULL, indices, 1);
}

static void parseTexturedTriangle(FILE *fp)
{
   int is_patch;
   int q;
//   Vec3f verts[3];
  // Vec3f norms[3];
   float tu[3],tv[3];
   char texturename[100];


   Vec3f *verts = (Vec3f*)malloc(3 * sizeof(Vec3f));
   Vec3f *norms = (Vec3f*)malloc(3 * sizeof(Vec3f));

   is_patch=getc(fp);
   if(is_patch!='p')
   {
      ungetc(is_patch,fp);
      is_patch=0;
   }
   
   fscanf(fp,"%s",texturename);

   for(q=0;q<3;q++)
   {
      if(fscanf(fp," %f %f %f",&verts[q][X],&verts[q][Y],&verts[q][Z])!=3)
	 goto parseErr;

      if(is_patch)
      {
	 if(fscanf(fp," %f %f %f",&norms[q][X],&norms[q][Y],&norms[q][Z])!=3)
	    goto parseErr;
      }

      if(fscanf(fp," %f %f ",&tu[q],&tv[q])!=2)
	 goto parseErr;
   }


   if(is_patch)
   {
      /* add a textured triangle patch here
       * e.g., viAddTexturedTriPatch(texturename,verts,norms,tu,tv);
       */
   }
   else
   {
      /* add a textured triangle here
       * e.g.,  viAddTexturedTriangle(texturename,verts,tu,tv);
       */
       viAddTexturedTriangle(texturename,verts,tu,tv);
   }
   return;
   
 parseErr:
   printf("Error: could not parse textured triangle\n");
   exit(1);

}


/*----------------------------------------------------------------------
  parseAnimatedTriangle()
  an animated triangle patch
  Description:  
    "tpa" texture_name filename

Format:
    tpa %d
        %g
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
	%g
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        %g %g %g  %g %g %g 
        .
        .
        .
       
    tpa num_times
        time0
        vert0_time0.x vert0_time0.y vert0_time0.z norm0_time0.x norm0_time0.y norm0_time0.y 
        vert1_time0.x vert1_time0.y vert1_time0.z norm1_time0.x norm1_time0.y norm1_time0.y 
        vert2_time0.x vert2_time0.y vert2_time0.z norm2_time0.x norm2_time0.y norm2_time0.y 
        time1
        vert0_time1.x vert0_time1.y vert0_time1.z norm0_time1.x norm0_time1.y norm0_time1.y 
        vert1_time1.x vert1_time1.y vert1_time1.z norm1_time1.x norm1_time1.y norm1_time1.y 
        vert2_time1.x vert2_time1.y vert2_time1.z norm2_time1.x norm2_time1.y norm2_time1.y 
        .
        .
        .

	
   Definition: this animated triangle patch depends on the time;
     1) if time<time0 then use the vertices and normals from time0,
        i.e., use the first triangle patch in the list
     2) if time>time_{num_times-1} then use the vertices and normals
        from time_{num_times-1}, i.e., the last triangle patch in
        the list.
     3) otherwise find two subsequent triangle patches with times time_a
        and time_b, such that time_a <= time <= time_b. Then interpolate
        linearly between these two triangle patches to find the 
        animated triangle patch. See viGetAnimatedTriangle() in render.c
----------------------------------------------------------------------*/

static void viAddAnimatedTriangle(int num_times, float *times, 
			Vec3f *verts, Vec3f *norms)
{
	int i, j;
	Triangle_TPA* tpa = new Triangle_TPA(num_times, times, verts, 
					norms, currentMaterial);
	rayTracer->scene->addTriangleTPA(tpa);	
	for(j = 0; j < 3; ++j){
		PositionKey * Ppos  = (PositionKey*)calloc(num_times, 
											sizeof(PositionKey));
		PositionKey * Pnorm = (PositionKey*)calloc(num_times, 
											sizeof(PositionKey));
		for (i = 0; i < num_times; ++i) {
			Ppos[i].t = times[i];
			Ppos[i].P.x = verts[3 * i + j][X];
			Ppos[i].P.y = verts[3 * i + j][Y];
			Ppos[i].P.z = verts[3 * i + j][Z];
			Ppos[i].tension  = 0;
			Ppos[i].continuity = 0;
			Ppos[i].bias = 0;

			Pnorm[i].t = times[i];
			Pnorm[i].P.x = norms[3 * i + j][X];
			Pnorm[i].P.y = norms[3 * i + j][Y];
			Pnorm[i].P.z = norms[3 * i + j][Z];
			Pnorm[i].tension  = 0;
			Pnorm[i].continuity = 0;
			Pnorm[i].bias = 0;
		}
		tpa->aniPos[j]  = KB_PosInitialize(num_times, Ppos);
		tpa->aniNorm[j] = KB_PosInitialize(num_times, Pnorm);
	}
}

static void parseAnimatedTriangle(FILE *fp)
{
   int q,w;
   int num_times;
   Vec3f *verts;
   Vec3f *norms;
   float *times;
   
   fscanf(fp,"%d",&num_times);
   times=(float*)malloc(sizeof(float)*num_times);
   verts=(Vec3f*)malloc(sizeof(Vec3f)*3*num_times);
   norms=(Vec3f*)malloc(sizeof(Vec3f)*3*num_times);

   for(q=0;q<num_times;q++)
   {
      if(fscanf(fp," %f",&times[q])!=1)
	 goto parseErr;

      for(w=0;w<3;w++)
      {
	 if(fscanf(fp," %f %f %f",&verts[q*3+w][X],&verts[q*3+w][Y],&verts[q*3+w][Z])!=3)
	    goto parseErr;
	 
      
	 if(fscanf(fp," %f %f %f",&norms[q*3+w][X],&norms[q*3+w][Y],&norms[q*3+w][Z])!=3)
	    goto parseErr;	 
      }
   }

   /* add a animated triangle here
    * e.g., viAddAnimatedTriangle(num_times,times,verts,norms); 
    */

   viAddAnimatedTriangle(num_times,times,verts,norms); 
   return;
   
 parseErr:
   printf("Error: could not parse animated triangle (tpa)\n");
   exit(1);

}

/*----------------------------------------------------------------------
  parseTextureStuff()
  Decide if we got a texture with starts with "t " or a 
  textured triangle (or tri patch), which starts with "tt"
  Currently, we removed the "t"
----------------------------------------------------------------------*/
static void parseTextureStuff(FILE *fp)
{
   int is_triangle;

   is_triangle=getc(fp);
   if(is_triangle=='t')
   {
      parseTexturedTriangle(fp);
   }
   else if(is_triangle=='p')
   {
      is_triangle=getc(fp);
      if(is_triangle=='a')    /*tpa = triangle, patch, animated */
      {
	 parseAnimatedTriangle(fp);
      }
   }
   else
   {
      printf("Error: tt and ttp are valid codes (not t%c).\n",(char)is_triangle);
      exit(1);
   }
}

static void eatWhitespace(FILE *f)
{
   char ch=getc(f);
   while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
      ch=getc(f);
   ungetc(ch,f);
}

/*----------------------------------------------------------------------
  parseKeyFrames()
  Format:
  "k" xform_name
  {
     type num_keyframes
     keyframe_data
  }

  Description:  
  The xform_name identifies the xform which is to be animated.

  For each type of keyframes (transl, rot, scale) there must
  be at least 4 keyframes, and the first and the last keyframes
  are only used internally to get starting tangents and similar stuff.
  There is also a fourth type of keyframes (visibility), which determines
  whether an object is visible or not at a certain time. This one needs
  at least one keyframe. 
  Each of the types may appear once in a keyframe description, i.e.,
  you can have a translation, scaling, rotation, and a visibility
  in the keyframe description (but not, say, two rotations).
  It is always the case that the total transform
  is T*R*S, where T=translation, R=rotation, and S=scaling.
  Is there is no, say, rotation, then R=I (identity matrix). This
  holds for the other as well.
  Note the order: the scaling is applied first, then rotation,
  the translation.

  Translation example:
  k the_ball
  {
    transl 5
    -0.50 0 -3.0 0  0 0 0
     0.00 0  0.0 0  0 0 0
     0.50 0  1.0 0  0 0 0
     1.00 0  0.0 0  0 0 0
     1.50 0 -3.0 0  0 0 0
  }
  In this example, we can only use times from 0.00 to 1.00
  Each row looks like this: time x y z tension continuity bias,
  where (x,y,z) is the translation at time, and tension, continuity, and
  bias, are the constants for interpolation at time.
 
  Rotation example:
  k the_ball2
  {
     rot 4
     -0.5  1 0 0 45  0 0 0
      0.0  1 0 0 0   0 0 0
      0.5  1 0 0 90  0 0 0
      1.0  0 1 1 10  0 0 0
  }
  Each row looks like this: time x y z degrees tension continuity bias, where  
  (x,y,z) is the rotation axis and degrees is the amound which is rotated
  around the axis at time, and tension, continuity, and
  bias, are the constants for interpolation at time.

  Scaling example:
  k the_ball3
  {
    scale 7
    -0.5 1 1 1 0 0 0
     0.0 1 1 1 0 0 0
     0.5 2 1 1 0 0 0
     1.0 1 2 1 0 0 0
     1.5 1 1 2 0 0 0
     2.0 1 1 1 0 0 0
     2.5 1 1 1 0 0 0
  }
  Each row looks like this: time x y z tension continuity bias, where  
  (x,y,z) is the scaling parameters, and tension, continuity, and
  bias, are the constants for interpolation at time.

  Visibility example:
  k the_ball4
  {
    visibility 2
    0.5 0
    2.0 1
  }
  Each row looks like this: time visbility_flag
  where visibility_flag is either 0 (invisible) or 1 (visible)
  From time=-infinity each object is assumed visible until
  the first visibility keyframe. At that time (0.5 in the example
  above) the visibility switches to what is given in that keyframe
  (0 in the example). At the next keyframe (time=2.0 in the example)
  the visibility may change again (changes to visible (1) above).
  The last visibility_flag determines the visibility until time=infinity.

  Note also that if the name of an animation is "camera", then
  the viewpoint should be animated after those key frames (only
  translation and rotation). Light sources can also be animated
  (only translation).
----------------------------------------------------------------------*/

static void parseKeyFrames(FILE *fp)
{   
   char name[200];
   char motion[200];
   char ch;
   int  c;
   int visibility;
   int  ret, i, nKeyFrames;
   float time, x, y, z, angle, te, co, bi;
   PositionKey* PKeys;
   RotationKey* RKeys;
   Animation* animation;
   struct AnimationList* animationlist;

   if(fscanf(fp,"%s",name)!=1)
   {
      printf("Error: could not read name of animation.\n");
      exit(1);
   }
   eatWhitespace(fp);
   ch=getc(fp);
   if(ch!='{')
   {
      printf("Error: could not find a { in animation %s.\n",name);
      exit(1);
   }
   
   /* insert a new animation in the AnimationList */
   animationlist= 
      (struct AnimationList*) calloc(1, sizeof(struct AnimationList));

   /* put the newly allocated a list somewhere,
    * e.g., 
    * animationlist->next = gScene.mAnimations;
    * gScene.mAnimations = animationlist;
    * animation = &(animationlist->animation);
    * gScene.mAnimations was our global list of animations
    */
	Scene *gScene = rayTracer->scene;
	animationlist->next = gScene->mAnimations;
	gScene->mAnimations = animationlist;
	animation = &(animationlist->animation);

   animation->translations=NULL;
   animation->rotations=NULL;
   animation->scales=NULL;
   animation->name=(char *)malloc(sizeof(name)); 
   strcpy(animation->name,name);

   eatWhitespace(fp);
   while( (c = getc(fp)) != '}' )
   {
      ungetc(c, fp);
      if(fscanf(fp, "%s %d", motion, &nKeyFrames)!=2)
      {
	 printf("Error: could not read name of motion or number of keyframes for animation.\n");
	 exit(1);
      }
      
      if(nKeyFrames<4 && strcmp("visibility",motion))
      { 
	 printf("Error: there must be at least 4 keyframes for %s.\n",name);
	 exit(1);
      }

      /* check whether the motion is a "transl" or a "rot" or a "scale" */
      if(strcmp(motion, "transl")==0)
      {
	 PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
	 for( i=0; i<nKeyFrames; i++ )
	 {
	    ret = fscanf(fp, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
			 &te, &co, &bi);
	    if(ret != 7)
	    {
	       printf("error in parsing translation keyframes for %s\n",
		      animation->name);
	       exit(1);
	    }
	    PKeys[i].t = time;
	    PKeys[i].P.x = x;
	    PKeys[i].P.y = y;
	    PKeys[i].P.z = z;
	    PKeys[i].tension = te;
	    PKeys[i].continuity = co;
	    PKeys[i].bias = bi;
	 }
	 animation->translations = KB_PosInitialize(nKeyFrames, PKeys);
	 free(PKeys);
      }
      else if(strcmp(motion, "rot")==0)
      {
	 RKeys = (RotationKey*) calloc(nKeyFrames, sizeof(RotationKey));
	 for( i=0; i<nKeyFrames; i++ )
	 {
	    ret = fscanf(fp," %f %f %f %f %f %f %f %f", &time, &x, &y, &z, 
			 &angle, &te, &co, &bi);
	    if(ret != 8)
	    {
	       printf("error in parsing rotation keyframes for %s\n",
		      animation->name);
	       exit(1);
	    }
	    RKeys[i].t = time;
	    RKeys[i].Rot.x = x;
	    RKeys[i].Rot.y = y;
	    RKeys[i].Rot.z = z;
	    RKeys[i].Rot.angle = angle*M_PI/180.0;
	    RKeys[i].tension = te;
	    RKeys[i].continuity = co;
	    RKeys[i].bias = bi;
	 }
	 animation->rotations = KB_RotInitialize(nKeyFrames, RKeys);
	 free(RKeys);
      }
      else if(strcmp(motion, "scale")==0)
      {
	 PKeys = (PositionKey*) calloc(nKeyFrames, sizeof(PositionKey));
	 for( i=0; i<nKeyFrames; i++ )
	 {
	    ret = fscanf(fp, " %f %f %f %f %f %f %f", &time, &x, &y, &z, 
			 &te, &co, &bi);
	    if(ret != 7)
	    {
	       printf("error in parsing scale keyframes for %s\n",
		      animation->name);
	       exit(1);
	    }
	    PKeys[i].t = time;
	    PKeys[i].P.x = x;
	    PKeys[i].P.y = y;
	    PKeys[i].P.z = z;
	    PKeys[i].tension = te;
	    PKeys[i].continuity = co;
	    PKeys[i].bias = bi;
	 }
	 animation->scales = KB_PosInitialize(nKeyFrames, PKeys);
	 free(PKeys);
      }
      else if(strcmp(motion, "visibility")==0)
      {
	 VisKey *viskeys=(VisKey*)  calloc(nKeyFrames, sizeof(VisKey));
	 for( i=0; i<nKeyFrames; i++ )
	 { 	    
	    ret = fscanf(fp, " %f %d", &time, &visibility);
	    if(ret != 2)
	    {
	       printf("error in parsing visibility keyframes for %s\n",
		      animation->name);
	       exit(1);
	    }
	    viskeys[i].time=time;
	    viskeys[i].visibility=visibility;	    
	 }
	 animation->visibilities=viskeys;
	 animation->numVisibilities+=nKeyFrames;
      }
      else
      {
	 printf("Error: unknown keyframe type (%s). Must be transl, rot, or scale.\n",motion);
	 exit(1);
      }
      eatWhitespace(fp);
   }   
}


/*----------------------------------------------------------------------
  viParseXform()
  parse transform (either static or keyframe animated)
  Description:
  "xs" sx sy sz
       rx ry rz angle_deg
       tx ty tz
  {
    here follows objects, materials, new transforms, etc that
    are to be statically transformed with: T*R*S 
    i.e., first scaling (sx,sy,sz) then rotation of angle_deg degrees
    around the axis (rx,ry,rz), and finally translation (tx,ty,tz)
  }

  or

  "x" transform_name
  {
    here follows objects, materials, new transforms, etc that
    are animated
  }
  The actual keyframes must be found later in the file, and
  these are given with the "k" descriptor. 
  Everything inside the { } is transformed, and are thus
  in its own coordinate system (in a subtree).
---------------------------------------------------------------------*/
static void viEndXform() 
{
//	transStack->Pop();
//	printf("now do noting in viEndXform\n");
	transformHierarchy.pop();  // pop 
}

static void viAddStaticXform(Vec3f scale, Vec3f rot, float deg, Vec3f trans)
{
	TransformHierarchy *child = new TransformHierarchy(true);
	Matrix4x4 m; // m is now a identity matrix
	m *= Translate(trans[0], trans[1], trans[2]);
	m *= Rotate(rot[0], rot[1], rot[2], deg2rad(deg));
	m *= Scale(scale[0], scale[1], scale[2]);
	child-> transformMatrix = m;
	TransformHierarchy* pa = transformHierarchy.top();
	if(pa != NULL) {
		pa->addChild(child);
	}
	transformHierarchy.push(child);	
}

void viAddXform(char *name)
{
	TransformHierarchy *child = new TransformHierarchy(false);
	child->xName = string(name);

	TransformHierarchy *pa = transformHierarchy.top();
	if(pa != NULL) {
		pa->addChild(child);
	}
	rayTracer->scene->addXForm(name, child);
	transformHierarchy.push(child);	
}

static void parseXform(FILE *f)
{
   char name[100];
   char ch;
   int is_static;

   is_static = getc(f);
   if(is_static != 's')
   {
      ungetc(is_static, f);
      is_static=0;
   }

   if(is_static)  /* is the transform a static one ? */
   {
      Vec3f scale, trans, rot;
      float deg;
      
      if(fscanf(f," %f %f %f %f %f %f %f %f %f %f", &scale[0], &scale[1], &scale[2],
		&rot[0], &rot[1], &rot[2], &deg,
		&trans[0], &trans[1], &trans[2])!=10)
      {
	 printf("Error: could not read static transform.\n");
	 exit(1);
      }
      eatWhitespace(f);
      ch=getc(f);
      if(ch!='{')
      {
	 printf("Error: { expected.\n");
	 exit(1);
      } 

      /* add a static transform here
       * e.g.,viAddStaticXform(scale,rot,deg,trans);
       */
      viAddStaticXform(scale,rot,deg,trans);
   }
   else   /* keyframe animated transform */
   {
      if(fscanf(f,"%s",name)!=1)
      {
	 printf("Error: could not read transform name.\n");
	 exit(1);
      }
      eatWhitespace(f);
      ch=getc(f);
      if(ch!='{')
      {
	 printf("Error: { expected.\n");
	 exit(1);
      }
      
      /* add an animated transform here
       * e.g., viAddXform(name);
       */
      viAddXform(name);
   }
}

/*----------------------------------------------------------------------
  parseAnimParams()
  parse animation parameters
  Description:  
    "a" start_time end_time num_frames

Format:
    a %g %g %d 

    start_time indicates the start of the animation
    end_time   indicates the end of the animation
    num_frames is the number of frames in the animation.
      Note: the step time (from one frame to the next) is then
      (end_time-start_time)/(num_frames-1)
----------------------------------------------------------------------*/

static void viSetupAnimParams(float start, float end, int num_frames)
{
//	fprintf(stderr, "animation not implementated yet\n");
	rayTracer->setTime(start, end, num_frames);
}

static void parseAnimParams(FILE *fp)
{
   float start,end;
   int num_frames;
   if(fscanf(fp,"%f %f %d",&start,&end,&num_frames)!=3)
   {
      printf("Error: could not parse animations parameters.\n");
      exit(1);
   }
   /* add animations parameters here
    * e.g., viSetupAnimParams(start,end,num_frames);
    */
   viSetupAnimParams(start,end,num_frames);
}

/*----------------------------------------------------------------------
  parseA()
  parse animation parameters and global ambient light

  Global ambient light description:  
    "am" red green blue

Format:
    am %g %g %d 

    There is one global ambient light source in the scene,
    and it can be set with, e.g., "am 0.5 0.5 0.5".
    Default value is "am 1.0 1.0 1.0".
----------------------------------------------------------------------*/
static void parseA(FILE *f)
{
   char name[100];
   char ch;
   int is_ambient;

   is_ambient = getc(f);
   if(is_ambient != 'm')
   {
      ungetc(is_ambient, f);
      is_ambient=0;
   }

   if(is_ambient)  /* we got "am" = global ambient light */
   {
      Vec3f amb;
      if(fscanf(f,"%f %f %f",&amb[0],&amb[1],&amb[2])!=3)
      {
	 printf("Error: could not parse ambient light (am).\n");
	 exit(1);
      }
      /* set up your globabl ambient light here using amb */
	rayTracer->scene->addLight(new AmbientLight(Color(amb[0], amb[1], amb[2])));
   }
   else
   {
      parseAnimParams(f);
   }
}

static void getVectors(FILE *fp,const char *type,int *num_vecs,Vec3f **vecs)
{
   int num,q;
   Vec3f *v=NULL;

   if(fscanf(fp,"%d",&num)!=1)
   {
      printf("Error: could not parse mesh (expected 'num_%s').\n",type);   
      exit(1);
   }


   v=(Vec3f*)malloc(sizeof(Vec3f)*num);
   if(v==NULL)
   {
      printf("Error: could not allocate memory for vertices of mesh.\n");
      exit(1); 
   }
   
   for(q=0;q<num;q++)
   {
      if(fscanf(fp,"%f %f %f ",&v[q][X],&v[q][Y],&v[q][Z])!=3)
      {
	 printf("Error: could not read %d %s of mesh.\n",num,type);
	 exit(1);      
      }
   }
   *vecs=v;
   *num_vecs=num;
}

static void getTextureCoords(FILE *fp,char *texturename,int *num,Vec2f **vecs)
{
   int q;
   int num_txts;
   Vec2f *txts;
   if(fscanf(fp,"%d",&num_txts)!=1)
   {
      printf("Error: could not parse mesh (expected 'num_txts').\n");   
      exit(1);
   }
   txts=(Vec2f*)malloc(sizeof(Vec2f)*num_txts);
   if(txts==NULL)
   {
      printf("Error: could not allocate memory for texturecoords of mesh.\n");
      exit(1); 
   }
   fscanf(fp,"%s",texturename);
   for(q=0;q<num_txts;q++)
   {
      if(fscanf(fp,"%f %f",&txts[q][X],&txts[q][Y])!=2)
      {
	 printf("Error: could not read %d texturecoords of mesh.\n",num_txts);
	 exit(1);      
      }	 
   }      
   *num=num_txts;
   *vecs=txts;
}

static void getTriangles(FILE *fp,int *num_tris,unsigned short **indices,
			 Vec3f *verts,Vec3f *norms,Vec2f *txts)
{
   int num;
   int q,w;
   int allocsize;
   unsigned short *idx;
   int i,v[3],n[3],t[3];
   
   allocsize=3;
   if(norms) allocsize+=3;
   if(txts) allocsize+=3;
   
   if(fscanf(fp,"%d",&num)!=1)
   {
      printf("Error: could not parse mesh (expected 'num_triangles').\n");   
      exit(1);      
   }

   idx=(unsigned short *)malloc(num*allocsize*sizeof(unsigned short));
   if(idx==NULL)
   {
      printf("Error: could not allocate memory for indices of mesh.\n");
      exit(1); 
   }
   i=0;
   for(q=0;q<num;q++)
   {
      if(fscanf(fp,"%d %d %d",&v[0],&v[1],&v[2])!=3)
      {
	 printf("Error: could not read %d vertex indices of mesh.\n",num);
	 exit(1);   
      }

      if(norms)
      {
	 if(fscanf(fp,"%d %d %d",&n[0],&n[1],&n[2])!=3)
	 {
	    printf("Error: could not read %d set of normal indices of mesh.\n",num);
	    exit(1);   
	 }
      }
      
      if(txts)
      {
	 if(fscanf(fp,"%d %d %d",&t[0],&t[1],&t[2])!=3)
	 {
	    printf("Error: could not read %d texturecoord indices of mesh.\n",num);
	    exit(1);   
	 }
      }
      
      /* indices appear in this order: [texture] [normals] vertices. []=optional */

      for(w=0;w<3;w++)
      {
	 if(txts) idx[i++]=t[w];
	 if(norms) idx[i++]=n[w];
	 idx[i++]=v[w];
/*	 printf("vv: %d\n",v[w]); */
      }
   }
   *indices=idx;
   *num_tris=num;
}

static void viAddMesh(Vec3f *verts, int num_verts, Vec3f *norms, int num_norms,
					Vec2f *txts, int num_txts, char *texturename,
					unsigned short* indices, int num_tris)
{
	int i;
	int size = 3, step = 1;

	assert(verts != NULL && num_verts >= 3);
	
	if(txts)  { size += 3; ++step; }
	if(norms) { size += 3; ++step; }
	Mesh *mesh = new Mesh(txts, num_txts, norms, num_norms, verts, num_verts);
	TransformHierarchy *th = transformHierarchy.top();
	if(th != NULL)
		th->meshPtr = mesh;

	mesh->material = currentMaterial;

	mesh->setTransformHierarchy(th);
	if(! mesh->isStatic) rayTracer->scene->addAniMesh(mesh);
	for(i = 0; i < num_tris; ++i) {
		Triangle* tri = new Triangle(mesh);
		int j = 0;
		if(txts){
			tri->setTxtIdx(indices[i * size + j],
				indices[i * size + step + j], indices[i * size + 2 *step + j]);
			++j;
		}
		if(norms){
			tri->setNormIdx(indices[i * size + j],
				indices[i * size + step + j],indices[i * size + 2 * step + j]);
			++ j;
		}
		tri->setVertIdx(indices[i * size + j], 
				indices[i * size + step + j], indices[i * size + 2 * step + j]);
		mesh->addTriangle(tri);
		rayTracer->scene->addShape(tri);
	}
	// read texture
	if(txts) {
		Texture *texture = rayTracer->scene->addTexture(texturename);
		mesh->texture = texture;
	}
	return;
}

static void parseMesh(FILE *fp)
{
   char str[200];
   int num_verts,num_norms,num_txts,num_tris;
   Vec3f *verts=NULL,*norms=NULL;
   Vec2f *txts=NULL;
   unsigned short *indices;
   char texturename[200];

   if(fscanf(fp,"%s",str)!=1)
   {
      printf("Error: could not parse mesh (could not find 'vertices').\n");
      exit(1);
   }
   if(strcmp(str,"vertices"))
   {
      printf("Error: could not parse mesh (expected 'vertices').\n");
      exit(1);
   }
   getVectors(fp,"vertices",&num_verts,&verts);

   fscanf(fp,"%s",str);
   if(!strcmp(str, "normals"))
   {
      getVectors(fp,"normals",&num_norms,&norms);
      fscanf(fp,"%s",str);
   }
   if(!strcmp(str, "texturecoords"))
   {
      getTextureCoords(fp,texturename,&num_txts,&txts);
      fscanf(fp,"%s",str);
   }
   if(!strcmp(str,"triangles"))
   {
      getTriangles(fp,&num_tris,&indices,verts,norms,txts);
   }
   else
   {
      printf("Error: expected 'triangles' in mesh.\n");
      exit(1);
   }
   /* add a mesh here
    * e.g.,viAddMesh(verts,num_verts,norms,num_norms,txts,num_txts,texturename,indices,num_tris);
    */
    viAddMesh(verts,num_verts,norms,num_norms,txts,num_txts,texturename,indices,num_tris);
}


/*----------------------------------------------------------------------
  viParseFile()
  Description:
    parses the animation file
----------------------------------------------------------------------*/
bool viParseFile(FILE *f)
{
   char ch;
   while((ch=getc(f))!=EOF)
   {  
      switch(ch)
      {
      case ' ':   /* white space */
      case '\t':
      case '\n':
      case '\f':
      case '\r':
	 continue;
      case '#':   /* comment */
      case '%':   /* comment */
	 parseComment(f); /* ok */
	 break;
      case 'v':   /* view point */
	 parseViewpoint(f); /* ok */
	 break;
      case 'l':   /* light source */
	 parseLight(f); /* ok */
	 break;
      case 'b':   /* background color */
	 parseBackground(f); /* ok */
	 break;
      case 'f':   /* fill material */
	 parseFill(f); /* ok */
	 break;
      case 'c':   /* cylinder or cone */
	 parseCone(f); /* ok */
	 break;
      case 's':   /* sphere */
      	 parseSphere(f); /* ok */
	 break;
      case 'p':   /* polygon or patch */
	 parsePoly(f);
	 break;
      case 'i':   /* include another file */
	 parseInclude(f);  /* ok */
	 break;
      case 'd':   /* detail level of file (used to exclude objects from rendering) */
	 parseDetailLevel(f); /* ok */
	 break;
      case 't':  /* textured triangle, or texture tripatch, or animated triangle */
	 parseTextureStuff(f);
	 break;
      case 'x':  /* transform */
	 parseXform(f);
	 break;
      case '}':
	 viEndXform();
	 break;
      case 'a':  /* animation parameters */
	 parseA(f);
	 break;
      case 'k':  /* key frames for transform (or the camera) */
	 parseKeyFrames(f);
	 break;
      case 'm':  /* triangle mesh */
	 parseMesh(f);
	 break;
	 case 'e':  // end
		fprintf(stderr, "[%s] come to end\n", __func__);
		return true; // i defined it for debug
      default:    /* unknown */
	 printf("unknown NFF primitive code: %c\n",ch);
	 exit(1);
      }
   }
   return true;
}
