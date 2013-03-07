varying vec3 normal_vector;

void main()
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  normal_vector = normalize(gl_NormalMatrix * gl_Normal);
  gl_Position = ftransform();
}
