/* This file contains the tcl interface routine to high level
   API for CCD frame processing
   Author : Dave Mills (The Random Factory
   Date   : 20 Dec 2000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include "fitsio.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#ifdef LINUX
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef WITH_CDL
#include "cdl.h"
#endif

int tcl_read_image(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_image(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_image32(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_image16(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);

int tcl_write_mef(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_mef16(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_image32s(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_cimage(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_fimage(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_dimage(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_zimage(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_write_simage(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_shmem_image(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
#ifdef WITH_CDL
int tcl_show_image(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
#endif
int tcl_write_calibrated(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_store_calib(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_list_buffers(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_set_biascols(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_set_biasrows(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_leakymem(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_leakyadd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_superadd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_frameadd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_framediv(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_openSocketReader(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_readSocket(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);
int tcl_backsub(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);


int writeimage(void *src_buffer, char *filename, int nx, int by, int ibx, int iby);
int readimage(char *buffer_name, char *filename);
int calibrateimage(Tcl_Interp *interp, int bnum, char *filename);
int printerror(int status);
int biascorrimage(Tcl_Interp *interp, void *src_buffer, char *filename, int nx, int ny);
int biassubtract(unsigned short *src,float *dest, int nx, int ny);
int ibiassubtract(unsigned int *src,unsigned int *dest, int nx, int ny);
int write_buffered_image(Tcl_Interp *interp, void *buffer_name, char *filename);
int write_buffered_image32(Tcl_Interp *interp, void *buffer_name, char *filename);
int write_buffered_image16(Tcl_Interp *interp, void *buffer_name, char *filename);
int write_buffered_mef(Tcl_Interp *interp, int nccd, void *buffer_name, char *filename);
int write_mef(Tcl_Interp *interp, void *buffer_name, char *filename);
int calculate_flatfield(float *image,float *work);
int calculate_dark(float *image);
int calculate_zero(float *image);
void determine_fbiascols(int buffernum);
void determine_ibiascols(int buffernum);
int calculate_skyflat(float *image, float *work);
float calculate_median(float *data, int n);
int image_i2tof(unsigned short *src,float *dest,int n);
int image_rawi2tof(unsigned short *src,float *dest,int n);
int image_i2toi4(unsigned short *src,int *dest,int n);
void subtractzero(float *rimg,float *temp,int nelements);
void subtractdark(float *rimg,float *temp,int nelements);                         
void divide(float *rimg,float *temp,int nelements);                         
int shmmapimage(void *buffer_name, int *shmid, size_t *size);
int disp_init (int w, int h, int fbconfig, int frame);
void converttobyte(float *src, unsigned char *dest,int n);
void create_fits_header(Tcl_Interp *interp, fitsfile *fptr);

unsigned int sdlcolors[256];

/* typedef unsigned int *PDATA;  */
typedef void *PDATA;  

PDATA CCD_locate_buffer(char *name, int idepth, int imgcols, int imgrows, int hbin, int vbin);
int   CCD_free_buffer();
int   CCD_locate_buffernum(char *name);

#define MAX_CCD_BUFFERS  1000
#define MAX_BIAS_COLS 20
#define MAX_ROWS 256000
#define MAX_COLS 256000
int     ierror;
int     CCD_BUFCOUNT = 0;

typedef struct {
     void           *pixels;
     int            size;
     int          xdim;
     int          ydim;
     int          zdim;
     int          xbin;
     int          ybin;
     int          type;
     char           name[64];
     int            shmid;
     size_t         shmsize;
     char           *shmem;
} CCD_FRAME;

CCD_FRAME CCD_Frame[MAX_CCD_BUFFERS];

int     CCD_cameranum =0;
float   exposure=0.0;
int	leakcount = 0;
FILE    *socketReader;
int     socketOpened;

unsigned int *error;
int     bias_start, bias_end, bcols;
int     bias_rstart, bias_rend, brows;
int     geometry_change = 0;
int     last_imgrows = 1;
int     last_imgcols = 1;
int     last_hbin = 1;
int     last_vbin = 1;

#ifdef WITH_CDL
CDLPtr cdl;                                                                     
#endif
                                               


PDATA CCD_locate_buffer(char *name, int idepth, int imgcols, int imgrows, int hbin, int vbin)
{
     int nb; 
     PDATA bptr;
     int found;
     int i;

     found = -1;
     i = 0;
     bptr = NULL;
 
     while (found<0 && i<MAX_CCD_BUFFERS) {

       if (CCD_Frame[i].pixels != NULL) {
         if (strcmp(name,CCD_Frame[i].name) == 0) {
            bptr = CCD_Frame[i].pixels;
            found = i;
         } 
       }
       i++;
     }
       
     geometry_change = 0;
     if (imgcols != last_imgcols) {geometry_change=1;}
     if (imgrows != last_imgrows) {geometry_change=1;}
     if (hbin != last_hbin) {geometry_change=1;}
     if (vbin != last_vbin) {geometry_change=1;}
     if (geometry_change == 1) {
        found = -1;
        CCD_free_buffer(name);
        CCD_free_buffer("calibrated");
     }

     if (found < 0) {
       nb = (imgrows/hbin)*imgcols/vbin*idepth+4;
       i=0;
       while (CCD_Frame[i].pixels != 0 && i<MAX_CCD_BUFFERS) {i++;}
       bptr = ((PDATA) malloc(nb));
       CCD_Frame[i].pixels = bptr;
       strcpy(CCD_Frame[i].name,name);
       CCD_Frame[i].size = nb;
       CCD_Frame[i].xdim = imgcols/hbin;
       CCD_Frame[i].ydim = imgrows/vbin;
       CCD_Frame[i].xbin = hbin;
       CCD_Frame[i].ybin = vbin;
       CCD_Frame[i].zdim = idepth;
       CCD_Frame[i].shmid = 0;
       CCD_Frame[i].shmsize = 0;
       CCD_Frame[i].shmem = NULL;
       if (geometry_change == 1) {
          last_imgcols = imgcols;
          last_imgrows = imgrows;
          last_hbin = hbin;
          last_vbin = vbin;
       }
     }
     return(bptr);
}


PDATA CCD_new_buffer(char *name, int nx, int ny, int idepth)
{
     int nb; 
     PDATA bptr;
     int i;

     i = 0;
     bptr = NULL;
  
     while (CCD_Frame[i].pixels != NULL && i < MAX_CCD_BUFFERS) {
       i++;
     }
  
     if (i < MAX_CCD_BUFFERS) {
       nb = nx * ny * idepth;
       bptr = ((PDATA) malloc(nb));
       CCD_Frame[i].pixels = bptr;
       strcpy(CCD_Frame[i].name,name);
       CCD_Frame[i].size = nb;
       CCD_Frame[i].xdim = nx;
       CCD_Frame[i].ydim = ny;
       CCD_Frame[i].xbin = 1;
       CCD_Frame[i].ybin = 1;
       CCD_Frame[i].zdim = idepth;
       CCD_Frame[i].shmid = 0;
       CCD_Frame[i].shmsize = 0;
       CCD_Frame[i].shmem = NULL;
     }
     return(bptr);
}

int CCD_locate_buffernum(char *name)
{
     int found;
     int i;

     found = -1;
     i = 0;
 
     while (found<0 && i<MAX_CCD_BUFFERS) {

       if (CCD_Frame[i].pixels != NULL) {
         if (strcmp(name,CCD_Frame[i].name) == 0) {
            found = i;
         } 
       }
       i++;
     }
  
     return(found);
}




int CCD_free_buffer(char *name)
{
  
     int found;
     int i;

     found = -1;
     i = 0;
 
     while (found<0 && i<MAX_CCD_BUFFERS) {

       if (CCD_Frame[i].pixels != NULL) {
         if (strcmp(name,CCD_Frame[i].name) == 0) {
            found = i;
         } 
       }
       i++;
     }
  
     if (found >= 0) {
        free(CCD_Frame[found].pixels);
        CCD_Frame[found].pixels = NULL;
        CCD_Frame[found].size = 0;
        strcpy(CCD_Frame[found].name,"NOT-IN-USE");
        if (CCD_Frame[found].shmem != NULL) {
           shmdt(CCD_Frame[found].shmem);
           CCD_Frame[found].shmem = NULL;
        }
     }
     return(0);
}


int CCD_free_bufferaddr(PDATA addr)
{
  
     int found;
     int i;

     found = -1;
     i = 0;
 
     while (found<0 && i<MAX_CCD_BUFFERS) {

       if (CCD_Frame[i].pixels == addr) {
            found = i;
       }
       i++;
     }
  
     if (found >= 0) {
        free(CCD_Frame[found].pixels);
        CCD_Frame[found].pixels = NULL;
        CCD_Frame[found].size = 0;
        strcpy(CCD_Frame[found].name,"NOT-IN-USE");
        if (CCD_Frame[found].shmem != NULL) {
           shmdt(CCD_Frame[found].shmem);
           CCD_Frame[found].shmem = NULL;
        }
     }
     return(0);
}


void CCD_buffer_init()
{
   
     int i;

     i = 0;
 
     while (i<MAX_CCD_BUFFERS) {
        CCD_Frame[i].pixels = NULL;
        CCD_Frame[i].shmem = NULL;
        CCD_Frame[i].size = 0;
        strcpy(CCD_Frame[i].name,"NOT-IN-USE");        
        i++;
     }
}

 
int tcl_list_buffers(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
   
     int i;

     i = 0;
 
     while (i<MAX_CCD_BUFFERS) {
        if (CCD_Frame[i].pixels != NULL) {
            printf("Buffer %4d %s = %d bytes cols=%d rows=%d depth=%d\n",i,CCD_Frame[i].name,
              CCD_Frame[i].size,CCD_Frame[i].xdim,CCD_Frame[i].ydim,CCD_Frame[i].zdim);
        }
        i++;
     }
     return TCL_OK;
}

  


