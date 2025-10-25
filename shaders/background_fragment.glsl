#version 330 core
in vec2 vUV;
out vec4 FragColor;

void main() {
    vec3 topColor = vec3(0.12, 0.16, 0.27);
    vec3 bottomColor = vec3(0.02, 0.03, 0.07);
    vec3 gradient = mix(bottomColor, topColor, smoothstep(0.0, 1.0, vUV.y));

    float distanceFromGlow = distance(vUV, vec2(0.5, 0.35));
    float glow = smoothstep(0.65, 0.0, distanceFromGlow);
    vec3 glowColor = vec3(0.30, 0.16, 0.46) * glow;

    FragColor = vec4(gradient + glowColor, 1.0);
}
