// Test autonome - fait tourner les 6 moteurs 180° en boucle

#include "board_config.h"
#include "board.h"
#include "clock_state.h"

bool go_to_12h = true;
unsigned long last_change = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("=== SLAVE TEST MODE ===");
  Serial.println("Motors alternate 180° every 3 seconds");
  board_begin();
}

void loop()
{
  board_loop();

  // Attendre que tous les moteurs soient arrêtés + 3 sec
  if (millis() - last_change > 3000 &&
      !clock_is_running(0) && !clock_is_running(1) && !clock_is_running(2))
  {
    last_change = millis();
    int target = go_to_12h ? 90 : 270;
    Serial.printf("-> %d°\n", target);

    t_clock state = {0};
    state.speed_h = 300;
    state.speed_m = 300;
    state.accel_h = 150;
    state.accel_m = 150;
    state.mode_h = MIN_DISTANCE;
    state.mode_m = MIN_DISTANCE;
    state.angle_h = target;
    state.angle_m = target;

    for (int i = 0; i < 3; i++)
      set_clock(i, state);

    go_to_12h = !go_to_12h;
  }
}

void setup1() {}
void loop1() {}
