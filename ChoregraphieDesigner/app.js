// ClockClock24 Choreography Designer
// Convention: Slave 1-8 (columns), Clock 0-2 (rows from top)
// Angles in clock convention: 0° = 12h, 90° = 3h, 180° = 6h, 270° = 9h

// State
let keyframes = [];
let currentKeyframeIndex = 0;
let selectedClocks = new Set();
let isPlaying = false;
let playInterval = null;

// Clipboard for copy/paste
let clipboard = null;

// Drag and drop state for keyframes
let draggedKeyframeIndex = null;

// Loop markers
let loopStart = null;  // keyframe index where loop starts
let loopEnd = null;    // keyframe index where loop ends
let loopCount = 3;     // number of repetitions (0 = infinite)

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    // Create first keyframe
    addKeyframe();
    renderMatrix();
    renderTimeline();
    updatePreview();

    // Setup keyboard shortcuts
    setupKeyboardShortcuts();
});

// Keyboard shortcuts
function setupKeyboardShortcuts() {
    document.addEventListener('keydown', (e) => {
        // Don't trigger shortcuts when typing in inputs
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') {
            return;
        }

        const isMac = navigator.platform.toUpperCase().indexOf('MAC') >= 0;
        const modifier = isMac ? e.metaKey : e.ctrlKey;

        // Copy: Ctrl/Cmd + C
        if (modifier && e.key === 'c') {
            e.preventDefault();
            copySelectedClocks();
        }

        // Paste: Ctrl/Cmd + V
        if (modifier && e.key === 'v' && !e.shiftKey) {
            e.preventDefault();
            pasteClocks();
        }

        // Vertical Symmetry: Ctrl/Cmd + Shift + V
        if (modifier && e.shiftKey && e.key === 'V') {
            e.preventDefault();
            applyVerticalSymmetry();
        }

        // Horizontal Symmetry: Ctrl/Cmd + Shift + H
        if (modifier && e.shiftKey && e.key === 'H') {
            e.preventDefault();
            applyHorizontalSymmetry();
        }

        // Select All: Ctrl/Cmd + A
        if (modifier && e.key === 'a') {
            e.preventDefault();
            selectAll();
        }

        // Delete selection: Delete or Backspace (reset to default)
        if (e.key === 'Delete' || e.key === 'Backspace') {
            if (selectedClocks.size > 0) {
                e.preventDefault();
                resetSelectedClocks();
            }
        }

        // Escape: Deselect all
        if (e.key === 'Escape') {
            selectNone();
        }

        // Arrow keys to navigate keyframes
        if (e.key === 'ArrowLeft' && !modifier) {
            e.preventDefault();
            previewPrevious();
        }
        if (e.key === 'ArrowRight' && !modifier) {
            e.preventDefault();
            previewNext();
        }
    });
}

// Copy selected clocks to clipboard
function copySelectedClocks() {
    if (selectedClocks.size === 0) {
        showNotification('Sélectionnez des cadrans à copier (Ctrl+C)', 'warning');
        return;
    }

    clipboard = {
        clocks: [],
        minSlave: Infinity,
        minClock: Infinity
    };

    selectedClocks.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        const clockData = JSON.parse(JSON.stringify(keyframes[currentKeyframeIndex].clocks[slave][clock]));
        clipboard.clocks.push({
            slave,
            clock,
            data: clockData
        });
        clipboard.minSlave = Math.min(clipboard.minSlave, slave);
        clipboard.minClock = Math.min(clipboard.minClock, clock);
    });

    showNotification(`${clipboard.clocks.length} cadran(s) copié(s)`, 'success');
}

// Paste clocks from clipboard
function pasteClocks() {
    if (!clipboard || clipboard.clocks.length === 0) {
        showNotification('Rien à coller. Copiez d\'abord (Ctrl+C)', 'warning');
        return;
    }

    let pastedCount = 0;

    // If multiple clocks selected, paste the same copied data to ALL selected clocks
    if (selectedClocks.size > 0) {
        // Use only the FIRST copied clock's data when pasting to multiple targets
        const sourceData = clipboard.clocks[0].data;

        selectedClocks.forEach(cellId => {
            const [slave, clock] = cellId.split('-').map(Number);
            keyframes[currentKeyframeIndex].clocks[slave][clock] = JSON.parse(JSON.stringify(sourceData));
            pastedCount++;
        });
    } else {
        // No selection: paste with offset from top-left (original behavior)
        clipboard.clocks.forEach(item => {
            // Check bounds
            if (item.slave >= 0 && item.slave < 8 && item.clock >= 0 && item.clock < 3) {
                keyframes[currentKeyframeIndex].clocks[item.slave][item.clock] = JSON.parse(JSON.stringify(item.data));
                pastedCount++;
            }
        });
    }

    renderMatrix();
    updatePreview();
    showNotification(`${pastedCount} cadran(s) collé(s)`, 'success');
}

// Vertical symmetry (mirror left/right)
function applyVerticalSymmetry() {
    const targets = selectedClocks.size > 0 ? selectedClocks : getAllClockIds();

    targets.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        const clockData = keyframes[currentKeyframeIndex].clocks[slave][clock];

        // Mirror angles horizontally: angle -> (360 - angle) % 360
        clockData.angleH = (360 - clockData.angleH) % 360;
        clockData.angleM = (360 - clockData.angleM) % 360;

        // Swap directions
        clockData.dirH = clockData.dirH === 'CW' ? 'CCW' : 'CW';
        clockData.dirM = clockData.dirM === 'CW' ? 'CCW' : 'CW';
    });

    renderMatrix();
    updatePreview();
    showNotification('Symétrie verticale appliquée', 'success');
}

