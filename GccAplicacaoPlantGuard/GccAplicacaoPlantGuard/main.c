/*
 * GccAplicacaoPlantGuard.c
 *
 * Criado: 21/11/2025 12:37:28
 * Microcontrolador: ATmega328P
 * Autor: sinvalluz
 * Descrição: Sistema de monitoramento de luminosidade para plantas (PlantGuard).
 *            Lê um sensor de luz (LDR), controla LEDs de status, um buzzer de alarme
 *            e um servo motor via comunicação serial.
 */

#define F_CPU 16000000UL // Define a frequência do clock do microcontrolador para 16MHz. Essencial para funções de delay.

#include <avr/io.h>        // Biblioteca principal para I/O do AVR.
#include <util/delay.h>    // Biblioteca para funções de espera (delay).
#include <avr/interrupt.h> // Biblioteca para gerenciamento de interrupções.
#include <stdlib.h>        // Biblioteca padrão do C, usada aqui para a função itoa() (integer to ascii).

// ---------- DEFINIÇÃO DE PINOS ----------
#define BUZZER_PIN PD7 // Pino para o buzzer.
#define LED_STATUS PD3 // Pino para o LED que indica atividade do sistema.
#define LED_ACAO PB5   // Pino para o LED que indica que uma ação no servo motor foi executada.

#define SERVO_PIN PB1    // Pino de controle do servo motor (saída PWM).
#define LED_VERDE PB0    // Pino para o LED de status "ideal".
#define LED_AMARELO PD4  // Pino para o LED de status "médio/alto".
#define LED_VERMELHO PB2 // Pino para o LED de status "baixo/muito alto" (alerta).

// Limites do sinal PWM para o servo motor (em contagens do Timer1).
// Estes valores correspondem à largura de pulso para posicionar o servo.
#define SERVO_MIN 2000 // Valor para pulso de ~1ms (posição 0 graus, "fechado").
#define SERVO_MAX 4400 // Valor para pulso de ~2.2ms (posição ~180 graus, "aberto").

// Limites de luminosidade, em porcentagem (0-100), para classificação da luz.
#define LIM_MUITO_BAIXA_MAX 15 // Abaixo deste valor, a luz é "muito baixa".
#define LIM_BAIXA_MAX 30       // Entre MUITO_BAIXA e este valor, a luz é "baixa".
#define LIM_MEDIA_MAX 50       // Entre BAIXA e este valor, a luz é "média".
#define LIM_IDEAL_MAX 70       // Entre MEDIA e este valor, a luz é "ideal".
#define LIM_MUITO_ALTA_MIN 90  // Acima deste valor, a luz é "muito alta".

volatile uint8_t alarmOn = 0; // Flag global para controlar o estado do buzzer. 'volatile' é usado pois ela é modificada na ISR e lida no código principal.

// ---------- PROTÓTIPOS DAS FUNÇÕES ----------
void setup_gpio(void);                                                        // Configura os pinos de entrada e saída (GPIO).
void setup_adc(void);                                                         // Configura o conversor Analógico-Digital (ADC).
uint16_t read_adc(uint8_t channel);                                           // Lê o valor de um canal do ADC.
void setup_uart(unsigned int ubrr);                                           // Configura a comunicação serial (UART).
void uart_transmit(unsigned char data);                                       // Envia um único byte via UART.
int uart_available(void);                                                     // Verifica se há dados recebidos na UART.
void uart_print_text(const char *str);                                        // Envia uma string (texto) via UART.
void setup_timer0_buzzer(void);                                               // Configura o Timer0 para gerar o som do buzzer.
void setup_timer1_servo(void);                                                // Configura o Timer1 para gerar o sinal PWM para o servo.
void set_buzzer_freq(uint16_t freq);                                          // Ativa/desativa o buzzer e define sua frequência.
long map_value(long x, long in_min, long in_max, long out_min, long out_max); // Mapeia um valor de uma faixa para outra.
void apaga_leds(void);                                                        // Desliga todos os LEDs de status.
void pisca_led_status(void);                                                  // Pisca o LED de status para indicar que o sistema está rodando.
void verifica_acao_motor(void);                                               // Verifica e processa comandos recebidos via UART para o motor.
int ler_sensor_e_atualizar(void);                                             // Função principal da lógica: lê o sensor e atualiza os atuadores (LEDs, buzzer).

// ---------- INTERRUP��O DO TIMER0 (BUZZER) ----------
ISR(TIMER0_COMPA_vect)
{
    if (alarmOn)
    {
        PORTD ^= (1 << BUZZER_PIN); // Alterna o pino � gera o som
    }
    else
    {
        PORTD &= ~(1 << BUZZER_PIN); // Garante buzzer desligado
    }
}

