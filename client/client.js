var net = require('net');

var client = net.Socket();
client.connect(3338, 'localhost', function() {
    console.log('Connected to server!');
    msg = {
        "twoj_stary":"pijany",
        "dupa":"wielka",
        "twoja_stara_od_tylu?":"jebana"
    }
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
