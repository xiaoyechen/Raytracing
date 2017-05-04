CC=g++
CFLAGS=-g -c -lm -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11
LFLAGS=-std=c++11 -g -lm -I/usr/X11R6/include -L/usr/X11R6/lib -lX11

Main: Main.o Raytracer.o Window.o Camera.o GenericObject.o Matrix.o templates.cpp
	$(CC) -o $@ $^ $(LFLAGS)

Main.o: Main.cpp Window.h Camera.h GenericObject.h Matrix.h model.h
	$(CC) $(CFLAGS) -o $@ $<

Window.o: Window.cpp Window.h Raytracer.h model.h
	$(CC) $(CFLAGS) -o $@ $<

Raytracer.o: Raytracer.cpp Raytracer.h Camera.h Matrix.h GenericObject.h
	$(CC) $(CFLAGS) -o $@ $<

GenericObject.o: GenericObject.cpp GenericObject.h Matrix.h model.h
	$(CC) $(CFLAGS) -o $@ $<

Camera.o: Camera.cpp Camera.h Matrix.h model.h
	$(CC) $(CFLAGS) -o $@ $<

Matrix.o: Matrix.cpp Matrix.h templates.cpp
	$(CC) $(CFLAGS) -o $@ $<

Window.h: Raytracer.h
Camera.h: Matrix.h model.h
GenericObject.h: Matrix.h model.h
model.h: Matrix.h

clean:
	\rm -rf *.o *~ main

