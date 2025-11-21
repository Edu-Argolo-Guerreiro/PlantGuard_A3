// Importa as bibliotecas necessárias
import express from 'express';
import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { Server } from 'socket.io';

// Inicializa o aplicativo Express
const app = express();
// Cria um servidor HTTP a partir do aplicativo Express
const server = createServer(app);
// Instancia o Socket.IO no servidor, com configurações de CORS para permitir qualquer origem
const io = new Server(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// Define a porta do servidor
const PORT = 3000;

// Obtém o caminho do diretório atual do arquivo do servidor
const __dirname = dirname(fileURLToPath(import.meta.url));

// Configura o Express para servir arquivos estáticos (HTML, CSS, JS do cliente) da pasta 'client'
app.use(express.static(join(__dirname, 'client')));

// Define a rota principal ('/') para servir o arquivo index.html
app.get('/', (req, res) => {
  res.sendFile(join(__dirname, 'client', 'index.html'));
});

console.clear();

// Evento disparado quando um novo cliente se conecta ao servidor Socket.IO
io.on('connection', (socket) => {
    
    console.log(`\n🎉 Um usuário conectado: ${socket.id}`);

    // Ouve o evento 'janela' (atualmente apenas registra o status recebido no console)
    socket.on('janela', (data) => {
        console.log('Status da janela: ' + data.status);
    });

    // Ouve o evento 'botao_clicado' vindo do cliente (navegador)
    socket.on('botao_clicado', (data) => {
        console.log('Botão clicado:', data);
        // Re-transmite a ação ('A' ou 'F') para todos os clientes conectados,
        // incluindo a ponte com o Arduino, para que o comando seja enviado pela porta serial.
        io.emit('botao_clicado', data.acao);
    });
    // Ouve o evento 'luminosidade' vindo da ponte Arduino
    socket.on('luminosidade', (data) => {
        // Re-transmite o valor da luminosidade para todos os clientes (navegadores)
        io.emit('luminosidade', { valor: data.valor });
    });
    // Evento disparado quando um cliente se desconecta
    socket.on('disconnect', () => {
        console.log(`\n❌ Usuário desconectado: ${socket.id}`);
    });
});
// Inicia o servidor na porta definida
server.listen(PORT, () => {
  console.log(`server running at http://localhost:${PORT}`);
});