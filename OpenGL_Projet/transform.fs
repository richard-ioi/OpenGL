#version 120


// SORTIE DU VERTEX SHADER et INPUT du FRAGMENT SHADER
varying vec2 v_texcoords;
varying vec4 v_color;
varying vec3 v_diff;


uniform sampler2D u_sampler;

void main(void) {
    vec4 texColor = texture2D(u_sampler, v_texcoords);
    // gl_FragColor est un vec4 predefini
    // = couleur du pixel a l'ecran
    gl_FragColor = texColor;
}
