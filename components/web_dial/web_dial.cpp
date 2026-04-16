#include "web_dial.h"

#include <algorithm>
#include <cstdio>

#include <ESPAsyncWebServer.h>

#include "esphome/core/log.h"

namespace esphome {
namespace web_dial {

static const char *const TAG = "web_dial";

void WebDialComponent::setup() {
  if (this->web_server_base_ == nullptr || this->target_number_ == nullptr) {
    ESP_LOGE(TAG, "web_server_base and target_number must be configured");
    return;
  }

  const std::string path = this->normalized_path_();
  this->apply_value_(this->current_value_);

  this->web_server_base_->get_server()->on(path.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, "text/html", this->render_html_().c_str());
  });

  this->web_server_base_->get_server()->on((path + "/set").c_str(), HTTP_POST, [this](AsyncWebServerRequest *request) {
    if (!request->hasParam("value")) {
      request->send(400, "application/json", "{\"ok\":false,\"error\":\"missing value\"}");
      return;
    }

    const String value_str = request->getParam("value")->value();
    float value = value_str.toFloat();
    this->apply_value_(value);

    char body[96];
    snprintf(body, sizeof(body), "{\"ok\":true,\"value\":%.3f}", this->current_value_);
    request->send(200, "application/json", body);
  });

  ESP_LOGI(TAG, "Dial route ready at %s", path.c_str());
}

void WebDialComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Web Dial");
  ESP_LOGCONFIG(TAG, "  Path: %s", this->normalized_path_().c_str());
  ESP_LOGCONFIG(TAG, "  Range: %.3f -> %.3f (step %.3f)", this->min_value_, this->max_value_, this->step_);
}

float WebDialComponent::clamp_value_(float value) const { return std::max(this->min_value_, std::min(this->max_value_, value)); }

std::string WebDialComponent::normalized_path_() const {
  if (this->path_.empty()) {
    return "/dial";
  }
  if (this->path_[0] != '/') {
    return "/" + this->path_;
  }
  return this->path_;
}

void WebDialComponent::apply_value_(float value) {
  if (this->max_value_ < this->min_value_) {
    std::swap(this->min_value_, this->max_value_);
  }

  this->current_value_ = this->clamp_value_(value);
  auto call = this->target_number_->make_call();
  call.set_value(this->current_value_);
  call.perform();
}

std::string WebDialComponent::render_html_() const {
  const std::string path = this->normalized_path_();

  char html[8192];
  snprintf(
      html, sizeof(html),
      R"HTML(<!doctype html>
<html lang='en'>
<head>
  <meta charset='utf-8' />
  <meta name='viewport' content='width=device-width,initial-scale=1,viewport-fit=cover' />
  <title>ESP Dial</title>
  <style>
    :root { color-scheme: dark light; }
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      font-family: system-ui, -apple-system, Segoe UI, Roboto, sans-serif;
      background: radial-gradient(circle at top, #222, #111 60%%);
      color: #f7f7f7;
      touch-action: none;
    }
    .card {
      width: min(92vw, 420px);
      padding: 18px;
      border-radius: 18px;
      background: rgba(255, 255, 255, 0.06);
      box-shadow: 0 12px 32px rgba(0,0,0,0.35);
      backdrop-filter: blur(2px);
    }
    .title { margin: 0 0 10px; font-size: 18px; opacity: .85; }
    svg { width: 100%%; height: auto; display: block; }
    .value {
      text-align: center;
      font-size: clamp(30px, 10vw, 52px);
      margin-top: -12px;
      font-variant-numeric: tabular-nums;
    }
    .hint { text-align: center; opacity: .6; font-size: 13px; }
  </style>
</head>
<body>
  <main class='card'>
    <p class='title'>Dial Control</p>
    <svg id='dial' viewBox='0 0 320 220' aria-label='Dial control'>
      <path id='bg' stroke='rgba(255,255,255,.2)' stroke-width='20' fill='none' stroke-linecap='round'></path>
      <path id='fg' stroke='#66d9ff' stroke-width='20' fill='none' stroke-linecap='round'></path>
      <circle id='thumb' r='12' fill='#66d9ff'></circle>
    </svg>
    <div class='value'><span id='value'>--</span></div>
    <div class='hint'>Drag the arc to set value</div>
  </main>
<script>
(() => {
  const cfg = {
    min: %f,
    max: %f,
    step: %f,
    value: %f,
    path: %s
  };

  const startDeg = 135;
  const endDeg = 405;
  const sweep = endDeg - startDeg;

  const svg = document.getElementById('dial');
  const bg = document.getElementById('bg');
  const fg = document.getElementById('fg');
  const thumb = document.getElementById('thumb');
  const valueEl = document.getElementById('value');

  const cx = 160;
  const cy = 170;
  const r = 120;

  const toRad = (deg) => deg * Math.PI / 180;
  const pointAt = (deg) => ({ x: cx + r * Math.cos(toRad(deg)), y: cy + r * Math.sin(toRad(deg)) });
  const arcPath = (a0, a1) => {
    const p0 = pointAt(a0), p1 = pointAt(a1);
    const large = (a1 - a0) > 180 ? 1 : 0;
    return `M ${p0.x.toFixed(2)} ${p0.y.toFixed(2)} A ${r} ${r} 0 ${large} 1 ${p1.x.toFixed(2)} ${p1.y.toFixed(2)}`;
  };

  bg.setAttribute('d', arcPath(startDeg, endDeg));

  const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));
  const snap = (v) => Math.round(v / cfg.step) * cfg.step;

  function render(v) {
    const t = (v - cfg.min) / (cfg.max - cfg.min || 1);
    const deg = startDeg + t * sweep;
    fg.setAttribute('d', arcPath(startDeg, deg));
    const p = pointAt(deg);
    thumb.setAttribute('cx', p.x);
    thumb.setAttribute('cy', p.y);
    valueEl.textContent = v.toFixed(2);
  }

  function angleToValue(deg) {
    let d = deg;
    if (d < startDeg) d += 360;
    d = clamp(d, startDeg, endDeg);
    const t = (d - startDeg) / sweep;
    return snap(clamp(cfg.min + t * (cfg.max - cfg.min), cfg.min, cfg.max));
  }

  let sending = false;
  async function push(v) {
    if (sending) return;
    sending = true;
    try {
      await fetch(`${cfg.path}/set?value=${encodeURIComponent(v)}`, { method: 'POST' });
    } finally {
      sending = false;
    }
  }

  function fromEvent(ev) {
    const pt = svg.createSVGPoint();
    pt.x = ev.clientX;
    pt.y = ev.clientY;
    const p = pt.matrixTransform(svg.getScreenCTM().inverse());
    const deg = (Math.atan2(p.y - cy, p.x - cx) * 180 / Math.PI + 360) %% 360;
    cfg.value = angleToValue(deg);
    render(cfg.value);
    push(cfg.value);
  }

  svg.addEventListener('pointerdown', (ev) => { svg.setPointerCapture(ev.pointerId); fromEvent(ev); });
  svg.addEventListener('pointermove', (ev) => { if (ev.buttons) fromEvent(ev); });

  render(cfg.value);
})();
</script>
</body>
</html>)HTML",
      this->min_value_, this->max_value_, this->step_, this->current_value_, ("'" + path + "'").c_str());

  return std::string(html);
}

}  // namespace web_dial
}  // namespace esphome
