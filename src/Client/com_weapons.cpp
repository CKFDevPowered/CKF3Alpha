#include <stdarg.h>
#include "hud.h"
#include "cl_util.h"
#include "com_weapons.h"

#include "const.h"
#include "entity_state.h"
#include "r_efx.h"

int g_runfuncs = 0;
struct local_state_s *g_finalstate = NULL;