/* Routine : ccdAppInit
   Purpose : This routine is called from tclAppInit and initializes all the ccd
             tcl interface routines
 */
int ccdAppInit(Tcl_Interp *interp)
{

/* Initialize the new commands */


   Tcl_CreateCommand(interp, "read_image", (Tcl_CmdProc *) tcl_read_image, NULL, NULL);
   Tcl_CreateCommand(interp, "write_image", (Tcl_CmdProc *) tcl_write_image, NULL, NULL);
   Tcl_CreateCommand(interp, "write32", (Tcl_CmdProc *) tcl_write_image32, NULL, NULL);
   Tcl_CreateCommand(interp, "write16", (Tcl_CmdProc *) tcl_write_image16, NULL, NULL);

   Tcl_CreateCommand(interp, "write_mef", (Tcl_CmdProc *) tcl_write_mef, NULL, NULL);
   Tcl_CreateCommand(interp, "write_mef16", (Tcl_CmdProc *) tcl_write_mef16, NULL, NULL);
   Tcl_CreateCommand(interp, "write32s", (Tcl_CmdProc *) tcl_write_image32s, NULL, NULL);

   Tcl_CreateCommand(interp, "write_cimage", (Tcl_CmdProc *) tcl_write_cimage, NULL, NULL);
   Tcl_CreateCommand(interp, "write_fimage", (Tcl_CmdProc *) tcl_write_fimage, NULL, NULL);
   Tcl_CreateCommand(interp, "write_dimage", (Tcl_CmdProc *) tcl_write_dimage, NULL, NULL);
   Tcl_CreateCommand(interp, "write_zimage", (Tcl_CmdProc *) tcl_write_zimage, NULL, NULL);
   Tcl_CreateCommand(interp, "write_simage", (Tcl_CmdProc *) tcl_write_simage, NULL, NULL);
   Tcl_CreateCommand(interp, "shmem_image", (Tcl_CmdProc *) tcl_shmem_image, NULL, NULL);
#ifdef WITH_CDL
   Tcl_CreateCommand(interp, "show_image", (Tcl_CmdProc *) tcl_show_image, NULL, NULL);
#endif
   Tcl_CreateCommand(interp, "store_calib", (Tcl_CmdProc *) tcl_store_calib, NULL, NULL);
   Tcl_CreateCommand(interp, "write_calibrated", (Tcl_CmdProc *) tcl_write_calibrated, NULL, NULL);
   Tcl_CreateCommand(interp, "list_buffers", (Tcl_CmdProc *) tcl_list_buffers, NULL, NULL);
   Tcl_CreateCommand(interp, "set_biascols", (Tcl_CmdProc *) tcl_set_biascols, NULL, NULL);
   Tcl_CreateCommand(interp, "set_biasrows", (Tcl_CmdProc *) tcl_set_biasrows, NULL, NULL);
   Tcl_CreateCommand(interp, "leakymem", (Tcl_CmdProc *) tcl_leakymem, NULL, NULL);
   Tcl_CreateCommand(interp, "superadd", (Tcl_CmdProc *) tcl_superadd, NULL, NULL);
   Tcl_CreateCommand(interp, "frameadd", (Tcl_CmdProc *) tcl_frameadd, NULL, NULL);
   Tcl_CreateCommand(interp, "framediv", (Tcl_CmdProc *) tcl_framediv, NULL, NULL);
   Tcl_CreateCommand(interp, "socketOpen", (Tcl_CmdProc *) tcl_openSocketReader, NULL, NULL);
   Tcl_CreateCommand(interp, "read_socket", (Tcl_CmdProc *) tcl_readSocket, NULL, NULL);
   Tcl_CreateCommand(interp, "backsub", (Tcl_CmdProc *) tcl_backsub, NULL, NULL);
   CCD_buffer_init();
   return TCL_OK;
}


int tcl_store_calib(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char calibname[32];
  char calibtype[8];
  char buffer_name[32];
 
  unsigned int *image;
  unsigned int *fbuffer;
  int  nx, ny, ibx, iby;
  int inum, bnum;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name type nnn\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%s", buffer_name);
  sscanf(argv[2], "%s", calibtype);
  sscanf(argv[3], "%d", &inum);
  sprintf(calibname,"%s%d",calibtype,inum);

  /* Locate named buffer */
  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to find buffer in CCD_locate_buffernum", (char *)NULL);
     return TCL_ERROR; 
  }
  image = CCD_Frame[bnum].pixels;

  /* Obtain/Reuse buffer */
  nx = CCD_Frame[bnum].xdim-bcols;
  ny = CCD_Frame[bnum].ydim;
  ibx = CCD_Frame[bnum].xbin;
  iby = CCD_Frame[bnum].ybin;
  fbuffer = CCD_locate_buffer(calibname, 2, nx, ny, ibx, iby);
  if (fbuffer == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }

  ibiassubtract(image,fbuffer,nx,ny);                                           

  return TCL_OK;
}

int tcl_backsub(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char buffer_name[32];
  unsigned char *from8, *to8;
  unsigned int *from16, *to16;
  unsigned int *from32, *to32;
  unsigned int *image;
  int i, new, bnum;
  int  address, nx, ny, bitpix, npix,leak, dpix;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 2) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%s", buffer_name);
  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to find buffer in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }
  image = CCD_Frame[bnum].pixels;
  nx = CCD_Frame[bnum].xdim;
  ny = CCD_Frame[bnum].ydim;
  npix = nx*ny;
  backsub(image,npix);
  return TCL_OK;
}

int backsub(unsigned int *image, int npix)
{
   int i,iminimum;

   iminimum = 999999;
   for (i=0;i<npix;i++) {
       if (image[i] < iminimum) iminimum=image[i];
   }
   for (i=0;i<npix;i++) {
       image[i] = image[i]-iminimum;
   }
   return 0;
}






int tcl_superadd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char buffer_name[32];
  char dbuffer_name[32];
  unsigned char *from8;
  unsigned int *to32;
  unsigned int *image,*dimage,*simage;
  double dx,dy;
  int i, iix,iiy,ifrom,ix,iy,iox,ioy, idx, idy, idata;
  int  address, nx, ny, npix;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 7) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," address name nx ny dx dy (dname)\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%d", &address);
  sscanf(argv[2], "%s", buffer_name);
  sscanf(argv[3], "%d", &nx);
  sscanf(argv[4], "%d", &ny);
  sscanf(argv[5], "%lf", &dx);
  sscanf(argv[6], "%lf", &dy);
    
  /* Locate named buffer */
  image = CCD_locate_buffer(buffer_name, 4, nx, ny, 1, 1);  

  simage = CCD_locate_buffer("superimage", 4, nx*5, ny*5, 1, 1);
  if (simage == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer for superimage", (char *)NULL);
     return TCL_ERROR; 
  }

  npix = nx*ny;

  to32 = (unsigned int *)image;
  from8 = (unsigned char *)address;
  
  if (argc == 8) {
    sscanf(argv[7], "%s", dbuffer_name);
    dimage = CCD_locate_buffer(dbuffer_name, 4, nx, ny, 1, 1);  
    for (i=0;i<npix;i++) {
      from8[i] = from8[i]-dimage[i]; 
    }
  }    

  if ( address == 0 ) {                     
    for (i=0;i<npix;i++) {
      to32[i] = (unsigned int)0; 
    }
    for (i=0;i<npix*25;i++) {
      simage[i] = (unsigned int)0; 
    }
  } else {
    ix = (int)(dx*5.0);
    iy = (int)(dy*5.0);
    for (iix=0;iix<640;iix++) {
      for (iiy=0;iiy<480;iiy++) {
          ifrom = iiy*640+iix;
          for (idx=-2;idx<2;idx++) { 
            for (idy=-2;idy<2;idy++) {
               iox = iix*5+idx+ix;
               ioy = iiy*5+idy+iy;
               if (iox > -1 && ioy > -1 && iox < 640*5 && ioy < 480*5) {
                  simage[640*5*ioy+iox] = simage[640*5*ioy+iox] + from8[ifrom];
               }
            }
          }
      }
    }
    for (iix=0;iix<640;iix++) {
      for (iiy=0;iiy<480;iiy++) {
          idata = 0;
          for (idx=-2;idx<2;idx++) { 
            for (idy=-2;idy<2;idy++) {
               iox = iix*5+idx;
               ioy = iiy*5+idy;
               idata = idata + simage[640*5*ioy+iox];
            }
          }
          to32[640*iiy+iix] = (unsigned int)(idata/25);
      }
    }
  }


  return TCL_OK;
}


