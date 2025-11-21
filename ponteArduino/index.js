import { SerialPort } from 'serialport';               // Biblioteca para comunicação serial (Node)
import { ReadlineParser } from '@serialport/parser-readline'; // Parser que quebra o stream por linhas
import io from 'socket.io-client';                     // Cliente Socket.IO para conectar ao servidor

const socket = io('http://localhost:3000');           // Conecta ao servidor Socket.IO local

// IIFE assíncrona que inicializa a comunicação serial
(async function initSerial() {
  try {
    const ports = await SerialPort.list();            // Lista portas seriais disponíveis
    console.log('Portas seriais disponíveis:', ports);

    // Função para identificar qual porta provavelmente é o Arduino
    const findArduinoPort = (ports) => {
      return ports.find(port => {
        // Verifica campos comuns que indicam um Arduino:
        // manufacturer contém "arduino", productId ou vendorId conhecidos
        const isArduino = 
          (port.manufacturer && port.manufacturer.toLowerCase().includes('arduino')) ||
          (port.productId === '0043') || // Exemplo: Arduino Uno
          (port.productId === '6001') || // Exemplo: Arduino Nano
          (port.vendorId === '2341');    // Vendor ID comum da Arduino LLC
        
        // Log de verificação para depuração
        console.log(`Verificando ${port.path || port.comName}: ${isArduino ? '✓ Arduino encontrado' : '✗'}`);
        return isArduino;
      });
    };

    const arduinoPort = findArduinoPort(ports);       // Tenta achar a porta do Arduino
    const defaultPath = arduinoPort?.path || 'COM5';  // Usa encontrada ou fallback para COM5

    console.log('Usando porta:', defaultPath);

    // Cria uma instância da porta serial (sem abrir automaticamente)
    const port = new SerialPort({
      path: defaultPath,
      baudRate: 9600,      // Taxa de comunicação deve bater com o sketch do Arduino
      autoOpen: false
    });
    
    // Trata erros emitidos pela porta serial
    port.on('error', (err) => {
      console.error('Erro na porta serial:', err.message);
    });

    // Abre a porta serial e trata possível erro de abertura
    port.open((err) => {
      if (err) {
        console.error('Erro ao abrir porta serial:', err.message);
        return;
      }
      console.log('Porta serial aberta em', defaultPath);
    });

    // Encadeia um parser que separa linhas (\n) do stream serial
    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

    // Ao receber dados do Arduino, emite evento 'luminosidade' para o servidor via socket
    parser.on('data', (data) => {
      socket.emit('luminosidade', { valor: data });
    });

    // Escuta comandos vindos do servidor e encaminha para o Arduino pela serial
    socket.on('botao_clicado', (comando) => {
      console.log(`Comando recebido do servidor: ${comando}`);
      port.write(comando, (err) => {
        if (err) {
          return console.log('Erro ao escrever na serial:', err.message);
        }
        console.log('Comando enviado com sucesso para o Arduino.');
      });
    });

  } catch (err) {
    // Captura falhas na inicialização (ex.: problema ao listar portas)
    console.error('Falha inicializando serial:', err);
  }
})();

