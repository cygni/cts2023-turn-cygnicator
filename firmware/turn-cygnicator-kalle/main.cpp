// FreeRTOS libs
#include <FreeRTOS.h>
#include <inttypes.h>
#include <semphr.h>
#include <stdio.h>
#include <task.h>

#include <climits>

#include "hardware/clocks.h"
#include "pico/stdlib.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

enum GPIO_PINS : uint32_t {
  L_INDCR_BTN = 14,
  R_INDCR_BTN = 15,
  BREAK_BTN = 20,
  HAZARD_BTN = 21,
  BUZZER = 22,
};

enum TellTaleCmd : uint32_t {
  Left = 0,
  Right = 1,
  Hazard = 2,
  Break = 3,
};

enum LEDMatrixIndex : uint32_t {
  FL = 0,
  FR = 1,
  RL = 2,
  RR = 3,
  SIZE_LIMIT,
};

uint32_t ledMatrix[LEDMatrixIndex::SIZE_LIMIT][LEDMatrixIndex::SIZE_LIMIT] = {
    {2, 3, 4, 5},      // FL
    {6, 7, 8, 9},      // FR
    {10, 11, 12, 13},  // RL
    {19, 18, 17, 16},  // RR
};

struct ButtonAction {
  uint32_t gpio;    // GPIO to poll
  TellTaleCmd cmd;  // command to send on interrupt
};

TaskHandle_t hLEDControllerFront;
TaskHandle_t hLEDControllerRear;
TaskHandle_t hLeftButtonHandler;
TaskHandle_t hRightButtonHandler;

TaskHandle_t notifyTasks[] = {hLEDControllerFront,
                              hLEDControllerRear};  // task to send command to

ButtonAction lIndcrAction = {L_INDCR_BTN, TellTaleCmd::Left};
ButtonAction rIndcrAction = {R_INDCR_BTN, TellTaleCmd::Right};
ButtonAction hazardAction = {HAZARD_BTN, TellTaleCmd::Hazard};
ButtonAction breakAction = {BREAK_BTN, TellTaleCmd::Break};

static void tExteriorLight(void *pvParameter) {
  TaskStatus_t xTaskDetails;
  vTaskGetInfo(
      /* Setting xTask to NULL will return information on the calling task.
       */
      NULL, &xTaskDetails, pdFALSE, eInvalid);

  printf("[tExteriorLight:%s]: Started\n", xTaskDetails.pcTaskName);

  uint32_t ulNotifiedValue;
  uint8_t tick = 0;  // start on tock

  while (true) {
    // Wait for notify to continue
    xTaskNotifyWait(0, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

    switch (ulNotifiedValue) {
      case TellTaleCmd::Left:
        printf("[tExteriorLight:%s]: Left %d\n", xTaskDetails.pcTaskName, tick);
        for (auto ledrow : ledMatrix[LEDMatrixIndex::FL]) {
          gpio_put(ledrow, tick);
        }
        break;

      case TellTaleCmd::Right:
        printf("[tExteriorLight:%s]: Right\n", xTaskDetails.pcTaskName);
        for (auto ledrow : ledMatrix[LEDMatrixIndex::FR]) {
          gpio_put(ledrow, tick);
        }
        break;

      case TellTaleCmd::Hazard:
        printf("[tExteriorLight]: Hazard\n");
        break;
    }

    tick = tick ? 0 : 1;
    vTaskDelay(1000);
  }
}

static void tButtonHandler(void *pvParameter) {
  // TaskStatus_t xTaskDetails;
  // vTaskGetInfo(
  //     /* Setting xTask to NULL will return information on the calling task.
  //     */ NULL, &xTaskDetails, pdFALSE, eInvalid);
  printf("[tButtonHandler]: Started\n");

  ButtonAction *btnAction = (ButtonAction *)pvParameter;
  configASSERT(btnAction);

  while (true) {
    if (!gpio_get(btnAction->gpio)) {
      for (auto task : notifyTasks) {
        printf("[tButtonHandler]: Notifying cmd %d\n", btnAction->cmd);
        xTaskNotify(task, static_cast<uint32_t>(btnAction->cmd),
                    eSetValueWithOverwrite);
      }
    }

    vTaskDelay(25);
  }
}

int main(void) {
  stdio_init_all();

  printf("Initialising RPico\n");

  gpio_init(L_INDCR_BTN);
  gpio_set_dir(L_INDCR_BTN, GPIO_IN);
  gpio_pull_up(L_INDCR_BTN);

  gpio_init(R_INDCR_BTN);
  gpio_set_dir(R_INDCR_BTN, GPIO_IN);
  gpio_pull_up(R_INDCR_BTN);

  gpio_init(HAZARD_BTN);
  gpio_set_dir(HAZARD_BTN, GPIO_IN);
  gpio_pull_up(HAZARD_BTN);

  gpio_init(BREAK_BTN);
  gpio_set_dir(BREAK_BTN, GPIO_IN);
  gpio_pull_up(BREAK_BTN);

  for (int i = 0; i < LEDMatrixIndex::SIZE_LIMIT; i++) {
    for (int j = 0; j < LEDMatrixIndex::SIZE_LIMIT; j++) {
      gpio_init(ledMatrix[i][j]);
      gpio_set_dir(ledMatrix[i][j], GPIO_OUT);
    }
  }

  // TODO init GPIOs
  xTaskCreate(tExteriorLight, "Front lights", 256, NULL, 1,
              &hLEDControllerFront);

  xTaskCreate(tExteriorLight, "Rear lights", 256, NULL, 1, &hLEDControllerRear);

  xTaskCreate(tButtonHandler, "Left Button handler", 256, &lIndcrAction, 1,
              &hLeftButtonHandler);
  // xTaskCreate(tButtonHandler, "Right Button handler", 256, &rIndcrAction, 1,
  //             &hRightButtonHandler);

  printf("Finished initialising RPico\n");
  vTaskStartScheduler();
}