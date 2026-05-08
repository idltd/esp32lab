let panel, interval, currentApi;

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
        <div class="card" id="sys-device">
            <h3>Device</h3>
            <div class="form-row">
                <label>Name</label>
                <input type="text" id="device-name-input" style="flex:1" placeholder="esp32lab">
                <button id="device-rename-btn" class="secondary">Rename</button>
            </div>
            <div class="form-row">
                <label>LED Pin</label>
                <input type="number" id="led-pin-input" min="-1" max="48" style="width:70px" placeholder="auto">
                <button id="led-pin-btn" class="secondary">Save</button>
            </div>
            <div class="form-row" style="margin-top:4px">
                <button id="identify-btn">Identify (Blink LED)</button>
            </div>
            <div id="device-hint" style="font-size:12px;color:var(--text-dim);margin-top:8px;display:none"></div>
        </div>
        <div class="card">
            <h3>WiFi Configuration</h3>
            <div id="wifi-status" style="font-size:13px;margin-bottom:12px;color:var(--text-dim)">Loading...</div>
            <div class="form-row">
                <label>Network</label>
                <input type="text" id="wifi-ssid" placeholder="Network name (SSID)" style="flex:1">
                <button id="wifi-scan-btn" class="secondary">Scan</button>
            </div>
            <div id="wifi-scan-results" style="display:none;border:1px solid var(--border);border-radius:6px;margin-bottom:8px;max-height:200px;overflow-y:auto"></div>
            <div class="form-row">
                <label>Password</label>
                <input type="password" id="wifi-pass" placeholder="Password" style="flex:1">
            </div>
            <div class="form-row">
                <button id="wifi-connect-btn">Connect to Network</button>
                <button id="wifi-forget-btn" class="secondary" style="display:none">Forget WiFi</button>
            </div>
            <div id="wifi-hint" style="font-size:12px;color:var(--text-dim);margin-top:8px;display:none;line-height:1.5"></div>
        </div>
        <div class="card">
            <h3>Firmware Update</h3>
            <div style="font-size:13px;color:var(--text-dim);margin-bottom:12px">
                Running: <span id="ota-current" style="color:var(--text);font-family:monospace">--</span>
                &nbsp;&mdash;&nbsp;upload a new .bin to update over WiFi.
            </div>
            <div class="form-row">
                <input type="file" id="ota-file" accept=".bin" style="flex:1;font-size:13px;min-width:0">
                <button id="ota-btn" class="secondary">Upload</button>
            </div>
            <div id="ota-progress" style="display:none;margin-top:10px">
                <div class="bar-wrap"><div class="bar-fill" id="ota-bar" style="width:0%"></div></div>
                <div id="ota-msg" style="font-size:13px;color:var(--text-dim);margin-top:6px"></div>
            </div>
        </div>
    `;

    panel.querySelector('#identify-btn').onclick = identifyDevice;
    panel.querySelector('#device-rename-btn').onclick = renameDevice;
    panel.querySelector('#led-pin-btn').onclick = saveLedPin;
    panel.querySelector('#ota-btn').onclick = startOta;
    panel.querySelector('#wifi-scan-btn').onclick = scanWifi;
    panel.querySelector('#wifi-connect-btn').onclick = connectWifi;
    panel.querySelector('#wifi-forget-btn').onclick = forgetWifi;
}

export function activateSystem(api) {
    currentApi = api;
    refresh(api);
    loadWifiConfig();
    interval = setInterval(() => refresh(api), 5000);
}

export function deactivateSystem() {
    if (interval) { clearInterval(interval); interval = null; }
    currentApi = null;
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
        ['SSID',        d.wifi?.ssid],
        ['IP',          d.wifi?.ip],
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

    const otaVer = panel.querySelector('#ota-current');
    if (otaVer && d.firmware) otaVer.textContent = 'v' + d.firmware;

    const nameIn = panel.querySelector('#device-name-input');
    if (nameIn && d.name && !nameIn.value) nameIn.value = d.name;

    const ledIn = panel.querySelector('#led-pin-input');
    if (ledIn && d.led_pin != null) {
        if (d.gpio_max != null) ledIn.max = d.gpio_max;
        if (ledIn.value === '') ledIn.value = d.led_pin;
    }
}

function identifyDevice() {
    if (!currentApi) return;
    const btn = panel.querySelector('#identify-btn');
    btn.disabled = true;
    fetch(`http://${currentApi.ip}/api/system/identify`, {method: 'POST'})
        .finally(() => { btn.disabled = false; });
}

