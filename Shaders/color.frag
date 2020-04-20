#version 140
precision mediump float; //Medium precision for float. highp and smallp can also be used


uniform vec4 uK;
uniform vec3 uColor;
uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform vec3 uCameraPosition;

uniform sampler2D uTexture;
varying vec2 vary_UV;	// text coord


// varying vec4 varyColor; //Sometimes we use "out" instead of "varying". "out" should be used in later version of GLSL.

varying vec4 varyPosition;
varying vec3 varyNormal;

void main()
{

	vec3 L = normalize(uLightPosition - varyPosition.xyz);
	vec3 V = normalize(uCameraPosition - varyPosition.xyz);
	

	vec3 ambient 	= uK.x * uColor * uLightColor;
	vec3 diffuse 	= uK.y * uColor * uLightColor * max(0.0, dot(varyNormal, L));
	
	vec3 specular 	= uK.z * (pow(max(0.0, dot(reflect(-L, varyNormal), V)), uK.w)) * uLightColor;

    vec4 texture = texture2D(uTexture, vary_UV);
	vec4 lighting = vec4(min(vec3(1.0, 1.0, 1.0), ambient + diffuse + specular), 1.0);
    
    // gl_FragColor = lighting * texture; same as :
    gl_FragColor = gl_LightModel.ambient * gl_FrontMaterial.ambient + lighting * texture;
}
