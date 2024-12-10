//project.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm_io.h"
#include "image_manip.h"

// Return (exit) codes
#define RC_SUCCESS            0
#define RC_MISSING_FILENAME   1
#define RC_OPEN_FAILED        2
#define RC_INVALID_PPM        3
#define RC_INVALID_OPERATION  4
#define RC_INVALID_OP_ARGS    5
#define RC_OP_ARGS_RANGE_ERR  6
#define RC_WRITE_FAILED       7
#define RC_UNSPECIFIED_ERR    8

void print_usage();
int handle_operations(char* input[], int argc);
int handle_grayscale(char* input[], int argc, Image im);
int handle_blend(char* input[], int argc, Image im);
int handle_rotate(char* input[], int argc, Image im);
int handle_blur(char* input[], int argc, Image im);
int handle_pointilism(char* input[], int argc, Image im);
int handle_saturate(char* input[], int argc, Image im);

int main (int argc, char* argv[]) {
  if (argc < 4) {
    printf("Please enter an image.ppm file\n");
    return RC_MISSING_FILENAME; 
  }
   
  return handle_operations(argv, argc);
}



void print_usage() {
  printf("USAGE: ./project <input-image> <output-image> <command-name> <command-args>\n");
  printf("SUPPORTED COMMANDS:\n");
  printf("   grayscale\n" );
  printf("   blend <target image> <alpha value>\n" );
  printf("   rotate-ccw\n" );
  printf("   pointilism\n" );
  printf("   blur <sigma>\n" );
  printf("   saturate <scale>\n" );
}

/*
function that handles all the input from the main function
*/

int handle_operations(char* input[], int argc) {
  FILE *image_name = fopen(input[1], "r");
  if (image_name == NULL) {
    fprintf(stderr, "Failed to open input file.\n");
    fclose(image_name);
    return RC_OPEN_FAILED;
  }
  
  //check to see if memory failed
  Image im = read_ppm(image_name);
  if (im.data == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    fclose(image_name);
    return RC_INVALID_PPM;
  }
  if(im.rows <= 0 || im.cols <= 0) {
    fprintf(stderr, "Issues with the image file\n");
    fclose(image_name);
    return RC_UNSPECIFIED_ERR;
  }

  fclose(image_name);
  
  //runs if command is grayscale
  if(strcmp(input[3], "grayscale") == 0) {
    handle_grayscale(input, argc, im);

    //runs if command is blend
  } else if(strcmp(input[3], "blend") == 0) {
      handle_blend(input, argc, im);

    //runs if command is rotate
  } else if(strcmp(input[3], "rotate-ccw") == 0) {
      handle_rotate(input, argc, im);

    //runs if command is pointilism
  } else if(strcmp(input[3], "pointilism") == 0) {
      handle_pointilism(input, argc, im);

    //runs if command is blur
  } else if(strcmp(input[3], "blur") == 0) {
      handle_blur(input, argc, im);

    //runs if command is saturate
  } else if(strcmp(input[3], "saturate") == 0) {
      handle_saturate(input, argc, im);

  } else {
    //unupported command
    fprintf(stderr, "Unsupported image processing operations\n");
    free_image(&im);
    return RC_INVALID_OPERATION;
  }

  return RC_SUCCESS;
}

int handle_grayscale(char* input[], int argc, Image im) {
  //checks for right number of arguments
    if (argc != 4) {
      fprintf(stderr, "Incorrect number of arguments for the specified operation");
	    free_image(&im);
	    return RC_INVALID_OP_ARGS;
    }

    //allocates output image
    FILE *output_file = fopen(input[2], "w");
    if (output_file == NULL) {
      fprintf(stderr, "Output file I/O error\n");
      free_image(&im);
      return RC_WRITE_FAILED;
    }

    Image out = grayscale(im);
    int chk = write_ppm(output_file, out);

    free_image(&im);
    free_image(&out);
    fclose(output_file);
    
    return chk;
}

