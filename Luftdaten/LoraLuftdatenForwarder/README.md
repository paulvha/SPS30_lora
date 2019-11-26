# LoraLuftdatenForwarder
Bridge software for receiving airborne particulate matter data from TheThingsNetwork and forwarding it to luftdaten.info

# this is special build based on the original to support SPS30 data and create a local datafile.
# Building the application
Execute the following commands:
* cd LoraLuftdatenForwarder
* cd gradle
* ./gradlew assemble

A compressed distribution archive (.tar/.zip) is now available in LoraLuftdatenForwarder/build/dist
Uncompress this and run:
  bin/LoraluftdatenForwarder
