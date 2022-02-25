CC=g++
CFLAGS=-std=c++11 -g -c -lm -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11
LFLAGS=-std=c++11 -g -lm -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -pthread

Main: Main.o Raytracer.o ScreenPainter.o Camera.o Light.o GenericObject.o Matrix.o templates.cpp
	$(CC) -o $@ $^ $(LFLAGS)

Main.o: Main.cpp ScreenPainter.h Camera.h GenericObject.h Matrix.h
	$(CC) $(CFLAGS) -o $@ $<

ScreenPainter.o: ScreenPainter.cpp ScreenPainter.h Raytracer.h Scene.h Model.h
	$(CC) $(CFLAGS) -o $@ $<

Raytracer.o: Raytracer.cpp Raytracer.h Scene.h Camera.h GenericObject.h Matrix.h 
	$(CC) $(CFLAGS) -o $@ $<

Light.o: Light.cpp Light.h Matrix.h Model.h
	$(CC) $(CFLAGS) -o $@ $<
	
GenericObject.o: GenericObject.cpp GenericObject.h Matrix.h Model.h
	$(CC) $(CFLAGS) -o $@ $<

Camera.o: Camera.cpp Camera.h Matrix.h Model.h
	$(CC) $(CFLAGS) -o $@ $<

Matrix.o: Matrix.cpp Matrix.h templates.cpp
	$(CC) $(CFLAGS) -o $@ $<

ScreenPainter.h: Matrix.h Model.h
Scene.h: Camera.h GenericObject.h Light.h
Camera.h: Matrix.h Model.h
GenericObject.h: Matrix.h Model.h
Light.h: Matrix.h Model.h
Model.h: Matrix.h

clean:
	\rm -rf *.o *~ Main

