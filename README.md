# Raytracing 
(Incomplete)

Simple ray tracer program using X11 library for display

Use 'make' command to compile and './Main test_input.txt' to run

Most of the constants are in model.h

Input file format:
	
	window_height aspect_ratio
	light_source_position(x y z) light_source_color(r g b)
	infinite_light_source_position(x y z) infinite_light_source_color(r g b)
	camera_position
	gazing_vector
	near_plane far_plane viewing_angle
	camera_up_vector(default is [0 0 1])
	number_of_objects
	
	object1_type_id
	object1_color(r g b)
	ambient_coefficient diffuse_coefficient specular_coefficient fallout_exponent
	transformation_matrix (4 rows by 4 columns)

