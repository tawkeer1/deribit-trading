const WebSocket = require('ws');

const LOCAL_PORT = 9002;
const TARGET_WSS = 'wss://test.deribit.com/ws/api/v2';

const proxyServer = new WebSocket.Server({ port: LOCAL_PORT });

proxyServer.on('connection', function connection(clientSocket) {
    console.log('Client connected');

    const deribitSocket = new WebSocket(TARGET_WSS);

    deribitSocket.on('open', () => {
        console.log('✅ Connected to Deribit');

        clientSocket.on('message', (msg) => {
            let msgStr = Buffer.isBuffer(msg) ? msg.toString('utf8') : msg;
            console.log('📨 From client:', msgStr);

            try {
                JSON.parse(msgStr);
            } catch (err) {
                console.warn('❌ JSON parse failed:', err.message);
            }

            if (deribitSocket.readyState === WebSocket.OPEN) {
                deribitSocket.send(msgStr);
                console.log('🔁 Sent to Deribit');
            }
        });

        deribitSocket.on('message', (data) => {
            console.log('📬 From Deribit:', data);

            if (clientSocket.readyState === WebSocket.OPEN) {
                try {
                    clientSocket.send(data);
                    console.log('✅ Forwarded to client');
                } catch (err) {
                    console.error('❌ Error sending to client:', err.message);
                }
            }
        });
    });

    deribitSocket.on('error', err => {
        console.error('❌ Deribit error:', err.message);
    });

    clientSocket.on('error', err => {
        console.error('❌ Client error:', err.message);
    });

    clientSocket.on('close', () => {
        console.log('Client disconnected');
        deribitSocket.close();
    });
});

console.log(`🛰️  WebSocket proxy listening at ws://localhost:${LOCAL_PORT} → ${TARGET_WSS}`);
