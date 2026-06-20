# Action Plan

## Current status

- Project is an ESP32 IR remote controller built with PlatformIO and Arduino.
- Firmware already exists in `src/main.cpp`.
- Current features:
  - Wi-Fi AP fallback (`ESP32-IR-Controller`)
  - Optional STA connection placeholders
  - REST endpoints for fixed IR commands
  - Serial IR sniffing for discovering codes
- Current gaps:
  - Wi-Fi SSID/password are still empty
  - No mobile app/client yet
  - No authentication
  - No persistence for learned commands
  - No structured API spec

## Goal

Build a phone-controlled universal IR gateway that can send commands to an ESP32 over the internet or local Wi-Fi, then emit IR signals reliably to the selected device category.

## Architecture principles

- The phone app should first choose a control category such as fan, light, TV, or AC.
- Each category should map to its own command set and UI layout.
- Keep the data model flat and simple; avoid a heavy nested storage structure.
- Prefer a small, extensible command registry over a complex database schema.

## Phases

### 1. Stabilize firmware

- Move Wi-Fi credentials and device config to a safer config flow.
- Keep AP mode as fallback, but make STA the primary path.
- Add health/status endpoint details.
- Standardize API responses and error codes.

### 2. Expand command management

- Replace hardcoded commands with a maintainable command table.
- Group commands by device category so the app can switch screens cleanly.
- Add command aliases and metadata.
- Support saving and loading learned IR codes.
- Ignore repeat frames when capturing new codes.

### 3. Add remote access layer

- Decide the internet path:
  - direct public API with port forwarding, or
  - a relay/server model, or
  - a tunnel/VPN approach
- Add authentication and basic request protection.
- Define the API contract for the mobile app.

### 4. Build the mobile app

- Create a simple control UI with buttons for each IR action.
- Show device online/offline state.
- Add Wi-Fi setup or device pairing flow.
- Add command history and retry feedback.

### 5. Improve reliability

- Add rate limiting for IR send actions.
- Add logging for send failures and Wi-Fi reconnects.
- Handle reboot/reconnect recovery cleanly.
- Validate protocol support before sending.

### 6. Polish and document

- Add a README for setup, wiring, and usage.
- Document endpoints and command naming.
- Document the category-to-command mapping so future device types can be added without redesign.
- Add a project checklist for future expansions.

## Suggested next actions

1. Define the remote-access architecture.
2. Define the device category model for fan/light/TV/etc.
3. Replace hardcoded Wi-Fi settings with configurable setup.
4. Document the API endpoints.
5. Start the mobile app skeleton.
