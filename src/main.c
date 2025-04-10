// File: src/main.c
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

// UART initialization for 9600 baud (assuming F_CPU = 16MHz)
void uart_init(void) {
    // Enable transmitter only.
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8 data bits, no parity, 1 stop bit (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Set baud rate for 9600 baud: UBRR = (F_CPU/(16*baud))-1 = 103
    UBRR0H = 0;
    UBRR0L = 103;
}

// Transmit a single character over UART.
void uart_transmit(char data) {
    // Wait until the transmit buffer is empty.
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

// Transmit a string over UART.
void uart_print(const char *str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

// ADC Initialization for reading voltage on A15
void adc_init(void) {
    // For many devices (e.g., ATmega2560):
    // - Use AVcc as the reference voltage (REFS0 = 1).
    // - To read A15, set lower multiplexer bits to 7 and set the extended bit (MUX5).
    ADMUX = (1 << REFS0) | (7 & 0x07);  // Lower MUX bits = 7
    ADCSRB = (1 << MUX5);               // Extended MUX bit to select channel A15 (8+7)
    
    // Enable ADC and set prescaler to 128 for a safe ADC clock (assuming 16 MHz system clock).
    // (Division factor: 128 gives an ADC clock ≈ 125 kHz.)
    ADCSRA = (1 << ADEN) | (7 << ADPS0);
}

// Read a 10-bit value from the ADC.
uint16_t adc_read(void) {
    // Start the ADC conversion.
    ADCSRA |= (1 << ADSC);
    // Wait until the conversion is complete (ADSC becomes 0).
    while (ADCSRA & (1 << ADSC))
        ;
    return ADC;
}

int main(void) {
    char buffer[16];

    // Initialize UART for output.
    uart_init();
    // Initialize the ADC to read from A15.
    adc_init();

    // Wait a moment for the hardware (UART and ADC) to stabilize.
    _delay_ms(1000);

    while (1) {
        // Read raw ADC value.
        uint16_t adcValue = adc_read();
        // Convert the ADC value (0–1023) to voltage, assuming a 5V reference.
        // (Voltage = ADC_value * Vref / 1023)
        float voltage = (adcValue * 5.0) / 1023.0;

        // Format the voltage into the buffer with two decimals.
        snprintf(buffer, sizeof(buffer), "Voltage: %.2f V", voltage);
        // Print the formatted voltage over UART.
        uart_print(buffer);
        uart_print("\r\n");

        _delay_ms(1000);  // Wait 1 second before the next reading.
    }
    
    return 0;
}
