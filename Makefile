
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -O
LDLIBS = -lm

project: project.o image_manip.o ppm_io.o
	$(CC) $(CFLAGS) -o project project.o image_manip.o ppm_io.o $(LDLIBS)

project.o: project.c image_manip.h ppm_io.h
	$(CC) $(CFLAGS) -c project.c

image_manip.o: image_manip.c image_manip.h ppm_io.h
	$(CC) $(CFLAGS) -c image_manip.c 

test: img_cmp.o ppm_io.o
	$(CC) $(CFLAGS) -o test img_cmp.o ppm_io.o

img_cmp.o: img_cmp.c ppm_io.h
	$(CC) $(CFLAGS) -c img_cmp.c ppm_io.h

ppm_io.o: ppm_io.c ppm_io.h
	$(CC) $(CFLAGS) -c ppm_io.c

clean:
	rm -f *.o project test
