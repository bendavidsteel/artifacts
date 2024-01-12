#version 440

#define PI 3.141592

uniform ivec2 resolution;
uniform float time;

uniform sampler2DRect last1;
uniform sampler2DRect last2;
uniform sampler2DRect last3;
uniform sampler2DRect last4;

uniform sampler2DRect mask;
uniform sampler2DRect eye;
uniform sampler2DRect camera;

uniform sampler2DRect attractors;
uniform sampler2DRect fractals;
uniform sampler2DRect scrolls;
uniform sampler2DRect temple;

uniform float bass;
uniform float treble;
uniform float bpm;
uniform int beat;
uniform int isOnset;
uniform int sortPixels;

out vec4 out_color;

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


mat2 rot_mat(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat2(c, s, -s, c);
}

mat2 refl_mat(float theta) {
    float c = cos(2 * theta);
    float s = sin(2 * theta);
    return mat2(c, s, s, -c);
}

mat3 get_colour_rotation(int theta)
{
    theta = theta % 6;
    if (theta == 0)
    {
        return mat3(1., 0., 0., 0., 1., 0., 0., 0., 1.);
    }
    else if (theta == 1)
    {
        return mat3(0.5, 0.5, 0., 0., 0.5, 0.5, 0.5, 0., 0.5);
    }
    else if (theta == 2)
    {
        return mat3(0., 1., 0., 0., 0., 1., 1., 0., 0.);
    }
    else if (theta == 3)
    {
        return mat3(0., 0.5, 0.5, 0.5, 0., 0.5, 0.5, 0.5, 0.);
    }
    else if (theta == 4)
    {
        return mat3(0., 0., 1., 1., 0., 0., 0., 1., 0.);
    }
    else if (theta == 5)
    {
        return mat3(0.5, 0., 0.5, 0.5, 0.5, 0., 0., 0.5, 0.5);
    }
}

vec3 pixelSorter() {
    vec2 sortDir = vec2(0.5, 0.5);
    vec3 lastColour = texture(last3, gl_FragCoord.xy).rgb;
    vec3 lastUpColour = texture(last3, gl_FragCoord.xy + sortDir).rgb;
    vec3 lastDownColour = texture(last3, gl_FragCoord.xy - sortDir).rgb;

    float maskCentre = texture(mask, gl_FragCoord.xy).a;
    float maskUp = texture(mask, gl_FragCoord.xy + sortDir).a;
    float maskDown = texture(mask, gl_FragCoord.xy - sortDir).a;

    float lastBright = dot(lastColour, vec3(0.333));
    float lastUpBright = dot(lastUpColour, vec3(0.333));
    float lastDownBright = dot(lastDownColour, vec3(0.333));

    if (maskUp > 0. && lastUpBright > lastBright) {
        return lastUpColour;
    } else if (maskDown > 0. && lastDownBright < lastBright) {
        return lastDownColour;
    } else {
        return lastColour;
    }
}

vec3 get_scrolls(vec2 coord) {
    float bps = 3.;
    vec2 uv = coord / resolution.xy;
    float w = 10.;
    uv += 0.05 * vec2(sin(w * uv.y + time * bps), cos(w * uv.x + time * bps));
    coord = uv * resolution.xy;
    vec3 colour = texture(scrolls, coord).rgb;
    if (colour.r >= 0.5) {
        colour = vec3(1., 0., 0.);
    }
    return colour;
}

void main(){
    float bpm = 130;
    float bps = bpm / 60;

    float bass_boost = pow(bass, 3.);

    vec2 coord = gl_FragCoord.xy;
    vec2 uv = coord / vec2(resolution);

    vec2 centre = vec2(0.5, 0.58);
    vec2 from_centre = uv - centre;

    float theta = atan(from_centre.y, from_centre.x);
    float r = length(from_centre);

    if (beat == 3) {
        out_color.rgb = texture(attractors, coord).rgb;
    }
    out_color.a = 1.;
}