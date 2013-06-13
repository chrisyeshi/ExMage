void main()
{
  gl_TexCoord[2] = gl_MultiTexCoord2;
  gl_Position = ftransform();
  return;
}