// Horizontal symmetry (mirror top/bottom)
function applyHorizontalSymmetry() {
    const targets = selectedClocks.size > 0 ? selectedClocks : getAllClockIds();

    targets.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        const clockData = keyframes[currentKeyframeIndex].clocks[slave][clock];

        // Mirror angles vertically: angle -> (180 - angle + 360) % 360
        clockData.angleH = (180 - clockData.angleH + 360) % 360;
        clockData.angleM = (180 - clockData.angleM + 360) % 360;

        // Swap directions
        clockData.dirH = clockData.dirH === 'CW' ? 'CCW' : 'CW';
        clockData.dirM = clockData.dirM === 'CW' ? 'CCW' : 'CW';
    });

    renderMatrix();
    updatePreview();
    showNotification('Symétrie horizontale appliquée', 'success');
}

// Reset selected clocks to default (180°)
function resetSelectedClocks() {
    selectedClocks.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        keyframes[currentKeyframeIndex].clocks[slave][clock] = {
            angleH: 180,
            angleM: 180,
            dirH: 'CW',
            dirM: 'CW'
        };
    });

    renderMatrix();
    updatePreview();
    showNotification('Cadrans réinitialisés à 6h', 'info');
}

// Show notification
function showNotification(message, type = 'info') {
    // Remove existing notification
    const existing = document.querySelector('.notification');
    if (existing) existing.remove();

    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    document.body.appendChild(notification);

    // Auto remove after 2 seconds
    setTimeout(() => {
        notification.classList.add('fade-out');
        setTimeout(() => notification.remove(), 300);
    }, 2000);
}

// Keyframe structure
function createEmptyKeyframe() {
    const keyframe = {
        id: Date.now(),
        comment: '',
        // Motion parameters
        speed: 400,           // Motor speed (steps/sec)
        accel: 150,           // Acceleration/deceleration (steps/sec²)
        delayMs: 0,           // Delay AFTER reaching this keyframe (ms)
        // Cascade effect
        cascadeMode: 'none',  // none, column, row, diagonal
        cascadeDelayMs: 100,  // Delay between groups when cascade is active
        clocks: [] // 8 slaves × 3 clocks
    };

    // Initialize 8 slaves, each with 3 clocks
    for (let slave = 0; slave < 8; slave++) {
        keyframe.clocks[slave] = [];
        for (let clock = 0; clock < 3; clock++) {
            keyframe.clocks[slave][clock] = {
                angleH: 180, // 6 o'clock (default calibration position)
                angleM: 180,
                dirH: 'CW',  // Direction to reach NEXT keyframe
                dirM: 'CW'
            };
        }
    }

    return keyframe;
}

// Keyframe management
function addKeyframe() {
    const keyframe = createEmptyKeyframe();
    keyframes.push(keyframe);
    currentKeyframeIndex = keyframes.length - 1;
    renderTimeline();
    renderMatrix();
    updatePreview();
}

function duplicateKeyframe() {
    if (keyframes.length === 0) return;

    const current = keyframes[currentKeyframeIndex];
    const duplicate = JSON.parse(JSON.stringify(current));
    duplicate.id = Date.now();
    duplicate.comment = current.comment + ' (copie)';

    keyframes.splice(currentKeyframeIndex + 1, 0, duplicate);
    currentKeyframeIndex++;
    renderTimeline();
    renderMatrix();
    updatePreview();
}

function deleteKeyframe() {
    if (keyframes.length <= 1) {
        alert('Il faut au moins un keyframe');
        return;
    }

    keyframes.splice(currentKeyframeIndex, 1);
    if (currentKeyframeIndex >= keyframes.length) {
        currentKeyframeIndex = keyframes.length - 1;
    }
    renderTimeline();
    renderMatrix();
    updatePreview();
}

function selectKeyframe(index) {
    currentKeyframeIndex = index;
    renderTimeline();
    renderMatrix();
    updatePreview();
}

