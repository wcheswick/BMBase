# BMBase
Access and processing software for Broodminder devices

Broodminder devices are built to be installed in and under beehives to help with hive management. These devices can store up to a years worth of data running on a single 2032 coin battery, 
and the data can be extracted using proprietary Broodminder protocols.

But access to this data requires presence at the hives with a smart phone and app, or the purchase
of a dedicated system, which can be a bit pricey.

But the Broodminder devices include the current readings in their Bluetooth advertising,
access that is publicly-documented. This means that a nearby computer can pull them
periodically, extracting the useful data for further processing.
This software does that extraction and some of the processing, and runs in a Raspberry Pi.

Note that there are other uses for these excellent sensors.  While the hive scales are the most
useful for the bees, we are using the temperature sensors to monitor the temperature in various spots
in a greenhouse, and generate frost warnings.

This software runs on Linux on a recent Raspberry Pi (RPi), using low power Bluetooth to periodically extract the data from the Broodminder sensors. This data can be pushed  to a server, 
where the data can be plotted and  further processed.  This package contains a simple plot
routine based on gnuplot (ugh).

A modern RPi and this software are all you need to monitor an area if there is power
and Wi-Fi available.  Remote power and network access is a problem we haven’t solved yet.
All of the solar power solutions I have found for the RPi require some assembly: 
there do not appear to be any turnkey solutions available out there.

For network access, the cell system is available through RPi cellular shields.  There are inexpensive
data plans running as slow as GPRS. Typical data reporting could be as low as one or two UDP packets per hour, about 3KB.  This slow, non-priority traffic should be cheap.
,
The software consists of two parts. On the device, the BMBase software typically runs once an
hour for a minute or so, reaping the sensor data.  Currently, this is pushed over WiFi to a 
serving computer.  At this point, the RPi could shut down for an hour, preserving power,
if a device were available to turn the power back on 59 minutes later.  I haven’t found anything
that can do that, to my surprise: IoT needs this.

