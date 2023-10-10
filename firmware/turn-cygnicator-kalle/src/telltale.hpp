#ifndef TELLTALE_H
#define TELLTALE_H

#include <stdint.h>

// Freertos
#include <FreeRTOS.h>
#include "event_groups.h"
#include "portmacro.h"

// cygnicator
#include "cygnicator_headlights.h"

#define FRONT_LEFT_SYNC_BIT (1 << FRONT_LEFT)
#define FRONT_RIGHT_SYNC_BIT (1 << FRONT_RIGHT)
#define REAR_LEFT_SYNC_BIT (1 << REAR_LEFT)
#define REAR_RIGHT_SYNC_BIT (1 << REAR_RIGHT)
#define FRONT_SYNC_BITS (FRONT_LEFT_SYNC_BIT | FRONT_RIGHT_SYNC_BIT)
#define REAR_SYNC_BITS (REAR_LEFT_SYNC_BIT | REAR_RIGHT_SYNC_BIT)
#define LEFT_SYNC_BITS (FRONT_LEFT_SYNC_BIT | REAR_LEFT_SYNC_BIT)
#define RIGHT_SYNC_BITS (FRONT_RIGHT_SYNC_BIT | REAR_RIGHT_SYNC_BIT)
#define ALL_SYNC_BITS (FRONT_SYNC_BITS | REAR_SYNC_BITS)
#define NO_SYNC_BIT 0xFF

uint8_t static const headlights[HEADLIGHT_SIZE_LIMIT][HEADLIGHT_SIZE_LIMIT] = {
    [FRONT_LEFT] = {2, 3, 4, 5},
    [FRONT_RIGHT] = {6, 7, 8, 9},
    [REAR_LEFT] = {10, 11, 12, 13},
    [REAR_RIGHT] = {19, 18, 17, 16},
};

char static const *headlightRowStrings[HEADLIGHT_SIZE_LIMIT] = {
    [FRONT_LEFT] = "Front left",
    [FRONT_RIGHT] = "Front right",
    [REAR_LEFT] = "Rear left",
    [REAR_RIGHT] = "Rear right",
};

enum HeadlightRowAction : uint8_t {
  LEFT_TO_RIGHT, // Pattern sweep LEDs left to right
  RIGHT_TO_LEFT, // Pattern sweep LEDS right to left
  SIMULTANEOUSLY, // Blink all simultaneously
  TOGGLE, // Toggle LED on/off
  NOP, // Do nothing
};

enum TellTaleCmd : uint32_t {
  Left = 0,
  Right = 1,
  Hazard = 2,
  Brake = 3,
};

struct ButtonAction {
  uint32_t gpio;    // GPIO to poll
  TellTaleCmd cmd;  // command to send on high read
};

struct HeadlightRowParameters {
  HeadlightRowAction action;
  EventGroupHandle_t eventGroupHandle = nullptr;
  EventBits_t thisTasksSyncBit = NO_SYNC_BIT;  // Sync bit that is set by task
  EventBits_t allSyncBits =
      NO_SYNC_BIT;  // Sync bits to wait for before task continues
  TickType_t ticksToWaitForSync = 100 / portTICK_PERIOD_MS;  // wait max 10ms
};

struct HeadlightTaskParameters {
  HeadlightRowParameters onHazard;
  HeadlightRowParameters onBrake;
  HeadlightRowParameters onTurnRight;
  HeadlightRowParameters onTurnLeft;
  uint8_t headlightRow;
};

struct ButtonTaskParameters {
  ButtonAction onHazard;
  ButtonAction onBrake;
  ButtonAction onTurnRight;
  ButtonAction onTurnLeft;
};

void handleHeadlightRowAction(uint8_t const *leds,
                                     HeadlightRowAction action);

void handleTelltaleCmd(HeadlightTaskParameters *parameters,
                              TellTaleCmd cmd, uint8_t const *leds);

#endif