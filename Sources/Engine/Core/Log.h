#pragma once

#include <iostream>

#include "Platform.h"

namespace Eugenix
{

}

#if EUGENIX_DEBUG
#define EUGENIX_LOG(n) std::cout << "INFO: " << n << std::endl;
#define EUGENIX_ERROR(n) std::cout << "ERROR: " << n << std::endl;
#else 
#define EUGENIX_LOG(n)
#define EUGENIX_ERROR(n)
#endif