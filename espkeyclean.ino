#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const char* apSsid = "ESP-Keyboard";
const char* apPass = "12345678";

ESP8266WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <style>
    :root{
      --bg:#0b0d12; --card:#121826; --key:#1a2336; --key2:#24314d;
      --text:#e8eefc; --muted:#93a4c7; --accent:#5aa2ff;
      --radius:14px;
    }
    *{box-sizing:border-box; -webkit-tap-highlight-color: transparent;}
    body{margin:0;padding:14px;background:linear-gradient(180deg,#070913,#0b0d12 45%,#070913);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;}
    .wrap{max-width:900px;margin:0 auto;display:flex;flex-direction:column;gap:12px;}
    .card{background:rgba(18,24,38,.92);border:1px solid rgba(255,255,255,.06);border-radius:18px;padding:12px;box-shadow:0 10px 30px rgba(0,0,0,.35);}
    .top{display:flex;gap:10px;align-items:center;margin-bottom:10px;}
    input{flex:1;padding:12px 14px;border-radius:14px;border:1px solid rgba(255,255,255,.10);background:rgba(0,0,0,.25);color:var(--text);font-size:16px;outline:none;}
    input::placeholder{color:rgba(147,164,199,.8)}
    .btn{padding:12px 14px;border-radius:14px;border:1px solid rgba(255,255,255,.10);background:linear-gradient(180deg, rgba(90,162,255,.25), rgba(90,162,255,.12));color:var(--text);font-weight:700;font-size:14px;min-width:84px}
    .grid{display:flex;flex-direction:column;gap:8px;}
    .row{display:flex;gap:8px;}
    .key{flex:1;padding:12px 0;text-align:center;border-radius:var(--radius);border:1px solid rgba(255,255,255,.08);
      background:linear-gradient(180deg, rgba(26,35,54,.98), rgba(15,20,32,.98));color:var(--text);font-size:16px;font-weight:650;user-select:none;touch-action:manipulation;}
    .key.secondary{background:linear-gradient(180deg, rgba(36,49,77,.98), rgba(18,24,38,.98));font-weight:800;}
    .key.accent{background:linear-gradient(180deg, rgba(90,162,255,.40), rgba(90,162,255,.18));border-color:rgba(90,162,255,.35);}
    .key.wide{flex:1.7}.key.xwide{flex:3.2}.key.small{font-size:14px}.on{color:#bfe0ff;border-color:rgba(90,162,255,.35);background:rgba(90,162,255,.12)}
    .status{display:flex;justify-content:space-between;align-items:center;margin-top:10px;color:var(--muted);font-size:12px}
    .pill{padding:6px 10px;border-radius:999px;border:1px solid rgba(255,255,255,.10);background:rgba(0,0,0,.18);}
    /* Mouse pad */
    .mouseWrap{display:flex;gap:10px;align-items:stretch;}
    .pad{flex:1;min-height:220px;border-radius:18px;border:1px solid rgba(255,255,255,.08);
      background:radial-gradient(1200px 500px at 30% 20%, rgba(90,162,255,.18), rgba(0,0,0,.15)),
                 linear-gradient(180deg, rgba(26,35,54,.92), rgba(15,20,32,.92));
      position:relative; touch-action:none; user-select:none; overflow:hidden;}
    .pad:after{content:"Trackpad";position:absolute;left:12px;top:10px;font-size:12px;color:rgba(147,164,199,.9)}
    .scroll{width:70px;border-radius:18px;border:1px solid rgba(255,255,255,.08);
      background:linear-gradient(180deg, rgba(36,49,77,.92), rgba(18,24,38,.92));
      display:flex;flex-direction:column;justify-content:space-between; padding:10px; touch-action:none; user-select:none;}
    .scroll .label{font-size:12px;color:rgba(147,164,199,.9);text-align:center;}
    .scroll .bar{flex:1;margin:10px 0;border-radius:999px;border:1px solid rgba(255,255,255,.08);background:rgba(0,0,0,.18);position:relative;overflow:hidden;}
    .dot{position:absolute;left:50%;transform:translateX(-50%);width:16px;height:16px;border-radius:50%;
      background:rgba(90,162,255,.8); top:50%;}
    .controls{display:flex;gap:8px; margin-top:10px;}
    .slider{flex:1;display:flex;gap:10px;align-items:center;color:rgba(147,164,199,.9);font-size:12px}
    input[type="range"]{width:100%}
  </style>
</head>
<body>
<div class="wrap">

  <!-- MOUSE CARD -->
  <div class="card">
    <div class="mouseWrap">
      <div class="pad" id="pad"></div>
      <div class="scroll" id="scroll">
        <div class="label">Scroll</div>
        <div class="bar"><div class="dot" id="dot"></div></div>
        <div class="label">▲/▼</div>
      </div>
    </div>

    <div class="controls">
      <div class="key secondary" id="lbtn" ontouchstart="bDown('L')" ontouchend="bUp('L')" onmousedown="bDown('L')" onmouseup="bUp('L')">Left</div>
      <div class="key secondary" id="mbtn" onclick="bClick('M')">Middle</div>
      <div class="key secondary" id="rbtn" ontouchstart="bDown('R')" ontouchend="bUp('R')" onmousedown="bDown('R')" onmouseup="bUp('R')">Right</div>
      <div class="key accent" onclick="send('S:WIN_TAP')">Win</div>
    </div>

    <div class="controls">
      <div class="slider">
        <span>Speed</span>
        <input id="spd" type="range" min="0.4" max="3.0" step="0.1" value="1.4">
      </div>
      <div class="slider">
        <span>Scroll</span>
        <input id="scr" type="range" min="0.4" max="3.0" step="0.1" value="1.2">
      </div>
    </div>

    <div class="status">
      <div class="pill">Tip: drag pad to move, hold Left/Right for drag</div>
      <div class="pill" id="ms">Mouse: ready</div>
    </div>
  </div>

  <!-- KEYBOARD CARD -->
  <div class="card">
    <div class="top">
      <input id="txt" placeholder="Type a whole word/sentence here, then Send" autocapitalize="none" autocomplete="off" autocorrect="off" />
      <button class="btn" onclick="sendText()">Send</button>
    </div>

    <div class="grid">
      <div class="row">
        <div class="key secondary small" onclick="send('S:ESC')">Esc</div>
        <div class="key small" onclick="send('S:F1')">F1</div><div class="key small" onclick="send('S:F2')">F2</div><div class="key small" onclick="send('S:F3')">F3</div>
        <div class="key small" onclick="send('S:F4')">F4</div><div class="key small" onclick="send('S:F5')">F5</div><div class="key small" onclick="send('S:F6')">F6</div>
        <div class="key small" onclick="send('S:F7')">F7</div><div class="key small" onclick="send('S:F8')">F8</div><div class="key small" onclick="send('S:F9')">F9</div>
        <div class="key small" onclick="send('S:F10')">F10</div><div class="key small" onclick="send('S:F11')">F11</div><div class="key small" onclick="send('S:F12')">F12</div>
      </div>

      <div class="row">
        <div class="key secondary small" onclick="send('S:TAB')">Tab</div>
        <div class="key secondary small" onclick="send('S:BKSP')">⌫</div>
        <div class="key secondary small" onclick="send('S:ENTER')">Enter</div>
        <div class="key secondary small" onclick="send('S:DEL')">Del</div>
      </div>

      <div class="row">
        <div class="key" onclick="t('q')">Q</div><div class="key" onclick="t('w')">W</div><div class="key" onclick="t('e')">E</div><div class="key" onclick="t('r')">R</div><div class="key" onclick="t('t')">T</div>
        <div class="key" onclick="t('y')">Y</div><div class="key" onclick="t('u')">U</div><div class="key" onclick="t('i')">I</div><div class="key" onclick="t('o')">O</div><div class="key" onclick="t('p')">P</div>
      </div>
      <div class="row">
        <div class="key" onclick="t('a')">A</div><div class="key" onclick="t('s')">S</div><div class="key" onclick="t('d')">D</div><div class="key" onclick="t('f')">F</div><div class="key" onclick="t('g')">G</div>
        <div class="key" onclick="t('h')">H</div><div class="key" onclick="t('j')">J</div><div class="key" onclick="t('k')">K</div><div class="key" onclick="t('l')">L</div>
      </div>
      <div class="row">
        <div class="key secondary wide" id="shiftKey" onclick="toggle('SHIFT')">Shift</div>
        <div class="key" onclick="t('z')">Z</div><div class="key" onclick="t('x')">X</div><div class="key" onclick="t('c')">C</div><div class="key" onclick="t('v')">V</div>
        <div class="key" onclick="t('b')">B</div><div class="key" onclick="t('n')">N</div><div class="key" onclick="t('m')">M</div>
        <div class="key secondary wide" onclick="send('S:BKSP')">⌫</div>
      </div>

      <div class="row">
        <div class="key secondary" id="ctrlKey" onclick="toggle('CTRL')">Ctrl</div>
        <div class="key secondary" id="altKey" onclick="toggle('ALT')">Alt</div>
        <div class="key accent" onclick="send('S:WIN_TAP')">Win</div>
        <div class="key xwide" onclick="send('S:SPACE')">Space</div>
        <div class="key secondary" onclick="send('S:ENTER')">Enter</div>
      </div>

      <div class="row">
        <div class="key secondary" onclick="send('S:LEFT')">◀</div>
        <div class="key secondary" onclick="send('S:UP')">▲</div>
        <div class="key secondary" onclick="send('S:DOWN')">▼</div>
        <div class="key secondary" onclick="send('S:RIGHT')">▶</div>
      </div>
    </div>

    <div class="status">
      <div class="pill">Wi-Fi: ESP-Keyboard</div>
      <div class="pill" id="modPill">Mods: none</div>
    </div>
  </div>
</div>

<script>
let mods = {SHIFT:false, CTRL:false, ALT:false};

function updateModsUI(){
  const pill = document.getElementById('modPill');
  const on = [];
  if(mods.CTRL) on.push('CTRL');
  if(mods.ALT) on.push('ALT');
  if(mods.SHIFT) on.push('SHIFT');
  pill.textContent = 'Mods: ' + (on.length ? on.join('+') : 'none');

  const setOn = (id, state) => document.getElementById(id)?.classList.toggle('on', state);
  setOn('ctrlKey', mods.CTRL);
  setOn('altKey', mods.ALT);
  setOn('shiftKey', mods.SHIFT);
}

async function send(cmd){
  fetch('/k?c=' + encodeURIComponent(cmd), {cache:'no-store'}).catch(()=>{});
}

function t(ch){ send('T:' + ch); }

function toggle(name){
  mods[name] = !mods[name];
  send('S:' + name);
  updateModsUI();
}
function sendText(){
  const el = document.getElementById('txt');
  const v = el.value;
  if(!v) return;
  send('P:' + v);
  el.value = '';
}
updateModsUI();

/* ===== TRACKPAD ===== */
const pad = document.getElementById('pad');
const scroll = document.getElementById('scroll');
const dot = document.getElementById('dot');
const ms = document.getElementById('ms');

let lastX=null, lastY=null;
let lastSend=0;

function now(){ return performance.now(); }

// throttle sending to ~60Hz
function sendMove(dx,dy,sc){
  const t = now();
  if (t - lastSend < 16) return;
  lastSend = t;

  dx = Math.max(-127, Math.min(127, dx|0));
  dy = Math.max(-127, Math.min(127, dy|0));
  sc = Math.max(-127, Math.min(127, sc|0));

  send(`M:${dx},${dy},${sc}`);
}

function padPoint(e){
  const t = (e.touches && e.touches[0]) ? e.touches[0] : e;
  return {x:t.clientX, y:t.clientY};
}

pad.addEventListener('touchstart', (e)=>{
  e.preventDefault();
  const p = padPoint(e);
  lastX=p.x; lastY=p.y;
  ms.textContent = 'Mouse: moving';
}, {passive:false});

pad.addEventListener('touchmove', (e)=>{
  e.preventDefault();
  const p = padPoint(e);
  if(lastX==null) { lastX=p.x; lastY=p.y; return; }

  const spd = parseFloat(document.getElementById('spd').value);
  let dx = (p.x - lastX) * spd;
  let dy = (p.y - lastY) * spd;

  // invert dy? (comment/uncomment)
  // dy = -dy;

  sendMove(dx, dy, 0);
  lastX=p.x; lastY=p.y;
}, {passive:false});

pad.addEventListener('touchend', (e)=>{
  lastX=null; lastY=null;
  ms.textContent = 'Mouse: ready';
}, {passive:false});

// Mouse support for desktop browser testing
pad.addEventListener('mousedown', (e)=>{ lastX=e.clientX; lastY=e.clientY; });
pad.addEventListener('mousemove', (e)=>{
  if(lastX==null) return;
  const spd = parseFloat(document.getElementById('spd').value);
  sendMove((e.clientX-lastX)*spd, (e.clientY-lastY)*spd, 0);
  lastX=e.clientX; lastY=e.clientY;
});
window.addEventListener('mouseup', ()=>{ lastX=null; lastY=null; });

/* ===== SCROLL STRIP ===== */
let sLastY=null;
scroll.addEventListener('touchstart', (e)=>{
  e.preventDefault();
  const p = padPoint(e);
  sLastY = p.y;
}, {passive:false});

scroll.addEventListener('touchmove', (e)=>{
  e.preventDefault();
  const p = padPoint(e);
  if(sLastY==null){ sLastY=p.y; return; }

  const scr = parseFloat(document.getElementById('scr').value);
  const dy = (p.y - sLastY) * scr;

  // moving finger down should scroll down (positive wheel on many systems is down)
  // If yours is reversed, flip the sign here:
  const wheel = Math.max(-127, Math.min(127, (dy)|0));
  sendMove(0, 0, wheel);

  // move dot for fun
  const rect = scroll.getBoundingClientRect();
  const rel = Math.max(0, Math.min(1, (p.y - rect.top) / rect.height));
  dot.style.top = (rel*100).toFixed(1) + '%';

  sLastY = p.y;
}, {passive:false});

scroll.addEventListener('touchend', ()=>{ sLastY=null; }, {passive:false});

/* ===== BUTTONS ===== */
function bClick(which){
  if(which==='L') send('B:LC');
  if(which==='R') send('B:RC');
  if(which==='M') send('B:MC');
}
function bDown(which){
  if(which==='L') send('B:LD');
  if(which==='R') send('B:RD');
}
function bUp(which){
  if(which==='L') send('B:LU');
  if(which==='R') send('B:RU');
}
</script>
</body>
</html>
)HTML";


void handleRoot() { server.send(200, "text/html", FPSTR(PAGE)); }

void handleKey() {
  if (!server.hasArg("c")) { server.send(400, "text/plain", "missing"); return; }
  Serial.println(server.arg("c"));
  server.send(204, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPass);
  delay(200);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/k", handleKey);
  server.onNotFound(handleRoot);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
