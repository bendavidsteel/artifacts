#version 440

uniform ivec2 resolution;
uniform float time;

uniform sampler2DRect last;
uniform sampler2DRect tex;

uniform sampler2DRect mask;
uniform sampler2DRect eye1;
uniform sampler2DRect eye2;
uniform sampler2DRect eye3;

uniform float low;
uniform float high;

out vec4 out_color;

void main(){
    vec2 coord = gl_FragCoord.xy;
    vec2 uv = coord / vec2(resolution);
    
    vec3 colour = vec3(0., 0., 0.);
    colour = texture(tex, coord).rgb;

    int feedback = 1;
    if (feedback == 1) {
        vec2 centre = vec2(0.5, 0.5);
        vec2 from_centre = uv - centre;
        float r = length(from_centre);
        float theta = atan(from_centre.y, from_centre.x);

        float scale = 1 + 0.2 * low;
        r *= scale;

        vec2 scaled_uv = centre + vec2(cos(theta), sin(theta)) * r;
        vec3 scaled_colour = texture(last, scaled_uv * vec2(resolution)).rgb;
        // colour += 0.8 * scaled_colour;
    }

    // colour = 1 - colour;
    out_color = vec4(colour, 1.);

    // APPLY MASK
    //out_color.a *= texture(mask, coord).a;
}