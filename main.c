#include <getopt.h>
#include "s3d.h"
#include "ter.h"
#include "zon.h"
#include "draw.h"

float start_x=0,start_y=0,start_z=0;
unsigned short height=800,width=600;

int screen_shot=0;
char *zone;

int main(int argc, char **argv) {
  char *eqg_name, *zon_name;
  uchar *ter_buf, *zon_buf;
  ZoneMesh mesh;
  s3d_object s3d;
  ter_object ter;
  zon_object zon;
  int ter_len;
  extern int optind;
  extern char *optarg;
  int arg,arg_error=0;
  char *eq_dir=0;
  FILE *fp;

  while((arg=getopt(argc,argv,"d:x:y:z:sh:w:")) != EOF) {
    switch(arg) {
      case 'd':
        eq_dir=optarg;
	break;
      case 'h':
	height=atoi(optarg);
	break;
      case 'w':
	width=atoi(optarg);
	break;
      case 'x':
	sscanf(optarg,"%f",&start_x);
	break;
      case 'y':
	sscanf(optarg,"%f",&start_y);
	break;
      case 'z':
	sscanf(optarg,"%f",&start_z);
	break;
      case 's':
#ifdef HAVE_GD
        screen_shot=1;
#else
	fprintf(stderr,"Screen shot mode only valid with GD support.\n");
	arg_error=1;
#endif
	break;
      default:
        arg_error=1;
	break;
    }
  }

  if(argc-optind < 1 || arg_error) {
    fprintf(stderr,"Usage: freaku [options] (zone/ter filename) [s3d/eqg name]\n");
    fprintf(stderr,"\t-d dir    Everquest is in dir\n");
    fprintf(stderr,"\t-x x      Start at x when rendering\n");
    fprintf(stderr,"\t-y y      Start at y when rendering\n");
    fprintf(stderr,"\t-z z      Start at z when rendering\n");
    fprintf(stderr,"\tYou must either pass a zone name, or a .ter filename and an s3d/eqg filename.\n");
    fprintf(stderr,"Usage: freaku [options] (zone/ter filename) [s3d/eqg name]\n");
    return -1;
  }

  if(argc-optind == 1) {
    zone=argv[optind];

    if(eq_dir) {
      eqg_name = (char *) malloc(strlen(eq_dir) + 1 + strlen(zone) + 5);
      sprintf(eqg_name, "%s/%s.eqg", eq_dir, zone);
    }
    else {
      eqg_name = (char *) malloc(strlen(zone) + 5);
      sprintf(eqg_name, "%s.eqg", zone);
    }
    S3D_Init(&s3d, fopen(eqg_name, "rb"));
    free(eqg_name);

    zon_name = (char *) malloc(strlen(zone) + 5);
    sprintf(zon_name, "%s.zon", zone);
    S3D_GetFile(&s3d, zon_name, &zon_buf);
    free(zon_name);

    ZON_init(&zon, zon_buf);
    free(zon_buf);

    ter_len = S3D_GetFile(&s3d, zon.ter_name, &ter_buf);
  }
  else {
    S3D_Init(&s3d, fopen(argv[optind + 1], "rb"));

    fp = fopen(argv[optind], "rb");
    fseek(fp, 0, 2);
    ter_len = ftell(fp) + 1;
    fseek(fp, 0, 0);
    ter_buf = (uchar *) malloc(ter_len);
    fread(ter_buf, ter_len, 1, fp);
  }

  TER_Init(&ter, ter_buf, ter_len, &s3d);
  free(ter_buf);

  Draw(&(ter.mesh), &s3d);

  return 0;
}
