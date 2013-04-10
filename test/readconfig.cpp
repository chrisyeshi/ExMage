#include "../ConfigReader.h"

#include <iostream>

int main(void)
{
    ConfigReader& config = ConfigReader::getInstance();
    std::vector<CameraCore> cams = config.GetCameras();
    for (unsigned int i = 0; i < cams.size(); ++i)
    {
        Point pos = cams[i].Position;
        Point foc = cams[i].Focal;
        Vector vup = cams[i].ViewUp;
        std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
        std::cout << foc.x << ", " << foc.y << ", " << foc.z << std::endl;
        std::cout << vup.x << ", " << vup.y << ", " << vup.z << std::endl;
    }

    std::vector<int> ts = config.GetTimeStepRange();
    std::cout << "time step range: " << ts[0] << ", " << ts[1] << std::endl;

    return 0;
}