int tcl_leakymem(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char buffer_name[32];
  unsigned char *from8, *to8;
  unsigned int *from16, *to16;
  unsigned int *from32, *to32;
  unsigned int *image;
  double dgray;
  int i, new;
  int  address, nx, ny, bitpix, npix,leak, dpix, ipix;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 7) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," address name bitpix nx ny leak\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%d", &address);
  sscanf(argv[2], "%s", buffer_name);
  sscanf(argv[3], "%d", &bitpix);
  sscanf(argv[4], "%d", &nx);
  sscanf(argv[5], "%d", &ny);
  sscanf(argv[6], "%d", &leak);
  dpix = bitpix/8;
  if (bitpix ==816) {dpix=2;}
  if (bitpix ==832) {dpix=4;}
  if (bitpix ==1632) {dpix=4;}
  if (bitpix ==2432) {dpix=4;}
  leakcount = leak;

  /* Locate named buffer */
  image = CCD_locate_buffer(buffer_name, dpix, nx, ny, 1, 1);  

  npix = nx*ny;
  if (leak < 2 && bitpix < 33) {
     memcpy(image,address,npix*bitpix/8);
  } else {
     switch(bitpix) {
             case 8:
                            from8 = (unsigned char *)address;
                            to8 = (unsigned char *)image;
                            if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to8[i] = (unsigned char)from8[i]; 
                              }
                             } else {
                              for (i=0;i<npix;i++) {
                                    new = (unsigned int)to8[i]*leak/(leak+1) + (unsigned int)from8[i];
                                    to8[i] = (unsigned char)new;
                               }
                            }
                            break;
             case 816:
                            from8 = (unsigned char *)address;
                            to16 = (unsigned int *)image;
                            if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to16[i] = (unsigned int)from8[i]; 
                              }
                             } else {
                              for (i=0;i<npix;i++) {
                                    new = (unsigned int)to16[i]*leak/(leak+1)  + (unsigned int)from8[i];
                                    to16[i] = (unsigned int)new;
                               }
                            }
                            break;
             case 832:
                            from8 = (unsigned char *)address;
                            to32 = (unsigned int *)image;
                            if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to32[i] = (unsigned int)from8[i]; 
                              }
                             } else {
                              for (i=0;i<npix;i++) {
                                    new = (unsigned int)to32[i]*leak/(leak+1)  + (unsigned int)from8[i];
                                    to32[i] = (unsigned int)new;
                               }
                            }
                            break;
             case 2432:
                            from8 = (unsigned char *)address;
                            to32 = (unsigned int *)image;
                            if (leak < 2) {
                              ipix = 0;
                              for (i=0;i<npix;i++) {
		                     dgray = (0.299 * from8[ipix] + 0.587 * from8[ipix+1] + 0.114 * from8[ipix+2]) * 256.0;
                                     to32[i] = (unsigned int)dgray; 
				     ipix = ipix + 3;
                              }
                             } else {
                              ipix = 0;
                              for (i=0;i<npix;i++) {
		                    dgray = (0.299 * from8[ipix] + 0.587 * from8[ipix+1] + 0.114 * from8[ipix+2]) * 256.0;
                                    new = (unsigned int)to32[i]*leak/(leak+1)  + (unsigned int)dgray;
                                    to32[i] = (unsigned int)new;
				    ipix = ipix + 3;
                               }
                            }
                            break;
             case 16:
                            from16 = (unsigned int *)address;
                            to16 = (unsigned int *)image;
                             if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to16[i] = (unsigned int)from16[i]; 
                              }
                             } else {
                             for (i=0;i<npix;i++) {
                                    new = (unsigned int)to16[i]*leak/(leak+1)  + (unsigned int)from16[i];
                                    to16[i] = (unsigned int)new;
                               }
                            }
                            break;
            case 1632:
                            from16 = (unsigned int *)address;
                            to32 = (unsigned int *)image;
                             if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to32[i] = (unsigned int)from16[i];
                              }
                             } else {
                             for (i=0;i<npix;i++) {
                                    new = (unsigned int)to32[i]*leak/(leak+1)  + (unsigned int)from16[i];
                                    to32[i] = (unsigned int)new;
                               }
                            }
                            break;
             case 32:
                            from32 = (unsigned int *)address;
                            to32 = (unsigned int *)image;
                            if (leak < 2) {
                              for (i=0;i<npix;i++) {
                                     to32[i] = (unsigned int)from32[i]; 
                              }
                             } else {
                              for (i=0;i<npix;i++) {
                                    new = (unsigned int)to32[i]*leak/(leak+1) + (unsigned int)from32[i];
                                    to32[i] = (unsigned int)new;
                               }
                            }
                            break;
     }
  }
 
  return TCL_OK;
}



int tcl_frameadd(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char buffer_name[32];
  unsigned char *from8, *to8;
  unsigned int *from16, *to16;
  unsigned int *from32, *to32;
  unsigned int *image;
  int i;
  int  address, nx, ny, bitpix, npix,dpix;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 6) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," address name bitpix nx ny\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%d", &address);
  sscanf(argv[2], "%s", buffer_name);
  sscanf(argv[3], "%d", &bitpix);
  sscanf(argv[4], "%d", &nx);
  sscanf(argv[5], "%d", &ny);
  dpix = bitpix/8;
  if (bitpix ==816) {dpix=2;}
  if (bitpix ==832) {dpix=4;}
  if (bitpix ==1632) {dpix=4;}
  if (bitpix ==2432) {dpix=4;}

  /* Locate named buffer */
  image = CCD_locate_buffer(buffer_name, dpix, nx, ny, 1, 1);  

  npix = nx*ny;
     switch(bitpix) {
             case 8:
                            from8 = (unsigned char *)address;
                            to8 = (unsigned char *)image;
                              for (i=0;i<npix;i++) {
                                     to8[i] = to8[i] + (unsigned char)from8[i]; 
                              }
                            break;
             case 816:
                            from8 = (unsigned char *)address;
                            to16 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to16[i] = to16[i] + (unsigned int)from8[i]; 
                              }
                            break;
             case 832:
                            from8 = (unsigned char *)address;
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] +  (unsigned int)from8[i]; 
                              }
                            break;
             case 16:
                            from16 = (unsigned int *)address;
                            to16 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to16[i] = to16[i] + (unsigned int)from16[i]; 
                              }
                            break;
            case 1632:
                            from16 = (unsigned int *)address;
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] + (unsigned int)from16[i];
                              }
                            break;
             case 32:
                            from32 = (unsigned int *)address;
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] + (unsigned int)from32[i]; 
                              }
                            break;
     }
 
  return TCL_OK;
}



int tcl_framediv(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char buffer_name[32];
  unsigned char *to8;
  unsigned int *to16;
  unsigned int *to32;
  unsigned int *image;
  int i;
  int nx, ny, bitpix, npix,dpix, idiv;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 6) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name bitpix nx ny idiv\"", (char *)NULL);
     return TCL_ERROR;
  }

  sscanf(argv[1], "%s", buffer_name);
  sscanf(argv[2], "%d", &bitpix);
  sscanf(argv[3], "%d", &nx);
  sscanf(argv[4], "%d", &ny);
  sscanf(argv[5], "%d", &idiv);
  dpix = bitpix/8;
  if (bitpix ==816) {dpix=2;}
  if (bitpix ==832) {dpix=4;}
  if (bitpix ==1632) {dpix=4;}
  if (bitpix ==2432) {dpix=4;}

  /* Locate named buffer */
  image = CCD_locate_buffer(buffer_name, dpix, nx, ny, 1, 1);  

  npix = nx*ny;
     switch(bitpix) {
             case 8:
                            to8 = (unsigned char *)image;
                              for (i=0;i<npix;i++) {
                                     to8[i] = to8[i] / idiv; 
                              }
                            break;
             case 816:
                            to16 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to16[i] = to16[i] / idiv; 
                              }
                            break;
             case 832:
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] / idiv; 
                              }
                            break;
             case 16:
                            to16 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to16[i] = to16[i] / idiv; 
                              }
                            break;
            case 1632:
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] / idiv;
                              }
                            break;
             case 32:
                            to32 = (unsigned int *)image;
                              for (i=0;i<npix;i++) {
                                     to32[i] = to32[i] / idiv; 
                              }
                            break;
     }
 
  return TCL_OK;
}



int tcl_openSocketReader(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  
  char channel_name[32];
  int status;
  int ret, sock, sock_buf_size;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 2) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0],"channel-id\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(channel_name,argv[1]);                                  
  status = Tcl_GetOpenFile(interp,channel_name,0,1,(FILE *)&socketReader);
  sock = socketReader->_fileno;
  sock_buf_size = 4000000;
  ret = setsockopt( sock, SOL_SOCKET, SO_RCVBUF,
                   (char *)&sock_buf_size, sizeof(sock_buf_size) );
  socketOpened = 1;
  
  return status;
}



int tcl_write_image(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  strcpy(filename,argv[2]);                                  
  write_buffered_image(interp, buffer_name, filename);
  CCD_free_buffer(buffer_name);
  
  return TCL_OK;
}

int tcl_write_image16(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  strcpy(filename,argv[2]);                                  
  write_buffered_image16(interp, buffer_name, filename);
  CCD_free_buffer(buffer_name);
  
  return TCL_OK;
}


int tcl_write_mef(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];
  int nccd;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0],"nccd name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[1],"%d",&nccd);
  strcpy(buffer_name,argv[2]);                                  
  strcpy(filename,argv[3]);                                  
  write_buffered_mef(interp, nccd, buffer_name, filename);
  
  return TCL_OK;
}



int tcl_write_mef16(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];
  int nccd;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0],"nccd name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[1],"%d",&nccd);
  strcpy(buffer_name,argv[2]);                                  
  strcpy(filename,argv[3]);                                  
  write_buffered_mef16(interp, nccd, buffer_name, filename);
  
  return TCL_OK;
}


int tcl_write_image32(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  strcpy(filename,argv[2]);                                  
  write_buffered_image32(interp, buffer_name, filename);
  CCD_free_buffer(buffer_name);
  
  return TCL_OK;
}


int tcl_write_image32s(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  strcpy(filename,argv[2]);                                  
  write_buffered_image32s(interp, buffer_name, filename);
  CCD_free_buffer(buffer_name);
  
  return TCL_OK;
}


int tcl_set_biascols(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," bias-start bias-end\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */                            
  sscanf(argv[1], "%d", &bias_start);
  sscanf(argv[2], "%d", &bias_end);
  bcols = bias_end - bias_start+1;
  return TCL_OK;
}


int tcl_set_biasrows(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," bias-start bias-end\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */                            
  sscanf(argv[1], "%d", &bias_rstart);
  sscanf(argv[2], "%d", &bias_rend);
  brows = bias_rend - bias_rstart+1;
  return TCL_OK;
}