function renameDevice() {
    if (!currentApi) return;
    const name = panel.querySelector('#device-name-input').value.trim();
    if (!name) { alert('Please enter a device name.'); return; }
    const hint = panel.querySelector('#device-hint');
    const btn  = panel.querySelector('#device-rename-btn');
    btn.disabled = true;
    hint.style.display = 'block';
    hint.textContent = 'Saving and restarting…';
    fetch(`http://${currentApi.ip}/api/system/name`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name})
    }).then(() => {
        hint.textContent = `Device restarting. Reconnect at http://${name}.local/`;
    }).catch(() => {
        hint.textContent = `Device restarting. Reconnect at http://${name}.local/`;
    });
}

function saveLedPin() {
    if (!currentApi) return;
    const pin = parseInt(panel.querySelector('#led-pin-input').value, 10);
    if (isNaN(pin) || pin < -1) { alert('Enter a valid pin number (-1 to disable LED).'); return; }
    const btn = panel.querySelector('#led-pin-btn');
    btn.disabled = true;
    currentApi.post('/api/system/ledpin', { pin })
        .then(() => { btn.disabled = false; })
        .catch(e => { alert('Error: ' + e.message); btn.disabled = false; });
}

function scanWifi() {
    if (!currentApi) return;
    const btn  = panel.querySelector('#wifi-scan-btn');
    const list = panel.querySelector('#wifi-scan-results');
    btn.disabled = true;
    btn.textContent = 'Scanning…';
    list.style.display = 'none';
    list.innerHTML = '';

    function poll() {
        fetch(`http://${currentApi.ip}/api/wifi/scan`)
            .then(r => r.status === 202 ? null : r.json())
            .then(networks => {
                if (!networks) { setTimeout(poll, 1500); return; }
                btn.disabled = false;
                btn.textContent = 'Scan';
                if (networks.length === 0) {
                    list.style.display = 'block';
                    list.innerHTML = '<div style="padding:8px;font-size:13px;color:var(--text-dim)">No networks found.</div>';
                    return;
                }
                list.style.display = 'block';
                list.innerHTML = networks.map(n => {
                    const sig = n.rssi > -60 ? '●●●' : n.rssi > -75 ? '●●○' : '●○○';
                    const lock = n.secure ? ' 🔒' : '';
                    return `<div class="wifi-net" data-ssid="${n.ssid.replace(/&/g,'&amp;').replace(/"/g,'&quot;')}"
                        style="padding:8px 10px;cursor:pointer;display:flex;justify-content:space-between;align-items:center;font-size:13px;border-bottom:1px solid var(--border)">
                        <span>${n.ssid}${lock}</span>
                        <span style="color:var(--text-dim);font-size:11px;white-space:nowrap;margin-left:8px">${sig} ${n.rssi} dBm</span>
                    </div>`;
                }).join('');
                list.querySelectorAll('.wifi-net').forEach(el => {
                    el.addEventListener('pointerover',  () => el.style.background = 'var(--surface2)');
                    el.addEventListener('pointerout',   () => el.style.background = '');
                    el.onclick = () => {
                        panel.querySelector('#wifi-ssid').value = el.dataset.ssid;
                        panel.querySelector('#wifi-pass').focus();
                        list.style.display = 'none';
                    };
                });
            })
            .catch(() => { btn.disabled = false; btn.textContent = 'Scan'; });
    }
    poll();
}

