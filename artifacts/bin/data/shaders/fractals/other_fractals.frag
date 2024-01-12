#version 440

#define PI 3.14159265

uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform float time;
uniform int fractal_type;

out vec4 outColor;

vec3 fold(vec3 p0){
    vec3 p = p0;
    if(length(p) > 2.) return p;
    p = mod(p, 2.) - 1.;
    return p;
}

float de1(vec3 p0){
    vec4 p = vec4(p0, 1.);
    float escape = 0.;
    if(p.x > p.z) p.xz = p.zx;
    if(p.z > p.y) p.zy = p.yz;
    if(p.y > p.x) p.yx = p.xy;
    p = abs(p);
    for(int i = 0; i < 8; i++){
        p.xyz = fold(p.xyz);
        p.xyz = fract(p.xyz*0.5 - 1.)*2. - 1.0;
        p *= (1.1 / clamp(dot(p.xyz, p.xyz), -0.1, 1.));
    }
    p /= p.w;
    return abs(p.x) * 0.25;
}

vec3 rotate43(vec3 p,vec3 axis,float theta){
    vec3 v = cross(axis,p), u = cross(v, axis);
    return u * cos(theta) + v * sin(theta) + axis * dot(p, axis);
  }

  vec2 pmod43(vec2 p, float r){
    float a = mod(atan(p.y, p.x), (PI*2) / r) - 0.5 * (PI*2) / r;
    return length(p) * vec2(-sin(a), cos(a));
  }

  float de43(vec3 p){
    for(int i=0;i<5;i++){
      p.xy = pmod43(p.xy,12.0); p.y-=4.0;
      p.yz = pmod43(p.yz,16.0); p.z-=6.8;
    }
    return dot(abs(p),rotate43(normalize(vec3(2,1,3)),
        normalize(vec3(7,1,2)),1.8))-0.3;
  }

// vec3 getNormal(vec3 p) {
//     const float h = 0.001;
//     const vec2 k = vec2(1,-1);
//     return normalize(vec3(de(p + k.xyy*h) - de(p + k.yyx*h),
//                           de(p + k.yxy*h) - de(p + k.xyx*h),
//                           de(p + k.yyx*h) - de(p + k.xxy*h)));
// }

float rayMarch(vec3 ro, vec3 rd, float start, float end) {
    float depth = start;
    for (int i = 0; i < 1000; i++) {
        vec3 p = ro + rd * depth;
        float dist;
        if (fractal_type == 1) {
            dist = de1(p);
        } else if (fractal_type == 2) {
            dist = de43(p);
        }
        if (dist < 0.001) {
            return depth;
        }
        depth += dist;
        if (depth >= end) {
            return end;
        }
    }
    return end;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec3 fragPos = vec3(uv * 2.0 - 1.0, 0.0);
    float timeFactor = 0.1;
    vec3 cameraPos = vec3 (sin(time * timeFactor) * 2., 0., cos(time * timeFactor) * 2.);
    vec3 cameraDir = vec3 (sin(time * timeFactor) * 2., 0., cos(time * timeFactor) * 2.) + cameraPos;
    vec3 ro = cameraPos; 
    // get two perpendicular vectors
    vec3 u = normalize(cross(cameraDir, vec3(0., 1., 0.)));
    vec3 v = normalize(cross(u, cameraDir));
    vec3 rd = normalize((cameraDir + uv.x * u + uv.y * v) - cameraPos);

    vec3 color = vec3(0.0);
    float p = rayMarch(ro, rd, 0.1, 1000.0);
    if (p != vec3(100.0)) {
        color = vec3(p / 1000.0, p / 100., p / 10.);
        // vec3 normal = getNormal(ro + rd * p.x);
        // vec3 lightDir = normalize(lightPos - p);
        // float diff = max(dot(normal, lightDir), 0.0);
        // color = diff * vec3(1.0, 0.5, 0.2);
    }

    outColor = vec4(color, 1.0);
}