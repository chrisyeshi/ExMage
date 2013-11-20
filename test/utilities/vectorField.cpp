#include "VectorField.h"

int main(void)
{
	float x[] = { 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f };
	float y[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float z[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float s[] = { 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f };

	VectorField<float> flow;
	std::vector<float> scalars, entry;

	std::vector<Field<float> > fields(4);
	fields[0].set(x, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	fields[1].set(y, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	fields[2].set(z, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	fields[3].set(s, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	flow.set(fields);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "1" << std::endl;

	std::vector<float*> buffer(4);
	buffer[0] = x; buffer[1] = y; buffer[2] = z; buffer[3] = s;
	flow.set(buffer, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "2" << std::endl;

	float *raw[4];
	raw[0] = x; raw[1] = y; raw[2] = z; raw[3] = s;
	flow.set(raw, 4, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "3" << std::endl;

	std::vector<Field<float> > vFields(3);
	vFields[0].set(x, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	vFields[1].set(y, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	vFields[2].set(z, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	std::vector<Field<float> > sFields(1);
	sFields[0].set(s, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	flow.setVelocities(vFields);
	flow.setScalars(sFields);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "4" << std::endl;

	std::vector<float*> vBuffers(3);
	vBuffers[0] = x; vBuffers[1] = y; vBuffers[2] = z;
	std::vector<float*> sBuffers(1);
	sBuffers[0] = s;
	flow.setVelocities(vBuffers, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	flow.setScalars(sBuffers, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "5" << std::endl;

	float *vRaws[3];
	vRaws[0] = x; vRaws[1] = y; vRaws[2] = z;
	float *sRaws[1];
	sRaws[0] = s;
	flow.setVelocities(vRaws, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	flow.setScalars(sRaws, 1, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(flow.getVelocity(Vector<>(0.5, 0.5, 0.5)) == Vector<>(0.5, 0.f, 0.f));
	scalars = flow.getScalars(Vector<>(0.5, 0.5, 0.5));
	assert(scalars.size() == 1);
	entry = flow.get(Vector<>(0.5, 0.5, 0.5));
	assert(entry.size() == 4);
	assert(fabs(entry[0] - 0.5) < 0.0001);
	assert(fabs(entry[1] - 0.f) < 0.0001);
	assert(fabs(entry[2] - 0.f) < 0.0001);
	assert(fabs(entry[3] - scalars[0]) < 0.0001);
	assert(fabs(entry[3] - 0.5) < 0.0001);
	std::cout << "6" << std::endl;

	flow.addScalar(s, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(flow.getScalars(Vector<>(0.5, 0.5, 0.5)).size() == 2);

	return 0;
}