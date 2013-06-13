varying vec3 normal_vector;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	normal_vector = normalize(gl_NormalMatrix * gl_Normal);
	gl_Position = ftransform();
}
