## Wiring

* Button Up:     NC - GND; NO - R180* - 5V; C - D2
* Button Down:   NC - GND; NO - R820** - 5V; C - D3
* Button Left:   NC - GND; NO - R820** - 5V; C - D4
* Button Right:  NC - GND; NO - R820** - 5V; C - D5
* Button Go:     NC - GND; NO - R820** - 5V; C - D6  
(*) faulty button, need stronger pull-up  
(**) common R820 rail
* Matrix 64RGB:  V+ - 5V; V- - GND; V- - 220uF - V+; In - R330 - D7
* Face2 input:   GND -reeds - D8
* Final LED:     GND -LED - R - D9
## Algo

Program a path up to 8 steps (correct one is 8 steps)

* Init mode: black
  * hidden buttons commands to set brightness (stored in eeprom) and to start game
  * cycle demo modes + black with LEFT+GO. Prevent UP/DOWN on black screen
  * demos: rainbow, heartbeat, labyrinth explorer, bonfire
* Programming mode when input grounded (with reeds from face 2)
  * Hidden reset: maintain LEFT then press GO
  * START Led blinking green with fading
  * Bar Led 1 blinking blue (edition cursor)
  * When moves are programmed, Bar Led turns solid and next one blinking
  * Extra moves above 8 are ignored
  * GO without programmed move: ignored
  * Timeout reset of 10 min of programming mode
* Play mode when GO pressed (and moves programmed)
  * Path Led turns solid, follows moves progressively with smooth fading behind, old path positions stay dimmer
  * Wall: Path becomes blinking red, wrong move in bar becomes red too, and all next moves
  * Wrong end: path blinking red, bar stays blue (no single wrong move to point out)
  * Partial path: path blinking red, next missing move becomes red
* Success mode:
  * Turn on extra Led
  * Animate correct path
  * Rainbow effect after 4 s
  * GO: reset
  * Timeout reset of 10 min
* Fail mode:
  * blink path in red a few times then move to programming mode after 3 s

## Hidden buttons commands

During init phase:
* maintain UP       then press GO  => brightness up   (and store in eeprom)
* maintain DOWN     then press GO  => brightness down (and store in eeprom)
* maintain UP+RIGHT then press GO  => bypass waiting for previous game
* maintain LEFT     then press GO  => cycle through demos

During programmation:
* maintain LEFT     then press GO  => reset

## Remarks

Consumption:
* rainbow: 80mA at smallest intensity, 1000mA(!) at highest intensity.

UP button faulty, very strange, kind of 1-10kohms pull-down (between center and NO), I've put a strong external pull-up of 180 ohms.

Added extra pull-up rail on the other buttons just in case, with 820 ohms.

No pull-up on the start input so far (D8)
