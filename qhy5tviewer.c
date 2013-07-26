/*
 * qhy5tviewer
 * 
 * Simple viewer and grabber for QHY5T camera
 * 
 * Copyright 2013 Joaquin Bogado <joaquinbogado at gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

//to build:
//gcc -o qhy5tviewer qhy5t.c qhy5tviewer.c -lSDL -lpthread -lusb -lcfitsio

#include <SDL/SDL.h>
#include "qhy5t.h"
#include <getopt.h>
#include <fitsio.h>

//pix position relative in source image [rgrg...gbgb]
#define SRCTL(ptr) (*(ptr-w-1))
#define SRCTR(ptr) (*(ptr-w+1))
#define SRCBL(ptr) (*(ptr+w-1))
#define SRCBR(ptr) (*(ptr+w+1))
#define SRCL(ptr) (*(ptr-1))
#define SRCR(ptr) (*(ptr+1))
#define SRCT(ptr) (*(ptr-w))
#define SRCB(ptr) (*(ptr+w))
//pix position relative in target [rgbrgbrgbrgb] color: 0=r, 1=g, 2=blue
#define TGTTL(ptr,color) (*(ptr-w*3-3+color))
#define TGTTR(ptr,color) (*(ptr-w*3+3+color))
#define TGTBL(ptr,color) (*(ptr+w*3-3+color))
#define TGTBR(ptr,color) (*(ptr+w*3+3+color))
#define TGTL(ptr,color) (*(ptr-3+color))
#define TGTR(ptr,color) (*(ptr+3+color))
#define TGTT(ptr,color) (*(ptr-w*3+color))
#define TGTB(ptr,color) (*(ptr+w*3+color))

#define TGR(ptr) (*(ptr))
#define TGG(ptr) (*(ptr+1))
#define TGB(ptr) (*(ptr+2))

#define max(a,b) (a)>(b)?(a):(b)
#define min(a,b) (a)<(b)?(a):(b)

enum color {red, green1, green2, blue};

void * debayer_data_jwack(void * data, void * dest, qhy5t_driver * qhy5t){
	uint16_t i, j, h, w;
	uint8_t * tgt;
	uint8_t * src;
	
	unsigned int tr,tg,tb;
	w = qhy5t->width;
	h = qhy5t->height;
	tgt = (uint8_t *)dest;
	src = (uint8_t *)data;
	enum color c = red;
	
	tgt = memset(tgt, 1,w*h*3);
	//lineal four neighbour interpolation 
	*tgt = 255;
	src += w;
	tgt += w*3;
	for (j=1; j < h; j++){
			*tgt=255;
		for (i=1; i <= w; i++){
			if (i%2 == 0 && j%2 ==0){// red pixel
				tr = *src;
				tb = 0.25*(SRCTL(src) + SRCTR(src) + SRCBL(src) + SRCBR(src));
				tg = 0.25*(SRCT(src) + SRCL(src) + SRCB(src) + SRCR(src));
			}
			if (i%2 == 1 && j%2 ==0){//green1 pixel
				tg = *src;
				tb = 0.50*(SRCT(src) + SRCB(src));
				tr = 0.50*(SRCL(src) + SRCR(src));
			}
			if (i%2 == 0 && j%2 ==1){//green2 pixel
				tg = *src;
				tr = 0.50*(SRCT(src) + SRCB(src));
				tb = 0.50*(SRCL(src) + SRCR(src));
			}
			if (i%2 == 1 && j%2 ==1){// blue pixel
				tb = *src;
				tr = 0.25*(SRCTL(src) + SRCTR(src) + SRCBL(src) + SRCBR(src));
				tg = 0.25*(SRCT(src) + SRCL(src) + SRCB(src) + SRCR(src));
			}
			src++;
			*tgt++ = (uint8_t)tb;
			*tgt++ = (uint8_t)tg;
			*tgt++ = (uint8_t)tr;
		}
		
	}
	return dest;
}
void write_ppm6(void * data, int width, int height, char *basename){
	FILE *fp;
	char * filename = malloc(strlen(basename) + 15);
	sprintf(filename, "%s%s","_dby_", basename );
	fp = fopen(filename, "w");
	if (fp == NULL) {printf("%s\n", filename);return;}
	fprintf(fp, "P6\n"); 
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "255\n");
	fwrite(data, width*height*3, 1, fp);
	free(data);
	fclose(fp);
}

void printerror( int status)
{
	if (status)
	{
		fits_report_error(stderr, status);
		exit(status); /* terminate the program, returning error status *///HAY QUE VE ESTO!!!
	}
	return;
}

