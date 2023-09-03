#include <FreeRTOS.h>
#include <inttypes.h>
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

enum GPIO_PINS : uint32_t {
  L_INDCR_BTN = 14,
  R_INDCR_BTN = 15,
  BRAKE_BTN = 20,
  HAZARD_BTN = 21,
  BUZZER = 22,
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

struct HeadlightTaskParameters {
  HeadlightRowAction onHazard;
  HeadlightRowAction onBrake;
  HeadlightRowAction onTurnRight;
  HeadlightRowAction onTurnLeft;
  uint8_t const *leds;
};

struct ButtonTaskParameters {
  ButtonAction onHazard;
  ButtonAction onBrake;
  ButtonAction onTurnRight;
  ButtonAction onTurnLeft;
};

QueueHandle_t mailbox;

TaskHandle_t hLEDControllerFrontRight;
TaskHandle_t hLEDControllerFrontLeft;
TaskHandle_t hLEDControllerRearRight;
TaskHandle_t hLEDControllerRearLeft;
TaskHandle_t hButtonHandler;

TaskHandle_t notifyTasks[] = {hLEDControllerFrontRight, hLEDControllerFrontLeft,
                              hLEDControllerRearRight, hLEDControllerRearLeft};

void play_tone() {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, 8590);
  pwm_set_gpio_level(gpio_speaker, 8192);
}

void stop_tone() { pwm_set_gpio_level(gpio_speaker, 0); }

static void ledRightToLeft(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = 0; i < HeadlightRow::SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, true);
    vTaskDelay(delayTicks);
    gpio_put(led, false);
    vTaskDelay(delayTicks);
  }
}

static void ledLeftToRight(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = HeadlightRow::SIZE_LIMIT - 1; i > 0; i--) {
    uint8_t led = leds[i];
    gpio_put(led, true);
    vTaskDelay(delayTicks);
    gpio_put(led, false);
    vTaskDelay(delayTicks);
  }
}

static void ledHazard(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = 0; i < HeadlightRow::SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, true);
  }
  vTaskDelay(delayTicks);
  for (uint8_t i = 0; i < HeadlightRow::SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, false);
  }
}

// static void ledToggle(uint8_t const *leds) {
//   for (uint8_t i = 0; i < HeadlightRow::SIZE_LIMIT; i++) {
//     uint8_t led = leds[i];
//     gpio_put(led);
//   }
// }

static void handleHeadlightRowAction(uint8_t const *leds,
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

static void handleTelltaleCmd(HeadlightTaskParameters *parameters,
                              TellTaleCmd cmd) {
  switch (cmd) {
    case (TellTaleCmd::Brake): {
      handleHeadlightRowAction(parameters->leds, parameters->onBrake);
      break;
    }
    case (TellTaleCmd::Hazard): {
      handleHeadlightRowAction(parameters->leds, parameters->onHazard);
      break;
    }
    case (TellTaleCmd::Right): {
      handleHeadlightRowAction(parameters->leds, parameters->onTurnRight);
      break;
    }
    case (TellTaleCmd::Left): {
      handleHeadlightRowAction(parameters->leds, parameters->onTurnLeft);
      break;
    }
  }
}

static void tHeadlight(void *parameters) {
  HeadlightTaskParameters *headlightParameters =
      (HeadlightTaskParameters *)(parameters);
  uint8_t const *leds = headlightParameters->leds;
  // printf("[tExteriorLight:%s]: Started\n", xTaskDetails.pcTaskName);

  uint32_t telltaleCmd;

  while (true) {
    // Wait for notify to continue
    // xTaskNotifyWait(0, ULONG_MAX, &telltaleCmd, portMAX_DELAY);
    xQueueReceive(mailbox, &telltaleCmd, portMAX_DELAY);

    handleTelltaleCmd(headlightParameters, (TellTaleCmd)telltaleCmd);

    vTaskDelay(250);
  }
}

static bool readButtonNotify(ButtonAction *action) {
  bool btnPressed = !gpio_get(action->gpio);

  if (btnPressed) {
    for (TaskHandle_t task : notifyTasks) {
      printf("[tButtonHandler]: Notifying cmd %d\n", action->cmd);
      // xTaskNotify(task, static_cast<uint32_t>(action->cmd),
      //             eSetValueWithOverwrite);
      xQueueSend(mailbox, &action->cmd, 0);
    }
  }
  return btnPressed;
}

static void tButtonHandler(void *parameters) {
  ButtonTaskParameters *btnParameters = (ButtonTaskParameters *)parameters;
  // configASSERT(btnAction);

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

  mailbox = xQueueCreate(4, sizeof(uint32_t));

  HeadlightTaskParameters paramsFrontRight = {
      .onHazard = HeadlightRowAction::SIMULTANEOUSLY,
      .onBrake = HeadlightRowAction::NOP,
      .onTurnRight = HeadlightRowAction::LEFT_TO_RIGHT,
      .onTurnLeft = HeadlightRowAction::NOP,
      headlights[FRONT_RIGHT],
  };
  HeadlightTaskParameters paramsFrontLeft = {
      .onHazard = HeadlightRowAction::SIMULTANEOUSLY,
      .onBrake = HeadlightRowAction::NOP,
      .onTurnRight = HeadlightRowAction::NOP,
      .onTurnLeft = HeadlightRowAction::RIGHT_TO_LEFT,
      headlights[FRONT_LEFT],
  };
  HeadlightTaskParameters paramsRearRight = {
      .onHazard = HeadlightRowAction::SIMULTANEOUSLY,
      .onBrake = HeadlightRowAction::SIMULTANEOUSLY,
      .onTurnRight = HeadlightRowAction::LEFT_TO_RIGHT,
      .onTurnLeft = HeadlightRowAction::NOP,
      headlights[REAR_RIGHT],
  };
  HeadlightTaskParameters paramsRearLeft = {
      .onHazard = HeadlightRowAction::SIMULTANEOUSLY,
      .onBrake = HeadlightRowAction::SIMULTANEOUSLY,
      .onTurnRight = HeadlightRowAction::NOP,
      .onTurnLeft = HeadlightRowAction::RIGHT_TO_LEFT,
      headlights[REAR_LEFT],
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
  // xTaskCreate(tButtonHandler, "Right Button handler", 50, &rIndcrAction, 0,
  //             &hRightButtonHandler);
  // xTaskCreate(tButtonHandler, "Hazard Button handler", 50, &hazardAction, 0,
  //             &hHazardButtonHandler);
  // xTaskCreate(tButtonHandler, "Brake Button handler", 50, &brakeAction, 0,
  //             &hBrakeButtonHandler);

  printf("Started");
  vTaskStartScheduler();
}