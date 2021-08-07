#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "tmod/Spaceship.h"
#include "tmod/Seaship.h"
#include "tmod/IShip.h"
#include "tmod/cGetShipFuel.h"

ValeInt tmod_cGetShipFuel(tmod_IShip* sPtr) {
  tmod_IShip s = *sPtr;
  ValeInt result = 0;
  switch (s.type) {
    case tmod_IShip_Type_Seaship: {
      tmod_Seaship* ship = (tmod_Seaship*)s.obj;
      result = ship->leftFuel + ship->rightFuel;
      break;
    }
    case tmod_IShip_Type_Spaceship: {
      tmod_Spaceship* ship = (tmod_Spaceship*)s.obj;
      result = ship->fuel;
      break;
    }
    default:
      exit(1);
  }
  free(s.obj);
  return result;
}