int main(void)
{
    char stringLuminosidade[10];
    int valorLuminosidade;

    // Inicializa��es
    setup_gpio();
    setup_adc();
    setup_uart(103); // UBRR = 103 ? 9600 baud
    setup_timer0_buzzer();
    setup_timer1_servo();

    sei(); // Habilita interrup��es globais

    apaga_leds();
    PORTD &= ~(1 << LED_STATUS);

    OCR1A = SERVO_MIN; // Servo come�a fechado

    while (1)
    {
        // L� sensor e atualiza LEDs/buzzer
        valorLuminosidade = ler_sensor_e_atualizar();

        // Converte para texto
        itoa(valorLuminosidade, stringLuminosidade, 10);

        // Envia via UART
        uart_print_text(stringLuminosidade);
        uart_transmit('\n');

        verifica_acao_motor(); // Processa comandos UART ('A' / 'F')
        pisca_led_status();    // Pisca LED de atividade

        _delay_ms(500);
    }
}

// ---------- FUN��ES UART ----------
void uart_print_text(const char *str)
{
    while (*str)
    {
        uart_transmit(*str++);
    }
}

void setup_uart(unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr >> 8);    // Baud rate parte alta
    UBRR0L = (unsigned char)ubrr;           // Baud rate parte baixa
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);   // Habilita RX e TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits
}

void uart_transmit(unsigned char data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;        // Espera buffer liberar
    UDR0 = data; // Envia o byte
}

int uart_available(void)
{
    return (UCSR0A & (1 << RXC0)); // Retorna 1 se chegou dado
}

// ---------- GPIO ----------
void setup_gpio(void)
{
    // LEDs e servo como sa�da
    DDRB |= (1 << LED_VERDE) | (1 << SERVO_PIN) | (1 << LED_VERMELHO) | (1 << LED_ACAO);

    // LEDs e buzzer no PORTD
    DDRD |= (1 << LED_STATUS) | (1 << LED_AMARELO) | (1 << BUZZER_PIN);

    DDRC &= ~(1 << PC0); // PC0 como entrada (LDR)
}

// ---------- ADC ----------
void setup_adc(void)
{
    ADMUX = (1 << REFS0);                                              // Refer�ncia AVCC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
}

uint16_t read_adc(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Seleciona canal
    ADCSRA |= (1 << ADSC);                     // Inicia convers�o
    while (ADCSRA & (1 << ADSC))
        ; // Aguarda
    return ADC;
}

// ---------- TIMER0 (BUZZER) ----------
void setup_timer0_buzzer(void)
{
    TCCR0A = (1 << WGM01);              // Modo CTC
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
    TIMSK0 = (1 << OCIE0A);             // Habilita interrup��o
}

void set_buzzer_freq(uint16_t freq)
{
    if (freq == 0)
    {
        alarmOn = 0;
    }
    else
    {
        uint8_t compareValue = (F_CPU / (2UL * 64UL * freq)) - 1;
        OCR0A = compareValue;
        alarmOn = 1;
    }
}

// ---------- TIMER1 (SERVO PWM) ----------
void setup_timer1_servo(void)
{
    DDRB |= (1 << SERVO_PIN);

    // Fast PWM, TOP = ICR1, prescaler 8
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    ICR1 = 39999;      // Define ciclo de 20ms
    OCR1A = SERVO_MIN; // Posi��o inicial
}

// ---------- FUN��ES GERAIS ----------
long map_value(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void apaga_leds(void)
{
    PORTB &= ~(1 << LED_VERDE);
    PORTD &= ~(1 << LED_AMARELO);
    PORTB &= ~(1 << LED_VERMELHO);
    PORTD &= ~(1 << LED_STATUS);
}

void pisca_led_status(void)
{
    PORTD |= (1 << LED_STATUS);
    _delay_ms(8);
    PORTD &= ~(1 << LED_STATUS);
    _delay_ms(8);
}

// ---------- COMANDO DO SERVO POR UART ----------
void verifica_acao_motor(void)
{
    if (uart_available())
    {
        char comando = UDR0;

        if (comando == 'A')
        {
            PORTB |= (1 << LED_ACAO);
            OCR1A = SERVO_MAX; // Abre servo
        }
        else if (comando == 'F')
        {
            PORTB &= ~(1 << LED_ACAO);
            OCR1A = SERVO_MIN; // Fecha servo
        }
    }
}

// ---------- SENSOR LDR / L�GICA DO SISTEMA ----------
int ler_sensor_e_atualizar(void)
{
    uint16_t ldrValue = read_adc(0); // L� ADC
    int valorLuminosidade = map_value(ldrValue, 0, 1023, 0, 100);

    apaga_leds();
    set_buzzer_freq(0);

    if (valorLuminosidade <= LIM_MUITO_BAIXA_MAX)
    {
        PORTB |= (1 << LED_VERMELHO);
        set_buzzer_freq(2000);
    }
    else if (valorLuminosidade <= LIM_BAIXA_MAX)
    {
        PORTB |= (1 << LED_VERMELHO);
    }
    else if (valorLuminosidade <= LIM_MEDIA_MAX)
    {
        PORTD |= (1 << LED_AMARELO);
    }
    else if (valorLuminosidade <= LIM_IDEAL_MAX)
    {
        PORTB |= (1 << LED_VERDE);
    }
    else if (valorLuminosidade <= LIM_MUITO_ALTA_MIN)
    {
        PORTD |= (1 << LED_AMARELO);
    }
    else
    {
        PORTB |= (1 << LED_VERMELHO);
        set_buzzer_freq(4000);
    }

    return valorLuminosidade;
}
