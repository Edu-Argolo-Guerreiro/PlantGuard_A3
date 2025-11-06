🌱 PlantGuard – Sistema de Monitoramento de Luminosidade para Plantas com Alerta Sonoro e Supervisão Web

Resumo:
O sistema PlantGuard utiliza um microcontrolador Arduino com sensor de luminosidade (LDR), LEDs e buzzer para monitorar as condições de luz ideais para o crescimento de plantas.
Quando há falta ou excesso de luminosidade, o sistema emite alertas visuais e sonoros.
Os dados são enviados via Serial para um servidor Python (Flask), que exibe as leituras em uma página web e permite o controle remoto dos atuadores.

Arquitetura:

Camada Física: Sensor LDR, LEDs e buzzer conectados ao Arduino.

Camada de Controle: Código C++ no Arduino processa os dados e envia/recebe comandos via Serial.

Camada Servidor: Script Python (server.py) usando Flask e PySerial.

Camada Web: Interface HTML/CSS/JS exibe o valor da luminosidade e oferece botões de controle.

[LDR/LEDs/Buzzer] ⇄ [Arduino] ⇄ (USB Serial) ⇄ [Servidor Flask] ⇄ [Página Web]



Semana 1 (Planejamento + Desenvolvimento Base):
- Divisão de funções e setup do ambiente
- Codificação do Arduino
- Montagem do Flask + HTML
- Criação do esqueleto do relatório

Semana 2 (Integração + Testes + Apresentação):
- Teste real com Arduino
- Ajustes de comunicação
- Finalização do relatório e slides
- Apresentação (vídeo ou demonstração ao vivo)