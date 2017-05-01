# Raytracing

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
	object1_diffuse_color(r g b) diffuse_coefficient
	object1_specular_color(r g b) specular_coefficient
	object1_ambient_color(r g b) ambient_coefficient
	fallout_exponent
	transformation_matrix (4 rows by 3 columns)

