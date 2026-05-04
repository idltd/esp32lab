import { ESP32LabAPI } from './api.js';
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

// Auto-fill IP: when served from the device itself, use its hostname
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
