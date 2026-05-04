let panel, currentApi;
const gpioLog = [];

export function initGpio(el) {
    panel = el;
    panel.innerHTML = `
        <div class="card">
            <h3>GPIO Control</h3>
            <div class="wiring-note" style="margin-bottom:12px">
                Safe pins on a standard ESP32 DevKit: 4, 5, 13, 14, 16–27, 32–33.
                Input-only (no output): 34–39. Avoid 0, 1, 2, 3, 6–12, 15.
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
            .then(r => addLog(`Pin ${r.pin} → mode: ${r.mode}`))
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

    panel.querySelector('#gpio-clear-btn').onclick = () => {
        gpioLog.length = 0;
        renderLog();
    };
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
