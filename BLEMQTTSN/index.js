const bluetooth = require('node-bluetooth');
var prettyjson = require('prettyjson');
var mqttsn = require('mqttsn-packet');
var dgram = require('dgram');
var sizeof = require('object-sizeof');

var settings = {
  HOST: "127.0.0.1",
  PORT: 1884,
}

var perif

/*Creation of UDP Forwarder to Broker*/
var createMqttsnForwarder = function(udpClient) {
  var parser = mqttsn.parser();
  parser.on('packet', function(packet) {
      console.log(prettyjson.render(packet));
      var buffer = mqttsn.generate(packet) ;
      udpClient.send(buffer, 0, buffer.length, settings.PORT, settings.HOST, function(err, bytes) {
        console.log("Packet sent");
      });
  });
  return parser;
}

process.on('exit', function() {
  if (perif != null) {
      console.log("goodbye, disconnecting...")
      peripheral.once('disconnect');
  }
})

/*SERIAL --> BROKER */
var writeToUdp = function(client) {
  var mqttsnForwarder = createMqttsnForwarder(client);
  return function(data) {
      console.log("Message received via BLE serial");
      console.log('Message: ', data.slice(0, -1));
      mqttsnForwarder.parse(data); // use this method to reconstitute full MQTT-SN packet
      console.log('Message forwarded to the broker');
  }
}

// create bluetooth device instance
const device = new bluetooth.DeviceINQ();

device
.on('finished', console.log.bind(console, 'finished'))
.on('found', function found(address, name){

  // find serial port channel
  device.findSerialPortChannel(address, function(channel){
    console.log('Found RFCOMM channel for serial port on %s: ', name, channel);

      // make bluetooth connect to remote device
      if (name == 'DESKTOP-7FJLDKR') bluetooth.connect(address, channel, function(err, connection){
      
        if(err) return console.error(err);
      
        console.log("Connected to device");
      
        var client = dgram.createSocket('udp4');

        //connection.delimiter = Buffer.from('\n', 'utf8'); //review
        connection.on('data', writeToUdp(client));
        /*BROKER --> SERIAL*/
        client.on('message', function(data, remote) {
          console.log('Received a message from the broker: ', data.slice(0, -1));
          connection.write(Buffer.alloc(sizeof(data), data, 'utf8'), () => {
            console.log('Message sent via BLE serial');
          });
        })
      });
  });
});
device.scan();
