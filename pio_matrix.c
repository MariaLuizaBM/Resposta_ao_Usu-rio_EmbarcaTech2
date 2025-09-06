#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

//arquivo .pio
#include "pio_matrix.pio.h"

// Configurações da matriz WS2812
#define LED_PIN 7
#define NUM_PIXELS 25
#define MATRIZ_LARGURA 5
#define MATRIZ_ALTURA 5

// Buffer da matriz (cores em GRB)
static uint32_t matriz_leds[NUM_PIXELS];

// Variáveis do PIO
PIO pio = pio0;
uint sm;

// ---------------- Funções ---------------- //

// Converte coordenada (x,y) para índice do LED no array
int coordenada_para_indice(int x, int y) {
    if (x < 0 || x >= MATRIZ_LARGURA || y < 0 || y >= MATRIZ_ALTURA) {
        return -1; 
    }

    if (y % 2 == 0) {
        // linhas pares: esquerda -> direita
        return y * MATRIZ_LARGURA + x;
    } else {
        // linhas ímpares: direita -> esquerda
        return y * MATRIZ_LARGURA + (MATRIZ_LARGURA - 1 - x);
    }
}

// Envia buffer completo para a matriz
void atualiza_matriz() {
    for (int i = 0; i < NUM_PIXELS; i++) {
        // WS2812 usa formato GRB (shift p/ alinhar com protocolo)
        uint32_t cor = matriz_leds[i];
        pio_sm_put_blocking(pio, sm, cor << 8u);
    }
}

// Acende LED em (x,y) com uma cor RGB
void acende_led_matriz(int x, int y, uint32_t cor) {
    int idx = coordenada_para_indice(x, y);
    if (idx >= 0) {
        matriz_leds[idx] = cor;
        atualiza_matriz();
    }
}

// Apaga LED em (x,y)
void apaga_led_matriz(int x, int y) {
    int idx = coordenada_para_indice(x, y);
    if (idx >= 0) {
        matriz_leds[idx] = 0x000000; // preto
        atualiza_matriz();
    }
}

// ---------------- Programa Principal ---------------- //

int main() {
    stdio_init_all();

    // Inicializa PIO para WS2812
    uint offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, LED_PIN);

    // Limpa a matriz no início
    for (int i = 0; i < NUM_PIXELS; i++) {
        matriz_leds[i] = 0x000000;
    }
    atualiza_matriz();

    printf("=== Controle de LEDs na Matriz 5x5 ===\n");
    printf("Digite no formato: A x y r g b  -> acende LED\n");
    printf("                   P x y        -> apaga LED\n\n");

    while (true) {
        char cmd;
        int x, y, r, g, b;

        // Lê comando
        if (scanf(" %c", &cmd) == 1) {
            if (cmd == 'A') {
                if (scanf("%d %d %d %d %d", &x, &y, &r, &g, &b) == 5) {
                    uint32_t cor = ((g & 0xFF) << 16) | ((r & 0xFF) << 8) | (b & 0xFF);
                    acende_led_matriz(x, y, cor);
                    printf("LED (%d,%d) aceso com cor RGB(%d,%d,%d)\n", x, y, r, g, b);
                }
            } else if (cmd == 'P') {
                if (scanf("%d %d", &x, &y) == 2) {
                    apaga_led_matriz(x, y);
                    printf("LED (%d,%d) apagado\n", x, y);
                }
            }
        }
        sleep_ms(100);
    }
}
