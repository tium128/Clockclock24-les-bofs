#include <Wire.h>

#include "board_config.h"
#include "board.h"
#include "clock_state.h"
#include "i2c.h"

const t_clock default_clock = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// int spin_num; //The spin lock number
spin_lock_t *spin_lock[3]; //The spinlock object that will be associated with spin_num

t_half_digit target_clocks_state;
t_half_digit current_clocks_state;

// I2C runs on main core (core 0)
void receiveEvent(int how_many)
{
  Serial.println("Received I2C message");
  Serial.printf("Size: %d bytes\n", how_many);
  
  if (how_many >= sizeof(half_digit))
  {
    t_half_digit tmp_state;
    I2C_readAnything (tmp_state);

    for (uint8_t i = 0; i < 3; i++)
    {
      Serial.printf("Clock %d - Mode H: %d, Mode M: %d, Angle H: %d, Angle M: %d\n", 
        i, 
        tmp_state.clocks[i].mode_h,
        tmp_state.clocks[i].mode_m,
        tmp_state.clocks[i].angle_h,
        tmp_state.clocks[i].angle_m);
        
      spin_lock_unsafe_blocking(spin_lock[i]);
      target_clocks_state.clocks[i] = tmp_state.clocks[i];
      target_clocks_state.change_counter[i] = tmp_state.change_counter[i];
      spin_unlock_unsafe(spin_lock[i]);
    }
  }
}

void setup()
{  
  Serial.begin(115200);
  Serial.println("clockclock24 replica by Vallasc slave v1.0");

  board_begin();
  target_clocks_state = {{default_clock, default_clock, default_clock}, {0, 0, 0}};

  for (uint8_t i = 0; i < 3; i++)
  {
    int spin_num = spin_lock_claim_unused(true); //Claim a free spin lock. If true the function will panic if none are available
    spin_lock[i] = spin_lock_init(spin_num); //Initialise a spin lock
  }

  Wire.setSDA(WIRE_SDA);
  Wire.setSCL(WIRE_SCL);
  Wire.begin(get_i2c_address());
  Wire.onReceive(receiveEvent);
}

void loop()
{
  delay(10);
}

void setup1() 
{
  current_clocks_state = {{default_clock, default_clock, default_clock}, {0, 0, 0}};
}

// Steppers on core 1
void loop1()
{
  board_loop();
  for (uint8_t i = 0; i < 3; i++)
  {
    if(!clock_is_running(i) && current_clocks_state.change_counter[i] != target_clocks_state.change_counter[i])
    {
      Serial.printf("Updating clock %d\n", i);
      
      spin_lock_unsafe_blocking(spin_lock[i]);
      current_clocks_state.clocks[i] = target_clocks_state.clocks[i];
      current_clocks_state.change_counter[i] = target_clocks_state.change_counter[i];
      spin_unlock_unsafe(spin_lock[i]);

      if(current_clocks_state.clocks[i].mode_h == ADJUST_HAND) {
        Serial.printf("Adjusting hour hand for clock %d by %d degrees\n", 
          i, current_clocks_state.clocks[i].adjust_h);
        adjust_h_hand(i, current_clocks_state.clocks[i].adjust_h);
      }

      if(current_clocks_state.clocks[i].mode_m == ADJUST_HAND) {
        Serial.printf("Adjusting minute hand for clock %d by %d degrees\n", 
          i, current_clocks_state.clocks[i].adjust_m);
        adjust_m_hand(i, current_clocks_state.clocks[i].adjust_m);
      }

      if(current_clocks_state.clocks[i].mode_h <= MAX_DISTANCE3) {
        Serial.printf("Setting clock %d to H: %d°, M: %d°\n", 
          i, 
          current_clocks_state.clocks[i].angle_h,
          current_clocks_state.clocks[i].angle_m);
        set_clock(i, current_clocks_state.clocks[i]);
      }
    }
  }
}