# phong_ir

ESP32-based universal IR controller, local-first.

## Current architecture

- `src/main.cpp`: firmware entrypoint, Wi-Fi, light command table, boot flow
- `src/api/`: HTTP routes and JSON responses
- `src/support/`: IR sniffing helpers

## Light endpoints

- `GET /api/status`
- `GET /api/ir/send?command=light_toggle`
- `GET /light/toggle`
- `GET /light/temp/down`
- `GET /light/temp/up`
- `GET /light/mode/3`
- `GET /light/brightness/down`

## Notes

- Light codes live in `src/main.cpp`.
- IR sniffing is kept in `src/support/`.
- Storage is intentionally omitted for now to keep the structure simple.