int tcl_shmem_image(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{

  char buffer_name[32];
  int shmid;
  size_t shmsize;
  int bnum;
  char result[128];

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 2) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  bnum = CCD_locate_buffernum(buffer_name);
  shmmapimage(buffer_name, &shmid, &shmsize);
  sprintf(result,"%d %d %d %d",shmid,shmsize,
                     CCD_Frame[bnum].xdim, CCD_Frame[bnum].ydim);

  Tcl_SetResult(interp,&result,TCL_STATIC);
  return TCL_OK;
}



int tcl_read_image(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];
  int buffernum;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 3) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  strcpy(filename,argv[2]);                                  
  CCD_free_buffer(buffer_name);
  readimage(buffer_name, filename);
  buffernum = CCD_locate_buffernum(buffer_name);
/*  determine_fbiascols(buffernum); */
  return TCL_OK;
}


int tcl_readSocket(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{

  char buffer_name[32];
  char progress[128];
  int ircount,chunk, iwait;
  int nx, ny, bpp;
  int remaining;
  char *image;
  int status;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 5) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name nx ny bpp chunk\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]); 
  sscanf(argv[2],"%d",&nx);
  sscanf(argv[3],"%d",&ny);
  sscanf(argv[4],"%d",&bpp);
  sscanf(argv[5],"%d",&chunk);
  sscanf(argv[6],"%d",&iwait);
  
  if (socketOpened == 1) {                                 
    ircount = 0;
    remaining = nx*ny;
    image = CCD_locate_buffer(buffer_name, bpp, nx, ny, 1,1);
    while (remaining > 0) {
      if (feof(socketReader)) {
         sprintf(progress,"Rem: %d ircount = %d  ",remaining,ircount);
         Tcl_AppendResult(interp, "EOF : Read not completed", (char *)NULL);
         Tcl_AppendResult(interp, progress, (char *)NULL);
         return TCL_ERROR;
      }
 
 /*      status = fread(image,bpp,remaining,socketReader);  */
      status = fread(image,bpp,chunk,socketReader);
      remaining = remaining - status;
      image = image + status*bpp;
      ircount++;
      sprintf(progress,"Rem: %d ircount = %d\n",remaining,ircount);
      usleep(iwait);
    }
  } else {
     Tcl_AppendResult(interp, "Socket not attached for read", (char *)NULL);
     return TCL_ERROR;
  }

  sprintf(progress,"NReads = %d ",ircount);
  Tcl_AppendResult(interp, progress, (char *)NULL);

  return TCL_OK;
}

    

int tcl_write_cimage(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];
  unsigned int *image;
  int  nx, ny;
  int bnum;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name exposure filename\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);
  sscanf(argv[2], "%f", &exposure);
  strcpy(filename,argv[3]);
  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to find buffer in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }
  image = CCD_Frame[bnum].pixels;
  nx = CCD_Frame[bnum].xdim;
  ny = CCD_Frame[bnum].ydim;
  biascorrimage(interp, image, filename, nx, ny);      
  CCD_free_buffer(buffer_name);                     
  return TCL_OK;
}


int tcl_write_calibrated(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  char buffer_name[32];
  int bnum, keep;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 5) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name exposure filename keep\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);
  sscanf(argv[2], "%f", &exposure);
  strcpy(filename,argv[3]);
  sscanf(argv[4], "%d", &keep);
  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to find buffer in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }
  calibrateimage(interp, bnum, filename);      
/*  if (keep == 0) {
     CCD_free_buffer(buffer_name);                     
  }
*/

  return TCL_OK;
}


int tcl_write_fimage(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  int keep;
  int bnum;
  unsigned int *image;
  PDATA tflat;
  int status;
  int  nx, ny, ibx, iby;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," filename exposure keep\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[3], "%d", &keep);
  strcpy(filename,argv[1]);
  sscanf(argv[2], "%f", &exposure);
  bnum = CCD_locate_buffernum("FLAT1");
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to locate first frame of series (FLAT1)", (char *)NULL);
     return TCL_ERROR; 
  }
  nx = CCD_Frame[bnum].xdim;  
  ny = CCD_Frame[bnum].ydim;  
  ibx = CCD_Frame[bnum].xbin;  
  iby = CCD_Frame[bnum].xbin;  
  image = CCD_locate_buffer("CALIBRATION-FLAT", 4, nx, ny, ibx, iby);
  bnum = CCD_locate_buffernum("CALIBRATION-FLAT");
  if (image == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }
  tflat = CCD_locate_buffer("tempflat", 4, nx, ny, ibx, iby);

  calculate_flatfield((float *)image, (float *)tflat);   
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim-bcols;   */

  status =  write_buffered_image(interp, "CALIBRATION-FLAT", filename);
  if (status < 0) {
     Tcl_AppendResult(interp, "ERROR - failed to create flat-field FITS file", (char *)NULL);
     return TCL_ERROR; 
  }
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim+bcols;   */
 
  if (keep == 0) {
     CCD_free_buffer("CALIBRATION-FLAT");                     
  }
  CCD_free_buffer("tempflat");                     
  return TCL_OK;
}


int tcl_write_dimage(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  int keep;
  int bnum;
  unsigned int *image;
  int status;
  int  nx, ny, ibx, iby;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," filename exposure keep\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[2], "%f", &exposure);
  sscanf(argv[3], "%d", &keep);
  strcpy(filename,argv[1]);
  bnum = CCD_locate_buffernum("DARK1");
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to locate first frame of series (DARK1)", (char *)NULL);
     return TCL_ERROR; 
  }
  nx = CCD_Frame[bnum].xdim;  
  ny = CCD_Frame[bnum].ydim;  
  ibx = CCD_Frame[bnum].xbin;  
  iby = CCD_Frame[bnum].xbin;  
  image = CCD_locate_buffer("CALIBRATION-DARK", 4, nx, ny, ibx, iby);
  bnum = CCD_locate_buffernum("CALIBRATION-DARK");
  if (image == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }

  calculate_dark((float *)image);   
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim-bcols;   */

  status =  write_buffered_image(interp, "CALIBRATION-DARK", filename);
  if (status < 0) {
     Tcl_AppendResult(interp, "ERROR - failed to create dark FITS file", (char *)NULL);
     return TCL_ERROR; 
  }
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim+bcols;   */
 
  if (keep == 0) {
     CCD_free_buffer("CALIBRATION-DARK");                     
  }
  return TCL_OK;
}



int tcl_write_zimage(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  int keep;
  int bnum;
  unsigned int *image;
  int status;
  int  nx, ny, ibx, iby;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," filename exposure keep\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[2], "%f", &exposure);
  sscanf(argv[3], "%d", &keep);
  strcpy(filename,argv[1]);
  bnum = CCD_locate_buffernum("ZERO1");
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to locate first frame of series (ZERO1)", (char *)NULL);
     return TCL_ERROR; 
  }
  nx = CCD_Frame[bnum].xdim;  
  ny = CCD_Frame[bnum].ydim;  
  ibx = CCD_Frame[bnum].xbin;  
  iby = CCD_Frame[bnum].xbin;  
  image = CCD_locate_buffer("CALIBRATION-ZERO", 4,  nx, ny, ibx, iby);
  bnum = CCD_locate_buffernum("CALIBRATION-ZERO");
  if (image == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }

  calculate_zero((float *)image);   
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim-bcols;   */

  status =  write_buffered_image(interp, "CALIBRATION-ZERO", filename);
  if (status < 0) {
     Tcl_AppendResult(interp, "ERROR - failed to create zero FITS file", (char *)NULL);
     return TCL_ERROR; 
  }
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim+bcols;   */
 
  if (keep == 0) {
     CCD_free_buffer("CALIBRATION-ZERO");                     
  }
  return TCL_OK;
}



int tcl_write_simage(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
  char filename[255];
  int keep;
  int bnum;
  unsigned int *image;
  PDATA tflat;
  int status;
  int  nx, ny, ibx, iby;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 4) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," filename exposure keep\"", (char *)NULL);
     return TCL_ERROR;
  }

  /* Retrieve arguments */
  sscanf(argv[2], "%f", &exposure);
  sscanf(argv[3], "%d", &keep);
  strcpy(filename,argv[1]);
  bnum = CCD_locate_buffernum("FSKY1");
  if (bnum < 0) {
     Tcl_AppendResult(interp, "ERROR - Unable to locate first frame of series (FSKY1)", (char *)NULL);
     return TCL_ERROR; 
  }
  nx = CCD_Frame[bnum].xdim;  
  ny = CCD_Frame[bnum].ydim;  
  ibx = CCD_Frame[bnum].xbin;  
  iby = CCD_Frame[bnum].xbin;  
  image = CCD_locate_buffer("CALIBRATION-FSKY", 4, nx, ny, ibx, iby);
  bnum = CCD_locate_buffernum("CALIBRATION-FSKY");
  if (image == NULL) {
     Tcl_AppendResult(interp, "ERROR - Memory allocation failure in CCD_locate_buffer", (char *)NULL);
     return TCL_ERROR; 
  }
  tflat = CCD_locate_buffer("tempflat", 4,  nx, ny, ibx, iby);

  calculate_skyflat((float *)image, (float *)tflat);   
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim-bcols;   */

  status =  write_buffered_image(interp, "CALIBRATION-FSKY", filename);
  if (status < 0) {
     Tcl_AppendResult(interp, "ERROR - failed to create skyflat FITS file", (char *)NULL);
     return TCL_ERROR; 
  }
/*  CCD_Frame[bnum].xdim = CCD_Frame[bnum].xdim+bcols;   */
 
  if (keep == 0) {
     CCD_free_buffer("CALIBRATION-FSKY");                     
  }
  CCD_free_buffer("tempflat");                     
  return TCL_OK;
}

