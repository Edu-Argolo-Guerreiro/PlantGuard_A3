import threading
import time

import serial
from flask import Flask, jsonify, request, render_template_string

# ==============================
# CONFIGURAÇÃO DA PORTA SERIAL
# ==============================
# 👉 TROQUE 'COM3' pela porta do seu Arduino (ex: 'COM4', '/dev/ttyUSB0' no Linux)
SERIAL_PORT = 'COM3'
BAUD_RATE = 9600

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Conectado à porta {SERIAL_PORT}")
except Exception as e:
    print(f"ERRO ao abrir a porta serial {SERIAL_PORT}: {e}")
    ser = None

# ==============================
# ESTADO COMPARTILHADO
# ==============================
state = {
    "ldr": 0,
    "monitor": 1,
    "alarm": 1,
    "last_raw": ""
}

app = Flask(__name__)

# ==============================
# LEITURA CONTÍNUA DA SERIAL
# ==============================
def read_serial_loop():
    if ser is None:
        print("Serial não inicializada. Loop de leitura não será executado.")
        return

    while True:
        try:
            line_bytes = ser.readline()
            if not line_bytes:
                time.sleep(0.01)
                continue

            line = line_bytes.decode(errors='ignore').strip()
            if not line:
                continue

            # Guarda a última linha bruta recebida
            state["last_raw"] = line
            # print("Recebido:", line)  # descomente se quiser debugar

            # Parse das mensagens do tipo: DATA;LDR=123;MON=1;ALARM=0
            if line.startswith("DATA;") or line.startswith("STATUS;"):
                parts = line.split(';')
                for p in parts[1:]:
                    if '=' in p:
                        k, v = p.split('=', 1)
                        v = v.strip()
                        if k == "LDR":
                            try:
                                state["ldr"] = int(v)
                            except ValueError:
                                pass
                        elif k == "MON":
                            state["monitor"] = int(v) if v in ("0", "1") else state["monitor"]
                        elif k == "ALARM":
                            state["alarm"] = int(v) if v in ("0", "1") else state["alarm"]

        except Exception as e:
            print("Erro lendo da serial:", e)
            time.sleep(0.5)

# Inicia a thread de leitura da serial
thread = threading.Thread(target=read_serial_loop, daemon=True)
thread.start()

# ==============================
# ROTAS FLASK
# ==============================

