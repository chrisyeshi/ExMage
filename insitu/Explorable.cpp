#include "Explorable.h"

#include "ConfigReader.h"
#include "mpi.h"
#include "GlobalComposite.h"
#include "DomainInfo.h"

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
#ifdef STATUS_TEXT
    static int progress = 0;
    std::cout << "ExMage: (Rank " << DomainInfo::myRank() << ") updated (Progress " << ++progress << ")" << std::endl;
#endif
}

void Explorable::output()
{
    tuber.Output();
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