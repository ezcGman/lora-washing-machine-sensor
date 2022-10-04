![The PCB](https://github.com/ezcGman/lora-washing-machine-sensor/blob/master/pictures/Washing-Machine-Sensor-v1r1-render-front.png?raw=true)

# LoRA Washing Machine & Dryer Sensor
A small ESP8266 & RFM95W based sensor board that utilizes CT clamps to measuer your washing machine and dryers power consumption and report the reading via LoRa, e.g. to my [LoRa-To-MQTT-Gateway](https://github.com/ezcGman/lora-gateway)! Using LoRa enables you to also get reading froma washing machine hidden in the far end of a basement and report the readings multiple floors up to your apartment that hosts the LoRa gateway (and probably your Home Assistant).

---

**WARNING: This project will require you do mains / AC wiring! If you have never done that and don't know what you're doing: Don't read further!**

---

## Credits
As for all my PCBs: Huge thanks to Quindor from QuinLED.info and also the folks from #pcb-design-talk on his Discord server! Would not have been able to do anything without these people!!

## KiCad dependencies
It might happen that KiCad is missing my custom footprints. You can find and download them here and then add them to KiCad: https://github.com/ezcGman/andys-footprints

## Key features
- Very small size, fits in small DIY boxes
- 5-18V support! However, I recommend powering it with 12V max; the heat that LDO will produce might be a bit much.
- External Antenna!
- The ESP sits on pin headers instead of being soldered onto it. This enables you to remove / exchange it, e.g. if your code on that ESP doesn't have any OTA update feature, or if you want to exchange it for another ESP type
- Mounting holes to screw it onto something
- Made in Germany! ;)

## What do I need to do to get one?
### 1. Get the PCBs
You want to get the PCBs printed at a PCB prototype factory of your choice, like JLCPCB or PCBWay. I've included the Gerber files for both in the respective folder. If you want to use a different service provider, you need to check if they may accept these Gerbers or generate them yourself.

### 2. Get the components
Please see the [iBOM](/ibom/ESPBreakoutBoard-Min-Cutout-v4r3.html), which you can find in the `ibom` folder: It lists most components (except the ones below) to easily find them on AliExpress, LCSC or your preferred component sourcing platform :)

#### Extra Parts
These parts are not directly listed in the iBOM or need some further detail links:
- RFM95W, the famous low-cost LoRa module: https://www.aliexpress.com/wholesale?SearchText=rfm95w
- YHDC SCT-013, the CT clamps to actually measure power: https://www.aliexpress.com/wholesale?SearchText=sct-013
    - Get the 10A one, 15A at the maximum; you don't need more. I personally use the 15A ones in my install and code, just because I had them.
- The socket / jack for the external antenna: This can get a bit tricky and you may need to search AliExpress a tiny bit. The key important facts are a bit hard to state, as every seller lists something different:
    - The actual spacing of the outer holes from middle to middle of the hole is 5.08mm
    - Some sellers list the outer spacing of the pins, which is 6mm
    - Some just list the inner spacing of the pins, which is 4mm
    - But all of the above are ok, they gonna fit. There's also a [picture here](/pictures/rf-jack-spacing.jpg) to maybe help clarifying things
    - This seller always has a bunch at the time of writing: https://www.aliexpress.com/store/1100046103

### 3. Build the board and choose a power supply
Again, check the iBOM HTML file which gives you nice soldering instructions / overview, find it in the `ibom` folder. When you've soldered the board, wire it up with a 5V power source: It consumes less than 300mA, so you can easily use an old 500mA, 1A (or higher) phone charger for it.

### 4. Find the correct calibration value for the CT clamp
Or just trust me :)

Seriously: The library that actually reads current measurement values from the CT clamps needs a "calibration" value. This can vary on quite a few factors, but you can *roughly* say: If you have a 15A clamp, the value is somewhat near that number. For me, 14 gave the most accurate readings. Ultimately for this sensor, we don't really care about accurate readings, just readings / knowing if the machine still runs or is finished.

