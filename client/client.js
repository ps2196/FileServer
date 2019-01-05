var net = require('net');

var client = net.Socket();
client.connect(3333, 'localhost', function() {
    console.log('Connected to server!');
    // msg = {
    //     "type":"REQUEST",
    //     "command":"AUTH",
    //     "username":"root",
    //     "password":"root1"
    // }
    msg = "kskaskd"
    client.write(JSON.stringify(msg)+'\0');
    //client.write('Hello server! This is client speaking!\0');
});

client.on('data', function(data) {
    console.log('Received from server: ' + data);
    //client.destroy();
});


client.on('close', function() {
    console.log('Connection closed');
});
