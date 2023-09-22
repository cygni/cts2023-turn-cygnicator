#include "cygnicator_headlights.h"

#include "telltale.hpp"
#include "buzzer.hpp"
#include "led.hpp"

void handleHeadlightRowAction(uint8_t const *leds,
                                     HeadlightRowAction action) {
  switch (action) {
    case (HeadlightRowAction::RIGHT_TO_LEFT): {
      play_tone();
      ledRightToLeft(leds, pdMS_TO_TICKS(25));
      stop_tone();
      break;
    }
    case (HeadlightRowAction::LEFT_TO_RIGHT): {
      play_tone();
      ledLeftToRight(leds, pdMS_TO_TICKS(25));
      stop_tone();
      break;
    }
    case (HeadlightRowAction::SIMULTANEOUSLY): {
      ledHazard(leds, pdMS_TO_TICKS(250));
      break;
    }
    case (HeadlightRowAction::TOGGLE): {
      ledHazard(leds, pdMS_TO_TICKS(250));
      break;
    }
    case (HeadlightRowAction::NOP):
    default: {
      // Do nothing
    }
  }
}

static inline bool waitForSync(HeadlightRowParameters *parameters) {
  if (parameters->eventGroupHandle == nullptr) {
    return false;
  }
  EventBits_t uxReturn = xEventGroupSync(parameters->eventGroupHandle, parameters->thisTasksSyncBit,
                  parameters->allSyncBits, parameters->ticksToWaitForSync);
  return ((uxReturn & parameters->allSyncBits) == parameters->allSyncBits);
}

void handleTelltaleCmd(HeadlightTaskParameters *parameters,
                              TellTaleCmd cmd, uint8_t const *leds) {
  switch (cmd) {
    case (TellTaleCmd::Brake): {
      if (!waitForSync(&parameters->onBrake)) {
        return;
      }
      handleHeadlightRowAction(leds, parameters->onBrake.action);
      break;
    }
    case (TellTaleCmd::Hazard): {
      if (!waitForSync(&parameters->onHazard)) {
        return;
      }
      handleHeadlightRowAction(leds, parameters->onHazard.action);
      break;
    }
    case (TellTaleCmd::Right): {
      if (!waitForSync(&parameters->onTurnRight)) {
        return;
      }
      handleHeadlightRowAction(leds, parameters->onTurnRight.action);
      break;
    }
    case (TellTaleCmd::Left): {
      if (!waitForSync(&parameters->onTurnLeft)) {
        return;
      }
      handleHeadlightRowAction(leds, parameters->onTurnLeft.action);
      break;
    }
  }
}