// Render timeline with drag & drop support and loop markers
function renderTimeline() {
    const timeline = document.getElementById('timeline');
    timeline.innerHTML = '';

    keyframes.forEach((kf, index) => {
        // Add loop start marker before keyframe if this is the loop start
        if (loopStart === index) {
            const startMarker = document.createElement('div');
            startMarker.className = 'loop-marker loop-start';
            startMarker.innerHTML = `<span class="loop-bracket">[</span>`;
            startMarker.title = 'Début de boucle';
            startMarker.onclick = () => {
                loopStart = null;
                renderTimeline();
                showNotification('Début de boucle supprimé', 'info');
            };
            timeline.appendChild(startMarker);
        }

        const thumb = document.createElement('div');
        let thumbClass = 'keyframe-thumb';
        if (index === currentKeyframeIndex) thumbClass += ' active';
        if (loopStart !== null && loopEnd !== null && index >= loopStart && index <= loopEnd) {
            thumbClass += ' in-loop';
        }
        thumb.className = thumbClass;
        thumb.draggable = true;
        thumb.dataset.index = index;

        thumb.innerHTML = `
            <div class="kf-drag-handle">⋮⋮</div>
            <strong>KF ${index + 1}</strong>
            <span>${kf.comment ? kf.comment.substring(0, 12) + '...' : ''}</span>
        `;

        // Click to select
        thumb.onclick = (e) => {
            if (!e.target.classList.contains('kf-drag-handle')) {
                selectKeyframe(index);
            }
        };

        // Drag & Drop events
        thumb.ondragstart = (e) => {
            draggedKeyframeIndex = index;
            thumb.classList.add('dragging');
            e.dataTransfer.effectAllowed = 'move';
            e.dataTransfer.setData('text/plain', index);
        };

        thumb.ondragend = () => {
            thumb.classList.remove('dragging');
            draggedKeyframeIndex = null;
            // Remove all drag-over classes
            document.querySelectorAll('.keyframe-thumb').forEach(el => {
                el.classList.remove('drag-over-left', 'drag-over-right');
            });
        };

        thumb.ondragover = (e) => {
            e.preventDefault();
            if (draggedKeyframeIndex === null || draggedKeyframeIndex === index) return;

            const rect = thumb.getBoundingClientRect();
            const midX = rect.left + rect.width / 2;

            // Remove previous indicators
            thumb.classList.remove('drag-over-left', 'drag-over-right');

            // Show indicator on left or right side
            if (e.clientX < midX) {
                thumb.classList.add('drag-over-left');
            } else {
                thumb.classList.add('drag-over-right');
            }
        };

        thumb.ondragleave = () => {
            thumb.classList.remove('drag-over-left', 'drag-over-right');
        };

        thumb.ondrop = (e) => {
            e.preventDefault();
            if (draggedKeyframeIndex === null || draggedKeyframeIndex === index) return;

            const rect = thumb.getBoundingClientRect();
            const midX = rect.left + rect.width / 2;
            const insertBefore = e.clientX < midX;

            moveKeyframe(draggedKeyframeIndex, index, insertBefore);

            thumb.classList.remove('drag-over-left', 'drag-over-right');
        };

        timeline.appendChild(thumb);

        // Add loop end marker after keyframe if this is the loop end
        if (loopEnd === index) {
            const endMarker = document.createElement('div');
            endMarker.className = 'loop-marker loop-end';
            endMarker.innerHTML = `<span class="loop-bracket">]</span><span class="loop-count">${loopCount === 0 ? '∞' : 'x' + loopCount}</span>`;
            endMarker.title = 'Fin de boucle (cliquer pour supprimer)';
            endMarker.onclick = () => {
                loopEnd = null;
                renderTimeline();
                showNotification('Fin de boucle supprimée', 'info');
            };
            timeline.appendChild(endMarker);
        }
    });

    // Update loop controls display
    updateLoopControlsDisplay();
}

// Update loop controls visibility and state
function updateLoopControlsDisplay() {
    const loopInfo = document.getElementById('loopInfo');
    if (loopInfo) {
        if (loopStart !== null && loopEnd !== null) {
            loopInfo.textContent = `Boucle: KF ${loopStart + 1} → KF ${loopEnd + 1} (${loopCount === 0 ? '∞' : loopCount + 'x'})`;
            loopInfo.style.display = 'inline';
        } else if (loopStart !== null) {
            loopInfo.textContent = `Début: KF ${loopStart + 1} (fin non définie)`;
            loopInfo.style.display = 'inline';
        } else if (loopEnd !== null) {
            loopInfo.textContent = `Fin: KF ${loopEnd + 1} (début non défini)`;
            loopInfo.style.display = 'inline';
        } else {
            loopInfo.style.display = 'none';
        }
    }
}

// Move keyframe from one position to another
function moveKeyframe(fromIndex, toIndex, insertBefore) {
    if (fromIndex === toIndex) return;

    // Remove the keyframe from its original position
    const [movedKeyframe] = keyframes.splice(fromIndex, 1);

    // Calculate new insertion index
    let newIndex = toIndex;
    if (fromIndex < toIndex) {
        // If moving forward, account for the removal
        newIndex = insertBefore ? toIndex - 1 : toIndex;
    } else {
        // If moving backward
        newIndex = insertBefore ? toIndex : toIndex + 1;
    }

    // Insert at new position
    keyframes.splice(newIndex, 0, movedKeyframe);

    // Update current selection to follow the moved keyframe
    if (currentKeyframeIndex === fromIndex) {
        currentKeyframeIndex = newIndex;
    } else if (fromIndex < currentKeyframeIndex && newIndex >= currentKeyframeIndex) {
        currentKeyframeIndex--;
    } else if (fromIndex > currentKeyframeIndex && newIndex <= currentKeyframeIndex) {
        currentKeyframeIndex++;
    }

    renderTimeline();
    renderMatrix();
    updatePreview();
    showNotification(`Keyframe déplacé en position ${newIndex + 1}`, 'success');
}

// Move keyframe up (earlier in sequence)
function moveKeyframeUp() {
    if (currentKeyframeIndex <= 0) {
        showNotification('Déjà en première position', 'warning');
        return;
    }

    // Swap with previous
    [keyframes[currentKeyframeIndex - 1], keyframes[currentKeyframeIndex]] =
    [keyframes[currentKeyframeIndex], keyframes[currentKeyframeIndex - 1]];

    currentKeyframeIndex--;
    renderTimeline();
    renderMatrix();
    updatePreview();
    showNotification(`Keyframe déplacé en position ${currentKeyframeIndex + 1}`, 'success');
}

// Move keyframe down (later in sequence)
function moveKeyframeDown() {
    if (currentKeyframeIndex >= keyframes.length - 1) {
        showNotification('Déjà en dernière position', 'warning');
        return;
    }

    // Swap with next
    [keyframes[currentKeyframeIndex], keyframes[currentKeyframeIndex + 1]] =
    [keyframes[currentKeyframeIndex + 1], keyframes[currentKeyframeIndex]];

    currentKeyframeIndex++;
    renderTimeline();
    renderMatrix();
    updatePreview();
    showNotification(`Keyframe déplacé en position ${currentKeyframeIndex + 1}`, 'success');
}

// Loop marker management
function setLoopStart() {
    if (loopEnd !== null && currentKeyframeIndex > loopEnd) {
        showNotification('Le début de boucle doit être avant la fin', 'warning');
        return;
    }
    loopStart = currentKeyframeIndex;
    renderTimeline();
    showNotification(`Début de boucle: KF ${currentKeyframeIndex + 1}`, 'success');
}

