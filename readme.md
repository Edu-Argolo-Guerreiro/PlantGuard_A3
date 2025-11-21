# PlantGuard

Sistema de monitoramento e controle automático de uma janela para plantas baseado em Arduino, Node.js e WebSocket.

## 📋 Descrição

O PlantGuard é um projeto IoT que automatiza o controle de uma janela para plantas com base em níveis de luminosidade. O sistema coleta dados de luz de um sensor Arduino e permite abrir/fechar a janela remotamente através de uma interface web em tempo real.

**Componentes principais:**

- **Arduino**: Lê dados do sensor de luminosidade e controla a janela
- **Ponte Arduino (Node.js)**: Comunica com a placa Arduino via porta serial
- **Servidor Node.js**: Gerencia a comunicação entre cliente e Arduino usando WebSocket
- **Cliente Web**: Interface interativa para monitorar e controlar a janela

## 🎯 Funcionalidades

- ✅ Leitura em tempo real de luminosidade do sensor Arduino
- ✅ Interface web responsiva com barra de progresso visual
- ✅ Abertura/fechamento da janela por clique de botão
- ✅ Comunicação bidirecional via WebSocket (Socket.IO)
- ✅ Conexão serial com Arduino de forma automática
- ✅ Feedback visual da intensidade luminosa com código de cores

## 📁 Estrutura do Projeto

```
PlantGuard_A3/
├── GccAplicacaoPlantGuard/          # Código Arduino (Atmel Studio)
│   ├── GccAplicacaoPlantGuard.cproj
│   ├── main.c                        # Firmware do Arduino
│   └── Debug/
├── ponteArduino/                     # Ponte de comunicação serial
│   ├── index.js
│   └── package.json
├── server/                           # Servidor Node.js
│   ├── index.js                      # Servidor Express + Socket.IO
│   ├── package.json
│   └── client/                       # Cliente web
│       ├── index.html
│       ├── client.js
│       └── style.css
└── README.md
```

## 🔧 Instalação

### Pré-requisitos

- **Node.js** (v16.0.0 ou superior)
- **npm** (incluído com Node.js)
- **Arduino IDE** ou ferramenta de upload (para programar o Arduino)
- **Porta serial** COM disponível para comunicação com Arduino

### Passo 1: Clonar o repositório

```bash
git clone git@github.com:Edu-Argolo-Guerreiro/PlantGuard_A3.git
cd PlantGuard_A3
```

### Passo 2: Instalar dependências do servidor

```bash
cd server
npm install
```

### Passo 3: Instalar dependências da ponte Arduino

```bash
cd ../ponteArduino
npm install
```

### Passo 4: Programar o Arduino

1. Abra o projeto em `GccAplicacaoPlantGuard/main.c` no Atmel Studio
2. Configure a porta serial (padrão: COM5) no firmware
3. Upload do código para a placa Arduino

### Passo 5: Executar o projeto

**Terminal 1 - Servidor Node.js:**

```bash
cd server
npm run start
```

**Terminal 2 - Ponte Arduino:**

```bash
cd ponteArduino
npm run start
```

Acesse a aplicação em `http://localhost:3000`

## 📚 Bibliotecas Usadas

### Servidor (Node.js)

| Biblioteca    | Versão   | Descrição                              |
| ------------- | -------- | -------------------------------------- |
| **express**   | ^4.18.0+ | Framework web para Node.js             |
| **socket.io** | ^4.5.0+  | Comunicação bidirecional via WebSocket |
| **node:http** | Built-in | Módulo HTTP nativo do Node.js          |
| **node:path** | Built-in | Manipulação de caminhos de arquivo     |
| **node:url**  | Built-in | Parsing de URLs                        |

### Ponte Arduino (Node.js)

| Biblioteca                      | Versão   | Descrição                               |
| ------------------------------- | -------- | --------------------------------------- |
| **serialport**                  | ^10.0.0+ | Comunicação com porta serial do Arduino |
| **@serialport/parser-readline** | ^10.0.0+ | Parser para ler dados linha por linha   |
| **socket.io-client**            | ^4.5.0+  | Cliente WebSocket para Node.js          |

### Cliente (Browser)

| Biblioteca             | Versão  | Descrição                        |
| ---------------------- | ------- | -------------------------------- |
| **socket.io-client**   | ^4.5.0+ | Cliente WebSocket para navegador |
| **HTML5**              | -       | Markup e estrutura               |
| **CSS3**               | -       | Estilização                      |
| **Vanilla JavaScript** | -       | Manipulação de DOM               |

### Arduino

| Biblioteca        | Descrição                         |
| ----------------- | --------------------------------- |
| **Serial**        | Comunicação via porta serial      |
| **Sensor de Luz** | Leitura analógica de luminosidade |

## 🔌 Configuração de Conexão

### Porta Serial

- **Velocidade (Baud Rate)**: 9600 bps
- **Porta padrão**: COM5 (configurável em `ponteArduino/index.js`)
- **Detecção automática**: O script tenta detectar Arduino automaticamente

### WebSocket

- **URL do servidor**: `http://localhost:3000`
- **Protocolo**: Socket.IO
- **CORS**: Permitido de qualquer origem

## 🎮 Como Usar

1. **Abrir a interface web**: Acesse `http://localhost:3000` no navegador
2. **Monitorar luminosidade**: Observe a barra de progresso atualizar em tempo real
3. **Controlar a janela**: Clique no botão para abrir ou fechar a janela

### Indicadores de Cor

- 🔴 **Vermelho**: ≤ 30% ou > 90% (baixa ou excessiva luminosidade)
- 🟡 **Amarelo**: 31-50% ou 71-90% (luminosidade baixa/média ou média/alta)
- 🟢 **Verde**: 51-70% (luminosidade ideal)

## 🔄 Fluxo de Dados

```
Arduino → Ponte Arduino (Serial) → Servidor Node.js → Cliente Web (WebSocket)
                                        ↓
                                    WebSocket
                                        ↓
          Comando Web → Servidor → Ponte Arduino → Arduino
```

## 🐛 Troubleshooting

### Porta serial não encontrada

- Verifique a conexão USB do Arduino
- Altere manualmente a porta em `ponteArduino/index.js` (variável `defaultPath`)
- Use o Device Manager para confirmar a porta COM

### Servidor não inicia

- Certifique-se de que a porta 3000 está disponível
- Verifique se todas as dependências foram instaladas: `npm install`

### WebSocket não conecta

- Confirme que o servidor está rodando
- Verifique o console do navegador para mensagens de erro
- Teste a URL manualmente: `http://localhost:3000`

## 📧 Contato

Para dúvidas ou sugestões, abra uma issue no repositório.
