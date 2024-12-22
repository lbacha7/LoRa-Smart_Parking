const mqtt = require('mqtt');
const http = require('http');

const TTN_USERNAME = 'final-project-parking@ttn';
const TTN_PASSWORD = 'NNSXS.54WZG4EMUOEX2I326XKDGXPOTAEIHKAN4UILREY.NPWGTBPHTRLY2RD3CLPMJMAYWUUPNIHXXXULD6GRMEXCUNV757VA';
const TTN_BROKER = 'nam1.cloud.thethings.network';
const TTN_PORT = 8883;

const client = mqtt.connect(`mqtts://${TTN_BROKER}`, {
  port: TTN_PORT,
  username: TTN_USERNAME,
  password: TTN_PASSWORD
});

client.on('connect', () => {
  console.log('Connected to TTN MQTT broker');
  client.subscribe(`v3/final-project-parking@ttn/devices/eui-70b3d57ed0067164/up`, (err) => {
    if (err) {
      console.error('Error subscribing:', err);
    } else {
      console.log('Subscribed to device uplink topic');
    }
  });
});

let parkingStatus = {
    p1: 'green',
    p2: 'green',
    p3: 'green',
    p4: 'green',
    p5: 'green',
    p6: 'green'
};

client.on('message', (topic, message) => {
  console.log('Received message from topic:', topic);
  //console.log('Message:', message.toString());
  const decodedPayload = JSON.parse(message.toString()).uplink_message.decoded_payload;
  console.log('Decoded Payload: ', decodedPayload);
  console.log('parking id: ', decodedPayload.parking_id);
  console.log('park status: ', decodedPayload.park_status);
  const parkingId = `p${decodedPayload.parking_id}`;
  if (decodedPayload.park_status == '0') {
    parkingStatus[parkingId] = 'green';
  } else {
    parkingStatus[parkingId] = 'red';
  }
});

client.on('error', (error) => {
  console.error('MQTT error:', error);
});

const server = http.createServer((req, res) => {
    if (req.url === '/') {
      res.setHeader('Content-Type', 'text/html');
      res.write(`
      <html>
      <head>
        <title>Parking Lot</title>
        <style>
          .row {
            display: flex;
            flex-wrap: wrap;
            width: 100%;
          }
      
          .column {
            display: flex;
            flex-direction: column;
            flex-basis: 25%;
            flex: 1;
          }
      
          .parking-spot {
            background-color: gray;
            height: 150px;
            width: 75%;
            margin-bottom: 10px; 
            position: relative; 
          }
      
          .green-rectangle {
            background-color: green;
            height: 50%; 
            width: 25%; 
            position: absolute; 
            top: 50%;
            left: 50%; 
            transform: translate(-50%, -50%); 
          }

          .red-rectangle {
            background-color: red;
            height: 50%;
            width: 25%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
          }
        </style>
      </head>
      <body>
        <h1>Parking Lot</h1>
        <div class='row'>
          <div class='column'>
            <div class='parking-spot'>
              <div class='${parkingStatus.p1}-rectangle', id = 'p1'></div>
            </div>
            <div class='parking-spot'>
              <div class='${parkingStatus.p2}-rectangle', id = 'p2'></div>
            </div>
            <div class='parking-spot'>
              <div class='${parkingStatus.p3}-rectangle', id = 'p3'></div>
            </div>
          </div>
          <div class='column'>
            <div class='parking-spot'>
              <div class='${parkingStatus.p4}-rectangle', id = 'p4'></div>
            </div>
            <div class='parking-spot'>
              <div class='${parkingStatus.p5}-rectangle', id = 'p5'></div>
            </div>
            <div class='parking-spot'>
              <div class='${parkingStatus.p6}-rectangle', id = 'p6'></div>
            </div>
          </div>
        </div>
        <script>
        setTimeout(() => {
          window.location.reload();
        }, 1000);
      </script>
      </body>
      </html>
      `);
      res.end();
    }
  });
  
  server.listen(8080, () => {
    console.log('Server running on port 8080');
  });

  
