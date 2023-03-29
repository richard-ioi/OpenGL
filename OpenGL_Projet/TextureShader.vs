#version 120

// attribute = input du VERTEX shader

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoords;
attribute vec3 a_color;

// varying = output du VERTEX shader et INPUT du FRAGMENT shader
varying vec2 v_texcoords;
varying vec4 v_color;
varying vec3 v_FragPos;
varying vec3 v_normal;

uniform float u_time;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_WorldMatrix;


uniform mat4 u_MVP;

void main(void) 
{
	v_normal = a_normal;
	// stpq alias de xyzw alias de rgba
	v_texcoords = vec2(a_texcoords.s, 1.0 - a_texcoords.t);

	//l'ordre des transformations est important -> sens de rotation
	//v' = PROJECTION * (T * R * S * v)
	gl_Position =  u_ProjectionMatrix  * u_WorldMatrix * vec4(a_position,1.0) ; //en changeant la pos, on eloigne le PDV/"camera"
	v_FragPos = vec3(u_WorldMatrix * vec4(a_position, 1.0));

	v_color = vec4(a_color, 1.0);
}