function setLoopEnd() {
    if (loopStart !== null && currentKeyframeIndex < loopStart) {
        showNotification('La fin de boucle doit être après le début', 'warning');
        return;
    }
    loopEnd = currentKeyframeIndex;
    renderTimeline();
    showNotification(`Fin de boucle: KF ${currentKeyframeIndex + 1}`, 'success');
}

function clearLoop() {
    loopStart = null;
    loopEnd = null;
    renderTimeline();
    showNotification('Boucle supprimée', 'info');
}

function updateLoopCount(value) {
    loopCount = parseInt(value) || 0;
    renderTimeline();
}

// Render clock matrix with sliders
function renderMatrix() {
    const matrix = document.getElementById('clockMatrix');
    matrix.innerHTML = '';

    const currentKf = keyframes[currentKeyframeIndex];
    document.getElementById('currentKeyframeNum').textContent = currentKeyframeIndex + 1;
    document.getElementById('keyframeComment').value = currentKf.comment || '';

    // Update motion parameters inputs
    document.getElementById('kfSpeed').value = currentKf.speed || 400;
    document.getElementById('kfAccel').value = currentKf.accel || 150;
    document.getElementById('kfDelay').value = currentKf.delayMs || 0;
    document.getElementById('kfCascadeMode').value = currentKf.cascadeMode || 'none';
    document.getElementById('kfCascadeDelay').value = currentKf.cascadeDelayMs || 100;

    // Show/hide cascade delay based on mode
    const cascadeDelayGroup = document.getElementById('cascadeDelayGroup');
    if (cascadeDelayGroup) {
        cascadeDelayGroup.style.display = (currentKf.cascadeMode || 'none') === 'none' ? 'none' : 'flex';
    }

    // Row labels and clocks
    for (let row = 0; row < 3; row++) {
        // Row label
        const rowLabel = document.createElement('div');
        rowLabel.className = 'row-label';
        rowLabel.textContent = `Clock ${row}`;
        matrix.appendChild(rowLabel);

        // Clocks for this row (one per slave/column)
        for (let slave = 0; slave < 8; slave++) {
            const clock = currentKf.clocks[slave][row];
            const cellId = `${slave}-${row}`;
            const isSelected = selectedClocks.has(cellId);

            const cell = document.createElement('div');
            cell.className = 'clock-cell' + (isSelected ? ' selected' : '');
            cell.dataset.slave = slave;
            cell.dataset.clock = row;

            cell.innerHTML = `
                <div class="clock-face">
                    <div class="hand hand-h" style="transform: rotate(${clock.angleH}deg)"></div>
                    <div class="hand hand-m" style="transform: rotate(${clock.angleM}deg)"></div>
                </div>
                <div class="clock-inputs">
                    <div class="input-group input-h">
                        <label>H <span class="angle-value">${clock.angleH}°</span></label>
                        <input type="range" class="slider slider-h" min="0" max="359" value="${clock.angleH}"
                               oninput="updateAngleSlider(${slave}, ${row}, 'H', this.value)">
                        <select onchange="updateDir(${slave}, ${row}, 'H', this.value)">
                            <option value="CW" ${clock.dirH === 'CW' ? 'selected' : ''}>CW</option>
                            <option value="CCW" ${clock.dirH === 'CCW' ? 'selected' : ''}>CCW</option>
                        </select>
                    </div>
                    <div class="input-group input-m">
                        <label>M <span class="angle-value">${clock.angleM}°</span></label>
                        <input type="range" class="slider slider-m" min="0" max="359" value="${clock.angleM}"
                               oninput="updateAngleSlider(${slave}, ${row}, 'M', this.value)">
                        <select onchange="updateDir(${slave}, ${row}, 'M', this.value)">
                            <option value="CW" ${clock.dirM === 'CW' ? 'selected' : ''}>CW</option>
                            <option value="CCW" ${clock.dirM === 'CCW' ? 'selected' : ''}>CCW</option>
                        </select>
                    </div>
                </div>
            `;

            // Selection handling
            cell.onclick = (e) => {
                if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') return;
                toggleClockSelection(slave, row, e.shiftKey);
            };

            matrix.appendChild(cell);
        }
    }
}

// Update functions - optimized for sliders (no full re-render)
function updateAngleSlider(slave, clock, hand, value) {
    const angle = parseInt(value) || 0;
    const normalizedAngle = ((angle % 360) + 360) % 360;

    if (hand === 'H') {
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleH = normalizedAngle;
    } else {
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleM = normalizedAngle;
    }

    // Update only the affected clock visually (without full re-render for performance)
    const cell = document.querySelectorAll('.clock-cell')[clock * 8 + slave];
    if (cell) {
        const handEl = cell.querySelector(hand === 'H' ? '.hand-h' : '.hand-m');
        const labelEl = cell.querySelector(`.input-${hand.toLowerCase()} .angle-value`);
        if (handEl) handEl.style.transform = `rotate(${normalizedAngle}deg)`;
        if (labelEl) labelEl.textContent = `${normalizedAngle}°`;
    }

    updatePreview();
}

function updateAngle(slave, clock, hand, value) {
    const angle = parseInt(value) || 0;
    const normalizedAngle = ((angle % 360) + 360) % 360;

    if (hand === 'H') {
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleH = normalizedAngle;
    } else {
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleM = normalizedAngle;
    }

    renderMatrix();
    updatePreview();
}

function updateDir(slave, clock, hand, value) {
    if (hand === 'H') {
        keyframes[currentKeyframeIndex].clocks[slave][clock].dirH = value;
    } else {
        keyframes[currentKeyframeIndex].clocks[slave][clock].dirM = value;
    }
}

