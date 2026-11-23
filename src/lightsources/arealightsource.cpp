#include "arealightsource.h"
#include <random>


AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::generateRandomPoint() const
{
    //required to be float otw we would be breaking the direction
    float u = (float)rand() / RAND_MAX; // they are integers. Divide by max to have range [0,1]
    float v = (float)rand() / RAND_MAX;

    return myAreaLightsource->corner + (myAreaLightsource->v1 * u) + (myAreaLightsource->v2 * v);
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    //FILL(...)
    // 
    //New Randam Pos inside Area Lightsource
    return generateRandomPoint();
}

