// ClockClock24 Choreography Designer
// Convention: Slave 1-8 (columns), Clock 0-2 (rows from top)
// Angles in clock convention: 0° = 12h, 90° = 3h, 180° = 6h, 270° = 9h

// State
let keyframes = [];
let currentKeyframeIndex = 0;
let selectedClocks = new Set();
let isPlaying = false;
let playInterval = null;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    // Create first keyframe
    addKeyframe();
    renderMatrix();
    renderTimeline();
    updatePreview();
});

// Keyframe structure
function createEmptyKeyframe() {
    const keyframe = {
        id: Date.now(),
        comment: '',
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

// Render timeline
function renderTimeline() {
    const timeline = document.getElementById('timeline');
    timeline.innerHTML = '';

    keyframes.forEach((kf, index) => {
        const thumb = document.createElement('div');
        thumb.className = 'keyframe-thumb' + (index === currentKeyframeIndex ? ' active' : '');
        thumb.innerHTML = `
            <strong>KF ${index + 1}</strong>
            <span>${kf.comment ? kf.comment.substring(0, 15) + '...' : 'Sans titre'}</span>
        `;
        thumb.onclick = () => selectKeyframe(index);
        timeline.appendChild(thumb);
    });
}

// Render clock matrix
function renderMatrix() {
    const matrix = document.getElementById('clockMatrix');
    matrix.innerHTML = '';

    const currentKf = keyframes[currentKeyframeIndex];
    document.getElementById('currentKeyframeNum').textContent = currentKeyframeIndex + 1;
    document.getElementById('keyframeComment').value = currentKf.comment || '';

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
                        <label>H (°)</label>
                        <input type="number" min="0" max="359" value="${clock.angleH}"
                               onchange="updateAngle(${slave}, ${row}, 'H', this.value)">
                        <select onchange="updateDir(${slave}, ${row}, 'H', this.value)">
                            <option value="CW" ${clock.dirH === 'CW' ? 'selected' : ''}>CW</option>
                            <option value="CCW" ${clock.dirH === 'CCW' ? 'selected' : ''}>CCW</option>
                        </select>
                    </div>
                    <div class="input-group input-m">
                        <label>M (°)</label>
                        <input type="number" min="0" max="359" value="${clock.angleM}"
                               onchange="updateAngle(${slave}, ${row}, 'M', this.value)">
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

// Update functions
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

// Animation preview
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
    const duration = parseInt(document.getElementById('transitionDuration').value) || 1000;

    function animateTransition() {
        const toIndex = (fromIndex + 1) % keyframes.length;
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
        version: '1.0',
        created: new Date().toISOString(),
        keyframes: keyframes
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
                renderTimeline();
                renderMatrix();
                updatePreview();
                alert('Projet importé avec succès!');
            } else {
                throw new Error('Format invalide');
            }
        } catch (err) {
            alert('Erreur lors de l\'import: ' + err.message);
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

    // Generate helper function
    code += `// Helper function to apply a keyframe\n`;
    code += `void apply_${projectName}_keyframe(int keyframe_index) {\n`;
    code += `    for (int slave = 0; slave < 8; slave++) {\n`;
    code += `        for (int clock = 0; clock < 3; clock++) {\n`;
    code += `            uint16_t angleH = ${projectName}_keyframes[keyframe_index][slave][clock][0];\n`;
    code += `            uint16_t angleM = ${projectName}_keyframes[keyframe_index][slave][clock][1];\n`;
    code += `            uint8_t dirH = ${projectName}_directions[keyframe_index][slave][clock][0];\n`;
    code += `            uint8_t dirM = ${projectName}_directions[keyframe_index][slave][clock][1];\n`;
    code += `            \n`;
    code += `            // Apply to your clock system\n`;
    code += `            // set_clock_position(slave, clock, angleH, angleM, dirH, dirM);\n`;
    code += `        }\n`;
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
