#include <FreeRTOS.h>
#include <event_groups.h>
#include <inttypes.h>
#include <queue.h>
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

#include <climits>

#include "cygnicator_gpio.h"
#include "cygnicator_headlights.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include "portmacro.h"

#include "led.hpp"
#include "telltale.hpp"
#include "buzzer.hpp"

enum GPIO_PINS : uint32_t {
  L_INDCR_BTN = 14,
  R_INDCR_BTN = 15,
  BRAKE_BTN = 20,
  HAZARD_BTN = 21,
  BUZZER = 22,
};

QueueHandle_t mailbox;

TaskHandle_t hLEDControllerFrontRight;
TaskHandle_t hLEDControllerFrontLeft;
TaskHandle_t hLEDControllerRearRight;
TaskHandle_t hLEDControllerRearLeft;
TaskHandle_t hButtonHandler;

TaskHandle_t notifyTasks[] = {hLEDControllerFrontRight, hLEDControllerFrontLeft,
                              hLEDControllerRearRight, hLEDControllerRearLeft};

static void tHeadlight(void *parameters) {
  HeadlightTaskParameters *headlightParameters =
      (HeadlightTaskParameters *)(parameters);

  uint32_t telltaleCmd;
  uint8_t const *leds = headlights[headlightParameters->headlightRow];

  while (true) {
    xQueuePeek(mailbox, &telltaleCmd, portMAX_DELAY);

    handleTelltaleCmd(headlightParameters, (TellTaleCmd)telltaleCmd, leds);

    vTaskDelay(250);
  }
}

static bool readButtonNotify(ButtonAction *action) {
  bool btnPressed = !gpio_get(action->gpio);

  if (btnPressed) {
    for (TaskHandle_t task : notifyTasks) {
      printf("[tButtonHandler]: Notifying cmd %d\n", action->cmd);
      xQueueOverwrite(mailbox, &action->cmd);
    }
  }
  return btnPressed;
}

static void tButtonHandler(void *parameters) {
  ButtonTaskParameters *btnParameters = (ButtonTaskParameters *)parameters;

  while (true) {
    vTaskDelay(1);

    // Continue to next cycle if one event is triggered.
    // This prevents multiple events in the mailbox at the same time.
    if (readButtonNotify(&btnParameters->onHazard)) {
      continue;
    }

    if (readButtonNotify(&btnParameters->onTurnRight)) {
      continue;
    }

    if (readButtonNotify(&btnParameters->onTurnLeft)) {
      continue;
    }

    if (readButtonNotify(&btnParameters->onBrake)) {
      // TODO send event if brake off
      continue;
    }
  }
}