function saveComment() {
    keyframes[currentKeyframeIndex].comment = document.getElementById('keyframeComment').value;
    renderTimeline();
}

// Keyframe motion parameters
function saveSpeed() {
    const value = parseInt(document.getElementById('kfSpeed').value) || 400;
    keyframes[currentKeyframeIndex].speed = Math.max(200, Math.min(5000, value));
}

function saveAccel() {
    const value = parseInt(document.getElementById('kfAccel').value) || 150;
    keyframes[currentKeyframeIndex].accel = Math.max(100, Math.min(2000, value));
}

function saveDelay() {
    const value = parseInt(document.getElementById('kfDelay').value) || 0;
    keyframes[currentKeyframeIndex].delayMs = Math.max(0, value);
}

function saveCascadeMode() {
    const mode = document.getElementById('kfCascadeMode').value;
    keyframes[currentKeyframeIndex].cascadeMode = mode;
    // Show/hide cascade delay
    const cascadeDelayGroup = document.getElementById('cascadeDelayGroup');
    if (cascadeDelayGroup) {
        cascadeDelayGroup.style.display = mode === 'none' ? 'none' : 'flex';
    }
}

function saveCascadeDelay() {
    const value = parseInt(document.getElementById('kfCascadeDelay').value) || 100;
    keyframes[currentKeyframeIndex].cascadeDelayMs = Math.max(0, value);
}

// Selection management
function toggleClockSelection(slave, clock, additive) {
    const cellId = `${slave}-${clock}`;

    if (!additive) {
        selectedClocks.clear();
    }

    if (selectedClocks.has(cellId)) {
        selectedClocks.delete(cellId);
    } else {
        selectedClocks.add(cellId);
    }

    renderMatrix();
}

function selectAll() {
    selectedClocks.clear();
    for (let slave = 0; slave < 8; slave++) {
        for (let clock = 0; clock < 3; clock++) {
            selectedClocks.add(`${slave}-${clock}`);
        }
    }
    renderMatrix();
}

function selectNone() {
    selectedClocks.clear();
    renderMatrix();
}

function selectColumn() {
    const col = prompt('Numéro du slave (1-8):');
    const slave = parseInt(col) - 1;
    if (isNaN(slave) || slave < 0 || slave > 7) return;

    selectedClocks.clear();
    for (let clock = 0; clock < 3; clock++) {
        selectedClocks.add(`${slave}-${clock}`);
    }
    renderMatrix();
}

function selectRow() {
    const row = prompt('Numéro du clock (0-2):');
    const clock = parseInt(row);
    if (isNaN(clock) || clock < 0 || clock > 2) return;

    selectedClocks.clear();
    for (let slave = 0; slave < 8; slave++) {
        selectedClocks.add(`${slave}-${clock}`);
    }
    renderMatrix();
}

// Quick actions
function setAllAngles(angleH, angleM) {
    const targets = selectedClocks.size > 0 ? selectedClocks : getAllClockIds();

    targets.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleH = angleH;
        keyframes[currentKeyframeIndex].clocks[slave][clock].angleM = angleM;
    });

    renderMatrix();
    updatePreview();
}

function setAllDirections(dirH, dirM) {
    const targets = selectedClocks.size > 0 ? selectedClocks : getAllClockIds();

    targets.forEach(cellId => {
        const [slave, clock] = cellId.split('-').map(Number);
        keyframes[currentKeyframeIndex].clocks[slave][clock].dirH = dirH;
        keyframes[currentKeyframeIndex].clocks[slave][clock].dirM = dirM;
    });

    renderMatrix();
}

function getAllClockIds() {
    const ids = new Set();
    for (let slave = 0; slave < 8; slave++) {
        for (let clock = 0; clock < 3; clock++) {
            ids.add(`${slave}-${clock}`);
        }
    }
    return ids;
}

// Preview
function updatePreview() {
    const canvas = document.getElementById('previewCanvas');
    const ctx = canvas.getContext('2d');
    const kf = keyframes[currentKeyframeIndex];

    // Clear
    ctx.fillStyle = '#1a1a2e';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    const clockSize = 28;
    const padding = 10;
    const startX = 40;
    const startY = 30;

    // Draw clocks
    for (let slave = 0; slave < 8; slave++) {
        for (let clock = 0; clock < 3; clock++) {
            const x = startX + slave * (clockSize * 2 + padding);
            const y = startY + clock * (clockSize * 2 + padding);
            const data = kf.clocks[slave][clock];

            // Clock face
            ctx.beginPath();
            ctx.arc(x + clockSize, y + clockSize, clockSize, 0, Math.PI * 2);
            ctx.fillStyle = '#fff';
            ctx.fill();
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 2;
            ctx.stroke();

            // Hour hand (red)
            const hRad = (data.angleH - 90) * Math.PI / 180;
            ctx.beginPath();
            ctx.moveTo(x + clockSize, y + clockSize);
            ctx.lineTo(
                x + clockSize + Math.cos(hRad) * clockSize * 0.5,
                y + clockSize + Math.sin(hRad) * clockSize * 0.5
            );
            ctx.strokeStyle = '#e74c3c';
            ctx.lineWidth = 3;
            ctx.stroke();

            // Minute hand (blue)
            const mRad = (data.angleM - 90) * Math.PI / 180;
            ctx.beginPath();
            ctx.moveTo(x + clockSize, y + clockSize);
            ctx.lineTo(
                x + clockSize + Math.cos(mRad) * clockSize * 0.7,
                y + clockSize + Math.sin(mRad) * clockSize * 0.7
            );
            ctx.strokeStyle = '#3498db';
            ctx.lineWidth = 2;
            ctx.stroke();

            // Center dot
            ctx.beginPath();
            ctx.arc(x + clockSize, y + clockSize, 3, 0, Math.PI * 2);
            ctx.fillStyle = '#333';
            ctx.fill();
        }
    }

    // Labels
    ctx.fillStyle = '#888';
    ctx.font = '10px sans-serif';
    ctx.textAlign = 'center';

    for (let slave = 0; slave < 8; slave++) {
        const x = startX + slave * (clockSize * 2 + padding) + clockSize;
        ctx.fillText(`S${slave + 1}`, x, 15);
    }

    ctx.textAlign = 'right';
    for (let clock = 0; clock < 3; clock++) {
        const y = startY + clock * (clockSize * 2 + padding) + clockSize + 4;
        ctx.fillText(`C${clock}`, 25, y);
    }
}