int handle_blend(char* input[], int argc, Image im) {
  //checks for right number of arguments
      if (argc != 6) {
        fprintf(stderr, "Incorrect number of arguments for the specified operation");
	      free_image(&im);
	      return RC_INVALID_OP_ARGS;
      }
      //allocates output image
      FILE *second_image = fopen(input[2], "r");
      if (second_image == NULL) {
        fprintf(stderr, "Input file I/O error");
	      free_image(&im);
	      return RC_OPEN_FAILED;
      }
      
      Image im2 = read_ppm(second_image);
      if (im2.data == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(second_image);
        return RC_INVALID_PPM;
      }

      if(im2.rows <= 0 || im2.cols <= 0) {
        fprintf(stderr, "Issues with the image file\n");
        fclose(second_image);
        return RC_UNSPECIFIED_ERR;
      }

      //allocates output image
      FILE *output_file = fopen(input[4], "w");
      if (output_file == NULL) {
        fprintf(stderr, "Output file I/O error\n");
	      fclose(second_image);
	      free_image(&im2);
	      free_image(&im);
	      return RC_WRITE_FAILED;
      }

      //checks if parameter is in bounds
      double alpha = strtod(input[5], NULL);
      if (alpha < 0 || alpha > 1) {
        fprintf(stderr, "Parameter not in bounds\n");
	      fclose(second_image);
	      fclose(output_file);
	      free_image(&im2);
	      free_image(&im);
	      return RC_OP_ARGS_RANGE_ERR;
      }

      //preform edit
      Image out = blend(im, im2, alpha);
      int chk = write_ppm(output_file, out);

      fclose(second_image);
      fclose(output_file);
      free_image(&im);
      free_image(&im2);
      free_image(&out);
    
      return chk;
}


int handle_rotate(char* input[], int argc, Image im) {
  //checks for right number of arguments
      if (argc != 4) {
        fprintf(stderr, "Incorrect number of arguments for the specified operation");
	      free_image(&im);
	      return RC_INVALID_OP_ARGS;
      }

      //allocates output image
      FILE *output_file = fopen(input[2], "w");
      if (output_file == NULL) {
        fprintf(stderr, "Output file I/O error\n");
	      free_image(&im);
	      return RC_WRITE_FAILED;
      }

      //preform edit
      Image out = rotate_ccw(im);
      int chk = write_ppm(output_file, out);

      fclose(output_file);
      free_image(&im);
      free_image(&out);
    
      return chk;
}

int handle_blur(char* input[], int argc, Image im) {
  //checks for right number of arguments
      if (argc != 5) {
        fprintf(stderr, "Incorrect number of arguments for the specified operation\n");
	      free_image(&im);
	      return RC_INVALID_OP_ARGS;
      }
    
      //allocates output image
      FILE *output_file = fopen(input[2], "w");
      if (output_file == NULL) {
        fprintf(stderr, "Output file I/O error\n");
	      free_image(&im);
	      return RC_WRITE_FAILED;
      }

      //checks if parameter is in bounds
      double sigma = strtod(input[4], NULL);
      if (sigma < 0.1) {
        fprintf(stderr, "Parameter not in bounds\n");
	      free_image(&im);
	      fclose(output_file);
	      return RC_OP_ARGS_RANGE_ERR;
      }

      //preform edit
      Image out = blur(im, sigma);
      if(out.data == NULL) {
        fprintf(stderr, "Failed to allocate memory for Gauss Array\n");
        free_image(&out);
        free_image(&im);
        return RC_UNSPECIFIED_ERR;
      }
      int chk = write_ppm(output_file, out);

      free_image(&out);
      free_image(&im);
      fclose(output_file);
      
      return chk;
}

int handle_pointilism(char* input[], int argc, Image im) {
  //checks for right number of arguments
      if (argc != 4) {
        fprintf(stderr, "Incorrect number of arguments for the specified operation");
	      free_image(&im);
	      return RC_INVALID_OP_ARGS;
      }
    
      //allocates output image
      FILE *output_file = fopen(input[2], "w");    
      if (output_file == NULL) {
        fprintf(stderr, "Output file I/O error\n");
	      free_image(&im);
	      return RC_WRITE_FAILED;
      }   

      //preform edit
      Image out = pointilism(im);
      int chk = write_ppm(output_file, out);

      free_image(&im);
      free_image(&out);
      fclose(output_file);
      
      return chk;
}

int handle_saturate(char* input[], int argc, Image im) {
  //checks for right number of arguments
      if (argc != 5) {
        fprintf(stderr, "Incorrect number of arguments for the specified operation");
	      free_image(&im);
	      return RC_INVALID_OP_ARGS;
      }

      //allocates output image
      FILE *output_file = fopen(input[2], "w");
      if (output_file == NULL) {
        fprintf(stderr, "Output file I/O error\n");
	      free_image(&im);
	      return RC_WRITE_FAILED;
      }

      //checks if parameter is in bounds
      double scale = strtod(input[4], NULL);
      if (scale < 0) {
        fprintf(stderr, "Parameter not in bounds\n");
	      free_image(&im);
	      fclose(output_file);
	      return RC_OP_ARGS_RANGE_ERR;
      }

      //preform edit
      Image out = saturate(im, scale);
      int chk = write_ppm(output_file, out);

      free_image(&out);
      free_image(&im);
      fclose(output_file);
      
      return chk;
}

