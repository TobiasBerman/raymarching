#version 330 core

precision highp float;

out vec4 FragColor;
in vec4 gl_FragCoord;

uniform float xSize;
uniform float ySize;
uniform vec3 camPos;
uniform float yaw;
uniform float pitch;
uniform float time;

vec2 sdfMandelbulb(vec3 p) {
    vec3 z = p; // Try setting z to p*sin(dim1) for a fun effect
    float dr = 1.0;
    float r = 0.0;

    const int ITERATIONS = 20; // futile to increase iterations past raymarch and normal precision (bottlenecked by that)
    float POWER = 10.0;
    const float BAILOUT = 2.8;

    int i = 0;
    for (; i < ITERATIONS; ++i) {
        r = length(z);
        if (r > BAILOUT) break;

        // Avoid division by zero
        float r_safe = max(r, 0.00001);
        float theta = acos(clamp(z.z / r_safe, -1.0, 1.0));
        float phi = atan(z.y, z.x); // Change phi to atan(z.y, z.x*z.y) for another interesting effect
        float zr = pow(r_safe, POWER);

        // Update derivative radius
        dr = pow(r_safe, POWER - 1.0) * POWER * dr + 1.0;

        // New point in fractal space
        float sinTheta = sin(theta * POWER);
        z = zr * vec3(
            sinTheta * cos(phi * POWER),
            sinTheta * sin(phi * POWER),
            cos(theta * POWER)
        );

        z += p;
    }

    r = max(r, 0.0001); // prevent log(0)
    return vec2(0.5 * log(r) * r / dr, i);
}

vec2 sdfScene(vec3 point) {
    return sdfMandelbulb(point);
}

vec3 estimateNormals(vec3 p) {
    const float EPSILON = 0.0000007; // Lower epsilon = more accurate normals
    
    // Estimating the gradient of the sdf
    float scene = sdfScene(vec3(p.x, p.y, p.z)).x;
    return normalize(vec3(
        sdfScene(vec3(p.x + EPSILON, p.y, p.z)).x - scene,
        sdfScene(vec3(p.x, p.y + EPSILON, p.z)).x - scene,
        sdfScene(vec3(p.x, p.y, p.z  + EPSILON)).x - scene
    ));
}

vec2 rayMarch(vec3 rayOrigin, vec3 rayDir) {
    vec2 totalDist = vec2(0.0, 0.0);
    
    for (int i = 0; i < 2048; ++i) {
        vec3 queryPoint = rayOrigin + rayDir * totalDist.x;
        
        vec2 info = sdfScene(queryPoint);
        float distToScene = info.x;
        if (distToScene < 0.002) break; // Improves detail of render, significantly at the cost of performance
        totalDist.x += distToScene;
        totalDist.y = info.y;
        
        if (totalDist.x > 32.0) break;
    }
    
    return totalDist;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(xSize, ySize);
    uv = uv - vec2(0.5, 0.5);  // Makes center of screen (0,0) instead of bottom left
    uv.x = uv.x * (xSize / ySize); // Scale normally
    
    vec3 rayOrigin = camPos;
    float z = uv.x * sin(yaw) + cos(yaw);
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
    vec3 rayDir = vec3(uv.x, uv.y * cosPitch - sinPitch, uv.y * sinPitch + cosPitch);
    rayDir = vec3(rayDir.x * cosYaw - rayDir.z * sinYaw, rayDir.y, rayDir.x * sinYaw + rayDir.z * cosYaw);
    rayDir = normalize(rayDir);
    
    vec2 info = rayMarch(rayOrigin, rayDir);
    float marchedDist = info.x;
    vec3 hitPoint = rayOrigin + rayDir*marchedDist;
    float intensity = dot(-rayDir, normalize(estimateNormals(hitPoint))); // Calculated based on normals
    float dist = distance(vec3(0.0), hitPoint);
    vec3 color = vec3(1) - vec3(sin(40/(dist * 0.5 + 0.5 * info.y)), cos((intensity)/(dist)), log(20/(dist)) );
    if (marchedDist > 16.0) color = vec3(1.0, 1.0, 1.0); // Make background white
    
    FragColor = vec4(color, 1.0);
}