function loadWifiConfig() {
    if (!currentApi) return;
    fetch(`http://${currentApi.ip}/api/wifi/config`)
        .then(r => r.json())
        .then(d => {
            const status  = panel.querySelector('#wifi-status');
            const forget  = panel.querySelector('#wifi-forget-btn');
            const hint    = panel.querySelector('#wifi-hint');
            const ssidIn  = panel.querySelector('#wifi-ssid');
            if (d.mode === 'sta') {
                status.textContent = `Connected to "${d.ssid}" — ${d.ip}`;
                forget.style.display = '';
                hint.style.display = 'none';
            } else {
                status.textContent = 'Hotspot mode — not connected to a network.';
                forget.style.display = 'none';
                if (d.saved_ssid) {
                    hint.style.display = 'block';
                    hint.textContent = `Last saved network “${d.saved_ssid}” was unreachable.`;
                }
            }
            if (d.saved_ssid && !ssidIn.value) ssidIn.value = d.saved_ssid;
        })
        .catch(() => {
            const status = panel.querySelector('#wifi-status');
            if (status) status.textContent = 'Unable to load WiFi status.';
        });
}

function connectWifi() {
    if (!currentApi) return;
    const ssid = panel.querySelector('#wifi-ssid').value.trim();
    const pass = panel.querySelector('#wifi-pass').value;
    if (!ssid) { alert('Please enter a network name.'); return; }
    const btn  = panel.querySelector('#wifi-connect-btn');
    const hint = panel.querySelector('#wifi-hint');
    btn.disabled = true;
    hint.style.display = 'block';
    hint.textContent = 'Saving credentials and restarting…';
    fetch(`http://${currentApi.ip}/api/wifi/connect`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ssid, password: pass})
    }).then(() => {
        hint.textContent = 'Device is restarting. Reconnect to your WiFi network and browse to http://esp32lab.local/';
    }).catch(() => {
        hint.textContent = 'Device is restarting. Reconnect to your WiFi network and browse to http://esp32lab.local/';
    });
}

function forgetWifi() {
    if (!currentApi) return;
    if (!confirm('Forget saved network? The device will restart in hotspot mode.')) return;
    const hint = panel.querySelector('#wifi-hint');
    hint.style.display = 'block';
    hint.textContent = 'Clearing credentials and restarting…';
    fetch(`http://${currentApi.ip}/api/wifi/forget`, {method: 'POST'})
        .then(() => { hint.textContent = 'Device restarted in hotspot mode. Connect to the “ESP32Lab” WiFi network.'; })
        .catch(() => { hint.textContent = 'Device restarted in hotspot mode. Connect to the “ESP32Lab” WiFi network.'; });
}

function startOta() {
    if (!currentApi) return;
    const file = panel.querySelector('#ota-file').files[0];
    if (!file) return;

    const bar = panel.querySelector('#ota-bar');
    const msg = panel.querySelector('#ota-msg');
    const btn = panel.querySelector('#ota-btn');
    panel.querySelector('#ota-progress').style.display = 'block';
    bar.style.width = '0%';
    bar.className = 'bar-fill';
    msg.textContent = 'Uploading...';
    btn.disabled = true;

    const form = new FormData();
    form.append('firmware', file, file.name);
    const xhr = new XMLHttpRequest();

    xhr.upload.onprogress = e => {
        if (e.lengthComputable) {
            const pct = Math.round(e.loaded / e.total * 100);
            bar.style.width = pct + '%';
            msg.textContent = `Uploading... ${pct}%`;
        }
    };
    xhr.onload = () => {
        btn.disabled = false;
        if (xhr.status === 200) {
            bar.style.width = '100%';
            msg.textContent = 'Update complete — device is rebooting. Reconnect in a few seconds.';
        } else {
            bar.className = 'bar-fill danger';
            try { msg.textContent = 'Failed: ' + JSON.parse(xhr.responseText).error; }
            catch { msg.textContent = 'Update failed.'; }
        }
    };
    xhr.onerror = () => {
        // Connection dropped after 100% upload = device rebooted after successful flash
        btn.disabled = false;
        bar.style.width = '100%';
        msg.textContent = 'Update complete — device is rebooting. Reconnect in a few seconds.';
    };
    xhr.open('POST', `http://${currentApi.ip}/api/system/update`);
    xhr.send(form);
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