// Animation preview with loop support
function playPreview() {
    if (isPlaying) {
        stopPreview();
        return;
    }

    if (keyframes.length < 2) {
        alert('Il faut au moins 2 keyframes pour animer');
        return;
    }

    isPlaying = true;
    document.getElementById('playBtn').textContent = '⏹️ Stop';

    let fromIndex = 0;
    let currentLoopIteration = 0;
    const maxLoopIterations = loopCount === 0 ? Infinity : loopCount;
    const duration = parseInt(document.getElementById('transitionDuration').value) || 1000;

    function animateTransition() {
        let toIndex = fromIndex + 1;

        // Check if we've reached the end of a loop
        if (loopStart !== null && loopEnd !== null && fromIndex === loopEnd) {
            currentLoopIteration++;
            if (currentLoopIteration < maxLoopIterations) {
                // Loop back to start
                toIndex = loopStart;
            } else {
                // Continue past the loop
                toIndex = loopEnd + 1;
                currentLoopIteration = 0; // Reset for next play
            }
        }

        // Check if we've reached the end of all keyframes
        if (toIndex >= keyframes.length) {
            toIndex = 0; // Loop back to beginning
            currentLoopIteration = 0; // Reset loop counter
        }

        animateBetweenKeyframes(fromIndex, toIndex, duration, () => {
            fromIndex = toIndex;
            if (isPlaying) {
                setTimeout(animateTransition, 500); // Pause between keyframes
            }
        });
    }

    animateTransition();
}

function stopPreview() {
    isPlaying = false;
    document.getElementById('playBtn').textContent = '▶️ Play';
}

function animateBetweenKeyframes(fromIdx, toIdx, duration, onComplete) {
    const fromKf = keyframes[fromIdx];
    const toKf = keyframes[toIdx];
    const startTime = performance.now();

    function animate(currentTime) {
        if (!isPlaying) return;

        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        const eased = easeInOutCubic(progress);

        drawAnimatedFrame(fromKf, toKf, eased);

        if (progress < 1) {
            requestAnimationFrame(animate);
        } else {
            if (onComplete) onComplete();
        }
    }

    requestAnimationFrame(animate);
}

function easeInOutCubic(t) {
    return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
}

function drawAnimatedFrame(fromKf, toKf, progress) {
    const canvas = document.getElementById('previewCanvas');
    const ctx = canvas.getContext('2d');

    ctx.fillStyle = '#1a1a2e';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    const clockSize = 28;
    const padding = 10;
    const startX = 40;
    const startY = 30;

    for (let slave = 0; slave < 8; slave++) {
        for (let clock = 0; clock < 3; clock++) {
            const x = startX + slave * (clockSize * 2 + padding);
            const y = startY + clock * (clockSize * 2 + padding);

            const from = fromKf.clocks[slave][clock];
            const to = toKf.clocks[slave][clock];

            // Calculate interpolated angles based on direction
            const angleH = interpolateAngle(from.angleH, to.angleH, from.dirH, progress);
            const angleM = interpolateAngle(from.angleM, to.angleM, from.dirM, progress);

            // Clock face
            ctx.beginPath();
            ctx.arc(x + clockSize, y + clockSize, clockSize, 0, Math.PI * 2);
            ctx.fillStyle = '#fff';
            ctx.fill();
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 2;
            ctx.stroke();

            // Hour hand
            const hRad = (angleH - 90) * Math.PI / 180;
            ctx.beginPath();
            ctx.moveTo(x + clockSize, y + clockSize);
            ctx.lineTo(
                x + clockSize + Math.cos(hRad) * clockSize * 0.5,
                y + clockSize + Math.sin(hRad) * clockSize * 0.5
            );
            ctx.strokeStyle = '#e74c3c';
            ctx.lineWidth = 3;
            ctx.stroke();

            // Minute hand
            const mRad = (angleM - 90) * Math.PI / 180;
            ctx.beginPath();
            ctx.moveTo(x + clockSize, y + clockSize);
            ctx.lineTo(
                x + clockSize + Math.cos(mRad) * clockSize * 0.7,
                y + clockSize + Math.sin(mRad) * clockSize * 0.7
            );
            ctx.strokeStyle = '#3498db';
            ctx.lineWidth = 2;
            ctx.stroke();

            // Center
            ctx.beginPath();
            ctx.arc(x + clockSize, y + clockSize, 3, 0, Math.PI * 2);
            ctx.fillStyle = '#333';
            ctx.fill();
        }
    }

    // Labels
    ctx.fillStyle = '#888';
    ctx.font = '10px sans-serif';
    ctx.textAlign = 'center';
    for (let slave = 0; slave < 8; slave++) {
        ctx.fillText(`S${slave + 1}`, startX + slave * (clockSize * 2 + padding) + clockSize, 15);
    }
    ctx.textAlign = 'right';
    for (let clock = 0; clock < 3; clock++) {
        ctx.fillText(`C${clock}`, 25, startY + clock * (clockSize * 2 + padding) + clockSize + 4);
    }
}

