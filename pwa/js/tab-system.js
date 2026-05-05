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

    panel.querySelector('#ota-btn').onclick = startOta;
}

export function activateSystem(api) {
    currentApi = api;
    refresh(api);
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