void determine_fbiascols(int buffernum)
{
    float *image;
    float sums[MAX_COLS];
    int ix, iy, nx, ny, ipix, mincol;
    float minsum;
    float datum;

    image = (float *)CCD_Frame[buffernum].pixels;
    nx = CCD_Frame[buffernum].xdim;
    ny = CCD_Frame[buffernum].ydim;
    for (ix=0;ix<nx;ix++) {
      sums[ix] = 0.0;
      for (iy=0;iy<ny;iy++) {
          ipix = nx*iy + ix;
          datum = image[ipix];
          sums[ix] = sums[ix] + datum;
      }
    }
    minsum = 32768.*4096.;
    mincol = 0;
    for (ix=0;ix<nx;ix++) {
        if (sums[ix] < minsum) {
            minsum = sums[ix];
            mincol = ix;
        }
    }      
    minsum = minsum + 2.0*sqrt(minsum);
    bias_start = -1;
    bias_end = -1;
    for (ix=0;ix<nx;ix++) {
       if (sums[ix] < minsum && bias_start < 0) {
           bias_start = ix;
       }
       if (sums[ix] < minsum) {
           bias_end = ix;
       }
    }
    bcols = bias_end - bias_start +1;
    printf("New bias determination %d to %d = %d\n",bias_start,bias_end,bcols);
}



void determine_ibiascols(int buffernum)
{
    unsigned int *image;
    float sums[MAX_COLS];
    int ix, iy, nx, ny, ipix, mincol;
    float minsum;
    float datum;

    image =(unsigned int *)CCD_Frame[buffernum].pixels;
    nx = CCD_Frame[buffernum].xdim;
    ny = CCD_Frame[buffernum].ydim;
    for (ix=0;ix<nx;ix++) {
      sums[ix] = 0.0;
      for (iy=0;iy<ny;iy++) {
          ipix = nx*iy + ix;
          if (image[ipix] == 32767) {
             datum = 65535.;
          } else {
             datum = (float)image[ipix];
             if (datum < 0.0001) { datum = datum+65535.; }
          }
          sums[ix] = sums[ix] + datum;
      }
    }
    minsum = 32768.*4096.;
    mincol = 0;
    for (ix=0;ix<nx;ix++) {
        if (sums[ix] < minsum) {
            minsum = sums[ix];
            mincol = ix;
        }
    }      
    minsum = minsum + 2.0*sqrt(minsum);
    bias_start = -1;
    bias_end = -1;
    for (ix=0;ix<nx;ix++) {
       if (sums[ix] < minsum && bias_start < 0) {
           bias_start = ix;
       }
       if (sums[ix] < minsum) {
           bias_end = ix;
       }
    }
    bcols = bias_end - bias_start +1;
    if (bcols > MAX_BIAS_COLS) { 
        bcols = 0;
        bias_start = MAX_COLS-1;
        bias_end = MAX_COLS-1;
        printf("Unable to determine bias columns - bias = min pixel value\n");
    } else {
        printf("New bias determination %d to %d = %d\n",bias_start,bias_end,bcols);
    }
    geometry_change = 0;
    CCD_free_buffer("calibrated");
    CCD_free_buffer("CALIBRATION-ZERO");
    CCD_free_buffer("CALIBRATION-DARK");
    CCD_free_buffer("CALIBRATION_FLAT");
    CCD_free_buffer("CALIBRATION-FSKY");
}





int calculate_flatfield(float *image,float *work)
{
   unsigned int *aflat,*flats[MAX_CCD_BUFFERS];
   float *bdark, *bzero;
   int   nflat,ndark,nzero;
   int   i,npix;
   int iix, iiy, icol, irow;
   int ipix, opix, ix, iy, nx, ny;
   float pixeldata[MAX_CCD_BUFFERS];
   float fmedian,mean,pzero;
   

   i = 0;
   npix = 0;
   mean = 0.0E0;
   nflat = 0;
   nx = 0;
   ny = 0;
   bzero = NULL;
   bdark = NULL;
   opix = 0;
   ndark = CCD_locate_buffernum("CALIBRATION-DARK");
   if (ndark > -1) {bdark = (float *)CCD_Frame[ndark].pixels;}
   nzero = CCD_locate_buffernum("CALIBRATION-ZERO");
   if (nzero > -1) {bzero = (float *)CCD_Frame[nzero].pixels;}
   while (i<MAX_CCD_BUFFERS) {
       if (CCD_Frame[i].pixels != NULL) {
          if (strncmp(CCD_Frame[i].name,"FLAT",4) == 0) {
             flats[nflat] = (unsigned int *)CCD_Frame[i].pixels;
             nflat++;
             nx = CCD_Frame[i].xdim;
             ny = CCD_Frame[i].ydim-1;
          }
       }
       i++;
   }
   if (nflat == 0) {return(1);}
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
          ipix = nx*iy + ix;
          opix = nx*iy + ix;
          for (i=0;i<nflat;i++) {
              aflat = (unsigned int *)flats[i];
              pixeldata[i] = (float) aflat[ipix];
          }
          fmedian = calculate_median(pixeldata,nflat);
          if (nzero > -1) {
             pzero = bzero[opix];
          } else {
             pzero = 0.0;
          }
          if (ndark > -1) {
             image[opix] = fmedian - bdark[opix]*exposure - pzero;
          } else {
             image[opix] = fmedian - pzero;
          }
          mean = mean+fmedian;
          npix++;
      }
      printf("done row %5d\n",iy);
   }
   mean = mean/(float)npix;
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
         mean = 0.0E0;
         ipix = nx*iy + ix;
         for (iix=-5;iix<6;iix++) { 
         for (iiy=-5;iiy<6;iiy++) { 
             icol = ix + iix;
             if (icol <0) {icol = icol+5;}
             if (icol >=nx) {icol = icol-5;}
             irow = iy + iiy;
             if (irow <0) {irow = irow+5;}
             if (irow >=ny) {irow = irow-5;}
             mean = mean + image[nx*icol+irow];
         }
         }
         work[ipix] = image[ipix] /  (mean/121.);
      }
   }
   for (ipix=0;ipix<npix;ipix++) {
       image[ipix] = work[ipix];
   }
   for (i=0;i<nflat;i++) {
       CCD_free_bufferaddr(flats[i]);
   }
   return(0);
}

float calculate_median(float *data, int n)
{
   int i;
   float fmedian;

   fmedian = 0.0;
   for (i=0;i<n;i++) {
      fmedian = fmedian + data[i];
   }
   fmedian = fmedian/(float)n;
   return(fmedian);
}



int calculate_skyflat(float *image,float *work)
{
   unsigned int *aflat,*flats[MAX_CCD_BUFFERS];
   float *bdark, *bzero;
   int   nflat,ndark,nzero;
   int   i,npix;
   int iix, iiy, icol, irow;
   int ipix, opix, ix, iy, nx, ny;
   float pixeldata[MAX_CCD_BUFFERS];
   float fmedian,mean,average,pzero;


   i = 0;
   npix = 0;
   mean = 0.0E0;
   nflat = 0;
   nx = 0;
   ny = 0;
   bdark = NULL;
   bzero = NULL;
   opix = 0;
   ndark = CCD_locate_buffernum("CALIBRATION-DARK");
   if (ndark > -1) {bdark = (float *)CCD_Frame[ndark].pixels;}
   nzero = CCD_locate_buffernum("CALIBRATION-ZERO");
   if (nzero > -1) {bzero = (float *)CCD_Frame[nzero].pixels;}
   while (i<MAX_CCD_BUFFERS) {
       if (CCD_Frame[i].pixels != NULL) {
          if (strncmp(CCD_Frame[i].name,"FSKY",4) == 0) {
             flats[nflat] = (unsigned int *)CCD_Frame[i].pixels;
             nflat++;
             nx = CCD_Frame[i].xdim;
             ny = CCD_Frame[i].ydim-1;
          }
       }
       i++;
   }
   if (nflat == 0) {return(1);}
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
          ipix = nx*iy + ix;
          opix = nx*iy + ix;
          for (i=0;i<nflat;i++) {
              aflat = flats[i];
              pixeldata[i] = (float) aflat[ipix];
          }
          fmedian = calculate_median(pixeldata,nflat);
          if (nzero > -1) {
             pzero = bzero[opix];
          } else {
             pzero = 0.0;
          }
          if (ndark > -1) {
             image[opix] = fmedian - bdark[opix]*exposure - pzero;
          } else {
             image[opix] = fmedian - pzero;
          }
          mean = mean+fmedian;
          npix++;
      }
      printf("done row %5d\n",iy);
   }
   mean = mean/(float)npix;
   average = 0.0E0;
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
         mean = 0.0E0;
         ipix = nx*iy + ix;
         for (iix=-5;iix<6;iix++) { 
         for (iiy=-5;iiy<6;iiy++) { 
             icol = ix + iix;
             if (icol <0) {icol = icol+5;}
             if (icol >=nx) {icol = icol-5;}
             irow = iy + iiy;
             if (irow <0) {irow = irow+5;}
             if (irow >=ny) {irow = irow-5;}
             mean = mean + image[nx*icol+irow];
         }
         }
         work[ipix] = mean/121.;
         average = average + work[ipix];
      }
   }
   average = average / (float)npix;
   for (ipix=0;ipix<npix;ipix++) {
       image[ipix] = work[ipix]/average;;
   }
   for (i=0;i<nflat;i++) {
       CCD_free_bufferaddr(flats[i]);
   }
   return(0);
}


