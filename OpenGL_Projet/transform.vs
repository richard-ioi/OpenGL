#version 120

// attribute = input du VERTEX shader

attribute vec3 a_position;
attribute vec2 a_texcoords;
attribute vec3 a_color;

// varying = output du VERTEX shader et INPUT du FRAGMENT shader
varying vec2 v_texcoords;
varying vec4 v_color;

uniform float u_time;

uniform mat4 u_projection;
mat4 u_view;
mat4 u_model;

uniform mat4 u_translation;
uniform mat4 u_scale;
uniform mat4 u_rotation;

void main(void) 
{
	// stpq alias de xyzw alias de rgba
	v_texcoords = vec2(a_texcoords.s, 1.0 - a_texcoords.t);

	//l'ordre des transformations est important -> sens de rotation
	//v' = PROJECTION * (T * R * S * v)
	u_model = u_translation * u_rotation * u_scale;
	gl_Position =  (u_projection * u_model) * vec4(a_position,1.0) ; //en changeant la pos, on eloigne le PDV/"camera"
	// X=a_position.x;Y=a_position.y;Z=0.0;W=1.0

	// xyzw ou rgba
	v_color = vec4(a_color, 1.0);
}