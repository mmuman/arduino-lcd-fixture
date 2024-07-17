# Arduino LCD panel backlight fixture controller

This allows reusing LCD displays with broken panels as a light fixture, controlled by an Arduino with up to 4 different channels each with different settings. Multiple panels can be controlled from a single channel if needed.

This was designed for the bench lighting of the electronics laboratory at [Fabrico](http://fabrico.fr/).


## LCD display scavenging

From the display we will reuse:

- the panel assembly without the panel itself: the frame, diffusers and LED backlight. Depending on the screen it can be a complicated to remove the panel though.
- the power supply, that we will control with a PWM signal, replacing the controller board. It will be connected to the panel backlight just as it was inside the screen.

### Identifying control pins on the PSU

On the Power Supply Unit (PSU), we will need to identify at least:

- a ground pin,
- the packlight PWM input,
- an Power On / Backlight On signal, to force the PSU to activate.

We might also want to use one of the PSUs to power the Arduino for us. For this we need to find a pin proving power. Ideally 5V, otherwise a step-down regulator will be required from a higher voltage. Either use an always-on PSU, or find the standby powerof another one.

XXX: 5V / 3.3V ?
use a diode to drop 1.7V?

### Known pinouts

#### Dell 24"

#### Others?

Feel free to contribute pinouts you found either by a pull-request or by mail.

## Wiring

For ease of installation and to limit interferences, it is recommended to reuse headphone cables and jacks, which should provide enough shielding.

## Configuration

The Sketch provides a serial interface to help with configuration. It accepts single letter commands followed by a newline:

  1..4  Select output
  l Select low setting
  h Select high setting
  +/- Change selected
  d dump settings
  D toggle debug
  ? for help

To apply a configuration, use the `dump` command, copy-paste the values in the sketch and recompile it.

Note: If you want to maintain it in git you can do so in a commit in a separate branch that you rebase.

## TODO list

- [X] Document Wiring in Fritzing