int calculate_dark(float *image)
{
   unsigned int *adark,*darks[MAX_CCD_BUFFERS];
   float *bzero;
   int   ndark, nzero;
   int   i,npix;
   int ipix, opix, ix, iy, nx, ny;
   float pixeldata[MAX_CCD_BUFFERS];
   float dmedian;


   i = 0;
   npix = 0;
   ndark = 0;
   nx = 0;
   ny = 0;
   bzero = NULL;
   opix = 0;
   nzero = CCD_locate_buffernum("CALIBRATION-ZERO");
   if (nzero > -1) {bzero = (float *)CCD_Frame[nzero].pixels;}
   while (i<MAX_CCD_BUFFERS) {
       if (CCD_Frame[i].pixels != NULL) {
          if (strncmp(CCD_Frame[i].name,"DARK",4) == 0) {
             darks[ndark] = (unsigned int *)CCD_Frame[i].pixels;
             ndark++;
             nx = CCD_Frame[i].xdim;
             ny = CCD_Frame[i].ydim-1;
          }
       }
       i++;
   }
   if (ndark == 0) {return(1);}
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
          ipix = nx*iy + ix;
          opix = nx*iy + ix;
          for (i=0;i<ndark;i++) {
              adark = darks[i];
              pixeldata[i] = (float) adark[ipix];
          }
          dmedian = calculate_median(pixeldata,ndark);
          if (nzero > -1) {
             image[opix] = (dmedian-bzero[opix])/exposure;
          } else {
             image[opix] = dmedian/exposure;
          }
          npix++;
      }
      printf("done row %5d\n",iy);
   }
   for (i=0;i<ndark;i++) {
       CCD_free_bufferaddr(darks[i]);
   }
   return(0);
}





int calculate_zero(float *image)
{
   unsigned int *azero;
   unsigned int *zeros[MAX_CCD_BUFFERS];
   int   nzero;
   int   i,npix;
   int ipix, opix, ix, iy, nx, ny;
   float pixeldata[MAX_CCD_BUFFERS];
   float zmedian;


   i = 0;
   npix = 0;
   nzero = 0;
   nx = 0;
   ny = 0;
   opix = 0;
   while (i<MAX_CCD_BUFFERS) {
       if (CCD_Frame[i].pixels != NULL) {
          if (strncmp(CCD_Frame[i].name,"ZERO",4) == 0) {
             zeros[nzero] = (unsigned int *)CCD_Frame[i].pixels;
             nzero++;
             nx = CCD_Frame[i].xdim;
             ny = CCD_Frame[i].ydim-1;
          }
       }
       i++;
   }
   if (nzero == 0) {return(1);}
   for (iy=0;iy<ny;iy++) {
      for (ix=0;ix<nx;ix++) {
          ipix = nx*iy + ix;
          opix = nx*iy + ix;
          for (i=0;i<nzero;i++) {
              azero = zeros[i];
              pixeldata[i] = (float) azero[ipix];
          }
          zmedian = calculate_median(pixeldata,nzero);
          image[opix] = zmedian;
          npix++;
      }
      printf("done row %5d\n",iy);
   }
   for (i=0;i<nzero;i++) {
       CCD_free_bufferaddr(zeros[i]);
   }
   return(0);
}







int readimage(char *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status;
  /* initialize FITS image parameters */
  float *image;
  int nx, ny;
  int badpix;
  int nkeys, keypos, hdutype, ii, jj;
  char card[FLEN_CARD];   /* standard string lengths defined in fitsioc.h */
  char dummy[80];

    status = 0;
    if ( fits_open_file(&fptr, filename, READONLY, &status) )
    {
         fits_report_error(stderr, status);
         return(status);
    }                                                                           

    status = 0;         /* initialize status before calling fitsio routines */
    badpix = 0; 
    fits_get_hdu_num(fptr, &ii);                                                
    for (; !(fits_movabs_hdu(fptr, ii, &hdutype, &status) ); ii++)
    {
        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status) )
            printerror( status );
 
        for (jj = 1; jj <= nkeys; jj++)  {
            if ( fits_read_record(fptr, jj, card, &status) )
                 printerror( status );
 
            if (strncmp(card,"NAXIS1",6) == 0) {
                sscanf(card,"%s = %d ",dummy,&nx);
            }
            if (strncmp(card,"NAXIS2",6) == 0) {
                sscanf(card,"%s = %d ",dummy,&ny);
            }
        }
    }                                                                       
    status = 0;
    fits_close_file(fptr, &status);
    fits_open_file(&fptr, filename, READONLY, &status);
    image = CCD_new_buffer(buffer_name,  nx, ny, 4);

    fpixel = 1;                               /* first pixel to write      */
    nelements = nx * ny;                      /* number of pixels to write */
 
    /* write the array of unsigned integers to the FITS file */
    if ( fits_read_img(fptr, TFLOAT, fpixel, nelements, 0, image, &badpix, &status) )
        printerror( status );
  
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    return(status);
}                                                                               



int write_buffered_image(Tcl_Interp *interp, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  FLOAT_IMG; /* 16-bit unsigned int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum; 
  PDATA image,rimg;
  int  nx, ny;


  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum > -1) {
    naxes[0] = CCD_Frame[bnum].xdim;
    naxes[1] = CCD_Frame[bnum].ydim; 
    nx = CCD_Frame[bnum].xdim;
    ny = CCD_Frame[bnum].ydim; 
    image = CCD_Frame[bnum].pixels;
    status = 0;         /* initialize status before calling fitsio routines */
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = Ushort_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed int integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
    if (CCD_Frame[bnum].zdim == 2) {
        rimg = CCD_locate_buffer("temp",4,nx,ny,1,1);
        image_i2tof(image,(float *)rimg,nelements);                                           
    } else {
        rimg = image;
    }
 
    /* write the array of unsigned integers to the FITS file */
    if ( fits_write_img(fptr, TFLOAT, fpixel, nelements, rimg, &status) )
        printerror( status );
 
    create_fits_header(interp, fptr);
 
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    CCD_free_buffer("temp");
  } else {
    status = -1;
  }

  return(status);
}                                                                               


int write_buffered_image16(Tcl_Interp *interp, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  USHORT_IMG; /* 16-bit unsigned int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum; 
  PDATA image;
  int  nx, ny;


  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum > -1) {
    naxes[0] = CCD_Frame[bnum].xdim;
    naxes[1] = CCD_Frame[bnum].ydim; 
    nx = CCD_Frame[bnum].xdim;
    ny = CCD_Frame[bnum].ydim; 
    image = CCD_Frame[bnum].pixels;
    status = 0;         /* initialize status before calling fitsio routines */
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = Ushort_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed int integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
 
    /* write the array of unsigned integers to the FITS file */
    if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, image, &status) )
        printerror( status );
 
    create_fits_header(interp, fptr);
 
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    CCD_free_buffer("temp");
  } else {
    status = -1;
  }

  return(status);
}                                                                               

int write_buffered_mef(Tcl_Interp *interp, int nccd, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status, hdtyp;
  /* initialize FITS image parameters */
  int bitpix   =  LONG_IMG; /* 32-bit unsigned pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum, iccd; 
  char tname[32];
  unsigned int *image;
  int  nx, ny;

  naxes[0] = 0;
  naxes[1] = 0;
  naxis = 0;
  status = 0;
  if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
  if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
  create_fits_header(interp, fptr); 

  for (iccd=0;iccd<nccd;iccd++) {
    sprintf(tname,"%s_%d",buffer_name,iccd);
    bnum = CCD_locate_buffernum(tname);
    if (bnum > -1) {
      naxis = 2;
      naxes[0] = CCD_Frame[bnum].xdim;
      naxes[1] = CCD_Frame[bnum].ydim; 
      nx = CCD_Frame[bnum].xdim;
      ny = CCD_Frame[bnum].ydim; 
      image = CCD_Frame[bnum].pixels;
      status = 0;         /* initialize status before calling fitsio routines */
      if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status ); 

      fpixel = 1;                               /* first pixel to write      */
      nelements = naxes[0] * naxes[1];          /* number of pixels to write */

/*      fits_movabs_hdu(fptr,iccd+1,&hdtyp,&status); */

      /* write the array of unsigned integers to the FITS file */
      if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, image, &status) )
        printerror( status );

    } else {
      status = -1;
    } 
  }

  if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
  return(status);
}                                                                               




int write_buffered_mef16(Tcl_Interp *interp, int nccd, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status, hdtyp;
  /* initialize FITS image parameters */
  int bitpix   =  USHORT_IMG; /* 16-bit unsigned pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum, iccd; 
  char tname[32];
  unsigned int *image;
  int  nx, ny;

  naxes[0] = 0;
  naxes[1] = 0;
  naxis = 0;
  status = 0;
  if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
  if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
  create_fits_header(interp, fptr); 

  for (iccd=0;iccd<nccd;iccd++) {
    sprintf(tname,"%s_%d",buffer_name,iccd);
    bnum = CCD_locate_buffernum(tname);
    if (bnum > -1) {
      naxis = 2;
      naxes[0] = CCD_Frame[bnum].xdim;
      naxes[1] = CCD_Frame[bnum].ydim; 
      nx = CCD_Frame[bnum].xdim;
      ny = CCD_Frame[bnum].ydim; 
      image = CCD_Frame[bnum].pixels;
      status = 0;         /* initialize status before calling fitsio routines */
      if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status ); 

      fpixel = 1;                               /* first pixel to write      */
      nelements = naxes[0] * naxes[1];          /* number of pixels to write */

/*      fits_movabs_hdu(fptr,iccd+1,&hdtyp,&status); */

      /* write the array of unsigned integers to the FITS file */
      if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, image, &status) )
        printerror( status );

    } else {
      status = -1;
    } 
  }

  if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
  return(status);
}                                                                               



int write_buffered_image32(Tcl_Interp *interp, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  LONG_IMG; /* 32-bit int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum; 
  PDATA image, rimg;
  int  nx, ny;


  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum > -1) {
    naxes[0] = CCD_Frame[bnum].xdim;
    naxes[1] = CCD_Frame[bnum].ydim; 
    nx = CCD_Frame[bnum].xdim;
    ny = CCD_Frame[bnum].ydim; 
    image = CCD_Frame[bnum].pixels;
    status = 0;         /* initialize status before calling fitsio routines */
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = LONG_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 32 (signed long integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
    rimg = CCD_locate_buffer("temp",4,nx,ny,1,1);
    image_i2toi4(image,(int *)rimg,nelements);                                           
 
    /* write the array of unsigned integers to the FITS file */
    if ( fits_write_img(fptr, TLONG, fpixel, nelements, rimg, &status) )
        printerror( status );
 
    create_fits_header(interp, fptr);
 
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    CCD_free_buffer("temp");
  } else {
    status = -1;
  }

  return(status);
}                                                                               


