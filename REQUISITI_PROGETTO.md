# Requisiti di Progetto

## Funzionali
| Codice | Descrizione                               | Priorità |
|--------|-------------------------------------------|----------|
| RF-ESP-OTA | Aggiornare ESP32 via HTTPS            | Alta |
| RF-STM-OTA | Inoltrare firmware STM32 su UART      | Alta |
| RF-UART-ENC| Cifratura AES-128 CTR sulla UART      | Media |

## Non funzionali
- Server OTA basato su **GitHub Releases** e manifest.json.
- Partizionamento ESP32 a doppio slot + rollback.
