#version 440

#define PI = 3.141592653589

struct Particle{
	vec4 pos;
	vec4 color;
};

struct Component{
	vec4 value;
};

layout(std140, binding=0) buffer particle{
    Particle p[];
};

layout(std140, binding=1) buffer particleBack{
    Particle p2[];
};

layout(std140, binding=2) buffer line1{
	Particle line11[];
};

layout(std140, binding=3) buffer line1Back{
	Particle line12[];
};

layout(std140, binding=4) buffer line2{
	Particle line21[];
};

layout(std140, binding=5) buffer line2Back{
	Particle line22[];
};

layout(std140, binding=6) buffer audio{
    Component allAudio[];
};

layout(std140, binding=7) buffer points{
    Component allPoints[];
};

uniform float timeLastFrame;
uniform float elapsedTime;
uniform float step_size;
uniform int attractor_type;
uniform int num_vertex;
uniform int numAudio;
uniform int numPoints;
uniform float dist;
uniform float weirdFactor1;
uniform float weirdFactor2;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct agent float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

vec3 lorenz(vec3 pos) {
	float sigma = 10.;
	float rho = 28.;
	float beta = 8. / 3.;

	sigma *= 1 + weirdFactor1;
	rho *= 1 + weirdFactor2;

	float dx = sigma * (pos.y - pos.x);
	float dy = pos.x * (rho - pos.z) - pos.y;
	float dz = pos.x * pos.y - beta * pos.z;
	return vec3(dx, dy, dz);
}

vec3 rossler(vec3 pos) {
	float a = 0.2;
	float b = 0.2;
	float c = 5.7;

	a *= 1 + weirdFactor1;
	b *= 1 + weirdFactor2;

	float dx = -pos.y - pos.z;
	float dy = pos.x + a * pos.y;
	float dz = b + pos.z * (pos.x - c);
	return vec3(dx, dy, dz);
}

vec3 aizawa(vec3 pos) {
	float a = 0.95;
	float b = 0.7;
	float c = 0.6;
	float d = 3.5;
	float e = 0.25;
	float f = 0.1;

	a *= 1 + weirdFactor1;
	b *= 1 + weirdFactor2;

	float dx = (pos.z - b) * pos.x - d * pos.y;
	float dy = d * pos.x + (pos.z - b) * pos.y;
	float dz = c + a * pos.z - (pos.z * pos.z * pos.z) / 3. - (pos.x * pos.x + pos.y * pos.y) * (1. + e * pos.z) + f * pos.z * pos.x * pos.x * pos.x;
	return vec3(dx, dy, dz);
}

vec3 chen(vec3 pos) {
	float a = 5.;
	float b = -10.;
	float c = -0.38;

	a *= 1 + weirdFactor1;
	b *= 1 + weirdFactor2;

	float dx = a * pos.x - pos.y * pos.z;
	float dy = b * pos.y + pos.x * pos.z;
	float dz = c * pos.z + (pos.x * pos.y / 3.);
	return vec3(dx, dy, dz);
}

vec3 dadras(vec3 pos) {
	float a = 3.;
	float b = 2.7;
	float c = 1.7;
	float d = 2.;
	float e = 9.;

	a *= 1 + weirdFactor1;
	b *= 1 + weirdFactor2;

	float dx = pos.y - a * pos.x + b * pos.y * pos.z;
	float dy = c * pos.y - pos.x * pos.z + pos.z;
	float dz = d * pos.x * pos.y - e * pos.z;
	return vec3(dx, dy, dz);
}

vec3 lorenz83(vec3 pos) {
	float a = 0.95;
	float b = 7.91;
	float f = 4.83;
	float g = 4.66;

	a *= 1 + weirdFactor1;
	b *= 1 + weirdFactor2;

	float dx = -a * pos.x + pos.y * pos.y - pos.z * pos.z + a * f;
	float dy = -pos.y + pos.x * pos.y - b * pos.x * pos.z + g;
	float dz = -pos.z + b * pos.x * pos.y + pos.x * pos.z;
	return vec3(dx, dy, dz);
}

vec3 halvorsen(vec3 pos) {
	float a = 1.89;
	float dx = -a * pos.x - 4. * pos.y - 4. * pos.z - pos.y * pos.y;
	float dy = -a * pos.y - 4. * pos.z - 4. * pos.x - pos.z * pos.z;
	float dz = -a * pos.z - 4. * pos.x - 4. * pos.y - pos.x * pos.x;
	return vec3(dx, dy, dz);
}

vec3 rabinovichfabrikant(vec3 pos) {
	float a = 0.14;
	float l = 0.1;
	float dx = pos.y * (pos.z - 1. + pos.x * pos.x) + l * pos.x;
	float dy = pos.x * (3. * pos.z + 1. - pos.x * pos.x) + l * pos.y;
	float dz = -2. * pos.z * (a + pos.x * pos.y);
	return vec3(dx, dy, dz);
}

