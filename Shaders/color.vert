#version 140
precision mediump float;

attribute vec3 vPosition; //Depending who compiles, these variables are not "attribute" but "in"

attribute vec3 vNormal;
attribute vec2 vUV;

// uniform float uScale;

uniform mat4 uMvp;
uniform mat4 uModelView;

varying vec4 varyPosition;
varying vec3 varyNormal;
varying vec2 vary_UV;


void main()
{
    gl_Position = uMvp * vec4(vPosition, 1.0);
    vary_UV     = -vUV + vec2(1.0, 0.0);

    varyNormal = normalize(transpose(inverse(mat3(uModelView))) * vNormal);
        
    varyPosition = uModelView * vec4(vPosition, 1.0);
    varyPosition /= varyPosition.w;
}
