// =====================================================================
// Embedded Sentry — In-browser FSM simulator
// Mirrors the Arduino firmware's finite-state machine so visitors can
// experience the gesture lock without any hardware.
// =====================================================================

const DIR = {
  RIGHT:    { id: 1, name: 'right',    icon: '▶', accel: { x: -6, y:  0, z:  9.81 } },
  LEFT:     { id: 2, name: 'left',     icon: '◀', accel: { x:  6, y:  0, z:  9.81 } },
  UP:       { id: 3, name: 'up',       icon: '▲', accel: { x:  0, y:  0, z: 15    } },
  DOWN:     { id: 4, name: 'down',     icon: '▼', accel: { x:  0, y:  0, z:  4    } },
  FORWARD:  { id: 5, name: 'forward',  icon: '↗', accel: { x:  0, y: -5, z:  9.81 } },
  BACKWARD: { id: 6, name: 'backward', icon: '↙', accel: { x:  0, y:  5, z:  9.81 } },
};
const DIR_BY_ID = Object.fromEntries(Object.values(DIR).map(d => [d.id, d]));
const SEQ_LEN = 3;

// FSM phases — same conceptual states as the firmware.
const PHASE = { RECORDING: 'RECORDING', UNLOCK: 'UNLOCK', OPEN: 'OPEN' };

const state = {
  phase: PHASE.RECORDING,
  recorded: [],
  attempt: [],
  busy: false,
};

// ----- DOM -----
const $ = (id) => document.getElementById(id);
const els = {
  device:   $('device'),
  led:      $('led'),
  lockText: $('lock-text'),
  lockDot:  $('lock-dot'),
  lockPill: $('lock-state'),
  phasePill:$('phase-pill'),
  stepRec:  $('step-rec'),
  stepUnlock: $('step-unlock'),
  seqRec:   $('seq-recorded'),
  seqAtt:   $('seq-attempt'),
  serial:   $('serial'),
  bar: { x: $('bar-x'), y: $('bar-y'), z: $('bar-z') },
  val: { x: $('val-x'), y: $('val-y'), z: $('val-z') },
  rings:    document.querySelectorAll('.dir-ring'),
};

// ----- Serial console -----
function log(msg, kind = 'sys') {
  const line = document.createElement('div');
  line.className = `l ${kind}`;
  line.textContent = `> ${msg}`;
  els.serial.appendChild(line);
  els.serial.scrollTop = els.serial.scrollHeight;
  // Cap log length
  while (els.serial.children.length > 60) els.serial.removeChild(els.serial.firstChild);
}

// ----- Accelerometer simulation -----
let accelTarget = { x: 0, y: 0, z: 9.81 };
let accel       = { x: 0, y: 0, z: 9.81 };

function setAccelFromDir(dir) {
  accelTarget = { ...dir.accel };
  // ease back to neutral
  setTimeout(() => { accelTarget = { x: 0, y: 0, z: 9.81 }; }, 280);
}

function renderAccel() {
  // smooth toward target
  accel.x += (accelTarget.x - accel.x) * 0.18;
  accel.y += (accelTarget.y - accel.y) * 0.18;
  accel.z += (accelTarget.z - accel.z) * 0.18;
  for (const ax of ['x', 'y', 'z']) {
    const v = accel[ax];
    els.val[ax].textContent = v.toFixed(2);
    // bar: center is 50%, range is ~±15
    const pct = Math.max(-1, Math.min(1, v / 15));
    const left = 50 + pct * 48; // 2..98%
    const w = Math.abs(pct) * 48; // half-width
    const bar = els.bar[ax];
    bar.style.left  = (pct >= 0 ? '50%' : `${left}%`);
    bar.style.width = `${Math.max(2, w)}%`;
  }
  requestAnimationFrame(renderAccel);
}
renderAccel();

// ----- Device tilt animation -----
function tiltDevice(dir) {
  const map = {
    1: 'rotateY(35deg)',                        // right
    2: 'rotateY(-35deg)',                       // left
    3: 'rotateX(-30deg) translateY(-12px)',     // up
    4: 'rotateX(30deg)  translateY(12px)',      // down
    5: 'rotateX(-20deg) rotateY(20deg) scale(0.9)',  // forward
    6: 'rotateX(20deg)  rotateY(-20deg) scale(1.1)', // backward
  };
  els.device.style.transform = map[dir.id] || '';
  setTimeout(() => { els.device.style.transform = ''; }, 380);
}

// ----- Sequence rendering -----
function renderSeq(slotsEl, arr, opts = {}) {
  const slots = slotsEl.querySelectorAll('.seq-slot');
  slots.forEach((slot, i) => {
    slot.classList.remove('filled', 'match', 'miss');
    if (arr[i]) {
      slot.classList.add('filled');
      slot.textContent = DIR_BY_ID[arr[i]].icon;
      if (opts.match && opts.match[i]) slot.classList.add('match');
      if (opts.miss === i) slot.classList.add('miss');
    } else {
      slot.textContent = '·';
    }
  });
}

