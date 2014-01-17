#include "Explorable.h"

#include "ConfigReader.h"

//
//
// Constructors / Destructors
//
//

Explorable::Explorable()
{
    tuber.Initialize();
}

Explorable::~Explorable()
{

}

//
//
// Public Methods
//
//

void Explorable::setConfigFile(const std::string& filename)
{
    ConfigReader::setFile(filename);
}

void Explorable::update(const std::vector<float*>& fields)
{
    advector.trace(fields);
    tuber.GenerateTubes(advector.prevParticles(), advector.nextParticles());
}

void Explorable::output()
{
    tuber.Output();
}

//
//
// Protected Methods
//
//

//
//
// Private Methods
//
//