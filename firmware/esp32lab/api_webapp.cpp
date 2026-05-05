// api_webapp.cpp — Serves the ESP32 Lab web app directly from firmware flash.
// All web files are embedded as raw string literals — no LittleFS needed.

#include "config.h"
#include "api_webapp.h"
#include "api_server.h"

// ── index.html ────────────────────────────────────────────────────────────────

static const char FILE_INDEX[] = R"WEBEND(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="theme-color" content="#38bdf8">
    <title>ESP32 Lab</title>
    <link rel="manifest" href="manifest.json">
    <link rel="stylesheet" href="css/style.css">
</head>
<body>
    <div id="connection-bar">
        <div class="conn-row">
            <input type="text" id="ip-input" value="192.168.4.1" placeholder="IP address">
            <button id="connect-btn">Connect</button>
            <span id="status-dot" class="dot disconnected"></span>
            <span id="status-text">Disconnected</span>
        </div>
        <div id="sys-summary" class="hidden"></div>
    </div>
    <nav id="tab-bar" role="tablist">
        <button class="tab active" data-tab="system" role="tab">System</button>
        <button class="tab" data-tab="gpio" role="tab">GPIO</button>
        <button class="tab" data-tab="grove" role="tab">Sensors</button>
    </nav>
    <main id="tab-content">
        <section id="tab-system" class="tab-panel active" role="tabpanel"></section>
        <section id="tab-gpio"   class="tab-panel" role="tabpanel"></section>
        <section id="tab-grove"  class="tab-panel" role="tabpanel"></section>
    </main>
    <script type="module" src="js/app.js"></script>
</body>
</html>
)WEBEND";

// ── css/style.css ─────────────────────────────────────────────────────────────

static const char FILE_STYLE_CSS[] = R"WEBEND(*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg: #0f172a;
    --surface: #1e293b;
    --surface2: #334155;
    --border: #475569;
    --text: #f1f5f9;
    --text-dim: #94a3b8;
    --accent: #38bdf8;
    --accent-dim: #0ea5e9;
    --green: #4ade80;
    --red: #f87171;
    --yellow: #fbbf24;
    --orange: #fb923c;
}

html, body {
    height: 100%; width: 100%;
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
    font-size: 15px;
    background: var(--bg);
    color: var(--text);
    overflow: hidden;
    -webkit-user-select: none; user-select: none;
}

#connection-bar {
    background: var(--surface);
    border-bottom: 2px solid var(--accent);
    padding: 10px 14px;
}

.conn-row { display: flex; align-items: center; gap: 8px; }

#ip-input {
    flex: 1; max-width: 160px;
    padding: 8px 10px; border-radius: 8px;
    border: 1px solid var(--border);
    background: var(--surface2); color: var(--text);
    font-size: 14px; font-family: monospace;
}

#connect-btn {
    padding: 8px 18px; border-radius: 8px; border: none;
    background: var(--accent); color: #0f172a;
    font-weight: 700; font-size: 14px; cursor: pointer;
}
#connect-btn:active { opacity: 0.8; }
#connect-btn.connected { background: var(--red); color: #fff; }

.dot { width: 12px; height: 12px; border-radius: 50%; flex-shrink: 0; }
.dot.disconnected { background: var(--red); }
.dot.connected    { background: var(--green); box-shadow: 0 0 8px var(--green); }
.dot.connecting   { background: var(--yellow); animation: pulse 1s infinite; }

@keyframes pulse { 50% { opacity: 0.4; } }

#status-text { font-size: 13px; color: var(--text-dim); }

#sys-summary {
    font-size: 12px; color: var(--text-dim);
    margin-top: 5px; font-family: monospace;
}

.hidden { display: none !important; }

#tab-bar {
    display: flex;
    background: var(--surface);
    border-bottom: 1px solid var(--border);
}

.tab {
    flex: 1; padding: 13px 8px; border: none; background: none;
    color: var(--text-dim); font-size: 14px; font-weight: 600;
    cursor: pointer; border-bottom: 3px solid transparent; letter-spacing: 0.2px;
}
.tab.active { color: var(--accent); border-bottom-color: var(--accent); }

body { display: flex; flex-direction: column; }

#tab-content { flex: 1; overflow-y: auto; -webkit-overflow-scrolling: touch; }

.tab-panel { display: none; padding: 14px; }
.tab-panel.active { display: block; }

.card {
    background: var(--surface); border: 1px solid var(--border);
    border-radius: 10px; padding: 14px; margin-bottom: 12px;
}
.card h3 {
    font-size: 11px; text-transform: uppercase; letter-spacing: 1px;
    color: var(--accent); margin-bottom: 10px;
}

