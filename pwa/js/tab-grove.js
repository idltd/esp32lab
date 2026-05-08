let panel, currentApi;
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

    panel.querySelector('#grove-log-clear').onclick = () => {
        groveLog.length = 0;
        renderLog();
    };
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

export function deactivateGrove() {
    stopStream();
    currentApi = null;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

function loadSensors() {
    if (!currentApi) return;
    currentApi.get('/api/grove/sensors').then(data => {
        sensors = data.sensors || [];
        const sel = panel.querySelector('#grove-type');
        sel.innerHTML = '<option value="">Select sensor type...</option>' +
            sensors.map(s =>
                `<option value="${s.id}">${s.name} (${s.vcc}${s.gpio_safe ? '' : ' ⚠'})</option>`
            ).join('');
        if (activeSensorId) {
            sel.value = activeSensorId;
            updateSafetyInfo(activeSensorId);
        }
    }).catch(() => {});
}

function updateSafetyInfo(id) {
    const safetyEl = panel.querySelector('#grove-safety');
    const wiringEl = panel.querySelector('#grove-wiring');
    const pinsEl   = panel.querySelector('#grove-pins');
    if (!id) {
        safetyEl.style.display = 'none';
        wiringEl.style.display = 'none';
        pinsEl.textContent = '';
        return;
    }
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
        safetyEl.style.cssText =
            'display:block;background:#0d2818;border:1px solid var(--green);' +
            'border-radius:8px;padding:10px 12px;margin-top:8px;font-size:13px;line-height:1.5';
        safetyEl.innerHTML =
            `<strong style="color:var(--green)">&#10003; Safe &mdash; ${s.vcc}</strong><br>${s.voltage_note}`;
    } else {
        safetyEl.style.cssText =
            'display:block;background:#2d1111;border:1px solid var(--red);' +
            'border-radius:8px;padding:10px 12px;margin-top:8px;font-size:13px;line-height:1.5';
        safetyEl.innerHTML =
            `<strong style="color:var(--red)">&#9888; Caution &mdash; ${s.vcc}</strong><br>${s.voltage_note}`;
    }
}

function buildWiringGuide(sensorId, d, d2) {
    const G = {
        digital_in: {
            rows: [['Signal', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']],
            note: 'For a button: connect one end to GPIO, other to GND — then use INPUT_PULLUP mode.'
        },
        digital_out: {
            rows: [['Output', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']],
            note: 'Connect an LED with a 330&#937; resistor in series. Or a relay/buzzer module.'
        },
        analog_in: {
            rows: [['Signal', `GPIO${d}  ← MAX 3.3V!`], ['VCC', '3.3V'], ['GND', 'GND']],
            note: 'Never apply more than 3.3V to this pin. ADC reads 0 (0V) to 4095 (3.3V).'
        },
        pwm_out: {
            rows: [['PWM Out', `GPIO${d}`], ['VCC', '3.3V'], ['GND', 'GND']],
            note: 'Connect an LED + 330&#937; resistor. Duty 0 = off, 128 = half brightness, 255 = full on.'
        },
        dht11: {
            rows: [['VCC', '3.3V'], ['GND', 'GND'], ['DATA', `GPIO${d}`]],
            note: 'Breakout modules include a pull-up resistor &mdash; no extra parts needed. Bare sensor: add 10k&#937; from DATA to 3.3V.'
        },
        ds18b20: {
            rows: [['VCC (red)', '3.3V'], ['GND (black)', 'GND'], ['DATA (yellow)', `GPIO${d}`]],
            note: 'Required: 4.7k&#937; resistor between 3.3V and DATA. Without it readings will fail.'
        },
        hcsr04: {
            rows: [
                ['VCC', '3.3V  ← HC-SR04P only!'],
                ['GND', 'GND'],
                ['TRIG', `GPIO${d}  (trigger)`],
                ['ECHO', `GPIO${d2}  (echo)`]
            ],
            note: '&#9888; Use HC-SR04<strong>P</strong> (3.3V version). Standard HC-SR04 outputs 5V on ECHO and will damage your ESP32.'
        },
        rotary: {
            rows: [['VCC (+)', '3.3V'], ['GND (–)', 'GND'], ['CLK', `GPIO${d}`], ['DT', `GPIO${d2}`]],
            note: 'Works with KY-040 encoder modules. The SW (button) pin is not used here.'
        },
    };
    const g = G[sensorId];
    if (!g) return '';
    const rows = g.rows.map(([sp, ep]) =>
        `<tr><td class="pin-sensor">${sp}</td>` +
        `<td style="color:var(--text-dim);padding:0 6px;font-family:sans-serif">→</td>` +
        `<td class="pin-esp32">${ep}</td></tr>`
    ).join('');
    return `<table class="wiring-table">` +
        `<thead><tr><th>Sensor Pin</th><th></th><th>ESP32 Pin</th></tr></thead>` +
        `<tbody>${rows}</tbody></table>` +
        `<div class="wiring-note">${g.note}</div>`;
}

function showOutputControls(id) {
    panel.querySelector('#grove-digital-row').style.display  = id === 'digital_out' ? 'flex'  : 'none';
    panel.querySelector('#grove-pwm-row').style.display      = id === 'pwm_out'     ? 'flex'  : 'none';
    panel.querySelector('#grove-rotary-ctrl').style.display  = id === 'rotary'      ? 'block' : 'none';
}

function startStream() {
    if (!currentApi || sse) return;
    const btn = panel.querySelector('#grove-stream-btn');
    btn.textContent = '■ Stop';
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
    if (r.temperature  !== undefined) parts.push(`${r.temperature}°C`);
    if (r.humidity     !== undefined) parts.push(`${r.humidity}% RH`);
    if (r.distance_cm  !== undefined) parts.push(`${r.distance_cm} cm`);
    if (r.steps        !== undefined) parts.push(`Steps: ${r.steps}`);
    if (r.note         !== undefined) parts.push(r.note);
    el.textContent = parts.join('  |  ') || '—';
}

function readingToString(r) {
    if (r.error) return `Error: ${r.error}`;
    const parts = [];
    if (r.label        !== undefined) parts.push(r.label);
    else if (r.value   !== undefined) parts.push(`val=${r.value}`);
    if (r.raw          !== undefined) parts.push(`raw=${r.raw} ${r.voltage}V`);
    if (r.temperature  !== undefined) parts.push(`temp=${r.temperature}°C`);
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
