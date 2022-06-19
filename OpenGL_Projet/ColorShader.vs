#version 120

// attribute = input du VERTEX shader

attribute vec3 a_position;
attribute vec3 a_color;

// varying = output du VERTEX shader et INPUT du FRAGMENT shader

varying vec4 v_color;

void main(void) {

	// X=a_position.x;Y=a_position.y;Z=a_position.z;W=1.0
	gl_Position = vec4(a_position, 1.0); 

	// xyzw ou rgba
	v_color = vec4(a_color, 1.0);
}