# Página principal com HTML + JS
@app.route("/")
def index():
    html = """
    <!doctype html>
    <html lang="pt-br">
    <head>
        <meta charset="utf-8">
        <title>Monitor de Luminosidade - A3</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background: #0f172a;
                color: #e5e7eb;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: flex-start;
                min-height: 100vh;
                padding: 20px;
            }
            h1 {
                margin-bottom: 5px;
            }
            .card {
                background: #111827;
                border-radius: 12px;
                padding: 20px;
                margin-top: 20px;
                box-shadow: 0 4px 12px rgba(0,0,0,0.4);
                width: 100%;
                max-width: 500px;
            }
            .valor {
                font-size: 2.5rem;
                font-weight: bold;
                margin: 10px 0;
            }
            .status {
                margin: 5px 0;
            }
            .badge {
                display: inline-block;
                padding: 4px 10px;
                border-radius: 999px;
                font-size: 0.8rem;
            }
            .badge-on {
                background: #16a34a;
                color: #ecfdf5;
            }
            .badge-off {
                background: #b91c1c;
                color: #fee2e2;
            }
            .buttons {
                display: grid;
                grid-template-columns: repeat(2, minmax(0, 1fr));
                gap: 10px;
                margin-top: 15px;
            }
            button {
                padding: 10px 12px;
                border-radius: 8px;
                border: none;
                cursor: pointer;
                font-weight: 600;
                background: #1d4ed8;
                color: #e5e7eb;
                transition: transform 0.1s ease, box-shadow 0.1s ease, background 0.1s ease;
            }
            button:hover {
                background: #2563eb;
                box-shadow: 0 4px 10px rgba(37,99,235,0.45);
                transform: translateY(-1px);
            }
            button.danger {
                background: #b91c1c;
            }
            button.danger:hover {
                background: #dc2626;
                box-shadow: 0 4px 10px rgba(220,38,38,0.45);
            }
            .raw {
                margin-top: 15px;
                font-size: 0.75rem;
                color: #9ca3af;
            }
        </style>
    </head>
    <body>
        <h1>Monitor de Luminosidade para Plantas</h1>
        <p>A3 - Microcontroladores: Arduino + Servidor Web</p>

        <div class="card">
            <h2>Leitura Atual</h2>
            <div class="valor" id="ldrValue">--</div>
            <div class="status">
                Monitoramento:
                <span id="monitorBadge" class="badge badge-off">Desativado</span>
            </div>
            <div class="status">
                Alarme:
                <span id="alarmBadge" class="badge badge-off">Desativado</span>
            </div>

            <div class="buttons">
                <button onclick="sendCommand('MONITOR_ON')">Ativar monitoramento</button>
                <button class="danger" onclick="sendCommand('MONITOR_OFF')">Desativar monitoramento</button>
                <button onclick="sendCommand('ALARM_ON')">Ativar alarme</button>
                <button class="danger" onclick="sendCommand('ALARM_OFF')">Desativar alarme</button>
            </div>

            <div class="buttons">
                <button onclick="requestStatus()">Atualizar status</button>
            </div>

            <div class="raw">
                Última mensagem bruta: <span id="rawLine">--</span>
            </div>
        </div>

        <script>
            async function fetchData() {
                try {
                    const resp = await fetch('/api/data');
                    const data = await resp.json();

                    document.getElementById('ldrValue').textContent = data.ldr;

                    const monBadge = document.getElementById('monitorBadge');
                    const alarmBadge = document.getElementById('alarmBadge');
                    const rawLine = document.getElementById('rawLine');

                    if (data.monitor === 1) {
                        monBadge.textContent = 'Ativado';
                        monBadge.className = 'badge badge-on';
                    } else {
                        monBadge.textContent = 'Desativado';
                        monBadge.className = 'badge badge-off';
                    }

                    if (data.alarm === 1) {
                        alarmBadge.textContent = 'Ativado';
                        alarmBadge.className = 'badge badge-on';
                    } else {
                        alarmBadge.textContent = 'Desativado';
                        alarmBadge.className = 'badge badge-off';
                    }

                    rawLine.textContent = data.last_raw || '--';

                } catch (e) {
                    console.error('Erro ao buscar dados:', e);
                }
            }

            async function sendCommand(cmd) {
                try {
                    await fetch('/api/command', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({ cmd })
                    });
                } catch (e) {
                    console.error('Erro ao enviar comando:', e);
                }
            }

            async function requestStatus() {
                await sendCommand('STATUS?');
            }

            // Atualiza a cada 1 segundo
            setInterval(fetchData, 1000);
            fetchData();
        </script>
    </body>
    </html>
    """
    return render_template_string(html)

# API para a página web buscar os dados
@app.route("/api/data")
def api_data():
    return jsonify(state)

# API para enviar comando para o Arduino
@app.route("/api/command", methods=["POST"])
def api_command():
    data = request.get_json(force=True)
    cmd = data.get("cmd", "").strip()

    if not cmd:
        return jsonify({"ok": False, "error": "Comando vazio"}), 400

    print("Enviando comando para Arduino:", cmd)

    if ser is not None and ser.is_open:
        try:
            # Envia o comando com quebra de linha, igual o Arduino espera
            ser.write((cmd + "\n").encode())
        except Exception as e:
            return jsonify({"ok": False, "error": str(e)}), 500
    else:
        return jsonify({"ok": False, "error": "Serial não disponível"}), 500

    return jsonify({"ok": True, "cmd": cmd})

# ==============================
# MAIN
# ==============================
if __name__ == "__main__":
    # Roda o servidor Flask em http://localhost:5000
    app.run(host="0.0.0.0", port=5000, debug=True)
