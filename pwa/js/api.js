export class ESP32LabAPI {
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

    disconnect() {
        this._setStatus(false);
    }

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
