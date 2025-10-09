#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    float iTime;
    vec2 iResolution;
    vec4 iMouse;
    int iFrame;
    int iScene;
    float iTransition;
    float iBass;
    float iMid;
    float iHigh;
    float iIntensity;
    int iKick;
    int iSnare;
} ubo;

#define MAX_STEPS 128
#define MAX_DIST 100.0
#define SURF_DIST 0.001
#define PI 3.14159265359

mat2 rot2D(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c);
}

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = p.x + p.y * 57.0 + 113.0 * p.z;
    return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
                   mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                   mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

float sdSphere(vec3 p, float r) {
    return length(p) - r;
}

float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sdTorus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdMenger(vec3 p) {
    float d = sdBox(p, vec3(1.0));
    float s = 1.0;
    
    for(int m = 0; m < 4; m++) {
        vec3 a = mod(p * s, 2.0) - 1.0;
        s *= 3.0;
        vec3 r = abs(1.0 - 3.0 * abs(a));
        float da = max(r.x, r.y);
        float db = max(r.y, r.z);
        float dc = max(r.z, r.x);
        float c = (min(da, min(db, dc)) - 1.0) / s;
        d = max(d, c);
    }
    
    return d;
}

vec3 plasmaTunnel(vec2 uv, float t) {
    float angle = atan(uv.y, uv.x);
    float radius = length(uv);
    
    float tunnelZ = t * 2.0 + ubo.iBass * 0.8;
    
    vec3 col = vec3(0.0);
    col.r = sin(angle * 3.0 + tunnelZ * 0.5 + ubo.iBass * 2.0) * 0.3 + 0.4;
    col.g = sin(radius * 8.0 - tunnelZ + ubo.iMid * 1.5) * 0.35 + 0.45;
    col.b = sin(angle * 5.0 - radius * 6.0 + tunnelZ * 0.3 + ubo.iHigh * 1.8) * 0.4 + 0.5;
    
    float tunnel = smoothstep(0.8, 0.4, radius);
    col *= tunnel;
    
    col += vec3(0.15, 0.35, 0.7) * smoothstep(0.6, 0.5, radius) * (sin(t * 10.0) * 0.3 + 0.4);
    
    if(ubo.iKick == 1) {
        col += vec3(0.5, 0.15, 0.08) * 0.2;
    }
    
    return col;
}

float sceneParticles(vec3 p) {
    vec3 id = floor(p / 2.0);
    vec3 q = fract(p / 2.0) - 0.5;
    
    float n = noise(id + ubo.iTime * 0.2);
    q.y += sin(ubo.iTime + n * 6.28) * 0.3 * ubo.iMid;
    
    float size = 0.1 + n * 0.1 + ubo.iHigh * 0.2;
    if(ubo.iKick == 1) size *= 1.5;
    
    return sdSphere(q, size);
}

float sceneMenger(vec3 p) {
    p.xz *= rot2D(ubo.iTime * 0.3);
    p.xy *= rot2D(ubo.iTime * 0.2);
    
    float scale = 1.0 + ubo.iMid * 0.5;
    return sdMenger(p / scale) * scale;
}

float sceneMorphing(vec3 p) {
    p.xz *= rot2D(ubo.iTime * 0.4);
    
    float t = fract(ubo.iTime * 0.2);
    int shape = int(ubo.iTime * 0.2) % 3;
    
    float d1, d2;
    
    if(shape == 0) {
        d1 = sdSphere(p, 1.0 + ubo.iBass * 0.3);
        d2 = sdBox(p, vec3(0.8 + ubo.iMid * 0.2));
    } else if(shape == 1) {
        d1 = sdBox(p, vec3(0.8));
        d2 = sdTorus(p, vec2(1.0, 0.3 + ubo.iHigh * 0.2));
    } else {
        d1 = sdTorus(p, vec2(1.0, 0.3));
        d2 = sdSphere(p, 1.0);
    }
    
    return mix(d1, d2, smoothstep(0.0, 1.0, t));
}

float sceneFinal(vec3 p) {
    float d = 1000.0;
    
    for(int i = 0; i < 3; i++) {
        vec3 offset = vec3(
            sin(ubo.iTime + float(i) * 2.0) * 3.0,
            cos(ubo.iTime * 0.7 + float(i)) * 2.0,
            float(i) * 2.0
        );
        
        vec3 q = p - offset;
        q.xy *= rot2D(ubo.iTime * (0.5 + float(i) * 0.2));
        
        float shape = mix(
            sdSphere(q, 0.5 + ubo.iBass * 0.3),
            sdBox(q, vec3(0.4)),
            sin(ubo.iTime + float(i)) * 0.5 + 0.5
        );
        
        d = min(d, shape);
    }
    
    vec3 torPos = p;
    torPos.xz *= rot2D(ubo.iTime * 0.3);
    d = min(d, sdTorus(torPos, vec2(5.0, 0.5 + ubo.iMid * 0.5)));
    
    return d;
}

