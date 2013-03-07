varying vec3 normal_vector;

uniform sampler1D tf;

void main()
{
  vec4 texel = texture1D(tf, gl_TexCoord[0].s);

  vec3 ct, cf;
  float intensity, at, af;
  vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
  intensity = max(dot(lightDir, normalize(normal_vector)), 0.0);
  cf = intensity * vec3(1.0);
  af = 1.0;
  ct = texel.rgb;
  at = texel.a;
  gl_FragColor = vec4(ct * cf, at * af);

  return;
}
