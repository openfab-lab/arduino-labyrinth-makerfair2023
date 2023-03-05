## Algo

Program a path up to 8 steps (correct one is 8 steps)

* Init undefined (CHANGED => rainbow, hidden buttons commands to set brightness (stored in eeprom) and to start game)
* Start when input grounded (with reeds from face 2)
* All buttons = reset (CHANGED => maintain LEFT then press GO)
* START Led blinking green with fading
* Bar Led 1 blinking blue (edition cursor)
* When moves are programmed, Bar Led turns solid and next one blinking
* undefined extra moves (CHANGED => moves > 8 are ignored)
* Go: Path Led turns solid, follows moves progressively (CHANGED: smooth moves with fading, old path stays dimmer)
* Wall: Path becomes blinking red, wrong move in bar becomes red too (CHANGED => and all next moves)
* Wrong end: path blinking red, 8th move too (CHANGED => no, last stays blue, last move not worse than others)
* Go without move: ignored
* Partial path: same as wrong end (CHANGED => but next missing move becomes red)
* success: turn on extra Led (5V signal to be used to start next game too)
* undefined end (CHANGED => stays in success/fail state and wait for Go. Go => reset game)

## Questions

* success/fail : +timeout for reset ? how much time?
* success: how output is used for next game? 5V if success is ok? Must be on for how long?
* fail: ok with reset or should we move to start_enter_program? (can ppl retry without redoing previous game?)

## Hidden buttons commands

During init phase:
* maintain UP       then press GO  => brightness up   (and store in eeprom)
* maintain DOWN     then press GO  => brightness down (and store in eeprom)
* maintain UP+RIGHT then press GO  => bypass waiting for previous game
* maintain LEFT     then press GO  => reset

During game:
* maintain LEFT     then press GO  => reset

## Remarks

Consumption:
* rainbow: 80mA at smallest intensity, 1000mA(!) at highest intensity.

UP button faulty, very strange, kind of 1-10kohms pull-down (between center and NO), I've put a strong external pull-up of 180 ohms.

Added extra pull-up rail on the other buttons, with 820 ohms.

No pull-up on the start input so far (D8)