vec3 threescroll(vec3 pos) {
	float a = 32.48;
	float b = 45.84;
	float c = 1.18;
	float d = 0.13;
	float e = 0.57;
	float f = 14.7;

	float dx = a * (pos.y - pos.x) + d * pos.x * pos.z;
	float dy = b * pos.x - pos.x * pos.z + f * pos.y;
	float dz = c * pos.z + pos.x * pos.y - e * pos.x * pos.x;
	return vec3(dx, dy, dz);
}

vec3 attractor(vec3 pos) {
	if (attractor_type == 0) {
		return lorenz(pos);
	} else if (attractor_type == 1) {
		return halvorsen(pos);
	} else if (attractor_type == 2) {
		return rossler(pos);
	} else if (attractor_type == 3) {
		return aizawa(pos);
	} else if (attractor_type == 4) {
		return chen(pos);
	} else if (attractor_type == 5) {
		return dadras(pos);
	} else if (attractor_type == 6) {
		return lorenz83(pos);
	} else if (attractor_type == 7) {
		return rabinovichfabrikant(pos);
	} else if (attractor_type == 8) {
		return threescroll(pos);
	}
}

float getAudio(float cassini) {
	float scale = cassini / pow(dist, 1.7);
    // float scale = length(from_centre) * 2;
    float index = scale * numAudio;
    if (ceil(index) >= numAudio) {
        index = index - (numAudio * floor(index/numAudio));
    }

    float lowerAudioVal = allAudio[max(int(floor(index)), 0)].value.x;
    float upperAudioVal = allAudio[min(int(ceil(index)), numAudio-1)].value.x;
    float audioVal = mix(lowerAudioVal, upperAudioVal, pow(fract(index), 2.));
    return audioVal;
}

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
void main(){
	uint particleIdx = gl_GlobalInvocationID.x;

	vec3 pos = p2[particleIdx].pos.xyz;

	vec3 vel = attractor(pos);

	pos += step_size * vel;

	p[gl_GlobalInvocationID.x].pos.xyz = pos;

    float cassini = 1.;
    for (int i = 0; i < numPoints; i++) {
        vec3 point = allPoints[i].value.xyz;
		vec3 relPos = pos - point;
        cassini *= dot(relPos, relPos);
    }
    float audio = getAudio(cassini);

	float rand = random(particleIdx);
	vec3 colour;
	if (rand < 0.3) {
		colour = vec3(1., 0., 0.);
	} else if (rand > 0.6) {
		colour = vec3(0.5, 0.5, 0.);
	} else {
		colour = vec3(0.5, 0., 0.5);
	}

	float norm_speed = length(vel) / 1000;
	colour += vec3(0., 0.5 * norm_speed, 1. * norm_speed);
	// colour += vec3(0., 1. * audio, 0.5 * audio);
	
	float audioSensitivity = 4.;

	p[gl_GlobalInvocationID.x].color.rgb = colour;
	p[gl_GlobalInvocationID.x].color.a = 0.1 * pow(audio, audioSensitivity);

	if (particleIdx == 0) {
		vec3 pos1 = line12[particleIdx].pos.xyz;

		vec3 vel1 = attractor(pos1);
		pos1 += step_size * vel1;

		line11[particleIdx].pos.xyz = pos1;

		vec3 pos2 = line22[particleIdx].pos.xyz;

		vec3 vel2 = attractor(pos2);
		pos2 += step_size * vel2;

		line21[particleIdx].pos.xyz = pos2;

	} else if (particleIdx < num_vertex) {
		vec3 oldPos1 = line12[particleIdx].pos.xyz;
		vec3 newPos1 = line12[particleIdx - 1].pos.xyz;

		line11[particleIdx].pos.xyz = newPos1;

		vec3 oldPos2 = line22[particleIdx].pos.xyz;
		vec3 newPos2 = line22[particleIdx - 1].pos.xyz;

		line21[particleIdx].pos.xyz = newPos2;
	}

	int audioSpeed = 3;
	if (particleIdx < audioSpeed) {
		float bass = allAudio[0].value.x;
		bass = pow(bass, audioSensitivity);
		line11[particleIdx].color.rgb = vec3(0.1, 0.1, 0.8);
		line11[particleIdx].color.a = bass;

		line21[particleIdx].color.rgb = vec3(0.2, 1., 0.8);
		line21[particleIdx].color.a = bass;

	} else if (particleIdx < num_vertex) {
		vec4 color1 = line12[particleIdx - audioSpeed].color.rgba;
		line11[particleIdx].color.rgba = color1;

		vec4 color2 = line22[particleIdx - audioSpeed].color.rgba;
		line21[particleIdx].color.rgba = color2;
	}
}