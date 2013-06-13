varying vec3 normal_vector;

uniform sampler1D tf;

void main()
{
  vec3 normalized_normal = (1.0 + normalize(normal_vector)) / 2.0;
  gl_FragColor = vec4(normalized_normal, 1.0);
}
