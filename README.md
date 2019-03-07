# festival-sword

This is a beautiful animated LED sword I made for Shambhala Music Festival in 2018. Shambhala is one of the most highly regarded electronic music festivals in Canada, often drawing comparisons to Burning Man. 

A crossguard and a case for the electronic components were 3d printed and glued to a polycarbonate tube holding the LED strip.

The design held up very well over 4 days of constant use and camping in quite rigorous conditions. It can be recharged easily from a portable lithium battery with USB outputs.

An oversite is that the sword is too bright, and it's animations are too fast for it to photograph well at night. 

The sword uses addressible APA102 LEDs, for there fast refresh rate allowing really impressive animations.
I intended to use a smaller controller and a smaller charging board, but inadvertently fried both while testing it. I had to substitute a bulkier Arduino Leonardo. 

### Parts:
* https://www.adafruit.com/product/2030 PowerBoost 1000 Basic - 5V USB Boost @ 1000mA from 1.8V+
* https://www.adafruit.com/product/1781 Lithium Ion Cylindrical Battery - 3.7v 2200mAh
* https://www.sparkfun.com/products/13777 SparkFun Battery Babysitter - LiPo Battery Manager - any LiPo charging board will do
* Arduino Leonardo, but I recommend one of many comparible boards with smaller form factors. 
* https://www.adafruit.com/product/2241 Adafruit DotStar Digital LED Strip - Black 144 LED/m - One Meter - BLACK, Or another APA102 LED strip
* SN74HC14N Inverting Schmitt Trigger
* Momentary push button
* 1000 uF capacitor
* 10 uF capacitor
* 1.2 kOhm resistor
* 10 kOhm resistor
* Transparent polycarbonate tube
