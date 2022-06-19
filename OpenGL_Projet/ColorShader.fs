#version 120


// SORTIE DU VERTEX SHADER et INPUT du FRAGMENT SHADER
varying vec2 v_texcoords;
varying vec4 v_color;
varying vec3 v_diff;

varying vec3 v_FragPos;
varying vec3 v_normal;

uniform sampler2D u_sampler;
uniform vec3 u_lightPos;

// Retourne la couleur diffuse pour l'illumination de Phong
vec4 diffuse(vec3 N, vec3 L){
    return max(dot(N,L),0) * 1;
}

void main(void) {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightPos - v_FragPos);
    gl_FragColor = v_color;
}
