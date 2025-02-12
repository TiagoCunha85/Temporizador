#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/dns.h"  // Move para cima
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "hardware/watchdog.h"

// Configurações
#define LED_PIN 25
#define TEMPO_TOTAL 10
#define WIFI_RETRY_DELAY_MS 5000
#define MQTT_RETRY_DELAY_MS 5000
#define MAX_RECONNECT_ATTEMPTS 5

// Credenciais e configurações de rede
#define WIFI_SSID "WIFI"
#define WIFI_PASS "102030405060"
#define MQTT_BROKER "mqtt3.thingspeak.com"
#define MY_MQTT_PORT 1883
#define API_KEY "8FHDBM9NHYPCE9VF"
#define CHANNEL_ID "2836844"

// Variáveis globais
static mqtt_client_t *mqtt_client;
static volatile bool mqtt_connected = false;
static volatile bool wifi_connected = false;
static absolute_time_t last_watchdog_reset;

// Protótipos de funções
static void init_watchdog(void);
static bool connect_wifi(void);
static void reset_watchdog(void);
static bool connect_mqtt(const ip_addr_t *broker_ip);

void publish_message(int tempo) {
    if (!mqtt_connected) {
        printf("MQTT não conectado. Tentando reconectar...\n");
        return;
    }

    char topic[100], payload[50];
    snprintf(topic, sizeof(topic), "channels/%s/publish/%s", CHANNEL_ID, API_KEY);
    snprintf(payload, sizeof(payload), "field1=%d", tempo);

    err_t err = mqtt_publish(mqtt_client, topic, payload, strlen(payload), 0, 0, NULL, NULL);
    if (err != ERR_OK) {
        printf("Erro ao publicar mensagem: %d\n", err);
    }
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    mqtt_connected = (status == MQTT_CONNECT_ACCEPTED);
    printf("Status MQTT: %s\n", mqtt_connected ? "Conectado" : "Desconectado");
}

void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr) {
        ip_addr_t *mqtt_broker_ip = (ip_addr_t *)callback_arg;
        *mqtt_broker_ip = *ipaddr;
    }
}

static void init_watchdog(void) {
    watchdog_enable(3000, true);
    last_watchdog_reset = get_absolute_time();
}

static void reset_watchdog(void) {
    if (absolute_time_diff_us(last_watchdog_reset, get_absolute_time()) >= 2000000) {
        watchdog_update();
        last_watchdog_reset = get_absolute_time();
    }
}

static bool connect_wifi(void) {
    printf("Conectando ao WiFi...\n");
    
    int attempts = 0;
    while (attempts < MAX_RECONNECT_ATTEMPTS) {
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, 
                                             CYW43_AUTH_WPA2_AES_PSK, 10000) == 0) {
            wifi_connected = true;
            printf("WiFi conectado!\n");
            return true;
        }
        
        printf("Tentativa %d falhou. Aguardando antes de reconectar...\n", attempts + 1);
        sleep_ms(WIFI_RETRY_DELAY_MS);
        attempts++;
    }
    
    return false;
}

static bool connect_mqtt(const ip_addr_t *broker_ip) {
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
        return false;
    }
    mqtt_client_connect(mqtt_client, broker_ip, MY_MQTT_PORT, mqtt_connection_cb, NULL, &client_info);
    return true;
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
    if (!connect_wifi()) {
        printf("Erro ao conectar ao Wi-Fi\n");
        return 1;
    }
    
    printf("Wi-Fi conectado!\n");

    // Resolver o endereço IP do broker MQTT
    ip_addr_t mqtt_broker_ip;
    err_t err = dns_gethostbyname(MQTT_BROKER, &mqtt_broker_ip, dns_callback, &mqtt_broker_ip);
    if (err == ERR_INPROGRESS) {
        // A resolução DNS está em andamento, aguarde a conclusão
        while (mqtt_broker_ip.addr == IPADDR_ANY) {
            cyw43_arch_poll();
        }
    } else if (err != ERR_OK) {
        printf("Erro ao resolver o endereço do broker MQTT\n");
        return 1;
    }

    if (!connect_mqtt(&mqtt_broker_ip)) {
        printf("Falha ao estabelecer conexão MQTT\n");
        return 1;
    }

    printf("Iniciando temporizador: %d segundos\n", TEMPO_TOTAL);
    
    for (int i = TEMPO_TOTAL; i > 0; i--) {
        reset_watchdog();
        printf("Tempo restante: %d segundos\n", i);
        publish_message(i);
        
        // Piscar LED durante a contagem
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(900);
    }

    printf("Tempo esgotado! Enviando alerta final.\n");
    publish_message(0);
    
    // Piscar LED rapidamente ao finalizar
    for (int i = 0; i < 10; i++) {
        reset_watchdog();
        gpio_put(LED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
        sleep_ms(200);
    }

    return 0;
}