int write_buffered_image32s(Tcl_Interp *interp, void *buffer_name, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  LONG_IMG; /* 32-bit unsigned int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int bnum; 
  PDATA image,rimg;
  int  nx, ny;


  bnum = CCD_locate_buffernum(buffer_name);
  if (bnum > -1) {
    naxes[0] = CCD_Frame[bnum].xdim;
    naxes[1] = CCD_Frame[bnum].ydim; 
    nx = CCD_Frame[bnum].xdim;
    ny = CCD_Frame[bnum].ydim; 
    image = CCD_Frame[bnum].pixels;
    status = 0;         /* initialize status before calling fitsio routines */
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = Ushort_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed int integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
 
    /* write the array of signed integers to the FITS file */
    if ( fits_write_img(fptr, TLONG, fpixel, nelements, image, &status) )
        printerror( status );
 
    create_fits_header(interp, fptr);
 
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    CCD_free_buffer("temp");
  } else {
    status = -1;
  }

  return(status);
}                                                                               


void create_fits_header(Tcl_Interp *interp, fitsfile *fptr)
{
    char *text;
    int status;
    float fvar;
    int ivar;
    int utcmon, utcyear;
    double utcday;
    double jdobs, mjdobs;
    struct tm *gmt;
    time_t t;

    status = 0;
    fits_write_key(fptr, TSTRING, "CREATOR", "Linux generic CCD control", "Data-taking program", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "site", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "OBSERVAT", text, "Observatory Site", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "name", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "TELESCOP", text, "Telescope Name",&status);
    text = Tcl_GetVar2(interp, "SCOPE", "latitude", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "LATITUDE", text, "[deg] Observatory Latitude", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "longitude", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "LONGITUD", text, "[deg west] Observatory Longtiude", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "camera", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "INSTRUME", text, "Instrument", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "detector", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "DETECTOR", text, "CCD Detector ID", &status);
    text = Tcl_GetVar2(interp, "SCOPE", "instrument", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "INSTID", text, "Instrument ID Code", &status);

    text = Tcl_GetVar2(interp, "SCOPE", "observer", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "OBSERVER", text, "Observer(s)", &status);

    text = Tcl_GetVar2(interp, "SCOPE", "target", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "OBJECT", text, "Target Name",&status);

    text = Tcl_GetVar2(interp, "SCOPE", "imagetype", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "OBSTYPE", text, "Image type code", &status);

    text = Tcl_GetVar2(interp, "SCOPE", "exposure", TCL_GLOBAL_ONLY); 
    sscanf(text,"%f", &fvar);
    fits_write_key_fixflt(fptr, "EXPTIME", fvar, 2, "[sec] Exposure time", &status);

    fits_write_key(fptr, TSTRING, "TIMESYS", "UTC", "Time System is UTC", &status);

    text = Tcl_GetVar2(interp, "SCOPE", "obsdate", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "DATE-OBS", text, "Date of start of observation", &status);

    text = Tcl_GetVar2(interp, "SCOPE", "obstime", TCL_GLOBAL_ONLY); 
    fits_write_key(fptr, TSTRING, "TIME-OBS", text, "Time of start of observation", &status);

    t = time(NULL);
    gmt = gmtime(&t);

    utcday = (double)(gmt->tm_mday) + ((double)(gmt->tm_hour) + (double)(gmt->tm_min)/60.0
                                   + (double)(gmt->tm_sec)/3600.0) / 24.0 ;

    utcmon = gmt->tm_mon + 1;

    utcyear = gmt->tm_year + 1900;

    cal_mjd(utcmon, utcday, utcyear, &mjdobs);

    jdobs = mjdobs + 2415020.0;

    mjdobs = jdobs - 2400000.5;

    fits_write_key_fixdbl( fptr, "MJD-OBS", mjdobs, 6, "MJD at start of obs", &status);
    fits_write_key_fixdbl( fptr, "JD", jdobs, 5, "Julian Date at start of obs", &status);
/*
    hjdobs = hjd(jdobs, raobj, decobj);
    fits_write_key_fixdbl( fptr, "HJD", hjdobs, 5, "Heliocentric Julian Date", &status);
 */

    text = Tcl_GetVar2(interp, "CAMSTATUS", "Temperature", TCL_GLOBAL_ONLY); 
    sscanf(text,"%f", &fvar);
    fits_write_key_fixflt(fptr, "CCDTEMP", fvar, 1, "[C] CCD temperature at readout", &status);

    text = Tcl_GetVar2(interp, "CAMSTATUS", "BinX", TCL_GLOBAL_ONLY); 
    sscanf(text,"%d", &ivar);
    fits_write_key(fptr, TSHORT, "CCDXBIN", &ivar, "Column Binning on detector", &status);
    text = Tcl_GetVar2(interp, "CAMSTATUS", "BinY", TCL_GLOBAL_ONLY); 
    sscanf(text,"%d", &ivar);
    fits_write_key(fptr, TSHORT, "CCDYBIN", &ivar, "Row Binning on detector", &status);

/*    sprintf(text, "[%d,%d:%d,%d]", ap7->skip_x + 1, ap7->skip_x + ap7->image_x,
            ap7->skip_y, ap7->skip_y + ap7->image_y - 1);
    fits_write_key(fptr, TSTRING, "DATASEC", text, "Data Section [xs,xe:ys,ye]", &status);

    sprintf(text,"[%d,%d:%d,%d]",ap7->pixels_x, ap7->pixels_x + ap7->overscan_x,
            ap7->skip_y, ap7->skip_y + ap7->image_y - 1);
    fits_write_key(fptr, TSTRING, "BIASSEC", text, "Bias Section [xs,xe:ys,ye]", &status);
 */



}




int biascorrimage(Tcl_Interp *interp, void *src_buffer, char *filename, int nx, int ny)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  unsigned int *array;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  FLOAT_IMG; /* 16-bit unsigned int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  float rimg[1024*1024];

    naxes[0] = nx-bcols;
    naxes[1] = ny; 
    array = src_buffer;
    status = 0;         /* initialize status before calling fitsio routines */
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = Ushort_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed int integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
    biassubtract(src_buffer,rimg,naxes[0],naxes[1]);                                           
 
    /* write the array of unsigned integers to the FITS file */
    if ( fits_write_img(fptr, TFLOAT, fpixel, nelements, rimg, &status) )
        printerror( status );
 
    create_fits_header(interp, fptr);
 
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
 
    return(status);
}                                                                               

#ifdef LINUX
int shmmapimage(void *buffer_name, int *shmid, size_t *sharedsize)
{
  
 
  long naxes[2];   
  char *sharedmem;
  int idepth;
  int bnum;
  struct shmid_ds sbuf;
  char *img;
  int lsize, lshmid;

    bnum = CCD_locate_buffernum(buffer_name);
    if (CCD_Frame[bnum].shmem == NULL) {
      naxes[0] = CCD_Frame[bnum].xdim;
      naxes[1] = CCD_Frame[bnum].ydim;
      idepth =   CCD_Frame[bnum].zdim;

      lsize = naxes[0]*naxes[1]*idepth;
      lshmid = shmget(atoi(buffer_name), lsize, 0);
      if (lshmid < 0) {
        lshmid = shmget(atoi(buffer_name), lsize, IPC_CREAT|0666);
      }
      if (shmctl(lshmid, IPC_STAT, &sbuf)<0) {
        return(1);
      }
      lsize = sbuf.shm_segsz;
      sharedmem  = (char *) shmat(lshmid, NULL, 0);
      CCD_Frame[bnum].shmsize = lsize;
      CCD_Frame[bnum].shmid = lshmid;
      CCD_Frame[bnum].shmem = sharedmem;
    } else {
      lsize = CCD_Frame[bnum].shmsize;
      lshmid = CCD_Frame[bnum].shmid;
      sharedmem = (char *)CCD_Frame[bnum].shmem;
    }

    img = (char *)CCD_Frame[bnum].pixels;    
    memcpy(sharedmem,img,lsize);
    *shmid = lshmid;
    *sharedsize = lsize;

    return(0);
}
#endif

int calibrateimage(Tcl_Interp *interp, int braw, char *filename)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  unsigned short *src_buffer;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  FLOAT_IMG; /* 16-bit unsigned int pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   
  int ibuf;
  float *rimg;
  float *temp;
  int nx,ny;

 
    naxes[0] = CCD_Frame[braw].xdim-bcols;
    naxes[1] = CCD_Frame[braw].ydim;
    nx = CCD_Frame[braw].xdim-bcols;
    ny = CCD_Frame[braw].ydim;
    rimg = CCD_locate_buffer("calibrated",4,nx,ny,1,1);

    src_buffer = (unsigned short *)CCD_Frame[braw].pixels;

    status = 0;         /* initialize status before calling fitsio routines */

    if (strcmp(filename,"NONE") != 0) {
      if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
      /* write the required keywords for the primary array image.     */
      /* Since bitpix = Ushort_IMG, this will cause cfitsio to create */
      /* a FITS image with BITPIX = 16 (signed int integers) with   */
      /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
      /* FITS uses to store unsigned integers.  Note that the BSCALE  */
      /* and BZERO keywords will be automatically written by cfitsio  */
      /* in this case.                                                */          
 
      if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
    }
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
    biassubtract(src_buffer,rimg,naxes[0],naxes[1]);                                           
    ibuf = CCD_locate_buffernum("CALIBRATION-ZERO");
    if (ibuf > -1) {
        temp = CCD_locate_buffer("CALIBRATION-ZERO",4,nx,ny,1,1);
        subtractzero(rimg,temp,nelements);
    } 
    ibuf = CCD_locate_buffernum("CALIBRATION-DARK");
    if (ibuf > -1) {
        temp = CCD_locate_buffer("CALIBRATION-DARK",4,nx,ny,1,1);
        subtractdark(rimg,temp,nelements);
    } 
    ibuf = CCD_locate_buffernum("CALIBRATION-FLAT");
    if (ibuf > -1) {
        temp = CCD_locate_buffer("CALIBRATION-FLAT",4,nx,ny,1,1);
        divide(rimg,temp,nelements);
    } 
    ibuf = CCD_locate_buffernum("CALIBRATION-FSKY");
    if (ibuf > -1) {
        temp = CCD_locate_buffer("CALIBRATION-FSKY",4,nx,ny,1,1);
        divide(rimg,temp,nelements);
    } 

    if (strcmp(filename,"NONE") != 0) {

      /* write the array of unsigned integers to the FITS file */
      if ( fits_write_img(fptr, TFLOAT, fpixel, nelements, rimg, &status) )
        printerror( status );
 
      create_fits_header(interp, fptr);
 
      if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );

    } 
    return(status);
}                                                                               