int main(void) {
  stdio_init_all();

  gpio_init_mask(gpio_output_pins_mask);
  gpio_init_mask(gpio_input_pins_mask);
  gpio_set_dir_out_masked(gpio_output_pins_mask);
  gpio_set_dir_in_masked(gpio_input_pins_mask);

  gpio_set_function(gpio_speaker, GPIO_FUNC_PWM);
  uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_enabled(slice_num, true);

  // say hello, so we know the program is running
  play_tone();
  gpio_set_mask(gpio_output_pins_mask);
  gpio_set_mask(gpio_input_pins_mask);
  sleep_ms(1000);
  stop_tone();
  gpio_clr_mask(gpio_input_pins_mask);
  gpio_clr_mask(gpio_output_pins_mask);

  gpio_init(L_INDCR_BTN);
  gpio_init(R_INDCR_BTN);
  gpio_init(HAZARD_BTN);
  gpio_init(BRAKE_BTN);
  gpio_set_dir(L_INDCR_BTN, GPIO_IN);
  gpio_set_dir(R_INDCR_BTN, GPIO_IN);
  gpio_set_dir(HAZARD_BTN, GPIO_IN);
  gpio_set_dir(BRAKE_BTN, GPIO_IN);
  gpio_pull_up(L_INDCR_BTN);
  gpio_pull_up(R_INDCR_BTN);
  gpio_pull_up(HAZARD_BTN);
  gpio_pull_up(BRAKE_BTN);

  mailbox = xQueueCreate(1, sizeof(uint32_t));

  EventGroupHandle_t onHazardEventGroup = xEventGroupCreate();
  EventGroupHandle_t onBrakeEventGroup = xEventGroupCreate();
  EventGroupHandle_t onTurnLeftEventGroup = xEventGroupCreate();
  EventGroupHandle_t onTurnRightEventGroup = xEventGroupCreate();

  HeadlightTaskParameters paramsFrontRight = {
      .onHazard = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                   .eventGroupHandle = onHazardEventGroup,
                   .thisTasksSyncBit = FRONT_RIGHT_SYNC_BIT,
                   .allSyncBits = ALL_SYNC_BITS},
      .onBrake = {.action = HeadlightRowAction::NOP},
      .onTurnRight = {.action = HeadlightRowAction::LEFT_TO_RIGHT,
                      .eventGroupHandle = onTurnRightEventGroup,
                      .thisTasksSyncBit = FRONT_RIGHT_SYNC_BIT,
                      .allSyncBits = RIGHT_SYNC_BITS},
      .onTurnLeft = {.action = HeadlightRowAction::NOP},
      FRONT_RIGHT,
  };
  HeadlightTaskParameters paramsFrontLeft = {
      .onHazard = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                   .eventGroupHandle = onHazardEventGroup,
                   .thisTasksSyncBit = FRONT_LEFT_SYNC_BIT,
                   .allSyncBits = ALL_SYNC_BITS},
      .onBrake = {.action = HeadlightRowAction::NOP},
      .onTurnRight = {.action = HeadlightRowAction::NOP,
                      .eventGroupHandle = onTurnRightEventGroup},
      .onTurnLeft = {.action = HeadlightRowAction::RIGHT_TO_LEFT,
                     .eventGroupHandle = onTurnLeftEventGroup,
                     .thisTasksSyncBit = FRONT_LEFT_SYNC_BIT,
                     .allSyncBits = LEFT_SYNC_BITS},
      FRONT_LEFT,
  };
  HeadlightTaskParameters paramsRearRight = {
      .onHazard = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                   .eventGroupHandle = onHazardEventGroup,
                   .thisTasksSyncBit = REAR_RIGHT_SYNC_BIT,
                   .allSyncBits = ALL_SYNC_BITS},
      .onBrake = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                  .eventGroupHandle = onBrakeEventGroup,
                  .thisTasksSyncBit = REAR_RIGHT_SYNC_BIT,
                  .allSyncBits = REAR_SYNC_BITS},
      .onTurnRight = {.action = HeadlightRowAction::LEFT_TO_RIGHT,
                      .eventGroupHandle = onTurnRightEventGroup,
                      .thisTasksSyncBit = REAR_RIGHT_SYNC_BIT,
                      .allSyncBits = RIGHT_SYNC_BITS},
      .onTurnLeft = {.action = HeadlightRowAction::NOP},
      REAR_RIGHT,
  };
  HeadlightTaskParameters paramsRearLeft = {
      .onHazard = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                   .eventGroupHandle = onHazardEventGroup,
                   .thisTasksSyncBit = REAR_LEFT_SYNC_BIT,
                   .allSyncBits = ALL_SYNC_BITS},
      .onBrake = {.action = HeadlightRowAction::SIMULTANEOUSLY,
                  .eventGroupHandle = onBrakeEventGroup,
                  .thisTasksSyncBit = REAR_LEFT_SYNC_BIT,
                  .allSyncBits = REAR_SYNC_BITS},
      .onTurnRight = {.action = HeadlightRowAction::NOP},
      .onTurnLeft = {.action = HeadlightRowAction::RIGHT_TO_LEFT,
                     .eventGroupHandle = onTurnLeftEventGroup,
                     .thisTasksSyncBit = REAR_LEFT_SYNC_BIT,
                     .allSyncBits = LEFT_SYNC_BITS},
      REAR_LEFT,
  };
  ButtonTaskParameters paramsButton = {
      .onHazard = {HAZARD_BTN, TellTaleCmd::Hazard},
      .onBrake = {BRAKE_BTN, TellTaleCmd::Brake},
      .onTurnRight = {R_INDCR_BTN, TellTaleCmd::Right},
      .onTurnLeft = {L_INDCR_BTN, TellTaleCmd::Left},
  };

  xTaskCreate(tHeadlight, headlightRowStrings[FRONT_RIGHT],
              configMINIMAL_STACK_SIZE, (void *)&paramsFrontRight, 0,
              &hLEDControllerFrontRight);
  xTaskCreate(tHeadlight, headlightRowStrings[FRONT_LEFT],
              configMINIMAL_STACK_SIZE, (void *)&paramsFrontLeft, 0,
              &hLEDControllerFrontLeft);
  xTaskCreate(tHeadlight, headlightRowStrings[REAR_LEFT],
              configMINIMAL_STACK_SIZE, (void *)&paramsRearLeft, 0,
              &hLEDControllerRearRight);
  xTaskCreate(tHeadlight, headlightRowStrings[REAR_RIGHT],
              configMINIMAL_STACK_SIZE, (void *)&paramsRearRight, 0,
              &hLEDControllerRearLeft);

  xTaskCreate(tButtonHandler, "Buttonhandlertask", configMINIMAL_STACK_SIZE,
              &paramsButton, 0, &hButtonHandler);

  printf("Started");
  vTaskStartScheduler();
}