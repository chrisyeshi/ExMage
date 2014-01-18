#include "Explorable.h"

#include "ConfigReader.h"
#include "mpi.h"
#include "GlobalComposite.h"

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
    // tuber.Output();
    GlobalComposite gc;
    gc.gather(tuber.getFrame(0));
    gc.composite();
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