If you actually want to get an accurate calibration value, Edis Tech Lab (sorry, German!) [made a great video](https://www.youtube.com/watch?v=zNbtCse6n3s) about the CT clamps themselves and also talks a bit about how to find the correct calibration value.

### 5. Install the software
This board is designed to work with my [LoRa-To-MQTT-Gateway](https://github.com/ezcGman/lora-gateway), hence the source is also designed around it. Feel free to develop your own code, if yout want!

Source is available in the `src` folder. Download Arduino IDE and check a few things before you compile:
1. Near the top of `lora-washing-machine-sensor.ino`, change the calibration value [according to 4. above](#4-find-the-correct-calibration-value-for-the-ct-clamp): `const byte ctClampCalibration = 14;`
2. Download `lora-ids.h` and `utils.h` from the [LoRa-To-MQTT-Gateway repo](https://github.com/ezcGman/lora-gateway/tree/master/src/lora-gateway-e32) and place them into the source code folder, effectively replacing the placeholders there.
3. If you gave your LoRa-Gateway a different ID than `0x1`, change it in `lora-config.h`: `#define LORA_GATEWAY_ID 0x1 // 1`
4. That's it: Compile and upload it to your ESP8266 module

### 6. Hook up your washing machine
Now it gets a bit tricky and again: If you have never done AC / mains wiring and especially if you're in an apartment building / rented home: Leave your fingers off the AC wiring: If you don't know what you're doing and if you're not careful, you may up hurting yourself or others with your install!

OK, so now that we got that sorted out: CT clamps can't just go around your 3-core power cable of your washing machine: They can only measure current if you install them around either the L or N wire, not both, only a single one. So you need to get creative on how you get access to that wire.

What I did (you can [see some photos here](/pictures/install-pics)) is getting three AC power sockets and mounted them to a box. Why three?
- One as power in, coming from the wall
- One to plug in the washing machine
- One to plug in the dryer

With this, I do not touch the actual cables of the machines, plus: I do not have loose wires around! Inside the box, the two sockets (where the washing machine and dryer will be plugged into) are simply powered by the socket the power comes in, BUT: By using some Wagos, I can easily put the CT clamp around the L wire of the socket the machine and dryer is connected to.

Additionally, I modified a standard / cheap 1A phone charger by sawing off the two pins that would go into a power socket and instead soldered two wires to the PCB that then go into the Wagos inside the Box. And the 5V USB output goes to the PCB :) Pics are also in the same folder as above.

With this, you have a safe and clean install, no AC wires open and you can easily put that box behind the machine and dryer.

### 7. Read the values in Home Assistant
This is pretty easy:

First create two MQTT sensors that will read the watts from the MQTT topic the gateway will drop the readings into:
```
mqtt:
  sensor:
  - name: "[Basement] Washing Machine Watts"
    state_topic: "lora-gateway-e32/devices/washing-machine-sensor/messages/power-consumption/watts"
    icon: mdi:flash

  - name: "[Basement] Dryer Watts"
    state_topic: "lora-gateway-e32/devices/dryer-sensor/messages/power-consumption/watts"
    icon: mdi:flash
```

Now we create two `input_boolean` helpers that will make writing the automations easier. They will simply store the "result" of an automation that detects if a machine is running:
```
washing_machine_status:
  name: "[zzz-Helpers] Washing Machine Status"
  initial: off
  icon: mdi:washing-machine
  
dryer_status:
  name: "[zzz-Helpers] Dryer Status"
  initial: off
  icon: mdi:tumble-dryer
```

And now, we can create two automations per machine to first check if it started and another one that detects if it's finished. You may wanna do a few runs of your machine to get some test data and see what can really be considered "finished". Our machine sometimes does a tiny break and doesn't consume anything. So I declare the machine being finished if the readings show a value below 15W for at least two minutes, to not fall for those tiny breaks:
```
- id: washing_machine_started_set_helper
  alias: "[zzz-Helpers] Set helper sensor if washing machine is running"
  trigger:
  - platform: numeric_state
    entity_id: sensor.basement_washing_machine_watts
    above: 1000
  condition:
  - condition: state
    entity_id: input_boolean.washing_machine_status
    state: 'off'
  action:
  - service: input_boolean.turn_on
    data:
      entity_id: input_boolean.washing_machine_status
      
- id: washing_machine_finished
  alias: "[Basement] Send notification and reset helper if washing machine has finished"
  trigger:
  - platform: numeric_state
    entity_id: sensor.basement_washing_machine_watts
    below: 15
    for: 00:02:00
  condition:
  - condition: state
    entity_id: input_boolean.washing_machine_status
    state: 'on'
  action:
  - service: input_boolean.turn_off
    data:
      entity_id: input_boolean.washing_machine_status
  - service: notify.all_phones
    data:
      title: "🧺 Washing Machine finished!"
      message: "Get that laundry out there quickly!"
```

## Tools list to solder the PCB
### Required
- A soldering iron
- Soldering tin

### Optional
- For SMD components:
  - A hot air soldering/reflow station. However, it's only two and u have space around them, so they can be done with a soldering iron. Also the LDO can be technically soldered with an iron.
  - Solder paste
  - Tweezers

## FAQ
### Why the AMS117? Why not another LDO?
Because: I have so many lying around here ¯\\_(ツ)\_/¯ It's not the best, but pretty famous, cheap and easy to get LDO. It theoretically can eat up to 18V, so you could power that board with 18V, if you wanted to. Not talking about the heat this thing generates, if you would ;)

Its downsides are clearly the quiescent current of at least 5mA up to 13mA! There are also smaller LDOs (the AMS1117 is SOT-223), but with that screw terminal I wanted, it wouldn't matter anyways.
If you want to change it, you're welcome to edit the PCB using KiCad :)

### You're routing the SPI traces under the ESP antenna!
Yes, I do :) And I know that it's usually a no-go to route (high speed) data traces under an RF antenna. But in this case, that's fine, because we're not using Wi-Fi: My source code turns off the antenna completely, waits for it to be actually off and then starts initializing the LoRa module using SPI.
