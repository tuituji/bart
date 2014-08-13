/*----------------------------------------------------------------------
  File:    texture.c
  Purpose: Stuff concerning textures
           reads PPM files (only in P6 format, i.e., RGB binary)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texture.h"


Color Texture::sample(float u, float v)
{
	int iu, iv;
	float tu, tv;
	u = u - (int)u;
	v = v - (int)v;
	u = u < 0 ?	u + 1.0 : u;
	v = v < 0 ?	v + 1.0 : v;
	u *= (mWidth - 3);
	v *= (mHeight - 3);
	
	iu = (int)u;
	iv = (int)v;
	
	tu = u - iu;
	tv = v - iv;
	
	Color c = getPixel(iu, iv) * (1 - tu) * (1 - tv) +
				getPixel(iu + 1, iv) * tu * (1 - tv) +
				getPixel(iu, iv + 1) * (1 - tu) * tv + 
				getPixel(iu + 1, iv + 1) * tu * tv;
	
	return c;
}

/* skip all rows that start with #, i.e., comments */
static void eatComments(FILE *f)
{
   int ch;
   while((ch=getc(f))=='#')
   {
      char str[1000];
      fgets(str,1000,f);
   }
   ungetc(ch,f);   
}

static void eatWhitespace(FILE *f)
{
   int ch;
   ch=getc(f);
   while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
   {
      ch=getc(f);
   }
   ungetc(ch,f);
}

/* reads a ppm file with the P6 header (means raw RGB), returns a struct Texture */
/* the header looks like this:
 *---------
 * P6
 * # comments if you want to
 * width height
 * 255
 *---------
 * then follows RGBRGBRGBRGBRGB...
 */
struct Texture *viReadPPM(const char *filename)
{
   FILE *f;
   char ch;
   int width, height, colres;
   if(f=fopen(filename,"rb"))
   {
      char str[100];
      struct Texture *tx;
      eatWhitespace(f);
      eatComments(f);
      eatWhitespace(f);
      fscanf(f,"%s",str);
      if(strcmp(str,"P6")!=0)
      {
	 printf("Error: the texture image file must be of raw color PPM format,\n");
	 printf("i.e., it must have P6 in the header. File: %s\n",filename);
	 exit(1);
      }
      eatWhitespace(f);
      eatComments(f);
      eatWhitespace(f);
      fscanf(f,"%d %d",&width,&height);
      if(width<=0 || height<=0)
      {
	 printf("Error: width and height of the image must be greater than zero. File: %s\n",filename);
	 exit(1);
      }
      eatWhitespace(f);
      eatComments(f);
      eatWhitespace(f);
      fscanf(f,"%d",&colres);
      if(colres!=255)
      {
	 printf("Error: color resolution must be 255.File: %s\n",filename);
	 return NULL;
      }
      /* gotta eat the newline too */
      ch=0;
      while(ch!='\n') fscanf(f,"%c",&ch);
      
      tx=(struct Texture*)malloc(sizeof(struct Texture));
      if(tx==NULL)
      {
	 printf("Error: could not allocate memory for texture struct. File: %s\n",filename);
	 return NULL;
      }
      tx->mWidth=width;
      tx->mHeight=height;
      tx->mRGB=(unsigned char*)malloc(3*width*height);
      if(tx->mRGB==NULL)
      {
	 printf("Error: could not allocate memory for the pixels of the texture. File: %s\n",filename);
	 exit(1);
      }
      
      if(fread(tx->mRGB,3*width*height,1,f)!=1)
      {
	 printf("Error: could not read 3 x %d bytes of pixel info. File: %s\n",width*height,filename);
	 return NULL;
      }
            
      fclose(f);

      return tx;
   }
   else
   {
      printf("Error: could not open %s.\n",filename);
      return NULL;
   }   
}

int viWritePPM(char *filename, int width, int height, unsigned char *pixels)
{
   FILE *f;
   if(f=fopen(filename,"wb"))
   {
      int q;
      fprintf(f,"P6\n%d %d\n255\n",width,height);
      /* write the bottommost line first, i.e., in reversed y-order */
      for(q=0;q<height;q++)
      {
	 unsigned char *adr;
	 adr=pixels+3*width*(height-1-q);
	 fwrite(adr,3*width,1,f);
      }
      fclose(f);
      return 1;
   }
   else
   {
      printf("Error: could not open file <%s>.\n",filename);
      return 0;
   }
}

