# web_dial agent knowledge

- Integration is monolithic in `__init__.py` and requires `web_server_base` + `number`.
- Runtime exposes two routes under `path`:
  - `GET <path>` serves the dial UI.
  - `POST <path>/set?value=<float>` applies and forwards the value to `target_number`.
- Value forwarding uses `number::Number::make_call().set_value(...).perform()`.
