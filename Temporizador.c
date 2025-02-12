#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/netif.h"
#include "lwip/dns.h"  // Include this header for dns_gethostbyname
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"  // Include this header for ip_addr_t

#define LED_PIN 25  // LED embutido no RP2040
#define TEMPO_TOTAL 10  // Tempo da contagem regressiva (segundos)

#define WIFI_SSID "WIFI"
#define WIFI_PASS "102030405060"
#define MQTT_BROKER "mqtt3.thingspeak.com"
#define MY_MQTT_PORT 1883
#define API_KEY "8FHDBM9NHYPCE9VF" // Chave MQTT do ThingSpeak
#define CHANNEL_ID "2836844" // Channel ID do ThingSpeak

mqtt_client_t *mqtt_client;

void publish_message(int tempo) {
    char topic[100], payload[50];
    snprintf(topic, sizeof(topic), "channels/%s/publish/%s", CHANNEL_ID, API_KEY);
    snprintf(payload, sizeof(payload), "field1=%d", tempo);

    mqtt_publish(mqtt_client, topic, payload, strlen(payload), 0, 0, NULL, NULL);
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT conectado com sucesso!\n");
    } else {
        printf("Falha na conexão MQTT. Status: %d\n", status);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar ao Wi-Fi\n");
        return 1;
    }
    
    printf("Wi-Fi conectado!\n");

    // Resolver o endereço IP do broker MQTT
    ip_addr_t mqtt_broker_ip;
    if (dns_gethostbyname(MQTT_BROKER, &mqtt_broker_ip, NULL, NULL) != ERR_OK) {
        printf("Erro ao resolver o endereço do broker MQTT\n");
        return 1;
    }

    // Informações do cliente MQTT
    struct mqtt_connect_client_info_t client_info = {
        .client_id = "pico_timer",
        .client_user = NULL,
        .client_pass = NULL,
        .keep_alive = 60
    };

    // Conectar ao servidor MQTT (ThingSpeak)
    mqtt_client = mqtt_client_new();
    if (mqtt_client == NULL) {
        printf("Erro ao criar cliente MQTT\n");
        return 1;
    }
    mqtt_client_connect(mqtt_client, &mqtt_broker_ip, MY_MQTT_PORT, mqtt_connection_cb, NULL, &client_info);

    printf("Temporizador iniciado: %d segundos\n", TEMPO_TOTAL);
    
    for (int i = TEMPO_TOTAL; i > 0; i--) {
        printf("Tempo restante: %d segundos\n", i);
        publish_message(i);  // Envia para o ThingSpeak
        sleep_ms(1000);
    }

    printf("Tempo esgotado! Enviando alerta.\n");
    publish_message(0);  // Envia o alerta final
    
    for (int i = 0; i < 10; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
        sleep_ms(200);
    }

    return 0;
}
