# Requisiti di Progetto

## Funzionali
| Codice | Descrizione                               | Priorità |
|--------|-------------------------------------------|----------|
| RF-ESP-OTA | Aggiornare ESP32 via HTTPS o altro metodo remoto           | Alta |
| RF-ESP-INT | Integrazione attuale firmware nel nuovo sistema           | Alta |
| RF-STM-OTA | Aggiornametno OTA STM32F446, inoltrare aggiornamento firmware STM32 su UART proveniente via ESP32     | Media |
| RF-UART-ENC| Cifratura AES-128 CTR sulla UART con meccanismo basato su ID microcontrollore      | Media |

## Non funzionali
- Server OTA basato su **GitHub Releases** e manifest.json oppure framework ESPHOME.
- Nel primo caso, Partizionamento ESP32 a doppio slot + rollback.
