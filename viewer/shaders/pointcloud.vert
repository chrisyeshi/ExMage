#version 120

uniform sampler1D tf;
uniform vec3 lightDir;

void main()
{
    // vertex position in view coordinate
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    // vertex color by sampling transfer function
    vec4 texel = texture1D(tf, gl_MultiTexCoord0.x);
    // lighting
    vec3 nLightDir = normalize(lightDir);
    vec3 normal = normalize(gl_Normal);
    float intensity = max(dot(nLightDir, normal), 0.0);
    vec3 cf = intensity * vec3(1.0) + vec3(0.3);
    float af = 1.0;
    // combine
    vec4 color = vec4(texel.rgb * cf, texel.a * af);
    // output color
    gl_FrontColor = color;
}
