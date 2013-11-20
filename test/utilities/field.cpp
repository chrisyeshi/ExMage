#include "Field.h"

int main(void)
{
	float unitField[] = { 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f };
	// DataLoc::Vertex
	Field<float> field(unitField, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(fabs(field.interpolate(Vector<3, float>(0.f, 0.f, 0.f)) - 0.f) < 0.0001);

	field.setBuffer(unitField);
	field.setDimension(Vector<3, int>(2, 2, 2));
	assert(fabs(field.interpolate(Vector<3, float>(1.0, 1.0, 1.0)) - 1.f) < 0.0001);

	field.set(unitField, Vector<3, int>(2, 2, 2), Field<float>::Vertex);
	assert(fabs(field.interpolate(Vector<3, float>(0.5, 0.5, 0.5)) - 0.5) < 0.0001);

	// DataLoc::Center
	field.set(unitField, Vector<3, int>(2, 2, 2), Field<float>::Center);
	assert(fabs(field.interpolate(Vector<3, float>(0.0, 0.0, 0.0)) - 0.0) < 0.0001);
	assert(fabs(field.interpolate(Vector<3, float>(2.0, 2.0, 2.0)) - 1.0) < 0.0001);
	assert(fabs(field.interpolate(Vector<3, double>(2.0, 2.0, 2.0)) - 1.0) < 0.0001);

	return 0;
}