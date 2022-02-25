# Raytracing

!Current scene preview(preview512x288.png)

Simple ray tracer program using X11 library

Platform: Linux (tested on Ubuntu)

- Use `make` to compile (may need to install xorg-dev: `sudo apt install xorg-dev`
- Use `./Main test_input.txt` to run
- When running, use `↑↓←→` keys to rotate the camera around; press `f`/`b` to move camera away from or closer to origin

Note: 
1. Most of the constants are in Model.h
2. Currently implemented object types: plane, sphere, cylinder and cone

Input file format:
	
	window_width window_height
	
	light_source_position_x light_source_position_y light_source_position_z light_source_color_r light_source_color_g light_source_color_b light_source_intensity
	infinite_light_source_position_x infinite_light_source_position_y infinite_light_source_position_z infinite_light_source_color_r infinite_light_source_color_g infinite_light_source_color_b light_source_intensity
	
	camera_position_x camera_position_y camera_position_z
	gazing_vector
	near_plane_distance far_plane_distance viewing_angle_degree
	camera_up_vector(default is 0 0 1)
	
	number_of_objects
	
	object1_type_id
	object1_color(r g b)
	ambient_coefficient diffuse_coefficient specular_coefficient fallout_exponent reflection_coefficient
	transformation_matrix (4 rows by 4 columns)
	...

