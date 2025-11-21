/*
 * GccAplicacaoPlantGuard.c
 *
 * Created: 21/11/2025 12:37:28
 * Microcontrolador: ATmega328P
 * Author : sinvalluz
 */ 

#define F_CPU 16000000UL   // Define o clock utilizado pelo microcontrolador

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>        // Usado para itoa()

// ---------- DEFINIÇÃO DE PINOS ----------
#define BUZZER_PIN    PD7
#define LED_STATUS    PD3
#define LED_ACAO      PB5

#define SERVO_PIN     PB1  
#define LED_VERDE     PB0
#define LED_AMARELO   PD4  
#define LED_VERMELHO  PB2

// Limites do sinal PWM do servo (em contagens do timer)
#define SERVO_MIN     2000     // ~1ms
#define SERVO_MAX     4400     // ~2.2ms

// Limites de luminosidade, convertidos em % (0-100)
#define LIM_MUITO_BAIXA_MAX 15
#define LIM_BAIXA_MAX       30
#define LIM_MEDIA_MAX       50
#define LIM_IDEAL_MAX       70
#define LIM_MUITO_ALTA_MIN  90

volatile uint8_t alarmOn = 0;  // Flag indicando se o buzzer deve tocar

// ---------- PROTÓTIPOS ----------
void setup_gpio(void);
void setup_adc(void);
uint16_t read_adc(uint8_t channel);
void setup_uart(unsigned int ubrr);
void uart_transmit(unsigned char data);
int uart_available(void);
void uart_print_text(const char *str);
void setup_timer0_buzzer(void);
void setup_timer1_servo(void);
void set_buzzer_freq(uint16_t freq);
long map_value(long x, long in_min, long in_max, long out_min, long out_max);
void apaga_leds(void);
void pisca_led_status(void);
void verifica_acao_motor(void);
int ler_sensor_e_atualizar(void);

// ---------- INTERRUPÇÃO DO TIMER0 (BUZZER) ----------
ISR(TIMER0_COMPA_vect) {
    if (alarmOn) {
        PORTD ^= (1 << BUZZER_PIN);  // Alterna o pino — gera o som
    } else {
        PORTD &= ~(1 << BUZZER_PIN); // Garante buzzer desligado
    }
}

int main(void) {
    char stringLuminosidade[10];
    int valorLuminosidade;

    // Inicializações
    setup_gpio();
    setup_adc();
    setup_uart(103);             // UBRR = 103 ? 9600 baud
    setup_timer0_buzzer();
    setup_timer1_servo();
    
    sei();                       // Habilita interrupções globais
    
    apaga_leds();
    PORTD &= ~(1 << LED_STATUS);

    OCR1A = SERVO_MIN;          // Servo começa fechado

    while (1) {
        // Lê sensor e atualiza LEDs/buzzer
        valorLuminosidade = ler_sensor_e_atualizar();
        
        // Converte para texto
        itoa(valorLuminosidade, stringLuminosidade, 10);

        // Envia via UART
        uart_print_text(stringLuminosidade);
        uart_transmit('\n');

        verifica_acao_motor();  // Processa comandos UART ('A' / 'F')
        pisca_led_status();     // Pisca LED de atividade
        
        _delay_ms(500);
    }
}

// ---------- FUNÇÕES UART ----------
void uart_print_text(const char *str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void setup_uart(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);  // Baud rate parte alta
    UBRR0L = (unsigned char)ubrr;         // Baud rate parte baixa
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Habilita RX e TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits
}

void uart_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));  // Espera buffer liberar
    UDR0 = data;                       // Envia o byte
}

int uart_available(void) {
    return (UCSR0A & (1 << RXC0));     // Retorna 1 se chegou dado
}

// ---------- GPIO ----------
void setup_gpio(void) {
    // LEDs e servo como saída
    DDRB |= (1 << LED_VERDE) | (1 << SERVO_PIN) | (1 << LED_VERMELHO) | (1 << LED_ACAO);
    
    // LEDs e buzzer no PORTD
    DDRD |= (1 << LED_STATUS) | (1 << LED_AMARELO) | (1 << BUZZER_PIN);

    DDRC &= ~(1 << PC0); // PC0 como entrada (LDR)
}

// ---------- ADC ----------
void setup_adc(void) {
    ADMUX = (1 << REFS0); // Referência AVCC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
}

uint16_t read_adc(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Seleciona canal
    ADCSRA |= (1 << ADSC);      // Inicia conversão
    while (ADCSRA & (1 << ADSC)); // Aguarda
    return ADC;
}

// ---------- TIMER0 (BUZZER) ----------
void setup_timer0_buzzer(void) {
    TCCR0A = (1 << WGM01);       // Modo CTC
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
    TIMSK0 = (1 << OCIE0A);      // Habilita interrupção
}

void set_buzzer_freq(uint16_t freq) {
    if (freq == 0) {
        alarmOn = 0;
    } else {
        uint8_t compareValue = (F_CPU / (2UL * 64UL * freq)) - 1;
        OCR0A = compareValue;
        alarmOn = 1;
    }
}

// ---------- TIMER1 (SERVO PWM) ----------
void setup_timer1_servo(void) {
    DDRB |= (1 << SERVO_PIN);

    // Fast PWM, TOP = ICR1, prescaler 8
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    ICR1 = 39999;           // Define ciclo de 20ms
    OCR1A = SERVO_MIN;      // Posição inicial
}

// ---------- FUNÇÕES GERAIS ----------
long map_value(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void apaga_leds(void) {
    PORTB &= ~(1 << LED_VERDE);
    PORTD &= ~(1 << LED_AMARELO);
    PORTB &= ~(1 << LED_VERMELHO);
    PORTD &= ~(1 << LED_STATUS);
}

void pisca_led_status(void) {
    PORTD |= (1 << LED_STATUS);
    _delay_ms(8);
    PORTD &= ~(1 << LED_STATUS);
    _delay_ms(8);
}

// ---------- COMANDO DO SERVO POR UART ----------
void verifica_acao_motor(void) {
    if (uart_available()) {
        char comando = UDR0;

        if (comando == 'A') {
            PORTB |= (1 << LED_ACAO);
            OCR1A = SERVO_MAX; // Abre servo
        }
        else if (comando == 'F') {
            PORTB &= ~(1 << LED_ACAO);
            OCR1A = SERVO_MIN; // Fecha servo
        }
    }
}

// ---------- SENSOR LDR / LÓGICA DO SISTEMA ----------
int ler_sensor_e_atualizar(void) {
    uint16_t ldrValue = read_adc(0);               // Lê ADC
    int valorLuminosidade = map_value(ldrValue, 0, 1023, 0, 100);

    apaga_leds();
    set_buzzer_freq(0);

    if (valorLuminosidade <= LIM_MUITO_BAIXA_MAX) {
        PORTB |= (1 << LED_VERMELHO);
        set_buzzer_freq(2000);
    }
    else if (valorLuminosidade <= LIM_BAIXA_MAX) {
        PORTB |= (1 << LED_VERMELHO);
    }
    else if (valorLuminosidade <= LIM_MEDIA_MAX) {
        PORTD |= (1 << LED_AMARELO);
    }
    else if (valorLuminosidade <= LIM_IDEAL_MAX) {
        PORTB |= (1 << LED_VERDE);
    }
    else if (valorLuminosidade <= LIM_MUITO_ALTA_MIN) {
        PORTD |= (1 << LED_AMARELO);
    }
    else {
        PORTB |= (1 << LED_VERMELHO);
        set_buzzer_freq(4000);
    }

    return valorLuminosidade;
}


