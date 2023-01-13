#ifndef intl_h
#define intl_h

#if defined(INTL_EN)
#include "intl_en.h"
#elif defined(INTL_FR)
#include "intl_fr.h"
#else
#warning No language defined
#include "intl_en.h"
#endif

#endif
