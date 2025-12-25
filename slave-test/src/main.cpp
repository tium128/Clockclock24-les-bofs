// Test autonome - fait tourner les 6 moteurs 180° en boucle

#include "board_config.h"
#include "board.h"
#include "clock_state.h"

bool go_to_12h = true;
bool waiting = false;
unsigned long wait_start = 0;

void move_all_to(int angle, int direction)
{
  t_clock state = {0};
  state.speed_h = 300;
  state.speed_m = 300;
  state.accel_h = 150;
  state.accel_m = 150;
  state.mode_h = direction;
  state.mode_m = direction;
  state.angle_h = angle;
  state.angle_m = angle;

  for (int i = 0; i < 3; i++)
    set_clock(i, state);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("=== SLAVE TEST MODE ===");
  board_begin();

  // Premier mouvement vers 12h (sens horaire)
  Serial.println("-> 90 (12h) CW");
  move_all_to(90, CLOCKWISE);
  go_to_12h = false;
}

void loop()
{
  board_loop();

  bool all_stopped = !clock_is_running(0) && !clock_is_running(1) && !clock_is_running(2);

  if (!waiting && all_stopped)
  {
    // Moteurs arrêtés, démarrer le timer de 3 sec
    waiting = true;
    wait_start = millis();
  }
  else if (waiting && millis() - wait_start > 3000)
  {
    // 3 sec écoulées, prochain mouvement
    waiting = false;
    int target = go_to_12h ? 90 : 270;
    int dir = go_to_12h ? CLOCKWISE : COUNTERCLOCKWISE;
    Serial.printf("-> %d° (%s) %s\n", target, go_to_12h ? "12h" : "6h", go_to_12h ? "CW" : "CCW");
    move_all_to(target, dir);
    go_to_12h = !go_to_12h;
  }
}

void setup1() {}
void loop1() {}
