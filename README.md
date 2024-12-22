# LoRa Smart Parking Sensor
  This project is a smart parking system designed to tackle the issue of parking congestion in
 urban areas. Currently, there is no way to know how much parking space is available in a
 parking lot until a person arrives at that parking lot. Additionally, even at smart parking lots which
 shine a light over the spot green or red based on availability tend to be incorrect when
 motorcycles or other shaped vehicles park in those spots. This project tackles these issues by
 using the touch sensor feature of the Wifi LoRa board which will require a vehicle to drive over a
 metal conductor that would span the parking space(for this scenario it will be aluminum foil).
 Once the touch sensor registers a value over the threshold the board will register a car as being
 parked. The LoRa board will simultaneously be sending updates to the Things Network with a
 data payload including the LoRa board's id, the parking lot space number, and its status
 (parked/not parked) based on the touch sensor. This will be linked to a viewable interface that
 will show which spots are available and which are not available based on the updated payloads
 sent by the LoRa board over the Things Network. The interface utilizes mqtt for Node.js to
 subscribe to the information from the Things Network and utilizes the parking_id and
 park_status parts of the payload to update a certain parking spot. It will take about 15-45
 seconds for this process to occur and to see the results of a car parked in the lot from its initial
 touch. This project would allow users to know beforehand how full a parking lot is which is
 especially useful in urban areas. 

  <img src="https://raw.githubusercontent.com/lbacha7/LoRa-Smart_Parking/main/parking.png" alt="Parking Diagram" height="500" width="700" />

# To Run:
 In one terminal move to the directory the project is located and run the interface by entering the
 command node mqtt.js (note: npm for Node js must first be installed on the computer). Next,
 have the things network running and run the LoraWAN program.