int oimage_i2tof(unsigned short *src,float *dest,int n)
{
   int i,t;
   for (i=0;i<n;i++) {
      t = src[i];
      if (src[i] == 32767) {
         dest[i] = 65535.;
      } else {
        dest[i] = (float)t - 32768.0;
        if (dest[i] < 0.0001) {dest[i] = dest[i]+65535;}
      }
   }
   return(0);
}


int image_i2tof(unsigned short *src,float *dest,int n)
{
   int i,t;
   for (i=0;i<n;i++) {
      t = src[i];
      dest[i] = (float)t;
   }
   return(0);
}

int image_i2toi4(unsigned short *src,int *dest,int n)
{
   int i,t;
   for (i=0;i<n;i++) {
      t = src[i];
      dest[i] = (int)t;
   }
   return(0);
}

int image_i4tof(unsigned int *src,float *dest,int n)
{
   unsigned int i,t;
   for (i=0;i<n;i++) {
      t = src[i];
      dest[i] = (float)t;
   }
   return(0);
}



void subtractzero(float *src, float *op, int n) 
{
   int i;
   for (i=0;i<n;i++) {
      src[i] = src[i] - op[i];
   }
}

void subtractdark(float *src, float *op, int n) 
{
   int i;
   for (i=0;i<n;i++) {
      src[i] = src[i] - op[i]*exposure;
   }
}


void divide(float *src, float *op, int n) 
{
   int i;
   for (i=0;i<n;i++) {
      src[i] = src[i] / op[i];
   }
}





int biassubtract(unsigned short *src,float *dest, int nx, int ny)
{
   double biases[MAX_COLS];
   double abiases;
   int ix,iy, oix;
   int ipix, opix;
   float amin,datum;

   if (bcols == 0) {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        amin = 65536.0;
        for (ix=1;ix<=nx;ix++) {
            ipix = nx*iy + ix;
            datum = (float)src[ipix];
            if (datum < amin) {biases[iy] = datum; amin=datum;}
        }
     }
   } else {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        for (ix=bias_start;ix<=bias_end;ix++) {
            ipix = (nx+bcols)*iy + ix-1;
            biases[iy] = biases[iy] + (float)src[ipix];
        }
        biases[iy] =  biases[iy] / (float)bcols;
     }
   }
   abiases = 0.0;   
   for (iy=0;iy<ny;iy++) {
     abiases = abiases + biases[iy];
   }
   abiases = abiases/(float)ny;
   for (iy=0;iy<ny;iy++) {
      oix = 0;
      for (ix=0;ix<nx+bcols;ix++) {
        if (ix < bias_start || ix > bias_end) {
          ipix = (nx+bcols)*iy + ix;
          opix = nx*iy + oix;
          if (src[ipix] == 32767) {
             dest[opix] = 65535.;
          } else {
             dest[opix] = (float)src[ipix] - abiases;
/*             if (dest[opix] < 0.0001) {dest[opix] = dest[opix]+65535;} */
          }
          oix++;
        }
      }
   }
   return(0);
}

 
int ibiassubtract(unsigned int *src,unsigned int *dest, int nx, int ny)
{
   double biases[MAX_COLS];
   double abiases;
   int ix,iy, oix;
   int ipix, opix;
   float amin, datum;

   if (bcols == 0) {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        amin = 65536.0;
        for (ix=1;ix<=nx;ix++) {
            ipix = nx*iy + ix;
            datum = (float)src[ipix];
            if (datum < amin) {biases[iy] = datum; amin=datum;}
        }
     }
   } else {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        for (ix=bias_start;ix<=bias_end;ix++) {
            ipix = (nx+bcols)*iy + ix-1;
            biases[iy] = biases[iy] + (float)src[ipix];
        }
        biases[iy] =  biases[iy] / (float)bcols;
     }
   }
   abiases = 0.0;   
   for (iy=0;iy<ny;iy++) {
     abiases = abiases + biases[iy];
   }
   abiases = abiases/(float)ny;
  
   for (iy=0;iy<ny;iy++) {
      oix = 0;
      for (ix=0;ix<nx+bcols;ix++) {
        if (ix < bias_start || ix > bias_end) {
          ipix = (nx+bcols)*iy + ix;
          opix = nx*iy + oix;
          if (src[ipix] == 32767) {
             dest[opix] = 65535.;
          } else {
            dest[opix] = src[ipix] - (int)abiases;
/*            if (dest[opix] < 0.0001) {dest[opix] = dest[opix]+65535;} */
          }
          oix++;
        }
      }
   }
   return(0);
}

 
 
int printerror( int status)
{
    /*****************************************************/
    /* Print out cfitsio error messages and exit program */
    /*****************************************************/
 
 
    if (status)
    {
       fits_report_error(stderr, status); /* print error report */
 
       exit( status );    /* terminate the program, returning error status */
    }
    return 0;
}



#ifdef WITH_CDL

/*----------------------------------------------------------------------
 * disp_init - initializion of the display library
 *
 * Opens a connection to the image server (ximtool) and prepare the
 * destination frame for display.
 *---------------------------------------------------------------------*/
int disp_init (int w, int h, int fbconfig, int frame)
{
  int fb_w, fb_h, nf;
  int frame_width, frame_height;

  /* initialize some local variables */
  frame_width = w;
  frame_height = h;

   
  /* open the connection to img server */
  if (!(cdl = cdl_open ((char *) getenv ("IMTDEV")))) {
    fprintf (stderr, "ERROR: can't open connection to image server\n");
    return (1);
  }

  /* Now select a frame buffer large enough for the image. The 
   * fbconfig number is passed in the WCS packet, but the display
   * calls below will compute the correct WCS for the image and
   * transmit that prior to display, all we're doing here is
   * setting up the FB to be used.
   */
  if (fbconfig == 0)
    cdl_selectFB (cdl, frame_width, frame_height, &fbconfig, &fb_w,
		  &fb_h, &nf, 1);
  else
    cdl_lookupFBSize (cdl, fbconfig, &fb_w, &fb_h, &nf);

  /* For the WCS we assume a simple linear transform where the image is
   * Y-flipped, the (x,y) translation is computed so it is correct
   * for an frame buffer >= than the image size.  The Z-transform is
   * fixed since we're using a test pattern with known values.
   */
  cdl_setWCS (cdl, "Apogee CCD", "", 1., 0., 0., -1.,
	      (float) (frame_width / 2) - (fb_w / 2) + 1,	/* X translation */
	      (float) (fb_h / 2) + (frame_height / 2),	/* Y translation */
	      0.0, 250.0, CDL_LINEAR);	/* Z transform   */

  /* Select and clear the initial frame prior to display. */
  cdl_setFrame (cdl, frame);
  cdl_clearFrame (cdl);

  /* Select guiding box line style and width */
  cdl_setLineWidth (cdl, 0);
  cdl_setLineStyle (cdl, L_SOLID);

  return (0);
}


int tcl_show_image(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{

  char buffer_name[32];
  int frame_width, frame_height;
  int buffernum;
  unsigned char *bframe;
  int nx,ny;

  /* Check number of arguments provided and return an error if necessary */
  if (argc < 2) {
     Tcl_AppendResult(interp, "wrong # args: should be \"",argv[0]," name\"", (char *)NULL);
     return TCL_ERROR;
  }


  /* Retrieve arguments */
  strcpy(buffer_name,argv[1]);                                  
  buffernum = CCD_locate_buffernum(buffer_name);
  frame_width = CCD_Frame[buffernum].xdim-bcols;
  frame_height = CCD_Frame[buffernum].ydim;
  nx = CCD_Frame[buffernum].xdim-bcols;
  ny = CCD_Frame[buffernum].ydim;
  bframe = CCD_locate_buffer("ximtool",1,frame_width,frame_height,1,1); 
  converttobyte((float *)CCD_Frame[buffernum].pixels,bframe, frame_width*frame_height);
  if (disp_init(frame_width,frame_height, 0, 1) == 0) {
    cdl_writeSubRaster (cdl, 0, 0, frame_width, frame_height,bframe);
    cdl_close(cdl);
    return TCL_OK;
  } else {
    Tcl_AppendResult(interp, "Unable to connect to display server", (char *)NULL);
    return TCL_ERROR;
  }
}


#endif

void converttobyte(float *src, unsigned char *dest,int n)
{
    int i;
    int min, max;

    min=99999;
    max=-99999;
    for (i=0;i<n;i++) {
        if (src[i] > max) {max = (int)src[i];}
        if (src[i] < min) {min = (int)src[i];}
    }
    for (i=0;i<n;i++) {
        dest[i] = (unsigned char)((src[i]-min)*255/(max-min+1));
    }
}




                                  








