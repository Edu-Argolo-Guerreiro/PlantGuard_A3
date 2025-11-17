import express from 'express';
import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { Server } from 'socket.io';

const app = express();
const server = createServer(app);
const io = new Server(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

const PORT = 3000;

const __dirname = dirname(fileURLToPath(import.meta.url));

app.use(express.static(join(__dirname, 'client')));

app.get('/', (req, res) => {
  res.sendFile(join(__dirname, 'client', 'index.html'));
});

console.clear();

io.on('connection', (socket) => {
    
    console.log(`\n🎉 Um usuário conectado: ${socket.id}`);

    socket.on('janela', (data) => {
        console.log('Status da janela: ' + data.status);
    });

    socket.on('botao_clicado', (data) => {
    console.log('Botão clicado:', data);
    io.emit('botao_clicado', data.acao);
});

    socket.on('luminosidade', (data) => {
        console.log('Luminosidade recebida:', data.valor);
        io.emit('luminosidade', { valor: data.valor });
    });


    // const intervalo = setInterval(() => {
    //     const valorLuminosidade = Math.floor(Math.random() * 101);
    //     socket.emit('luminosidade', { valor: valorLuminosidade });
    // }, 1000);

    socket.on('disconnect', () => {
        // clearInterval(intervalo);
        console.log(`\n❌ Usuário desconectado: ${socket.id}`);
    });
});

server.listen(PORT, () => {
  console.log(`server running at http://localhost:${PORT}`);
});