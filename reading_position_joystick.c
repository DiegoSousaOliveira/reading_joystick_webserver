#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include <math.h>

#define WIFI_SSID "Labirang"
#define WIFI_PASSWORD "1fp1*007"

#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1

typedef struct {
    uint x;
    uint y;
    const char* direction;
} JoystickData;

volatile JoystickData joystick;

void init_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void read_joystick() {
    adc_select_input(ADC_CHANNEL_0);
    sleep_us(10);
    joystick.x = adc_read();
    
    adc_select_input(ADC_CHANNEL_1);
    sleep_us(10);
    joystick.y = adc_read();

    float x_norm = ((int)joystick.x - 2048) / 2048.0f;
    float y_norm = ((int)joystick.y - 2048) / 2048.0f;

    float angle = atan2f(y_norm, x_norm) * 180.0f / M_PI;
    if (angle < 0) angle += 360.0f;

    if (joystick.x > 2000 && joystick.x < 2100 && joystick.y > 2000 && joystick.y < 2100) {
        joystick.direction = "Centro";
    } else if (angle >= 337.5 || angle < 22.5) {
        joystick.direction = "Norte";
    } else if (angle >= 22.5 && angle < 67.5) {
        joystick.direction = "Nordeste";
    } else if (angle >= 67.5 && angle < 112.5) {
        joystick.direction = "Leste";
    } else if (angle >= 112.5 && angle < 157.5) {
        joystick.direction = "Sudeste";
    } else if (angle >= 157.5 && angle < 202.5) {
        joystick.direction = "Sul";
    } else if (angle >= 202.5 && angle < 247.5) {
        joystick.direction = "Sudoeste";
    } else if (angle >= 247.5 && angle < 292.5) {
        joystick.direction = "Oeste";
    } else if (angle >= 292.5 && angle < 337.5) {
        joystick.direction = "Noroeste";
    } else {
        joystick.direction = "Indefinido";
    }
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    read_joystick();

    char html[2048];
    snprintf(html, sizeof(html),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "<meta charset='UTF-8'>\n"
             "<title>Joystick Control</title>\n"
             "<style>\n"
             "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
             "h1 { font-size: 48px; margin-bottom: 30px; }\n"
             ".joystick-info { font-size: 36px; margin: 20px; }\n"
             "</style>\n"
             "</head>\n"
             "<body>\n"
             "<h1>Posição do Joystick</h1>\n"
             "<div class=\"joystick-info\">Eixo X: %d</div>\n"
             "<div class=\"joystick-info\">Eixo Y: %d</div>\n"
             "<div class=\"joystick-info\">Direção: %s</div>\n"
             "</body>\n"
             "</html>\n",
             joystick.x, joystick.y, joystick.direction);

    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    free(request);
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

int main() {
    stdio_init_all();

    init_joystick();

    if (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return -1;
    }

    printf("Conectado ao Wi-Fi\n");

    if (netif_default) {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    struct tcp_pcb *server = tcp_new();
    if (!server) {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    printf("Servidor ouvindo na porta 80\n");

    while (true) {
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}
