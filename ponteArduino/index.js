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
          (port.productId && port.productId === '0043') || // Arduino Uno
          (port.productId && port.productId === '6001') || // Arduino Nano
          (port.vendorId && port.vendorId === '2341');     // Arduino LLC vendor ID
        
        console.log(`Verificando ${port.path || port.comName}: ${isArduino ? '✓ Arduino encontrado' : '✗'}`);
        return isArduino;
      });
    };

    const arduinoPort = findArduinoPort(ports);
    const defaultPath = process.env.SERIAL_PORT || arduinoPort?.path || (process.platform === 'win32' ? 'COM5' : '/dev/ttyACM0');

    console.log('Usando porta:', defaultPath);

    const port = new SerialPort({
      path: defaultPath,
      baudRate: 9600,
      autoOpen: false
    });
    
    port.on('error', (err) => {
      console.error('Erro na porta serial:', err.message);
    });

    port.open((err) => {
      if (err) {
        console.error('Erro ao abrir porta serial:', err.message);
        return;
      }
      console.log('Porta serial aberta em', defaultPath);
    });

    const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

    parser.on('data', (data) => {
      console.log(`Dados recebidos do Arduino: ${data}`);
      // const cleanData = parseInt((data || '').trim().split(':')[1]) || 0;

      // console.log(cleanData); // TESTE 
      console.log(data); // TESTE 

      socket.emit('luminosidade', { valor: data });
    });

    // especificar qual comando para o arduino mexer 
    socket.on('botao_clicado', (comando) => {
      console.log(`Comando recebido do servidor: ${comando}`);
      port.write(comando, (err) => {
        if (err) {
          return console.log('Erro ao escrever na serial:', err.message);
        }
        console.log('Comando enviado com sucesso para o Arduino.');
      });
    });

    // Emissão de luminosidade fake
    // setInterval(() => {
    //   const valorLuminosidade = Math.floor(Math.random() * 101);
    //   socket.emit('luminosidade', { valor: valorLuminosidade });
    //   console.log(`Luminosidade enviada: ${valorLuminosidade}`);
    // }, 1000);

  } catch (err) {
    console.error('Falha inicializando serial:', err);
  }
})();