.kv {
    display: flex; justify-content: space-between;
    padding: 6px 0; font-size: 14px;
    border-bottom: 1px solid var(--surface2);
}
.kv:last-child { border-bottom: none; }
.kv .label { color: var(--text-dim); }
.kv .value { font-family: monospace; color: var(--text); }

.bar-wrap {
    height: 8px; background: var(--surface2);
    border-radius: 4px; overflow: hidden; margin: 6px 0 4px;
}
.bar-fill {
    height: 100%; border-radius: 4px;
    background: var(--accent); transition: width 0.3s;
}
.bar-fill.warn   { background: var(--yellow); }
.bar-fill.danger { background: var(--red); }

.form-row {
    display: flex; gap: 8px; align-items: center;
    margin-bottom: 10px; flex-wrap: wrap;
}
.form-row label { font-size: 13px; color: var(--text-dim); min-width: 50px; }

input[type="text"], input[type="number"], select {
    padding: 8px 10px; border-radius: 8px; border: 1px solid var(--border);
    background: var(--surface2); color: var(--text);
    font-size: 14px; font-family: monospace; min-width: 0;
}

input[type="range"] { flex: 1; accent-color: var(--accent); }

button {
    padding: 9px 18px; border-radius: 8px; border: none;
    background: var(--accent); color: #0f172a;
    font-weight: 700; font-size: 14px; cursor: pointer; white-space: nowrap;
}
button:active { opacity: 0.8; }
button.secondary {
    background: var(--surface2); color: var(--text);
    border: 1px solid var(--border); font-weight: 600;
}
button.danger { background: var(--red); color: #fff; }

.log { max-height: 240px; overflow-y: auto; font-size: 13px; font-family: monospace; }
.log-entry {
    padding: 6px 8px; border-bottom: 1px solid var(--surface2); line-height: 1.4;
}
.log-entry:last-child { border-bottom: none; }
.log-time { color: var(--text-dim); margin-right: 8px; }

.toggle-group {
    display: flex; border-radius: 8px; overflow: hidden; border: 1px solid var(--border);
}
.toggle-group button {
    border-radius: 0; border: none; background: var(--surface2);
    color: var(--text-dim); padding: 9px 16px; font-size: 14px; font-weight: 600;
}
.toggle-group button.active { background: var(--accent); color: #0f172a; }

.wiring-table { width: 100%; border-collapse: collapse; font-size: 13px; margin: 8px 0; }
.wiring-table th {
    text-align: left; color: var(--text-dim); font-size: 11px;
    text-transform: uppercase; letter-spacing: 0.5px;
    padding: 4px 8px; border-bottom: 1px solid var(--border);
}
.wiring-table td { padding: 7px 8px; border-bottom: 1px solid var(--surface2); font-family: monospace; }
.wiring-table tr:last-child td { border-bottom: none; }
.pin-sensor { color: var(--yellow); }
.pin-esp32  { color: var(--green); }

.wiring-note {
    font-size: 12px; color: var(--text-dim); margin-top: 6px;
    padding: 8px 10px; background: var(--surface2);
    border-radius: 6px; line-height: 1.6;
}
)WEBEND";

// ── js/api.js ─────────────────────────────────────────────────────────────────

static const char FILE_API_JS[] = R"WEBEND(export class ESP32LabAPI {
    constructor() {
        this._ip = '';
        this._connected = false;
        this.onStatusChange = null;
    }

    get ip() { return this._ip; }
    get connected() { return this._connected; }

    connect(ip) {
        this._ip = ip;
        return this.get('/api/system/info').then(data => {
            this._setStatus(true);
            return data;
        }).catch(err => {
            this._setStatus(false);
            throw err;
        });
    }

    disconnect() { this._setStatus(false); }

    async get(path) {
        try {
            const resp = await fetch(`http://${this._ip}${path}`, {
                signal: AbortSignal.timeout(5000)
            });
            if (!resp.ok) {
                const body = await resp.json().catch(() => null);
                throw new Error(body?.error || `${resp.status} ${resp.statusText}`);
            }
            return resp.json();
        } catch (e) {
            if (e.name === 'TimeoutError' || e.name === 'AbortError')
                throw new Error('Request timed out — is the ESP32 reachable?');
            if (e.name === 'TypeError')
                throw new Error('Network error — check WiFi connection');
            throw e;
        }
    }

    async post(path, body) {
        try {
            const resp = await fetch(`http://${this._ip}${path}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(body),
                signal: AbortSignal.timeout(5000)
            });
            if (!resp.ok) {
                const data = await resp.json().catch(() => null);
                throw new Error(data?.error || `${resp.status} ${resp.statusText}`);
            }
            return resp.json();
        } catch (e) {
            if (e.name === 'TimeoutError' || e.name === 'AbortError')
                throw new Error('Request timed out — is the ESP32 reachable?');
            if (e.name === 'TypeError')
                throw new Error('Network error — check WiFi connection');
            throw e;
        }
    }

    _setStatus(connected) {
        this._connected = connected;
        if (this.onStatusChange) this.onStatusChange(connected);
    }
}
)WEBEND";

// ── js/app.js ─────────────────────────────────────────────────────────────────

static const char FILE_APP_JS[] = R"WEBEND(import { ESP32LabAPI } from './api.js';
import { initSystem, activateSystem, deactivateSystem } from './tab-system.js';
import { initGpio,   activateGpio,   deactivateGpio   } from './tab-gpio.js';
import { initGrove,  activateGrove,  deactivateGrove  } from './tab-grove.js';

const api = new ESP32LabAPI();

const tabs = {
    system: { init: initSystem, activate: activateSystem, deactivate: deactivateSystem },
    gpio:   { init: initGpio,   activate: activateGpio,   deactivate: deactivateGpio   },
    grove:  { init: initGrove,  activate: activateGrove,  deactivate: deactivateGrove  },
};

let activeTab = 'system';
let sysInfoInterval = null;

const ipInput    = document.getElementById('ip-input');
const connectBtn = document.getElementById('connect-btn');
const statusDot  = document.getElementById('status-dot');
const statusText = document.getElementById('status-text');
const sysSummary = document.getElementById('sys-summary');

const savedIp = localStorage.getItem('esp32lab-ip');
const servedFromDevice = location.hostname !== '' &&
    location.hostname !== 'localhost' &&
    location.hostname !== '127.0.0.1';

if (servedFromDevice) {
    ipInput.value = location.hostname;
} else if (savedIp) {
    ipInput.value = savedIp;
}

if (servedFromDevice && !api.connected) {
    statusDot.className = 'dot connecting';
    statusText.textContent = 'Connecting...';
    api.connect(location.hostname).catch(e => {
        statusText.textContent = e.message || 'Connection failed';
    });
}

api.onStatusChange = (connected) => {
    statusDot.className = 'dot ' + (connected ? 'connected' : 'disconnected');
    statusText.textContent = connected ? 'Connected' : 'Disconnected';
    connectBtn.textContent = connected ? 'Disconnect' : 'Connect';
    connectBtn.classList.toggle('connected', connected);

    if (connected) {
        startSysPolling();
        tabs[activeTab].activate(api);
    } else {
        stopSysPolling();
        sysSummary.classList.add('hidden');
        tabs[activeTab].deactivate(api);
    }
};

connectBtn.addEventListener('click', () => {
    if (api.connected) {
        api.disconnect();
    } else {
        const ip = ipInput.value.trim();
        localStorage.setItem('esp32lab-ip', ip);
        statusDot.className = 'dot connecting';
        statusText.textContent = 'Connecting...';
        api.connect(ip).catch(e => {
            statusText.textContent = e.message || 'Connection failed';
        });
    }
});

ipInput.addEventListener('keydown', e => {
    if (e.key === 'Enter' && !api.connected) connectBtn.click();
});

function startSysPolling() {
    pollSysInfo();
    sysInfoInterval = setInterval(pollSysInfo, 5000);
}
function stopSysPolling() {
    if (sysInfoInterval) { clearInterval(sysInfoInterval); sysInfoInterval = null; }
}
function pollSysInfo() {
    if (!api.connected) return;
    api.get('/api/system/info').then(data => {
        const upSec = Math.floor((data.uptime_ms || 0) / 1000);
        const m = Math.floor(upSec / 60), s = upSec % 60;
        sysSummary.textContent =
            `${data.chip?.model || 'ESP32'}  |  Heap: ${fmt(data.heap?.free)} free  |  Up: ${m}m${s}s`;
        sysSummary.classList.remove('hidden');
    }).catch(() => {});
}

function fmt(n) {
    if (n == null) return '?';
    if (n >= 1048576) return (n / 1048576).toFixed(1) + 'M';
    if (n >= 1024)    return (n / 1024).toFixed(0) + 'K';
    return String(n);
}

document.getElementById('tab-bar').addEventListener('click', e => {
    const btn = e.target.closest('.tab');
    if (!btn) return;
    const name = btn.dataset.tab;
    if (name === activeTab) return;

    if (api.connected) tabs[activeTab].deactivate(api);
    document.querySelector('.tab.active').classList.remove('active');
    document.querySelector('.tab-panel.active').classList.remove('active');

    activeTab = name;
    btn.classList.add('active');
    document.getElementById('tab-' + name).classList.add('active');
    if (api.connected) tabs[activeTab].activate(api);
});

for (const [name, tab] of Object.entries(tabs)) {
    tab.init(document.getElementById('tab-' + name), api);
}

window.addEventListener('beforeunload', () => { if (api.connected) api.disconnect(); });
)WEBEND";

// ── js/tab-system.js ──────────────────────────────────────────────────────────

static const char FILE_SYSTEM_JS[] = R"WEBEND(let panel, interval;

export function initSystem(el) {
    panel = el;
    panel.innerHTML = `
        <div class="card" id="sys-chip"><h3>Chip</h3><div class="kv-list"></div></div>
        <div class="card" id="sys-memory">
            <h3>Memory</h3>
            <div class="kv"><span class="label">Heap</span><span class="value" id="sys-heap-text">--</span></div>
            <div class="bar-wrap"><div class="bar-fill" id="sys-heap-bar"></div></div>
            <div class="kv"><span class="label">PSRAM</span><span class="value" id="sys-psram-text">--</span></div>
            <div class="bar-wrap"><div class="bar-fill" id="sys-psram-bar"></div></div>
        </div>
        <div class="card" id="sys-net"><h3>Network</h3><div class="kv-list"></div></div>
        <div class="card" id="sys-misc"><h3>System</h3><div class="kv-list"></div></div>
    `;
}

export function activateSystem(api) {
    refresh(api);
    interval = setInterval(() => refresh(api), 5000);
}

export function deactivateSystem() {
    if (interval) { clearInterval(interval); interval = null; }
}

function refresh(api) {
    api.get('/api/system/info').then(render).catch(() => {});
}

function render(d) {
    setKvList('#sys-chip .kv-list', [
        ['Model',    d.chip?.model],
        ['Revision', d.chip?.revision],
        ['Cores',    d.chip?.cores],
        ['Freq',     (d.chip?.freq_mhz ?? '--') + ' MHz'],
    ]);

    const heapUsed = (d.heap?.total || 0) - (d.heap?.free || 0);
    const heapPct  = d.heap?.total ? (heapUsed / d.heap.total * 100) : 0;
    document.getElementById('sys-heap-text').textContent =
        `${fmt(heapUsed)} used / ${fmt(d.heap?.total)} total (${heapPct.toFixed(0)}%)`;
    const heapBar = document.getElementById('sys-heap-bar');
    heapBar.style.width = heapPct + '%';
    heapBar.className = 'bar-fill' + (heapPct > 80 ? ' danger' : heapPct > 60 ? ' warn' : '');

    const psramUsed = (d.psram?.total || 0) - (d.psram?.free || 0);
    const psramPct  = d.psram?.total ? (psramUsed / d.psram.total * 100) : 0;
    document.getElementById('sys-psram-text').textContent = d.psram?.total
        ? `${fmt(psramUsed)} used / ${fmt(d.psram.total)} total (${psramPct.toFixed(0)}%)`
        : 'Not available';
    document.getElementById('sys-psram-bar').style.width = psramPct + '%';

    setKvList('#sys-net .kv-list', [
        ['SSID',         d.wifi?.ssid],
        ['IP',           d.wifi?.ip],
        ['WiFi Clients', d.wifi?.clients],
    ]);

    const upSec = Math.floor((d.uptime_ms || 0) / 1000);
    const h = Math.floor(upSec / 3600), m = Math.floor((upSec % 3600) / 60), s = upSec % 60;
    setKvList('#sys-misc .kv-list', [
        ['Uptime',      `${h}h ${m}m ${s}s`],
        ['SDK',         d.sdk_version],
        ['Flash',       fmt(d.flash?.size)],
        ['Flash Speed', fmt(d.flash?.speed) + 'Hz'],
    ]);
}

function setKvList(selector, pairs) {
    const el = panel.querySelector(selector);
    el.innerHTML = pairs.map(([k, v]) =>
        `<div class="kv"><span class="label">${k}</span><span class="value">${v ?? '--'}</span></div>`
    ).join('');
}

function fmt(n) {
    if (n == null) return '?';
    if (n >= 1048576) return (n / 1048576).toFixed(1) + 'M';
    if (n >= 1024)    return (n / 1024).toFixed(0) + 'K';
    return String(n);
}
)WEBEND";

// ── js/tab-gpio.js ────────────────────────────────────────────────────────────

static const char FILE_GPIO_JS[] = R"WEBEND(let panel, currentApi;
const gpioLog = [];

export function initGpio(el) {
    panel = el;
    panel.innerHTML = `
        <div class="card">
            <h3>GPIO Control</h3>
            <div class="wiring-note" style="margin-bottom:12px">
                Safe pins on a standard ESP32 DevKit: 4, 5, 13, 14, 16&ndash;27, 32&ndash;33.
                Input-only (no output): 34&ndash;39. Avoid 0, 1, 2, 3, 6&ndash;12, 15.
            </div>
            <div class="form-row">
                <label>Pin</label>
                <input type="number" id="gpio-pin" value="13" min="0" max="39" style="width:70px">
            </div>
            <div class="form-row">
                <label>Mode</label>
                <select id="gpio-mode">
                    <option value="input">INPUT</option>
                    <option value="output">OUTPUT</option>
                    <option value="input_pullup">INPUT_PULLUP</option>
                    <option value="input_pulldown">INPUT_PULLDOWN</option>
                </select>
                <button id="gpio-mode-btn" class="secondary">Set Mode</button>
            </div>
            <div class="form-row">
                <button id="gpio-read-btn" class="secondary">Read</button>
                <span id="gpio-read-val" style="font-family:monospace;font-size:16px">--</span>
            </div>
            <div class="form-row">
                <label>Write</label>
                <div class="toggle-group">
                    <button id="gpio-low" class="active">LOW</button>
                    <button id="gpio-high">HIGH</button>
                </div>
                <button id="gpio-write-btn">Write</button>
            </div>
        </div>
        <div class="card">
            <h3>Activity Log</h3>
            <button id="gpio-clear-btn" class="secondary" style="margin-bottom:8px;font-size:12px">Clear</button>
            <div class="log" id="gpio-log">
                <div style="color:var(--text-dim);padding:8px">No activity yet</div>
            </div>
        </div>
    `;

    let writeVal = 0;
    panel.querySelector('#gpio-low').onclick = () => {
        writeVal = 0;
        panel.querySelector('#gpio-low').classList.add('active');
        panel.querySelector('#gpio-high').classList.remove('active');
    };
    panel.querySelector('#gpio-high').onclick = () => {
        writeVal = 1;
        panel.querySelector('#gpio-high').classList.add('active');
        panel.querySelector('#gpio-low').classList.remove('active');
    };

    panel.querySelector('#gpio-mode-btn').onclick = () => {
        if (!currentApi) return;
        const pin = panel.querySelector('#gpio-pin').value;
        const mode = panel.querySelector('#gpio-mode').value;
        currentApi.post(`/api/gpio/${pin}/mode`, { mode })
            .then(r => addLog(`Pin ${r.pin} mode: ${r.mode}`))
            .catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#gpio-read-btn').onclick = () => {
        if (!currentApi) return;
        const pin = panel.querySelector('#gpio-pin').value;
        currentApi.get(`/api/gpio/${pin}`).then(r => {
            panel.querySelector('#gpio-read-val').textContent = r.value ? 'HIGH (1)' : 'LOW (0)';
            addLog(`Pin ${r.pin} = ${r.value ? 'HIGH' : 'LOW'}`);
        }).catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#gpio-write-btn').onclick = () => {
        if (!currentApi) return;
        const pin = panel.querySelector('#gpio-pin').value;
        currentApi.post(`/api/gpio/${pin}`, { value: writeVal })
            .then(r => addLog(`Pin ${r.pin} written ${r.value ? 'HIGH' : 'LOW'}`))
            .catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#gpio-clear-btn').onclick = () => { gpioLog.length = 0; renderLog(); };
}

export function activateGpio(api) { currentApi = api; }
export function deactivateGpio() { currentApi = null; }

function addLog(msg) {
    gpioLog.unshift({ time: new Date().toLocaleTimeString(), msg });
    if (gpioLog.length > 100) gpioLog.pop();
    renderLog();
}

function renderLog() {
    const el = panel.querySelector('#gpio-log');
    if (gpioLog.length === 0) {
        el.innerHTML = '<div style="color:var(--text-dim);padding:8px">No activity yet</div>';
        return;
    }
    el.innerHTML = gpioLog.map(e =>
        `<div class="log-entry"><span class="log-time">${e.time}</span>${e.msg}</div>`
    ).join('');
}
)WEBEND";

// ── js/tab-grove.js ───────────────────────────────────────────────────────────

static const char FILE_GROVE_JS[] = R"WEBEND(let panel, currentApi;
let sensors = [];
let activeSensorId = '';
let sse = null;
const groveLog = [];

export function initGrove(el) {
    panel = el;
    panel.innerHTML = `
        <div class="card">
            <h3>Sensor</h3>
            <div class="form-row">
                <label>Type</label>
                <select id="grove-type" style="flex:1">
                    <option value="">Connect to load sensors...</option>
                </select>
            </div>
            <div id="grove-safety" style="display:none"></div>
            <div id="grove-wiring" style="display:none;margin-top:10px"></div>
            <div id="grove-pins" style="margin-top:8px;font-size:13px;color:var(--text-dim);font-family:monospace"></div>
            <div class="form-row" style="margin-top:12px">
                <button id="grove-cfg-btn">Configure Sensor</button>
            </div>
        </div>
        <div class="card">
            <h3>Reading</h3>
            <div class="form-row">
                <button id="grove-read-btn" class="secondary">Read Once</button>
                <button id="grove-stream-btn" class="secondary">&#9654; Stream</button>
            </div>
            <div id="grove-value" style="font-size:22px;font-family:monospace;padding:12px 0;min-height:54px;color:var(--accent)">&#8212;</div>
            <div id="grove-digital-row" class="form-row" style="display:none">
                <label>Output</label>
                <div class="toggle-group">
                    <button id="grove-low-btn" class="active">LOW</button>
                    <button id="grove-high-btn">HIGH</button>
                </div>
                <button id="grove-write-btn">Write</button>
            </div>
            <div id="grove-pwm-row" class="form-row" style="display:none">
                <label>Duty</label>
                <input type="range" id="grove-duty" min="0" max="255" value="128" style="flex:1">
                <span id="grove-duty-val" style="font-family:monospace;min-width:28px">128</span>
                <button id="grove-pwm-btn">Set</button>
            </div>
            <div id="grove-rotary-ctrl" style="display:none;margin-top:8px">
                <button id="grove-rotary-reset" class="secondary">Reset Steps</button>
            </div>
        </div>
        <div class="card">
            <h3>Log</h3>
            <button id="grove-log-clear" class="secondary" style="margin-bottom:8px;font-size:12px">Clear</button>
            <div class="log" id="grove-log-el">
                <div style="color:var(--text-dim);padding:8px">No readings yet</div>
            </div>
        </div>
    `;

    const sel = panel.querySelector('#grove-type');
    sel.addEventListener('change', () => updateSafetyInfo(sel.value));

    let digitalWriteVal = 0;
    panel.querySelector('#grove-low-btn').onclick = () => {
        digitalWriteVal = 0;
        panel.querySelector('#grove-low-btn').classList.add('active');
        panel.querySelector('#grove-high-btn').classList.remove('active');
    };
    panel.querySelector('#grove-high-btn').onclick = () => {
        digitalWriteVal = 1;
        panel.querySelector('#grove-high-btn').classList.add('active');
        panel.querySelector('#grove-low-btn').classList.remove('active');
    };

    const dutySlider = panel.querySelector('#grove-duty');
    const dutyValEl  = panel.querySelector('#grove-duty-val');
    dutySlider.addEventListener('input', () => { dutyValEl.textContent = dutySlider.value; });

    panel.querySelector('#grove-cfg-btn').onclick = () => {
        if (!currentApi) return;
        const id = sel.value;
        if (!id) return;
        stopStream();
        currentApi.post('/api/grove/configure', { sensor: id }).then(r => {
            activeSensorId = r.sensor;
            showOutputControls(r.sensor);
            addLog(`Configured: ${r.name} on GPIO${r.pin_d}${r.uses_d2 ? ' + GPIO' + r.pin_d2 : ''}`);
        }).catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#grove-read-btn').onclick = () => {
        if (!currentApi) return;
        currentApi.get('/api/grove/read').then(r => {
            renderReading(r);
            addLog(readingToString(r));
        }).catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#grove-stream-btn').onclick = () => {
        if (sse) stopStream(); else startStream();
    };

    panel.querySelector('#grove-write-btn').onclick = () => {
        if (!currentApi) return;
        currentApi.post('/api/grove/write', { value: digitalWriteVal })
            .then(() => addLog(`Wrote: ${digitalWriteVal ? 'HIGH' : 'LOW'}`))
            .catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#grove-pwm-btn').onclick = () => {
        if (!currentApi) return;
        const duty = parseInt(dutySlider.value);
        currentApi.post('/api/grove/write', { duty })
            .then(() => addLog(`PWM duty: ${duty}`))
            .catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#grove-rotary-reset').onclick = () => {
        if (!currentApi) return;
        currentApi.post('/api/grove/rotary/reset', {}).then(() => {
            addLog('Rotary steps reset to 0');
            renderReading({ sensor: 'rotary', steps: 0, ts: Date.now() });
        }).catch(e => addLog(`Error: ${e.message}`));
    };

    panel.querySelector('#grove-log-clear').onclick = () => { groveLog.length = 0; renderLog(); };
}

export function activateGrove(api) {
    currentApi = api;
    loadSensors();
    api.get('/api/grove/config').then(r => {
        if (r.sensor) {
            activeSensorId = r.sensor;
            const sel = panel.querySelector('#grove-type');
            sel.value = r.sensor;
            updateSafetyInfo(r.sensor);
            showOutputControls(r.sensor);
        }
    }).catch(() => {});
}

export function deactivateGrove() { stopStream(); currentApi = null; }

function loadSensors() {
    if (!currentApi) return;
    currentApi.get('/api/grove/sensors').then(data => {
        sensors = data.sensors || [];
        const sel = panel.querySelector('#grove-type');
        sel.innerHTML = '<option value="">Select sensor type...</option>' +
            sensors.map(s =>
                `<option value="${s.id}">${s.name} (${s.vcc}${s.gpio_safe ? '' : ' &#9888;'})</option>`
            ).join('');
        if (activeSensorId) { sel.value = activeSensorId; updateSafetyInfo(activeSensorId); }
    }).catch(() => {});
}

function updateSafetyInfo(id) {
    const safetyEl = panel.querySelector('#grove-safety');
    const wiringEl = panel.querySelector('#grove-wiring');
    const pinsEl   = panel.querySelector('#grove-pins');
    if (!id) { safetyEl.style.display = 'none'; wiringEl.style.display = 'none'; pinsEl.textContent = ''; return; }
    const s = sensors.find(x => x.id === id);
    if (!s) return;

    const renderWiring = (d, d2) => {
        pinsEl.textContent = `D = GPIO${d}` + (s.uses_d2 ? `   |   D2 = GPIO${d2}` : '');
        wiringEl.innerHTML = buildWiringGuide(id, d, d2);
        wiringEl.style.display = '';
    };

    if (currentApi) {
        currentApi.get('/api/grove/config')
            .then(r => renderWiring(r.pin_d, r.pin_d2))
            .catch(() => renderWiring(4, 5));
    } else {
        renderWiring(4, 5);
    }

    if (s.gpio_safe) {
        safetyEl.style.cssText = 'display:block;background:#0d2818;border:1px solid var(--green);border-radius:8px;padding:10px 12px;margin-top:8px;font-size:13px;line-height:1.5';
        safetyEl.innerHTML = `<strong style="color:var(--green)">&#10003; Safe &mdash; ${s.vcc}</strong><br>${s.voltage_note}`;
    } else {
        safetyEl.style.cssText = 'display:block;background:#2d1111;border:1px solid var(--red);border-radius:8px;padding:10px 12px;margin-top:8px;font-size:13px;line-height:1.5';
        safetyEl.innerHTML = `<strong style="color:var(--red)">&#9888; Caution &mdash; ${s.vcc}</strong><br>${s.voltage_note}`;
    }
}

function buildWiringGuide(sensorId, d, d2) {
    const G = {
        digital_in:  { rows: [['Signal', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']], note: 'For a button: connect one end to GPIO, other to GND &mdash; use INPUT_PULLUP mode.' },
        digital_out: { rows: [['Output', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']], note: 'Connect an LED with a 330&#937; resistor in series, or a relay/buzzer module.' },
        analog_in:   { rows: [['Signal', `GPIO${d}  &larr; MAX 3.3V!`], ['VCC', '3.3V'], ['GND', 'GND']], note: 'Never apply more than 3.3V to this pin. ADC reads 0 (0V) to 4095 (3.3V).' },
        pwm_out:     { rows: [['PWM Out', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']], note: 'Connect an LED + 330&#937; resistor. Duty 0 = off, 128 = half, 255 = full on.' },
        dht11:       { rows: [['VCC', '3.3V'], ['GND', 'GND'], ['DATA', `GPIO${d}`]], note: 'Breakout modules include a pull-up &mdash; no extra parts needed. Bare sensor: add 10k&#937; from DATA to 3.3V.' },
        ds18b20:     { rows: [['VCC (red)', '3.3V'], ['GND (black)', 'GND'], ['DATA (yellow)', `GPIO${d}`]], note: 'Required: 4.7k&#937; resistor between 3.3V and DATA. Without it readings will fail.' },
        hcsr04:      { rows: [['VCC', '3.3V  &larr; HC-SR04P only!'], ['GND', 'GND'], ['TRIG', `GPIO${d}  (trigger)`], ['ECHO', `GPIO${d2}  (echo)`]], note: '&#9888; Use HC-SR04<strong>P</strong> (3.3V version). Standard HC-SR04 outputs 5V on ECHO and will damage the ESP32.' },
        rotary:      { rows: [['VCC (+)', '3.3V'], ['GND (&minus;)', 'GND'], ['CLK', `GPIO${d}`], ['DT', `GPIO${d2}`]], note: 'Works with KY-040 encoder modules. The SW (button) pin is not used here.' },
    };
    const g = G[sensorId];
    if (!g) return '';
    const rows = g.rows.map(([sp, ep]) =>
        `<tr><td class="pin-sensor">${sp}</td><td style="color:var(--text-dim);padding:0 6px;font-family:sans-serif">&rarr;</td><td class="pin-esp32">${ep}</td></tr>`
    ).join('');
    return `<table class="wiring-table"><thead><tr><th>Sensor Pin</th><th></th><th>ESP32 Pin</th></tr></thead><tbody>${rows}</tbody></table><div class="wiring-note">${g.note}</div>`;
}

function showOutputControls(id) {
    panel.querySelector('#grove-digital-row').style.display = id === 'digital_out' ? 'flex'  : 'none';
    panel.querySelector('#grove-pwm-row').style.display     = id === 'pwm_out'     ? 'flex'  : 'none';
    panel.querySelector('#grove-rotary-ctrl').style.display = id === 'rotary'      ? 'block' : 'none';
}

function startStream() {
    if (!currentApi || sse) return;
    const btn = panel.querySelector('#grove-stream-btn');
    btn.textContent = '&#9646; Stop';
    btn.classList.remove('secondary');
    sse = new EventSource(`http://${currentApi.ip}/api/grove/stream`);
    sse.addEventListener('reading', e => {
        const r = JSON.parse(e.data);
        renderReading(r);
        addLog(readingToString(r));
    });
    sse.onerror = () => { stopStream(); addLog('Stream closed'); };
    addLog('Stream started');
}

function stopStream() {
    if (sse) { sse.close(); sse = null; }
    const btn = panel.querySelector('#grove-stream-btn');
    if (btn) { btn.textContent = '▶ Stream'; btn.classList.add('secondary'); }
}

function renderReading(r) {
    const el = panel.querySelector('#grove-value');
    if (r.error) { el.style.color = 'var(--red)'; el.textContent = r.error; return; }
    el.style.color = 'var(--accent)';
    const parts = [];
    if (r.label        !== undefined) parts.push(r.label);
    else if (r.value   !== undefined) parts.push(`${r.value ? 'HIGH' : 'LOW'} (${r.value})`);
    if (r.raw          !== undefined) parts.push(`raw: ${r.raw}  ${r.voltage}V`);
    if (r.temperature  !== undefined) parts.push(`${r.temperature}&deg;C`);
    if (r.humidity     !== undefined) parts.push(`${r.humidity}% RH`);
    if (r.distance_cm  !== undefined) parts.push(`${r.distance_cm} cm`);
    if (r.steps        !== undefined) parts.push(`Steps: ${r.steps}`);
    if (r.note         !== undefined) parts.push(r.note);
    el.innerHTML = parts.join('  |  ') || '&mdash;';
}

function readingToString(r) {
    if (r.error) return `Error: ${r.error}`;
    const parts = [];
    if (r.label        !== undefined) parts.push(r.label);
    else if (r.value   !== undefined) parts.push(`val=${r.value}`);
    if (r.raw          !== undefined) parts.push(`raw=${r.raw} ${r.voltage}V`);
    if (r.temperature  !== undefined) parts.push(`temp=${r.temperature}C`);
    if (r.humidity     !== undefined) parts.push(`hum=${r.humidity}%`);
    if (r.distance_cm  !== undefined) parts.push(`dist=${r.distance_cm}cm`);
    if (r.steps        !== undefined) parts.push(`steps=${r.steps}`);
    return parts.join(' ') || JSON.stringify(r);
}

function addLog(msg) {
    groveLog.unshift({ time: new Date().toLocaleTimeString(), msg });
    if (groveLog.length > 100) groveLog.pop();
    renderLog();
}

function renderLog() {
    const el = panel.querySelector('#grove-log-el');
    if (groveLog.length === 0) {
        el.innerHTML = '<div style="color:var(--text-dim);padding:8px">No readings yet</div>';
        return;
    }
    el.innerHTML = groveLog.map(e =>
        `<div class="log-entry"><span class="log-time">${e.time}</span>${e.msg}</div>`
    ).join('');
}
)WEBEND";

// ── manifest.json ─────────────────────────────────────────────────────────────

static const char FILE_MANIFEST[] = R"WEBEND({
    "name": "ESP32 Lab",
    "short_name": "ESP32Lab",
    "description": "Browser sensor control for ESP32",
    "start_url": "/",
    "display": "standalone",
    "background_color": "#0f172a",
    "theme_color": "#38bdf8",
    "icons": []
}
)WEBEND";

// ── Route registration ────────────────────────────────────────────────────────

void setupWebApp() {
    apiServer.http().on("/", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "text/html",              FILE_INDEX);     });
    apiServer.http().on("/css/style.css", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "text/css",               FILE_STYLE_CSS); });
    apiServer.http().on("/js/api.js", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/javascript", FILE_API_JS);    });
    apiServer.http().on("/js/app.js", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/javascript", FILE_APP_JS);    });
    apiServer.http().on("/js/tab-system.js", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/javascript", FILE_SYSTEM_JS); });
    apiServer.http().on("/js/tab-gpio.js", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/javascript", FILE_GPIO_JS);   });
    apiServer.http().on("/js/tab-grove.js", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/javascript", FILE_GROVE_JS);  });
    apiServer.http().on("/manifest.json", HTTP_GET,
        [](AsyncWebServerRequest* r){ r->send(200, "application/json",       FILE_MANIFEST);  });

    Serial.println("[API] Web app routes registered");
}
