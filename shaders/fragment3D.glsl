#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 ambientColor;
uniform vec3 lightColor;
uniform vec3 cameraPosition;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 viewDir = normalize(cameraPosition - vWorldPos);
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfVector), 0.0), 24.0);

    float rim = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);
    vec3 rimColor = vec3(0.40, 0.35, 0.80) * rim * 0.4;
    vec3 baseColor = vColor * (ambientColor + vec3(0.25));
    vec3 shaded = baseColor + vColor * lightColor * diff + lightColor * 0.20 * spec + rimColor;
    FragColor = vec4(shaded, 1.0);
}
