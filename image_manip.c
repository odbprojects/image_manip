#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "image_manip.h"
#include "ppm_io.h"

double* gauss_matrix(double sigma);
Image apply_filter(double* g_filter, Image im1, Image im2, double sigma);
Image handleCase1(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows);
Image handleCase2(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows);
Image handleCase3(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows);
Image handleCase4(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows);

Image grayscale(const Image in) {
    Image gray_image = make_image(in.rows, in.cols);

    int num_pix = in.rows * in.cols;
    
    //iterate through all pixels
    for (int i = 0; i < num_pix; i++) {
      int r = (in.data[i]).r;
      int g = (in.data[i]).g;
      int b = (in.data[i]).b;

      //calculate grayscale value
      int gray_int = 0.3 * r + 0.59 * g + 0.11 * b;
      unsigned char gray = (unsigned char)gray_int;

      //assign grayscale value
      (gray_image.data[i]).r = gray;
      (gray_image.data[i]).g =	gray;
      (gray_image.data[i]).b =	gray;
    }
      
    return gray_image;
}

Image blend(const Image in1, const Image in2, double alpha) {
    Image blend_image = make_image(fmax(in1.rows, in2.rows), fmax(in1.cols, in2.cols));

    //calculate image parameters
    int min_rows = fmin(in1.rows, in2.rows);
    int max_rows = fmax(in1.rows, in2.rows);
    int min_cols = fmin(in1.cols, in2.cols);
    int max_cols = fmax(in1.cols, in2.cols);

    int num_pix = max_rows * max_cols; 
    
    //initialize new image to black
    for (int i = 0; i < num_pix; i++) {
      blend_image.data[i].r = 0;
      blend_image.data[i].g = 0;
      blend_image.data[i].b = 0;
    }
    
    //overlapped quadrant
    for (int i = 0; i < min_rows; i++) {
      for (int j = 0; j < min_cols; j++) {
	      double r = ((double)(in1.data[i * in1.cols + j].r) * alpha) + ((double)(in2.data[i * in2.cols + j].r) * (1 - alpha));
        
        
        blend_image.data[i * max_cols + j].r = (int)r; 
        
        double g = (double)(in1.data[i * in1.cols + j].g) * alpha + (double)(in2.data[i * in2.cols + j].g) * (1 - alpha);
	      blend_image.data[i * max_cols + j].g = (int)g; 

        double b = (double)(in1.data[i * in1.cols + j].b) * alpha + (double)(in2.data[i * in2.cols + j].b) * (1 - alpha);
	      blend_image.data[i * max_cols + j].b = (int)b; 
      }
    }

    //Handles the remaining pixels that aren't overlapped in four cases
    
    // 1 is a subset of 2
    blend_image = handleCase1(in1, in2, blend_image, max_cols, min_cols, min_rows, max_rows);

    //2 is a subset of 1
    blend_image = handleCase2(in1, in2, blend_image, max_cols, min_cols, min_rows, max_rows);

    //im2 is longer vertically than im1 and im1 is longer horizontally
    blend_image = handleCase3(in1, in2, blend_image, max_cols, min_cols, min_rows, max_rows);

    //im2 is longer horizontally and im1 is longer vertically
    blend_image = handleCase4(in1, in2, blend_image, max_cols, min_cols, min_rows, max_rows);

    return blend_image;
}


Image rotate_ccw(const Image in) {
    Image rotated_image = make_image(in.cols, in.rows);
    
    //iteratively transpose image
    for (int i = 0; i < in.rows; i++) {
      for (int j = 0; j < in.cols; j++) {
	rotated_image.data[(in.cols - j - 1) * in.rows + i] = in.data[i * in.cols + j];
      }
    }
    
    return rotated_image; 
}


Image pointilism(const Image in) {
    Image pointilism_image = make_image(in.rows, in.cols);

    int num_pix = in.rows * in.cols;

    //initialize output image to black
    for (int i = 0; i < num_pix; i++) {
      pointilism_image.data[i].r = 0;
      pointilism_image.data[i].g = 0;
      pointilism_image.data[i].b = 0;
    }

    //iterate through number of randomly generated pixels
    for (int k = 0; k < (num_pix * 0.03); k++) {
      int rand_col = rand() % (in.cols);
      int rand_row = rand() % (in.rows); 
      int radius = rand() % 5 + 1;

      Pixel rand_pixel = in.data[rand_row * in.cols + rand_col]; 

      int r = rand_pixel.r;
      int g = rand_pixel.g;
      int b = rand_pixel.b;

      //iterate through pixels around the given pixel
      for (int i = -radius; i <= radius; i++) {
	  for (int j = -radius; j <= radius; j++) {
	    //check that pixel is within the circle of radius radius
            if (pow(i, 2) + pow(j, 2) <= pow(radius, 2)) {
	
              int index = (rand_row * in.cols + rand_col) + i * in.cols + j;
	            int col_pos = rand_col + j;
		    //check that indexed pixels is not off any edge of the image
	            if ((index < 0) || (index > num_pix) || (col_pos > in.cols) || (col_pos < 0)) {
		      continue;
		    }

	            pointilism_image.data[index].r = r;
	            pointilism_image.data[index].g = g;
	            pointilism_image.data[index].b = b;
	          }

	      }
      }
    }


    return pointilism_image; 
}