float getDist(vec3 p) {
    if(ubo.iScene == 1) {
        return sceneMenger(p);
    } else if(ubo.iScene == 2) {
        return sceneParticles(p);
    } else if(ubo.iScene == 3) {
        return sceneMorphing(p);
    } else if(ubo.iScene == 4) {
        return sceneFinal(p);
    }
    
    return 1000.0;
}

float rayMarch(vec3 ro, vec3 rd) {
    float dO = 0.0;
    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dO;
        float dS = getDist(p);
        dO += dS;
        if(dO > MAX_DIST || abs(dS) < SURF_DIST) break;
    }
    return dO;
}

vec3 getNormal(vec3 p) {
    float d = getDist(p);
    vec2 e = vec2(SURF_DIST * 2.0, 0);
    
    vec3 n = d - vec3(
        getDist(p - e.xyy),
        getDist(p - e.yxy),
        getDist(p - e.yyx)
    );
    
    return normalize(n);
}

float getLight(vec3 p) {
    vec3 lightPos = vec3(5.0 + sin(ubo.iTime) * 3.0, 5.0, 5.0 + cos(ubo.iTime) * 3.0);
    vec3 l = normalize(lightPos - p);
    vec3 n = getNormal(p);
    
    float dif = clamp(dot(n, l), 0.0, 1.0);
    
    float d = rayMarch(p + n * SURF_DIST * 2.0, l);
    if(d < length(lightPos - p)) dif *= 0.3;
    
    return dif;
}

vec3 getSceneColor(vec3 p, vec3 rd, float d) {
    vec3 col = vec3(0.0);
    
    if(d < MAX_DIST) {
        float dif = getLight(p);
        vec3 n = getNormal(p);
        
        vec3 baseCol = vec3(0.4, 0.4, 0.5);
        baseCol.r += ubo.iBass * 0.15;
        baseCol.g += ubo.iMid * 0.2;
        baseCol.b += ubo.iHigh * 0.25;
        
        col = baseCol * dif;
        
        col += vec3(0.3, 0.5, 1.0) * pow(max(0.0, dot(n, -rd)), 2.0) * 0.3;
        
        float fresnel = pow(1.0 - max(0.0, dot(n, -rd)), 3.0);
        col += fresnel * vec3(1.0, 0.8, 0.5) * (ubo.iIntensity * 0.3 + 0.1);
        
        if(ubo.iKick == 1) {
            col += vec3(0.8, 0.2, 0.1) * 0.12;
        }
        if(ubo.iSnare == 1) {
            col += vec3(0.2, 0.6, 0.5) * 0.08;
        }
    } else {
        float starField = smoothstep(0.98, 1.0, noise(rd * 100.0));
        col = vec3(0.05, 0.05, 0.1) + starField * vec3(1.0, 0.9, 0.8);
        col += vec3(0.2, 0.1, 0.3) * (1.0 - rd.y) * 0.3;
    }
    
    return col;
}

vec3 postProcess(vec3 col, vec2 uv) {
    float vignette = 1.0 - length(uv) * 0.3;
    col *= vignette;
    
    if(ubo.iKick == 1) {
        col += vec3(0.6, 0.3, 0.15) * 0.05 * (1.0 - length(uv));
    }
    
    col += noise(vec3(uv * 100.0, ubo.iTime)) * 0.02;
    
    col = pow(col, vec3(0.4545));
    
    return col;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * ubo.iResolution.xy) / ubo.iResolution.y;
    
    vec3 col = vec3(0.0);
    
    if(ubo.iScene == 0) {
        col = plasmaTunnel(uv, ubo.iTime);
    } else {
        vec3 ro = vec3(0.0, 0.0, -5.0 - ubo.iIntensity * 2.0);
        
        if(ubo.iScene == 2) {
            ro.y += 1.0;
        } else if(ubo.iScene == 4) {
            ro = vec3(
                sin(ubo.iTime * 0.3) * 8.0,
                sin(ubo.iTime * 0.2) * 3.0 + 2.0,
                cos(ubo.iTime * 0.3) * 8.0
            );
        } else {
            ro.xz *= rot2D(ubo.iTime * 0.2);
        }
        
        vec3 target = vec3(0.0);
        vec3 forward = normalize(target - ro);
        vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
        vec3 up = cross(forward, right);
        
        float fov = 1.0 + ubo.iBass * 0.3;
        vec3 rd = normalize(forward * fov + uv.x * right + uv.y * up);
        
        float d = rayMarch(ro, rd);
        vec3 p = ro + rd * d;
        
        col = getSceneColor(p, rd, d);
    }
    
    col = postProcess(col, uv);
    
    col = mix(vec3(0.0), col, ubo.iTransition);
    
    outColor = vec4(col, 1.0);
}