void write_fits(void * array, int width, int height, char *fname )
{
	fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
	int status;
	long  fpixel, nelements;
	char filename[256] = "!";	// ! for deleting existing file and create new
	strcat(filename,fname);
	int bitpix   =  BYTE_IMG; /* 8-bit unsigned short pixel values       */
	long naxis    =   3;  /* 2-dimensional image                            */
	long naxes[3];   	/* image is 300 pixels wide by 200 rows */
	naxes[0]=width;
	naxes[1]=height;
	naxes[2]=3;
	status = 0;         /* initialize status before calling fitsio routines */

	if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
	printerror( status );           /* call printerror if error occurs */
	/* write the required keywords for the primary array image.     */
	/* Since bitpix = USHORT_IMG, this will cause cfitsio to create */
	/* a FITS image with BITPIX = 16 (signed short integers) with   */
	/* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
	/* FITS uses to store unsigned integers.  Note that the BSCALE  */
	/* and BZERO keywords will be automatically written by cfitsio  */
	/* in this case.                                                */
	if (fits_create_img(fptr, bitpix, naxis, naxes, &status)) printerror( status );
	fpixel = 1;                               /* first pixel to write      */
	nelements = naxes[0] * naxes[1];          /* number of pixels to write *///y ACA??? cuantos pixels escribo?
	/* write the array of unsigned integers to the FITS file */
	if (fits_write_img(fptr, TBYTE, fpixel, nelements, array, &status)) printerror( status );
	/* write another optional keyword to the header */
	/* Note that the ADDRESS of the value is passed in the routine */
	if (fits_update_key(fptr, TSTRING, "SOFTWARE", "qhy5tviewer","", &status)) printerror( status );
	if (fits_update_key(fptr, TSTRING, "CAMERA", "QHY5T","", &status)) printerror( status );
	//if ( fits_update_key(fptr, TLONG,   "GAIN", &gain,"", &status) )     printerror( status );
	//if ( fits_update_key(fptr, TLONG,   "EXPOSURE", &etime, "Total Exposure Time", &status) ) printerror( status );
	if (fits_close_file(fptr, &status)) printerror( status );  /* close the file */
	//return (status == 0);
}

void show_help(char * progname){
	printf("%s [options]\n", progname);
	printf("\t\t-x/--width <width>                specify width (default: 800)\n");
	printf("\t\t-y/--height <height>              specify height (default: 600)\n");
	printf("\t\t-g/--gain <gain>                  specify gain in permilles (default 10%%)\n");
	printf("\t\t-b/--binning <bin>                specify the binning mode (2x2 or default: 1x1)\n");
	printf("\t\t-t/--exposure <exposure>          specify exposure in msec (default: 100)\n");
	printf("\t\t-o/--file <filename>              specify filename to write to (if none, SDL output only)\n");
	printf("\t\t-c/--count <count>                specify how many sequential images to take. If -c isn't especified,\n");
	printf("\t\t                                  then the output file will be exactly <filename> and will be a fits file. \n"); 
	printf("\t\t                                  (This is for QHYImager compatibility). Else, will be <filename>0000x.<fmt>\n");
	printf("\t\t-m/--format <fmt>                 specify the file type (default: ppm, else fits file will be created.)\n");
	printf("\t\t-d/--debug                        enable debugging\n");
	printf("\t\t-f//--fits                        output to FITS file (default PPM)\n");
	printf("\t\t-h//--help                         show this message\n");
	exit(0);
}

void qhy5tviewer_exit(SDL_Surface * hello, qhy5t_driver * qhy5t){
	SDL_FreeSurface(hello);
	SDL_Quit();
	qhy5t_stopcapture(qhy5t);
	qhy5t_close(qhy5t);
	exit(1);
}