Image blur(const Image in, double sigma) {

  Image blur_image = make_image(in.rows, in.cols);

  //generate gaussian matrix 
  double* g_matrix = gauss_matrix(sigma);
  if (g_matrix == NULL) {
        fprintf(stderr, "Error: Gaussian matrix generation failed.\n");
        return blur_image;
    }

  //apply the convolution
  blur_image = apply_filter(g_matrix, in, blur_image, sigma);

  free(g_matrix);

  return blur_image; 
}

Image saturate(const Image in, double scale) {
  Image saturate_image = make_image(in.rows, in.cols);

  int num_pix = in.rows * in.cols;

  //iterate through number of pixels
  for (int i = 0; i < num_pix; i++) {
    int r = (in.data[i]).r;
    int g = (in.data[i]).g;
    int b = (in.data[i]).b;

    //make calculations
    unsigned char gray = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);

    int r_new = (in.data[i].r - gray) * scale + gray;
    int b_new = (in.data[i].b - gray) * scale + gray;
    int g_new = (in.data[i].g - gray) * scale + gray;

    //check that value is within bounds of 0 - 255
    if(r_new > 255) {
      r_new = 255;
    } else if (r_new < 0) {
      r_new = 0; 
    }
    if(b_new > 255) {
      b_new = 255;
    } else if (b_new < 0) {
      b_new = 0; 
    }
    if(g_new > 255) {
      g_new = 255;
    } else if (g_new < 0) {
      g_new = 0; 
    }

    //assign to output image data array
    saturate_image.data[i].r = r_new;
    saturate_image.data[i].b = b_new;
    saturate_image.data[i].g = g_new;
  }
  
  return saturate_image;
}

/*
Function that creates the gauss_matrix to be used in the blur function.
Takes in a sigma parameter. 
*/
double* gauss_matrix(double sigma) {
  int N = (double)(sigma * 10.0); 
  if (N % 2 == 0) {
    N += 1;
  }
  
  //mallocs a 2D matrix
  double* g_matrix = malloc(N * N * sizeof(double));
    if (g_matrix == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(g_matrix);
        return NULL;
    }


  double pi = 3.14159265358979323846;

  //iterate through size of matrix
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {

      //calculate distance from center
      int dx = abs(j - N/2);
      int dy = abs(i - N/2);

      //calculate and assign value
      g_matrix[i * N + j] = (1.0 / (2.0 * pi * (sigma * sigma))) * exp( -((dx * dx) + (dy * dy)) / (2 * (sigma * sigma)));
    }
  }
  
  return g_matrix; 
}

/*
Applies the gauss matrix created in the g_matrix function to the image.
takes in the filter, the original image and the new image as parameters
*/
Image apply_filter (double* g_filter, Image im1, Image im2, double sigma) {
  int N = (int)(sigma * 10.0); 
  if (N % 2 == 0) {
    N += 1;
  }

  //initialize output image to black
  for (int i = 0; i < im1.cols * im1.rows; i++) {
    im2.data[i].r = 0;
    im2.data[i].g = 0;
    im2.data[i].b = 0;
  }
    int center = N / 2;

    //iterate through pixels in image
  for (int y = 0; y < im1.rows; y++) {
    for (int x = 0; x < im1.cols; x++) {
	  //initialize running sums for rgb values and normalizing sum
	      double r_sum = 0.0;
        double g_sum = 0.0;
        double b_sum = 0.0;
        double norm = 0.0;
	    //iterate through pixels around the given pixel 
	    for (int i = -center; i <= center; i++) {
        for (int j = -center; j <= center; j++) {
          int yy = y + i;
          int xx = x + j;

		    //check that it is not indexing outside the edges of the image
		      if (yy >= 0 && yy < im1.rows && xx >= 0 && xx < im1.cols) {
            int pos = yy * im1.cols + xx;
            r_sum += im1.data[pos].r * g_filter[(i + center) * N + (j + center)];
            g_sum += im1.data[pos].g * g_filter[(i + center) * N + (j + center)];
            b_sum += im1.data[pos].b * g_filter[(i + center) * N + (j + center)];
            norm += g_filter[(i + center) * N + (j + center)];
          } else {
		        continue;
          }
        }
      }

	    //normalize the sums
        double r = r_sum / norm;
        double g = g_sum / norm;
        double b = b_sum / norm;

	    //index into output image
        im2.data[y * im1.cols + x].r = (unsigned char)r;
        im2.data[y * im1.cols + x].g = (unsigned char)g;
        im2.data[y * im1.cols + x].b = (unsigned char)b;
    }
  }
    
    
    return im2;
}

