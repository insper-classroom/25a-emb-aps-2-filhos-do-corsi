/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"



QueueHandle_t xQueueData;


// joystick
typedef struct adc {
    int axis;
    int val;
} adc_t;


// joystick
void x_task(void *p){
    adc_init();
    adc_gpio_init(26);

    int x_buffer[5] = {0}; 
    int x_index = 0; 

    while (1) {
        adc_select_input(1);  // Canal X
        uint16_t result = adc_read();
        
        int escala = ((result - 2047)*255)/2047; 

        if (escala>-60 && escala<60) {
            escala = 0;
        }


        x_buffer[x_index] = escala;
        x_index++;
        if (x_index >= 5) {
            x_index = 0;  
        }    
        int sum = 0; // media movel aq em baixo(tirar ruido)
        for (int i = 0; i < 5; i++) {
            sum += x_buffer[i];
        }
        int avg = sum / 5;
        
        if (avg != 0) {
            adc_t data = {0, avg};  // 0 para eixo X
            xQueueSend(xQueueData, &data, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// joystick
void y_task(void *p){
    adc_init();
    adc_gpio_init(27);

    int y_buffer[5] = {0}; 
    int y_index = 0;    

    while (1) {
        adc_select_input(0);  // Canal Y
        uint16_t result = adc_read();
        
        int escala = (result - 2047)*255/2047;

        if (escala>-60 && escala<60) {
            escala = 0;
        }

        y_buffer[y_index] = escala;
        y_index++;
        if (y_index >= 5) {
            y_index = 0;  
        }    
        int sum = 0; 
        for (int i = 0; i < 5; i++) {
            sum += y_buffer[i];
        }
        int avg = sum / 5;

        
        if (avg!=0) {
            adc_t data = {1, avg};
            xQueueSend(xQueueData, &data, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


// joystick
void uart_task(void *p) {
    while (1) {
        adc_t data;
        if (xQueueReceive(xQueueData, &data, portMAX_DELAY)) {
            uint8_t bytes[4];
            bytes[0] = (uint8_t)data.axis; 
            bytes[1] = (uint8_t)((data.val >> 8) & 0xFF); 
            bytes[2] = (uint8_t)(data.val & 0xFF);
            bytes[3] = 0xFF; 

            uart_write_blocking(uart0, bytes, sizeof(bytes));
        }
    }
}

void Botao_ataque(void *p){
    gpio_init(17);
    gpio_set_dir(17, GPIO_IN); 
    gpio_pull_up(17); 


    while (1) {
        if (gpio_get(17) == 0) { 
            printf("Botão ataque\n");

            uint8_t botao_msg[4] = {0xFF, 2, 0x00, 0x63};  
            uart_write_blocking(uart0, botao_msg, sizeof(botao_msg));

            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void botao_ataque_pesado(void *p){
    gpio_init(21);
    gpio_set_dir(21, GPIO_IN); 
    gpio_pull_up(21); 

    while (1) {
        if (gpio_get(21) == 0) { 
            printf("Botão ataque pesado\n");

            uint8_t botao_msg[4] = {0xFF, 3, 0x00, 0x70};  
            uart_write_blocking(uart0, botao_msg, sizeof(botao_msg));

            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}


void botao_flecha(void *p){
    gpio_init(16);
    gpio_set_dir(16, GPIO_IN); 
    gpio_pull_up(16); 

    while (1) {
        if (gpio_get(16) == 0) { 
            printf("Botão pulo\n");  // Verifique se está imprimindo aqui

            uint8_t botao_msg[4] = {0xFF, 4, 0x00, 0x66};  
            uart_write_blocking(uart0, botao_msg, sizeof(botao_msg));

            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void combo(void *p){
    gpio_init(18);
    gpio_set_dir(18, GPIO_IN); 
    gpio_pull_up(18); 

    uint32_t last_press_time = 0;
    const uint32_t debounce_delay = 200; // tempo de debounce em ms

    while (1) {
        if (gpio_get(18) == 0) { 
            // Verifica se o botão foi pressionado por tempo suficiente (debouncing)
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            if (current_time - last_press_time > debounce_delay) {
                printf("Botão combo\n");

                uint8_t botao_msg[4] = {0xFF, 5, 0x00, 0x66};  
                uart_write_blocking(uart0, botao_msg, sizeof(botao_msg));

                last_press_time = current_time;  // Atualiza o tempo do último pressionamento
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void led_carregamento_task(void *p) {
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);

    while (1) {
        if (uart_is_readable(uart0)) {
            char comando = uart_getc(uart0);

            if (comando == 'L') {
                gpio_put(2, 1);  // Acende o LED
            } else if (comando == 'D') {
                gpio_put(2, 0);  // Apaga o LED
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // pequena espera
    }
}


int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);  

    xQueueData = xQueueCreate(2, sizeof(adc_t)); // tamanho adc
  
    xTaskCreate(x_task, "X Task", 256, NULL, 1, NULL);
    xTaskCreate(y_task, "Y Task", 256, NULL, 1, NULL);
    xTaskCreate(uart_task, "UART Task", 256, NULL, 1, NULL);
    xTaskCreate(Botao_ataque, "Botao_ataque", 256, NULL, 1, NULL);
    xTaskCreate(botao_ataque_pesado, "Botao_ataque_pesado", 256, NULL, 1, NULL);
    xTaskCreate(botao_flecha, "botao_flecha", 256, NULL, 1, NULL);
    xTaskCreate(combo, "combo", 256, NULL, 1, NULL);
    xTaskCreate(led_carregamento_task, "LED Carregando", 256, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
