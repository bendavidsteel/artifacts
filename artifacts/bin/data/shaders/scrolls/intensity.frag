#version 440

uniform float time;
uniform ivec2 resolution;
uniform int numPoints;
uniform int numAudio;
uniform float bass;

uniform sampler2DRect camera;

out vec4 out_color;

struct Component{
	vec4 value;
};

layout(std140, binding=6) buffer audio{
    Component allAudio[];
};

layout(std140, binding=7) buffer points{
    Component allPoints[];
};


/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

/* skew constants for 3d simplex functions */
const float F3 =  0.3333333;
const float G3 =  0.1666667;

/* 3d simplex noise */
float simplex3d(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
	 
	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));
	 
	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;
	 
	 /* 2. find four surflets and store them in d */
	 vec4 w, d;
	 
	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);
	 
	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);
	 
	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);
	 
	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;
	 
	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}

/* const matrices for 3d rotation */
const mat3 rot1 = mat3(-0.37, 0.36, 0.85,-0.14,-0.93, 0.34,0.92, 0.01,0.4);
const mat3 rot2 = mat3(-0.55,-0.39, 0.74, 0.33,-0.91,-0.24,0.77, 0.12,0.63);
const mat3 rot3 = mat3(-0.71, 0.52,-0.47,-0.08,-0.72,-0.68,-0.7,-0.45,0.56);

/* directional artifacts can be reduced by rotating each octave */
float simplex3d_fractal(vec3 m) {
    return   0.5333333*simplex3d(m*rot1)
			+0.2666667*simplex3d(2.0*m*rot2)
			+0.1333333*simplex3d(4.0*m*rot3)
			+0.0666667*simplex3d(8.0*m);
}

    

float getSpectrum(float cassini, float dist) {
    float scale = pow(pow(cassini, 1. / (1. * dist)), 1.5) / bass;
    // float scale = length(from_centre) * 2;
    float index = scale * numAudio;
    if (ceil(index) >= numAudio) {
        index = numAudio - 1 - (index - numAudio);
    }

    float lowerSpectrumVal = allAudio[max(int(floor(index)), 0)].value.x;
    float upperSpectrumVal = allAudio[min(int(ceil(index)), numAudio-1)].value.x;
    float spectrumVal = mix(lowerSpectrumVal, upperSpectrumVal, pow(fract(index), 2.));
    // float spectrumMapped = exp((spectrumVal - 1) / 4);
    return spectrumVal;
}

void main(){

	ivec2 coord = ivec2(gl_FragCoord.xy);

    vec2 p = vec2(1.0) * coord.xy/resolution.xy;
    vec3 p3 = vec3(p, time*0.025);
    
    float flow_force = simplex3d_fractal(p3*8.0+8.0);

	// scale to 0-1 for image storage
	float flow = 0.5 + (0.5 * flow_force);

	vec2 uv = p;
    vec2 centre = vec2(0.5, 0.5);
    vec2 from_centre = uv - centre;

	float o;

    float cassini = 1;
    float a = 0;
    for (int i = 0; i < numPoints; i++) {
        vec2 pos = allPoints[i].value.xy;
        float strength = allPoints[i].value.z;
        a += strength;
        cassini *= mix(1., dot(pow(from_centre - pos, vec2(2.)), vec2(1.)), strength);
    }

	float audioShare = 0.;
	float noiseShare = 0.;
	float cameraShare = 1.;

    o += audioShare * getSpectrum(cassini, a);
    o += noiseShare * flow;
	o += cameraShare * dot(texture(camera, gl_FragCoord.xy).rgb, vec3(0.333));

    out_color = vec4(o, o, o, 1.);
}