// handler for case in blend where image 1 is strictly a subset of image 2
Image handleCase1(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows) {
  if (in1.rows < in2.rows && in1.cols < in2.cols) {
      for (int i = 0; i < max_rows; i++) {
	      for (int j = min_cols; j < max_cols; j++) {
	        blend_image.data[i * max_cols + j].r = in2.data[i * max_cols + j].r;
	        blend_image.data[i * max_cols + j].g = in2.data[i * max_cols + j].g;
	        blend_image.data[i * max_cols + j].b = in2.data[i * max_cols + j].b;
	      }
      }

      for (int i = min_rows; i < max_rows; i++) {
	      for (int j = 0; j < min_cols; j++) {
	        blend_image.data[i * max_cols + j].r = in2.data[i * max_cols + j].r;
          blend_image.data[i * max_cols + j].g = in2.data[i * max_cols + j].g;
          blend_image.data[i * max_cols + j].b = in2.data[i * max_cols + j].b;
	      }
      }
    }
    return blend_image; 
}

//handler for case in blend where image 2 is strictly a subset of image 1
Image handleCase2(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows) {
  if (in2.rows < in1.rows && in2.cols < in1.cols) {
      for (int i = 0; i < max_rows; i++) {
        for (int j = min_cols; j < max_cols; j++) {
          blend_image.data[i * max_cols + j].r = in1.data[i * max_cols + j].r;
          blend_image.data[i * max_cols + j].g = in1.data[i * max_cols + j].g;
          blend_image.data[i * max_cols + j].b = in1.data[i * max_cols + j].b;
	      }
      }

      for (int i = min_rows; i < max_rows; i++)	{
        for (int j = 0; j < min_cols; j++) {
          blend_image.data[i * max_cols + j].r = in1.data[i * max_cols + j].r;
          blend_image.data[i * max_cols + j].g = in1.data[i * max_cols + j].g;
          blend_image.data[i * max_cols + j].b = in1.data[i * max_cols + j].b;
        }
      }
    }
    return blend_image;
}

// handler for blend case when image 2 is longer vertically than image 1 and image 1 is longer horizontally         
Image handleCase3(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows) {
  if (in1.rows < in2.rows && in1.cols > in2.cols) {
      for (int i = 0; i < min_rows; i++) {
	      for (int j = min_cols; j < max_cols; j++) {
	        blend_image.data[i * max_cols + j].r = in1.data[i * max_cols + j].r;
	        blend_image.data[i * max_cols + j].g = in1.data[i * max_cols + j].g;
	        blend_image.data[i * max_cols + j].b = in1.data[i * max_cols + j].b;
	      }
      }

      for (int i = min_rows; i < max_rows; i++) {
	      for (int j = 0; j < min_cols; j++) {
	        blend_image.data[i * max_cols + j].r = in2.data[i * min_cols + j].r;
          blend_image.data[i * max_cols + j].g = in2.data[i * min_cols + j].g;
          blend_image.data[i * max_cols + j].b = in2.data[i * min_cols + j].b;
	      }
      }
    }
    return blend_image; 
}

// handler for blend case when image 1 is longer vertically than image 2 and image 2 is longer horizontally     
Image handleCase4(Image in1, Image in2, Image blend_image, int max_cols, int min_cols, int min_rows, int max_rows) {
  if (in1.rows > in2.rows && in1.cols < in2.cols) {
      for (int i = 0; i < min_rows; i++) {
        for (int j = min_cols; j < max_cols; j++) {
          blend_image.data[i * max_cols + j].r = in2.data[i * max_cols + j].r;
          blend_image.data[i * max_cols + j].g = in2.data[i * max_cols + j].g;
          blend_image.data[i * max_cols + j].b = in2.data[i * max_cols + j].b;
        }
      }

      for (int i = min_rows; i < max_rows; i++) {
        for (int j = 0; j < min_cols; j++) {
          blend_image.data[i * max_cols + j].r = in1.data[i * min_cols + j].r;
          blend_image.data[i * max_cols + j].g = in1.data[i * min_cols + j].g;
          blend_image.data[i * max_cols + j].b = in1.data[i * min_cols + j].b;
        }
      }
    }
    return blend_image; 
}
