RGBHueControl
=============

This little Arduino sketch will control your Philips Hue lights to adapt their color to reproduce ambient light.

The ambient light is measured on the Arduino by using three photoresistors to filter R/G/B color. The setup is based on the "Color Mixing Lamp" project from the Arduino Starter Kit.

The Hue lights are directly controlled through their local bridge. The commands are sent as HTTP using the Arduino Ethernet shield.

Check out the comments to find out how to address the Philips Hue lights, or check out http://developers.meethue.com.