// ----- Phase / status UI -----
function setPhase(p) {
  state.phase = p;
  els.phasePill.textContent = p;
  els.phasePill.className = 'phase-pill';
  if (p === PHASE.UNLOCK) els.phasePill.classList.add('unlock');
  if (p === PHASE.OPEN)   els.phasePill.classList.add('open');

  els.stepRec.classList.toggle('active', p === PHASE.RECORDING);
  els.stepRec.classList.toggle('done',   p !== PHASE.RECORDING);
  els.stepUnlock.classList.toggle('active', p === PHASE.UNLOCK);
  els.stepUnlock.classList.toggle('done',   p === PHASE.OPEN);
}

function setLocked(locked) {
  if (locked) {
    els.lockText.textContent = 'LOCKED';
    els.lockDot.className = 'dot dot-red';
    els.lockPill.classList.remove('open');
    els.led.classList.remove('on');
  } else {
    els.lockText.textContent = 'UNLOCKED';
    els.lockDot.className = 'dot dot-green';
    els.lockPill.classList.add('open');
    els.led.classList.add('on');
  }
}

// ----- Core FSM step (called when a direction is detected) -----
function onDirection(dir) {
  if (state.busy) return;
  setAccelFromDir(dir);
  tiltDevice(dir);

  // Highlight the ring briefly
  const ring = document.querySelector(`.dir-ring[data-dir="${dir.id}"]`);
  if (ring) {
    ring.classList.add('active');
    setTimeout(() => ring.classList.remove('active'), 280);
  }

  log(`${dir.name}`, 'evt');

  if (state.phase === PHASE.RECORDING) {
    state.recorded.push(dir.id);
    renderSeq(els.seqRec, state.recorded);
    log(`${dir.name} recorded (${state.recorded.length}/${SEQ_LEN})`, 'sys');
    if (state.recorded.length === SEQ_LEN) {
      setPhase(PHASE.UNLOCK);
      log('Sequence saved. Repeat it to unlock.', 'ok');
    }
    return;
  }

  if (state.phase === PHASE.UNLOCK) {
    const idx = state.attempt.length;
    const expected = state.recorded[idx];
    if (dir.id === expected) {
      state.attempt.push(dir.id);
      const matchMap = state.attempt.map(() => true);
      renderSeq(els.seqAtt, state.attempt, { match: matchMap });
      log(`that's ${idx + 1}, next..?`, 'ok');
      if (state.attempt.length === SEQ_LEN) {
        unlock();
      }
    } else {
      // wrong key
      state.attempt.push(dir.id);
      renderSeq(els.seqAtt, state.attempt, { miss: idx });
      log('Wrong key, please start over', 'err');
      state.busy = true;
      setTimeout(() => {
        state.attempt = [];
        renderSeq(els.seqAtt, []);
        state.busy = false;
      }, 900);
    }
    return;
  }
  // OPEN phase ignores input
}

const UNLOCK_HOLD_MS = 6000;
function unlock() {
  state.busy = true;
  setPhase(PHASE.OPEN);
  setLocked(false);
  log('Opened! Driving PIN 13 HIGH', 'ok');

  setTimeout(() => {
    setLocked(true);
    log('Closed again. Re-arm to unlock.', 'sys');
    state.attempt = [];
    renderSeq(els.seqAtt, []);
    setPhase(PHASE.UNLOCK);
    state.busy = false;
  }, UNLOCK_HOLD_MS);
}

// ----- Reset -----
function reset() {
  state.phase = PHASE.RECORDING;
  state.recorded = [];
  state.attempt = [];
  state.busy = false;
  renderSeq(els.seqRec, []);
  renderSeq(els.seqAtt, []);
  setPhase(PHASE.RECORDING);
  setLocked(true);
  els.serial.innerHTML = '';
  log('Adafruit MPU6050 boot OK', 'sys');
  log('Accelerometer range: ±8G', 'sys');
  log('Filter bandwidth: 21 Hz', 'sys');
  log('Ready. Record a 3-move sequence.', 'sys');
}

// ----- Demo sequence -----
async function runDemo() {
  if (state.busy) return;
  reset();
  await sleep(400);
  const demo = [DIR.RIGHT, DIR.UP, DIR.LEFT];
  for (const d of demo) { onDirection(d); await sleep(700); }
  await sleep(400);
  for (const d of demo) { onDirection(d); await sleep(700); }
}
const sleep = (ms) => new Promise(r => setTimeout(r, ms));

// ----- Input bindings -----
els.rings.forEach(r => {
  r.addEventListener('click', () => onDirection(DIR_BY_ID[+r.dataset.dir]));
});

document.addEventListener('keydown', (e) => {
  const map = {
    ArrowRight: DIR.RIGHT, ArrowLeft: DIR.LEFT,
    ArrowUp: DIR.UP, ArrowDown: DIR.DOWN,
    w: DIR.FORWARD, W: DIR.FORWARD,
    s: DIR.BACKWARD, S: DIR.BACKWARD,
  };
  if (map[e.key]) { e.preventDefault(); onDirection(map[e.key]); }
});

$('btn-reset').addEventListener('click', reset);
$('btn-demo').addEventListener('click', runDemo);

// ----- Boot -----
reset();
