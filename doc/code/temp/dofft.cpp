/** 
 * \file dofft.cpp
 * \brief Calculate FFT using the VIPS library functions
 * 
 * Use VIPS library to compute fft 
 *
 *  compile with $(pkg-config --cflags vips)
 *  link with $(pkg-config --libs vips)
 *
 */

#include <stdio.h>
#ifdef __GNUC__
#  if(__GNUC__ > 3 || __GNUC__ ==3)
#	define _GNUC3_
#  endif
#endif

#ifdef _GNUC3_
#  include <iostream>
#  include <fstream>
   using namespace std;
#else
#  include <iostream.h>
#  include <fstream.h>
#endif
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vips/vips.h"
int im_fwfft( VipsImage *in, VipsImage *out );

extern "C" {

void dofft(int width, int height, int *imageData, int* outputData) 
{
  int i,j;
        int *inpixels;
        double *outpixels;
        int memsize = 4*width*height;
	int fftData[memsize];
        float scale=0.0;

        VipsImage *vipsin = vips_image_new_from_memory_copy(imageData,memsize,width,height,1,VIPS_FORMAT_UINT);
        if (vipsin == NULL) vips_error_exit(NULL);
        VipsImage *vipsout = vips_image_new_from_memory_copy(&fftData,memsize*4,width,height,1,VIPS_FORMAT_COMPLEX);
        if (vipsout == NULL) vips_error_exit(NULL);
        vips_image_inplace(vipsin);
        vips_image_inplace(vipsout);
        im_fwfft(vipsin,vipsout);
        for (i=0;i<width;i++) {
           scale = scale + (float)imageData[i*width+i];
        } 
        scale = scale / width;
        for (i=0;i<width;i++) {
             for (j=0;j<height;j++) {
                 outpixels = (double *)VIPS_IMAGE_ADDR(vipsout,i,j);
                 outputData[i*width+j] = (int)(outpixels[0]*1000.+scale);
             }
        }
        g_object_unref(vipsin);
        g_object_unref(vipsout);
}

}



