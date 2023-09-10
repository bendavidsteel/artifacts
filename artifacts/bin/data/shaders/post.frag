#version 440

#define PI 3.141592

uniform ivec2 resolution;
uniform float time;

uniform sampler2DRect last;

uniform sampler2DRect mask;
uniform sampler2DRect eye;
uniform sampler2DRect artificer;

uniform sampler2DRect attractors;
uniform sampler2DRect fractals;
uniform sampler2DRect scrolls;

uniform float low;
uniform float high;
uniform float bps;

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

void main(){
    vec2 coord = gl_FragCoord.xy;
    vec2 uv = coord / vec2(resolution);

    // vec2 centre = vec2(0.5, 0.5);
    // vec2 from_centre = uv - centre;
    // from_centre *= 0.2;

    // float theta = atan(from_centre.y, from_centre.x);
    // float r = length(from_centre);
    // float refl_theta = PI * 0.25;
    // if (theta > refl_theta) {
    //     theta = refl_theta;;
    // }
    // if (r > 0.4) {
    //     r = 1 - r;
    // }
    // mat2 rot
    // if ((from_centre * refl_mat(PI * 0.)).y > 0.) {
    //     from_centre = from_centre * refl_mat(PI * 0.);
    // }

    // theta += 0.1 * random(time);

    // if ((from_centre * refl_mat(PI * 0.5)).x > 0.) {
    //     from_centre = from_centre * refl_mat(PI * 0.5);
    // }

    // if (length((from_centre * refl_mat(PI * 0.25)).xy) > 0.) {
    //     from_centre = from_centre * refl_mat(PI * 0.25);
    // }

    // theta = atan(from_centre.y, from_centre.x);
    // r = length(from_centre);
    // r *= 1. + pow(low / 8., 3.);

    // // theta += 0.5 * random(r);
    // // r += 0.01 * random(theta);

    // from_centre = vec2(cos(theta), sin(theta)) * r;

    // uv = centre + from_centre;

    // uv.y += 0.001 * pow(low + 1, 3.) * random(uv.x);
    // uv.x += 0.001 * pow(low + 1, 3.) * random(uv.y);

    // float scale = 1.2;
    // uv.x = (uv.x * scale) - (1 * floor(uv.x * scale/1));
    // uv.y = (uv.y * scale) - (1 * floor(uv.y * scale/1));
    // coord = uv * vec2(resolution);

    // float left = 0.;
    // float right = 1.;
    // float up = 1.0;
    // float down = 0.4;

    // vec2 mapped_uv;
    // mapped_uv.x = (uv.x - left) / (right - left);
    // mapped_uv.y = uv.y;

    // vec2 mapped_coord = mapped_uv.xy * resolution.xy;
    
    // vec3 colour = vec3(0., 0., 0.);
    // colour = texture(tex, coord).rgb;

    // vec2 from_centre2 = refl_mat(PI * 0.25) * from_centre;
    // vec2 uv2 = centre + from_centre2;
    // vec2 coord2 = uv2 * vec2(resolution);
    // vec3 colour2 = texture(tex, coord2).rgb;
    // colour = mix(colour, colour2, 0.5);

    // int feedback = 1;
    // if (feedback == 1) {
    //     vec2 centre = vec2(0.5, 0.5);
    //     vec2 from_centre = uv - centre;
    //     float r = length(from_centre);
    //     float theta = atan(from_centre.y, from_centre.x);

    //     // r *= 1. + 0.5 * pow(low, 2.);
    //     // theta += 0.01 * pow(high, 2.);

    //     vec2 scaled_uv = centre + vec2(cos(theta), sin(theta)) * r;

    //     // scaled_uv *= pow(low, 2.);

    //     vec3 scaled_colour = texture(last, scaled_uv * vec2(resolution)).rgb;
    //     scaled_colour = get_colour_rotation(int(time * 0.5)) * scaled_colour;
    //     // colour += 0.3 * scaled_colour * get_colour_rotation(int(3 + 3 * sin(bps * time)));
    // }

    // colour = 1 - colour;
    // colour = get_colour_rotation(int(2 * time * bps)) * colour;

    // eyes
    // vec3 eye_colour = texture(eye, coord).rgb;
    // if (length(colour) > 0.1) {
    //     colour = mix(colour, eye_colour * (1.2 + length(colour)), 0.1);
    //     colour = eye_colour * 2 * length(colour);
    // }

    // artificer
    // vec4 artificer_colour = texture(artificer, coord).rgba;
    // artificer_colour += 0.2 * texture(artificer, coord + ivec2(0, 1)).rgba;
    // artificer_colour += 0.2 * texture(artificer, coord + ivec2(1, 0)).rgba;
    // artificer_colour += 0.2 * texture(artificer, coord + ivec2(0, -1)).rgba;
    // artificer_colour += 0.2 * texture(artificer, coord + ivec2(-1, 0)).rgba;
    // if (artificer_colour.a > 0.1) {
    //     // colour = mix(colour, artificer_colour.rgb, 0.4);
    //     colour *= artificer_colour.rgb;
    // }

    // colour.r += 0.2 + 0.2 * cos(4 * time * bps);
    // float colour_b = 0;

    // colour += 0.2 * vec3(low, high, 0.) * get_colour_rotation(int(cos(time * bps)));

    // float real_low = low/-2.5;
    // if (colour.r + colour.g + colour.b + real_low*3 > 3.0) {
    //     colour.b = 0.5;
    //     colour.g *=2;
    // }

    // if (random(uv.xy) > 0.9999) {
    //     colour = vec3(random(r), random(r), random(r));
    // }

    vec3 colour;
    if (uv.y < 0.3) {
        colour = texture(fractals, gl_FragCoord.xy).rgb;
    } else if (uv.y >= 0.3 && uv.y < 0.6) {
        colour = texture(scrolls, gl_FragCoord.xy).rgb;
    } else if (uv.y >= 0.6) {
        colour = texture(attractors, gl_FragCoord.xy).rgb;
    }

    out_color = vec4(colour, 1.);

    // APPLY MASK
    // out_color.a *= texture(mask, coord).r;
}