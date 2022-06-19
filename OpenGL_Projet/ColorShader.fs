#version 120

// SORTIE DU VERTEX SHADER et INPUT du FRAGMENT SHADER
varying vec4 v_color;

void main(void) {

    // gl_FragColor est un vec4 predefini
    // = couleur du pixel a l'ecran

    gl_FragColor = v_color;
}
