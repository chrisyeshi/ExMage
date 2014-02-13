#ifndef __VectorFieldReader_h_
#define __VectorFieldReader_h_

#include <vector>
#include <string>

class VectorFieldReader
{
public:
    VectorFieldReader();
    virtual ~VectorFieldReader();

    bool read();
    std::vector<float *> getFields() const {return fields;}

protected:

private:
    std::vector<float *> fields;

    bool readHdf5();
    std::string ext() const;
    std::vector<int> timestepRange() const;
};

#endif