int main (int argc, char *argv[]){
	int width = 800;
	int height = 600;
	int offw = (2048 - width) / 2;
	int offh = (1536 - height) / 2;
	int count = 0;
	int bin = 1; //binmode default 1x1
	int bpp = 1;
	int hblank = 142;
	unsigned int vblank = 25;
	unsigned int gain = 100;
	unsigned int etime = 100;
	char fmt[10] = "ppm";
	char basename[256] = "image";
	char imagename[256];
	int debug=0;
	qhy5t_driver *qhy5t;
	int crossair=0;
	
	void (*writefunction)(void *, int, int, char *) = write_ppm;
	
	int write=0;
	/*Parsing main arguments*/
	struct option long_options[] = {
		{"exposure" ,required_argument, NULL, 't'},
		{"gain", required_argument, NULL, 'g'},
		{"binning", required_argument, NULL, 'b'},
		{"vblank", required_argument, NULL, 'k'},
		{"width", required_argument, NULL, 'x'},
		{"height", required_argument, NULL, 'y'},
		{"debug", required_argument, NULL, 'd'},
		{"file", required_argument, NULL, 'o'},
		{"count", required_argument, NULL, 'c'},
		{"format", required_argument, NULL, 'm'},
		{"help", no_argument , NULL, 'h'},
		{"fits", no_argument , NULL, 'f'},
		{"crossair", no_argument , NULL, 'X'},
		{0, 0, 0, 0}
	};

	while (1) {
		char c;
		c = getopt_long (argc, argv, 
                     "t:g:b:k:x:y:do:c:m:h:fX:",
                     long_options, NULL);
		if(c == EOF)
			break;
		switch(c) {
		case 't':
			etime = strtol(optarg, NULL, 0);
			break;
		case 'g':
			gain = strtol(optarg, NULL, 0);
			//Gain calculations for QHY5T from the datasheet
			//gain value must be a per mil
			gain = qhy5t_set_gain(gain);
			break;
		case 'b':
			bin = atoi(optarg);
			break;
		case 'k':
			vblank = atoi(optarg);
			break;
		case 'x':
			width = strtol(optarg, NULL, 0);
			offw = (2048 - width) / 2;
			break;
		case 'y':
			height = strtol(optarg, NULL, 0);
			offh = (1536 - height) / 2;
			break;
		case 'o':
			strncpy(basename, optarg, 255);
			write=0;
			break;
		case 'd':
			debug = 1;
			break;
		case 'c':
			count = strtol(optarg, NULL, 0);
			if ((count%2)==0) count--;
			break;
		case 'm':
			strncpy(fmt, optarg, 10);
			break;
		case 'h':
			show_help(argv[0]);
			break;
		case 'f':
			writefunction = write_fits;
			strncpy(fmt, "fits",4);
			break;
		case 'X':
			crossair=1;
			break;
		}
	}
	
	/*End parsing arguments*/
	
	if(width > 2048 || width < 1) {
		printf("Width must be between 1 and 1280\n");
		exit(1);
	}
	
	if(height > 1536 || height < 1) {
		printf("Height must be between 1 and 1536\n");
		exit(1);
	}
	//SDL artifacts initialization...
	SDL_Surface* hello = NULL;
	SDL_Surface* screen = NULL;
	SDL_Event event;
	int quit=0;
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode( width, height, 24, SDL_SWSURFACE |SDL_ANYFORMAT);
	if(!screen){
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		exit(-1);
	}
	hello = SDL_SetVideoMode( width, height, 24, SDL_SWSURFACE);
	if(!hello){
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		exit(-1);
	}
	//SDL_SetPalette(hello, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
	
	//Camera and capture settings and initialization...
	if((qhy5t = qhy5t_open())== NULL) {
		printf("Could not open the QHY5T device\n");
		exit(-1);
	}
	qhy5t_set_params(qhy5t, width, height, offw, offh, bin, gain, gain, gain, gain, vblank, hblank, bpp, etime);
	qhy5t_reconnect(qhy5t);
	qhy5t_programcamera(qhy5t, 0);
	qhy5t_start_exposure(qhy5t);
	void * data=NULL;
	//debayerized data pointer
	void * debdata = calloc(qhy5t->framesize*3,1);
	while (!quit){
		//process SDL events
		while( SDL_PollEvent( &event ) ){
			/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
			switch( event.type ){
			case SDL_KEYDOWN:
			//case SDL_KEYUP:
				//process online commands
				switch (event.key.keysym.sym){
				//toggle grabe frame
				case SDLK_s:
					write = !write;
					break;
				//quit viewer
				case SDLK_q:
					quit = 1;
					break;
				}
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			default:
				break;
			}
		}
		
		//display an image in the surface.
		data = qhy5t_read_exposure(qhy5t);
		if (data == NULL){
			qhy5tviewer_exit(hello, qhy5t);
			return 1;
		}
		SDL_LockSurface(hello);
		debdata = debayer_data_jwack(data, debdata, qhy5t);
		hello->pixels = debdata;
		SDL_UnlockSurface(hello);
		
		
		if (write){
			sprintf(imagename, "%s%05d.%s", basename, count, fmt);
			printf("Capturing %s\n", imagename);
			writefunction(data, qhy5t->width, qhy5t->height, imagename);
			count++;
		}
		if (SDL_BlitSurface(hello, NULL, screen, NULL)){
			printf("%s\n", SDL_GetError());
			SDL_UnlockSurface(hello);
			SDL_UnlockSurface(screen);
			SDL_BlitSurface(hello, NULL, screen, NULL);
		}
		SDL_Flip(screen);
	}
	qhy5tviewer_exit(hello, qhy5t);
	return 0;
}
