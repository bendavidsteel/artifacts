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
    vec3 lastColour = texture(last1, gl_FragCoord.xy).rgb;
    vec3 lastUpColour = texture(last1, gl_FragCoord.xy + sortDir).rgb;
    vec3 lastDownColour = texture(last1, gl_FragCoord.xy - sortDir).rgb;

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

    // r *= 1. + pow(bass_boost, 3.);

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

    // r *= 1 - 0.2 * bass_boost;

    // if (isOnset == 1) {
    //     theta += 0.1 * (2. * random(time) - 1.);
    // }

    // from_centre = vec2(cos(theta), sin(theta)) * r;
    // uv = centre + from_centre;
    // coord = uv * resolution.xy;

    // float pi = 3.141592;

    // // float frac_a = 0.4 + 0.1 * sin(bps * time + uv.x) + 0.1 * bass_boost;
    // // float frac_b = 0.5 + 0.1 * sin(bps * time + uv.x + 1) + 0.1 * bass_boost;
    // float frac_rate = 10.;
    // float wave_size = 2. * bass_boost;
    // // float frac_a = -1 * pi * 0.3 + wave_size * sin(frac_rate * r + time * bps * bass_boost);
    // // float frac_b = 1. * pi * 0.3 + wave_size * sin(frac_rate * r + time * bps * bass_boost);
    // // float frac_o = -1. * pi + wave_size * sin(frac_rate * r + time * bps * bass_boost);
    // // float frac_c = frac_o + 2 * pi;
    // float frac_a = 0.4 + 0.2 * sin(bps * time + uv.x) + 0.1 * bass_boost;
    // float frac_b = 0.5 + 0.2 * sin(bps * time + uv.x + 1) + 0.1 * bass_boost;
    // float frac_o = 0.3 + 0.2 * sin(bps * time + uv.x + 2) + 0.1 * bass_boost;
    // float frac_c = 0.6 + 0.2 * sin(bps * time + uv.x + 3) + 0.1 * bass_boost;

    // int centrepiece = int(1 + (0.5 + 0.5 * sin(bps * time / 3)) * 3);
    // float threshold = uv.y;

    // vec3 colour;
    // if (threshold >= frac_o && threshold < frac_a) {
    //     if (centrepiece == 1) {
    //         colour = texture(fractals, coord).rgb;
    //     } else if (centrepiece == 2) {
    //         colour = texture(attractors, coord).rgb;
    //     } else if (centrepiece == 3) {
    //         colour = get_scrolls(coord);
    //     }
        
    // } else if (threshold >= frac_a && threshold < frac_b) {
    //     if (centrepiece == 3) {
    //         colour = texture(fractals, coord).rgb;
    //     } else if (centrepiece == 1) {
    //         colour = texture(attractors, coord).rgb;
    //     } else if (centrepiece == 2) {
    //         colour = get_scrolls(coord);
    //     }
    // } else if (threshold >= frac_b && threshold < frac_c) {
    //     if (centrepiece == 2) {
    //         colour = texture(fractals, coord).rgb;
    //     } else if (centrepiece == 3) {
    //         colour = texture(attractors, coord).rgb;
    //     } else if (centrepiece == 1) {
    //         colour = get_scrolls(coord);
    //     }
    // }

    // vec3 eyeColour = texture(eye, coord).rgb;
    // vec3 eyeMix = eyeColour.b * vec3(1., 0.2, 1.);
    // if (bass > 0.5) {
    //     colour *= 2. * eyeMix;
    // }

    // if (sortPixels == 1) {
    //     colour = pixelSorter();
    // }

    // // colour = texture(camera, gl_FragCoord.xy).rbb;

    // colour = get_colour_rotation(int(2 * time * bps)) * colour;

    // float degs = theta * 180 / 3.14156;
    // float thetaBlock = float(int(degs / 10. * random(time)));
    // if (random(thetaBlock * time) < bass_boost) {
    //     colour = 3. * colour;
    // }

    // out_color = vec4(colour, 1.);

    // // APPLY MASK
    // // out_color.a *= texture(mask, coord).a;
    // float mask_alpha = texture(mask, coord).a;
    // if (mask_alpha > 0.) {
    //     out_color.rgb = 1 - out_color.rgb;
    // }

    // for (int i = -10; i < 10; i++) {
    //     for (int j = -10; j < 10; j++) {
    //         vec2 coord2 = coord + vec2(i, j);
    //         float mask2 = texture(mask, coord2).a;
    //         if (mask2 > 0.) {
    //             vec3 last_colour = texture(last, coord2).rgb;
    //             out_color.rgb += length(vec2(i, j)) * 0.001 * last_colour;
    //         }
    //     }
    // }

    if (beat == 1) {
        out_color.rgb = texture(fractals, coord).rgb;
    }
    out_color.rgb = texture(scrolls, coord).rgb;
    out_color.a = 1.;
}