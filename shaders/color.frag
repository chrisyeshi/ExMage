uniform sampler1D tf;

void main()
{
  vec4 texel = texture1D(tf, gl_TexCoord[0].s);
  gl_FragColor = vec4(texel.rgb, 1.0);
  return;
}