function interpolateAngle(from, to, direction, progress) {
    let delta;

    if (direction === 'CW') {
        // Clockwise: always go positive direction
        delta = to - from;
        if (delta <= 0) delta += 360;
    } else {
        // Counter-clockwise: always go negative direction
        delta = to - from;
        if (delta >= 0) delta -= 360;
    }

    return (from + delta * progress + 360) % 360;
}

function previewPrevious() {
    if (currentKeyframeIndex > 0) {
        selectKeyframe(currentKeyframeIndex - 1);
    }
}

function previewNext() {
    if (currentKeyframeIndex < keyframes.length - 1) {
        selectKeyframe(currentKeyframeIndex + 1);
    }
}

// Export/Import
function exportProject() {
    const projectName = document.getElementById('projectName').value || 'choreographie';
    const data = {
        name: projectName,
        version: '1.1',
        created: new Date().toISOString(),
        keyframes: keyframes,
        loop: {
            start: loopStart,
            end: loopEnd,
            count: loopCount
        }
    };

    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${projectName}.json`;
    a.click();
    URL.revokeObjectURL(url);
}

function importProject() {
    document.getElementById('importFile').click();
}

function handleFileImport(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const data = JSON.parse(e.target.result);
            if (data.keyframes && Array.isArray(data.keyframes)) {
                keyframes = data.keyframes;
                currentKeyframeIndex = 0;
                document.getElementById('projectName').value = data.name || 'imported';

                // Import loop settings if present
                if (data.loop) {
                    loopStart = data.loop.start;
                    loopEnd = data.loop.end;
                    loopCount = data.loop.count || 3;
                    document.getElementById('loopCountInput').value = loopCount;
                } else {
                    loopStart = null;
                    loopEnd = null;
                    loopCount = 3;
                }

                // Ensure all keyframes have motion parameters (for older files)
                keyframes.forEach(kf => {
                    if (kf.speed === undefined) kf.speed = 400;
                    if (kf.accel === undefined) kf.accel = 150;
                    if (kf.delayMs === undefined) kf.delayMs = 0;
                    if (kf.cascadeMode === undefined) kf.cascadeMode = 'none';
                    if (kf.cascadeDelayMs === undefined) kf.cascadeDelayMs = 100;
                });

                renderTimeline();
                renderMatrix();
                updatePreview();
                showNotification('Projet importé avec succès!', 'success');
            } else {
                throw new Error('Format invalide');
            }
        } catch (err) {
            showNotification('Erreur lors de l\'import: ' + err.message, 'error');
        }
    };
    reader.readAsText(file);
    event.target.value = ''; // Reset for next import
}

// Code generation
function generateCode() {
    const projectName = document.getElementById('projectName').value || 'choreographie';
    let code = `// ClockClock24 Choreographie: ${projectName}\n`;
    code += `// Generated: ${new Date().toISOString()}\n\n`;

    // Generate motion parameters struct
    code += `// Motion parameters per keyframe\n`;
    code += `typedef struct {\n`;
    code += `    uint16_t speed;         // Motor speed (steps/sec)\n`;
    code += `    uint16_t accel;         // Acceleration (steps/sec²)\n`;
    code += `    uint16_t delayMs;       // Delay after keyframe (ms)\n`;
    code += `    uint8_t cascadeMode;    // 0=none, 1=column, 2=row, 3=diagonal\n`;
    code += `    uint16_t cascadeDelayMs; // Delay between groups (ms)\n`;
    code += `} t_keyframe_params;\n\n`;

    // Generate motion parameters array
    code += `const t_keyframe_params ${projectName}_params[${keyframes.length}] = {\n`;
    keyframes.forEach((kf, idx) => {
        const cascadeModeValue = { 'none': 0, 'column': 1, 'row': 2, 'diagonal': 3 }[kf.cascadeMode || 'none'];
        code += `    { ${kf.speed || 400}, ${kf.accel || 150}, ${kf.delayMs || 0}, ${cascadeModeValue}, ${kf.cascadeDelayMs || 100} }`;
        if (idx < keyframes.length - 1) code += ',';
        code += ` // KF ${idx + 1}${kf.comment ? ': ' + kf.comment.substring(0, 30) : ''}\n`;
    });
    code += `};\n\n`;

    // Generate keyframe data structures
    code += `// Keyframe angles (slave 0-7, clock 0-2, [angleH, angleM])\n`;
    code += `const uint16_t ${projectName}_keyframes[${keyframes.length}][8][3][2] = {\n`;

    keyframes.forEach((kf, kfIdx) => {
        code += `    // Keyframe ${kfIdx + 1}${kf.comment ? ': ' + kf.comment : ''}\n`;
        code += `    {\n`;
        for (let slave = 0; slave < 8; slave++) {
            code += `        { `;
            for (let clock = 0; clock < 3; clock++) {
                const c = kf.clocks[slave][clock];
                code += `{${c.angleH}, ${c.angleM}}`;
                if (clock < 2) code += ', ';
            }
            code += ` }`;
            if (slave < 7) code += ',';
            code += ` // Slave ${slave + 1}\n`;
        }
        code += `    }`;
        if (kfIdx < keyframes.length - 1) code += ',';
        code += '\n';
    });
    code += `};\n\n`;

    // Generate direction data
    code += `// Direction modes (CW=1, CCW=0) for transitions TO next keyframe\n`;
    code += `const uint8_t ${projectName}_directions[${keyframes.length}][8][3][2] = {\n`;

    keyframes.forEach((kf, kfIdx) => {
        code += `    // Keyframe ${kfIdx + 1} -> ${kfIdx + 2 > keyframes.length ? 1 : kfIdx + 2}\n`;
        code += `    {\n`;
        for (let slave = 0; slave < 8; slave++) {
            code += `        { `;
            for (let clock = 0; clock < 3; clock++) {
                const c = kf.clocks[slave][clock];
                const dirH = c.dirH === 'CW' ? 'CLOCKWISE' : 'COUNTER_CLOCKWISE';
                const dirM = c.dirM === 'CW' ? 'CLOCKWISE' : 'COUNTER_CLOCKWISE';
                code += `{${dirH}, ${dirM}}`;
                if (clock < 2) code += ', ';
            }
            code += ` }`;
            if (slave < 7) code += ',';
            code += '\n';
        }
        code += `    }`;
        if (kfIdx < keyframes.length - 1) code += ',';
        code += '\n';
    });
    code += `};\n\n`;

    // Generate comments array
    code += `// Keyframe comments/instructions\n`;
    code += `const char* ${projectName}_comments[${keyframes.length}] = {\n`;
    keyframes.forEach((kf, idx) => {
        const comment = kf.comment ? kf.comment.replace(/"/g, '\\"') : '';
        code += `    "${comment}"`;
        if (idx < keyframes.length - 1) code += ',';
        code += '\n';
    });
    code += `};\n\n`;

    // Generate loop configuration
    code += `// Loop configuration\n`;
    code += `const int ${projectName}_loop_start = ${loopStart !== null ? loopStart : -1}; // -1 = no loop\n`;
    code += `const int ${projectName}_loop_end = ${loopEnd !== null ? loopEnd : -1};\n`;
    code += `const int ${projectName}_loop_count = ${loopCount}; // 0 = infinite\n\n`;

    // Generate cascade enum
    code += `// Cascade modes\n`;
    code += `enum CascadeMode { CASCADE_NONE = 0, CASCADE_COLUMN = 1, CASCADE_ROW = 2, CASCADE_DIAGONAL = 3 };\n\n`;

    // Generate helper function
    code += `// Helper function to apply a keyframe with motion parameters\n`;
    code += `void apply_${projectName}_keyframe(int keyframe_index) {\n`;
    code += `    const t_keyframe_params& params = ${projectName}_params[keyframe_index];\n`;
    code += `    \n`;
    code += `    // Apply motion settings to all motors\n`;
    code += `    for (int slave = 0; slave < 8; slave++) {\n`;
    code += `        for (int clock = 0; clock < 3; clock++) {\n`;
    code += `            uint16_t angleH = ${projectName}_keyframes[keyframe_index][slave][clock][0];\n`;
    code += `            uint16_t angleM = ${projectName}_keyframes[keyframe_index][slave][clock][1];\n`;
    code += `            uint8_t dirH = ${projectName}_directions[keyframe_index][slave][clock][0];\n`;
    code += `            uint8_t dirM = ${projectName}_directions[keyframe_index][slave][clock][1];\n`;
    code += `            \n`;
    code += `            // Calculate cascade delay based on mode\n`;
    code += `            uint16_t delay = 0;\n`;
    code += `            if (params.cascadeMode == CASCADE_COLUMN) {\n`;
    code += `                delay = slave * params.cascadeDelayMs;\n`;
    code += `            } else if (params.cascadeMode == CASCADE_ROW) {\n`;
    code += `                delay = clock * params.cascadeDelayMs;\n`;
    code += `            } else if (params.cascadeMode == CASCADE_DIAGONAL) {\n`;
    code += `                delay = (slave + clock) * params.cascadeDelayMs;\n`;
    code += `            }\n`;
    code += `            \n`;
    code += `            // Apply to your clock system with speed, accel, and delay\n`;
    code += `            // set_clock_position(slave, clock, angleH, angleM, dirH, dirM, params.speed, params.accel, delay);\n`;
    code += `        }\n`;
    code += `    }\n`;
    code += `    \n`;
    code += `    // Wait for keyframe delay after all clocks reach position\n`;
    code += `    // delay(params.delayMs);\n`;
    code += `}\n\n`;

    // Generate loop-aware playback function
    code += `// Play choreography with loop support\n`;
    code += `void play_${projectName}_choreography() {\n`;
    code += `    int current_kf = 0;\n`;
    code += `    int loop_iteration = 0;\n`;
    code += `    const int total_keyframes = ${keyframes.length};\n`;
    code += `    \n`;
    code += `    while (true) {\n`;
    code += `        apply_${projectName}_keyframe(current_kf);\n`;
    code += `        // wait_for_transition(); // Implement your timing\n`;
    code += `        \n`;
    code += `        // Check for loop\n`;
    code += `        if (${projectName}_loop_start >= 0 && ${projectName}_loop_end >= 0 &&\n`;
    code += `            current_kf == ${projectName}_loop_end) {\n`;
    code += `            loop_iteration++;\n`;
    code += `            if (${projectName}_loop_count == 0 || loop_iteration < ${projectName}_loop_count) {\n`;
    code += `                current_kf = ${projectName}_loop_start;\n`;
    code += `                continue;\n`;
    code += `            }\n`;
    code += `            loop_iteration = 0;\n`;
    code += `        }\n`;
    code += `        \n`;
    code += `        current_kf++;\n`;
    code += `        if (current_kf >= total_keyframes) break; // or loop: current_kf = 0;\n`;
    code += `    }\n`;
    code += `}\n`;

    document.getElementById('codeOutput').textContent = code;
}

function copyCode() {
    const code = document.getElementById('codeOutput').textContent;
    if (code) {
        navigator.clipboard.writeText(code).then(() => {
            alert('Code copié dans le presse-papier!');
        });
    }
}
