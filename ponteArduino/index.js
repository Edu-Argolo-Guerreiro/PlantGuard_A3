import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';
import io from 'socket.io-client';


const socket = io('http://localhost:3000');

(async function initSerial() {
  try {
    const ports = await SerialPort.list();
    console.log('Portas seriais disponíveis:', ports);

    // Função para identificar porta Arduino
    const findArduinoPort = (ports) => {
      return ports.find(port => {
        const isArduino = 
          (port.manufacturer && port.manufacturer.toLowerCase().includes('arduino')) ||
          (port.productId === '0043') || // Arduino Uno
          (port.productId === '6001') || // Arduino Nano
          (port.vendorId === '2341');    // Arduino LLC vendor ID
        
        console.log(`Verificando ${port.path || port.comName}: ${isArduino ? '✓ Arduino encontrado' : '✗'}`);
        return isArduino;
      });
    };

    const arduinoPort = findArduinoPort(ports);
    const defaultPath = arduinoPort?.path || 'COM5';

    console.log('Usando porta:', defaultPath);

    // Criando estancia da porta
    const port = new SerialPort({
      path: defaultPath,
      baudRate: 9600,
      autoOpen: false
    });
    
    // Tratamento de erro na porta serial
    port.on('error', (err) => {
      console.error('Erro na porta serial:', err.message);
    });

    // Abre a porta serial
    port.open((err) => {
      if (err) {
        console.error('Erro ao abrir porta serial:', err.message);
        return;
      }
      console.log('Porta serial aberta em', defaultPath);
    });

    
    const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

    // Recebe o valor de luminosidade do Arduino e manda para o servidor socket
    parser.on('data', (data) => {
      socket.emit('luminosidade', { valor: data });
    });

    // Recebe o comando do servidor socket e manda o comando para a porta serial
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
    console.error('Falha inicializando serial:', err);
  }
})();

