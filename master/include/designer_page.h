#ifndef DESIGNER_PAGE_H
#define DESIGNER_PAGE_H

// ClockClock24 Choreography Designer - Embedded version
// This page allows creating and testing choreographies directly on the ESP32

const char DESIGNER_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>ClockClock24 - Designer</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Tahoma,Helvetica,sans-serif;background:#212121;color:#fff;min-height:100vh;user-select:none;padding:15px}
.container{max-width:1400px;margin:0 auto}
header{display:flex;justify-content:space-between;align-items:center;margin-bottom:15px;padding-bottom:15px;border-bottom:1px solid #444;flex-wrap:wrap;gap:10px}
header h1{font-size:1.2rem;font-weight:400}
.project-controls{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.project-controls input[type="text"]{padding:6px 10px;border:none;background:transparent;color:#fff;width:160px;box-shadow:inset 0 0 2px #dfdfdf}
button{padding:6px 12px;border:none;background:transparent;color:#fff;cursor:pointer;font-size:13px;box-shadow:inset 0 0 2px #dfdfdf;transition:background .2s}
button:hover{background:#b4b4b44b}
button.active{background:#dfdfdf;color:#000}
button.danger{box-shadow:inset 0 0 2px #c60a0a;color:#ff6b6b}
button.live{box-shadow:inset 0 0 2px #2e7d32;color:#4caf50}
button.live.active{background:#2e7d32;color:#fff}
.btn-back{color:#8a8a8a;text-decoration:none;padding:6px 12px;box-shadow:inset 0 0 2px #dfdfdf}
.main-content{display:grid;grid-template-columns:1fr 220px;gap:15px}
.timeline-section{background:#2a2a2a;padding:12px;margin-bottom:15px;grid-column:1/-1;box-shadow:inset 0 0 1px #555}
.timeline-section h2{margin-bottom:8px;font-size:14px;color:#8a8a8a}
.timeline-controls{margin-bottom:8px;display:flex;gap:8px;flex-wrap:wrap}
.timeline{display:flex;gap:8px;overflow-x:auto;padding:8px 0}
.keyframe-thumb{min-width:80px;height:50px;background:#333;cursor:pointer;display:flex;flex-direction:column;align-items:center;justify-content:center;border:2px solid transparent;transition:all .2s;box-shadow:inset 0 0 1px #555;position:relative;padding-left:16px}
.keyframe-thumb:hover{background:#3a3a3a}
.keyframe-thumb.active{border-color:#dfdfdf;background:#3a3a3a}
.keyframe-thumb span{font-size:11px;color:#8a8a8a}
.keyframe-thumb strong{font-size:13px}
.kf-drag-handle{position:absolute;left:2px;top:50%;transform:translateY(-50%);color:#666;font-size:10px;cursor:grab}
.motion-params{display:flex;flex-wrap:wrap;gap:12px;align-items:center;background:#1a1a1a;padding:10px;margin-bottom:12px;box-shadow:inset 0 0 1px #555}
.param-group{display:flex;align-items:center;gap:6px}
.param-group label{font-size:12px;color:#8a8a8a;white-space:nowrap}
.param-group input,.param-group select{width:70px;padding:4px 6px;border:1px solid #555;background:#2a2a2a;color:#fff;font-size:12px;text-align:center}
.param-group select{width:90px;text-align:left}
.param-unit{font-size:10px;color:#666}
.matrix-section{background:#2a2a2a;padding:15px;box-shadow:inset 0 0 1px #555}
.matrix-section h2{margin-bottom:12px;font-size:14px}
.matrix-header{margin-bottom:8px}
.slave-labels{display:grid;grid-template-columns:50px repeat(8,1fr);gap:6px;font-size:11px;color:#8a8a8a;text-align:center}
.clock-matrix{display:grid;grid-template-columns:50px repeat(8,1fr);gap:6px}
.row-label{display:flex;align-items:center;justify-content:center;font-size:11px;color:#8a8a8a}
.clock-cell{background:#1a1a1a;padding:8px;display:flex;flex-direction:column;align-items:center;gap:6px;border:2px solid transparent;transition:all .2s;cursor:pointer;box-shadow:inset 0 0 1px #444}
.clock-cell:hover{background:#252525}
.clock-cell.selected{border-color:#dfdfdf}
.clock-face{width:50px;height:50px;border-radius:50%;background:#fff;position:relative;border:2px solid #333}
.clock-face::before{content:'';position:absolute;top:50%;left:50%;width:5px;height:5px;background:#333;border-radius:50%;transform:translate(-50%,-50%);z-index:10}
.hand{position:absolute;bottom:50%;left:50%;transform-origin:bottom center;border-radius:2px}
.hand-h{width:3px;height:15px;background:#c60a0a;margin-left:-1.5px}
.hand-m{width:2px;height:20px;background:#3498db;margin-left:-1px}
.clock-inputs{display:grid;grid-template-columns:1fr 1fr;gap:3px;width:100%}
.input-group{display:flex;flex-direction:column;gap:1px}
.input-group label{font-size:9px;color:#8a8a8a;text-align:center}
.input-group input[type="range"]{width:100%;height:4px;-webkit-appearance:none;background:#1a1a1a;cursor:pointer}
.input-group input[type="range"]::-webkit-slider-thumb{-webkit-appearance:none;width:10px;height:10px;border-radius:50%;cursor:pointer}
.slider-h::-webkit-slider-thumb{background:#c60a0a}
.slider-m::-webkit-slider-thumb{background:#3498db}
.input-group select{width:100%;padding:2px;border:1px solid #444;background:#2a2a2a;color:#fff;font-size:10px}
.input-h select{border-color:#c60a0a}
.input-m select{border-color:#3498db}
.quick-actions{background:#2a2a2a;padding:12px;height:fit-content;box-shadow:inset 0 0 1px #555}
.quick-actions h3{margin-bottom:12px;font-size:13px}
.action-group{margin-bottom:12px}
.action-group h4{font-size:11px;color:#8a8a8a;margin-bottom:6px}
.action-group button{display:block;width:100%;margin-bottom:4px;padding:5px 8px;font-size:12px;background:#333;box-shadow:inset 0 0 1px #555}
.preview-section{background:#2a2a2a;padding:15px;margin-bottom:15px;box-shadow:inset 0 0 1px #555}
.preview-section h2{margin-bottom:12px;font-size:14px;color:#8a8a8a}
.preview-controls{display:flex;gap:8px;align-items:center;margin-bottom:12px;flex-wrap:wrap}
#previewCanvas{background:#1a1a1a;width:100%;max-width:700px}
.live-mode{background:#1a2a1a;padding:10px;margin-bottom:15px;box-shadow:inset 0 0 2px #2e7d32}
.live-mode h3{color:#4caf50;font-size:13px;margin-bottom:8px}
.live-mode p{font-size:11px;color:#8a8a8a;margin-bottom:8px}
.notification{position:fixed;top:15px;right:15px;padding:10px 16px;font-size:12px;z-index:1000;animation:slideIn .3s ease}
.notification-success{background:#2e7d32;color:#fff}
.notification-warning{background:#f57c00;color:#fff}
.notification-error{background:#c60a0a;color:#fff}
.notification-info{background:#1976d2;color:#fff}
@keyframes slideIn{from{transform:translateX(100%);opacity:0}to{transform:translateX(0);opacity:1}}
@media(max-width:1000px){.main-content{grid-template-columns:1fr}.quick-actions{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.quick-actions h3{grid-column:1/-1}}
</style>
</head>
<body>
<div class="container">
<header>
<h1>Choreography Designer</h1>
<div class="project-controls">
<input type="text" id="projectName" placeholder="Nom" value="nouvelle_choregraphie">
<button onclick="exportProject()">Export</button>
<button onclick="importProject()">Import</button>
<input type="file" id="importFile" accept=".json" style="display:none" onchange="handleFileImport(event)">
<a href="/choreography" class="btn-back">Retour</a>
</div>
</header>

<div class="live-mode">
<h3>Mode Live</h3>
<p>Envoyez la keyframe actuelle directement a l'horloge pour la tester en temps reel.</p>
<button class="live" id="liveBtn" onclick="toggleLiveMode()">Activer Live</button>
<button class="live" onclick="sendCurrentKeyframe()">Envoyer Keyframe</button>
<button onclick="sendAllToStop()">Tout a 6h</button>
</div>

<div class="timeline-section">
<h2>Keyframes</h2>
<div class="timeline-controls">
<button onclick="addKeyframe()">+ Ajouter</button>
<button onclick="duplicateKeyframe()">Dupliquer</button>
<button onclick="deleteKeyframe()" class="danger">Supprimer</button>
</div>
<div class="timeline" id="timeline"></div>
</div>

<div class="main-content">
<div class="matrix-section">
<h2>Keyframe <span id="currentKeyframeNum">1</span></h2>
<div class="motion-params">
<div class="param-group">
<label>Vitesse</label>
<input type="number" id="kfSpeed" value="400" min="200" max="5000" step="50" onchange="saveSpeed()">
<span class="param-unit">pas/s</span>
</div>
<div class="param-group">
<label>Accel</label>
<input type="number" id="kfAccel" value="150" min="100" max="2000" step="50" onchange="saveAccel()">
</div>
<div class="param-group">
<label>Delai</label>
<input type="number" id="kfDelay" value="0" min="0" max="10000" step="100" onchange="saveDelay()">
<span class="param-unit">ms</span>
</div>
<div class="param-group">
<label>Cascade</label>
<select id="kfCascadeMode" onchange="saveCascadeMode()">
<option value="none">Aucune</option>
<option value="column">Colonne</option>
<option value="row">Ligne</option>
<option value="diagonal">Diagonale</option>
</select>
</div>
</div>
<div class="matrix-header">
<div class="slave-labels">
<span></span>
<span>S1</span><span>S2</span><span>S3</span><span>S4</span><span>S5</span><span>S6</span><span>S7</span><span>S8</span>
</div>
</div>
<div class="clock-matrix" id="clockMatrix"></div>
</div>

<div class="quick-actions">
<h3>Actions rapides</h3>
<div class="action-group">
<h4>Presets</h4>
<button onclick="setAllAngles(0,0)">12h (0/0)</button>
<button onclick="setAllAngles(90,90)">3h (90/90)</button>
<button onclick="setAllAngles(180,180)">6h (180/180)</button>
<button onclick="setAllAngles(270,270)">9h (270/270)</button>
</div>
<div class="action-group">
<h4>Directions</h4>
<button onclick="setAllDirections('CW','CW')">Tout CW</button>
<button onclick="setAllDirections('CCW','CCW')">Tout CCW</button>
</div>
<div class="action-group">
<h4>Selection</h4>
<button onclick="selectAll()">Tout</button>
<button onclick="selectNone()">Aucun</button>
</div>
</div>
</div>

<div class="preview-section">
<h2>Apercu</h2>
<div class="preview-controls">
<button onclick="previewPrevious()">Prec</button>
<button onclick="playPreview()" id="playBtn">Play</button>
<button onclick="previewNext()">Suiv</button>
</div>
<canvas id="previewCanvas" width="600" height="220"></canvas>
</div>
</div>

<script>
let keyframes=[];
let currentKeyframeIndex=0;
let selectedClocks=new Set();
let isPlaying=false;
let liveMode=false;

document.addEventListener('DOMContentLoaded',()=>{addKeyframe();renderMatrix();renderTimeline();updatePreview()});

function showNotification(msg,type='info'){
const existing=document.querySelector('.notification');
if(existing)existing.remove();
const n=document.createElement('div');
n.className='notification notification-'+type;
n.textContent=msg;
document.body.appendChild(n);
setTimeout(()=>{n.remove()},2000);
}

function createEmptyKeyframe(){
const kf={id:Date.now(),speed:400,accel:150,delayMs:0,cascadeMode:'none',cascadeDelayMs:100,clocks:[]};
for(let s=0;s<8;s++){kf.clocks[s]=[];for(let c=0;c<3;c++){kf.clocks[s][c]={angleH:180,angleM:180,dirH:'CW',dirM:'CW'}}}
return kf;
}

function addKeyframe(){keyframes.push(createEmptyKeyframe());currentKeyframeIndex=keyframes.length-1;renderTimeline();renderMatrix();updatePreview()}
function duplicateKeyframe(){if(keyframes.length===0)return;const dup=JSON.parse(JSON.stringify(keyframes[currentKeyframeIndex]));dup.id=Date.now();keyframes.splice(currentKeyframeIndex+1,0,dup);currentKeyframeIndex++;renderTimeline();renderMatrix();updatePreview()}
function deleteKeyframe(){if(keyframes.length<=1){alert('Min 1 keyframe');return}keyframes.splice(currentKeyframeIndex,1);if(currentKeyframeIndex>=keyframes.length)currentKeyframeIndex=keyframes.length-1;renderTimeline();renderMatrix();updatePreview()}
function selectKeyframe(i){currentKeyframeIndex=i;renderTimeline();renderMatrix();updatePreview();if(liveMode)sendCurrentKeyframe()}

function renderTimeline(){
const tl=document.getElementById('timeline');tl.innerHTML='';
keyframes.forEach((kf,i)=>{
const t=document.createElement('div');
t.className='keyframe-thumb'+(i===currentKeyframeIndex?' active':'');
t.innerHTML='<div class="kf-drag-handle">⋮⋮</div><strong>KF '+(i+1)+'</strong>';
t.onclick=()=>selectKeyframe(i);
tl.appendChild(t);
});
}

function renderMatrix(){
const m=document.getElementById('clockMatrix');m.innerHTML='';
const kf=keyframes[currentKeyframeIndex];
document.getElementById('currentKeyframeNum').textContent=currentKeyframeIndex+1;
document.getElementById('kfSpeed').value=kf.speed||400;
document.getElementById('kfAccel').value=kf.accel||150;
document.getElementById('kfDelay').value=kf.delayMs||0;
document.getElementById('kfCascadeMode').value=kf.cascadeMode||'none';

for(let row=0;row<3;row++){
const rl=document.createElement('div');rl.className='row-label';rl.textContent='C'+row;m.appendChild(rl);
for(let s=0;s<8;s++){
const cl=kf.clocks[s][row];
const cell=document.createElement('div');
cell.className='clock-cell'+(selectedClocks.has(s+'-'+row)?' selected':'');
cell.innerHTML='<div class="clock-face"><div class="hand hand-h" style="transform:rotate('+cl.angleH+'deg)"></div><div class="hand hand-m" style="transform:rotate('+cl.angleM+'deg)"></div></div><div class="clock-inputs"><div class="input-group input-h"><label>H '+cl.angleH+'</label><input type="range" class="slider-h" min="0" max="359" value="'+cl.angleH+'" oninput="updateAngle('+s+','+row+',\'H\',this.value)"><select onchange="updateDir('+s+','+row+',\'H\',this.value)"><option value="CW"'+(cl.dirH==='CW'?' selected':'')+'>CW</option><option value="CCW"'+(cl.dirH==='CCW'?' selected':'')+'>CCW</option></select></div><div class="input-group input-m"><label>M '+cl.angleM+'</label><input type="range" class="slider-m" min="0" max="359" value="'+cl.angleM+'" oninput="updateAngle('+s+','+row+',\'M\',this.value)"><select onchange="updateDir('+s+','+row+',\'M\',this.value)"><option value="CW"'+(cl.dirM==='CW'?' selected':'')+'>CW</option><option value="CCW"'+(cl.dirM==='CCW'?' selected':'')+'>CCW</option></select></div></div>';
cell.onclick=(e)=>{if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT')return;toggleClockSelection(s,row,e.shiftKey)};
m.appendChild(cell);
}}
}

function updateAngle(s,c,h,v){
const a=parseInt(v)||0;
if(h==='H')keyframes[currentKeyframeIndex].clocks[s][c].angleH=a;
else keyframes[currentKeyframeIndex].clocks[s][c].angleM=a;
renderMatrix();updatePreview();
if(liveMode)sendCurrentKeyframe();
}

function updateDir(s,c,h,v){if(h==='H')keyframes[currentKeyframeIndex].clocks[s][c].dirH=v;else keyframes[currentKeyframeIndex].clocks[s][c].dirM=v}
function saveSpeed(){keyframes[currentKeyframeIndex].speed=Math.max(200,Math.min(5000,parseInt(document.getElementById('kfSpeed').value)||400))}
function saveAccel(){keyframes[currentKeyframeIndex].accel=Math.max(100,Math.min(2000,parseInt(document.getElementById('kfAccel').value)||150))}
function saveDelay(){keyframes[currentKeyframeIndex].delayMs=Math.max(0,parseInt(document.getElementById('kfDelay').value)||0)}
function saveCascadeMode(){keyframes[currentKeyframeIndex].cascadeMode=document.getElementById('kfCascadeMode').value}

function toggleClockSelection(s,c,add){const id=s+'-'+c;if(!add)selectedClocks.clear();if(selectedClocks.has(id))selectedClocks.delete(id);else selectedClocks.add(id);renderMatrix()}
function selectAll(){selectedClocks.clear();for(let s=0;s<8;s++)for(let c=0;c<3;c++)selectedClocks.add(s+'-'+c);renderMatrix()}
function selectNone(){selectedClocks.clear();renderMatrix()}

function setAllAngles(aH,aM){
const t=selectedClocks.size>0?selectedClocks:getAllClockIds();
t.forEach(id=>{const[s,c]=id.split('-').map(Number);keyframes[currentKeyframeIndex].clocks[s][c].angleH=aH;keyframes[currentKeyframeIndex].clocks[s][c].angleM=aM});
renderMatrix();updatePreview();if(liveMode)sendCurrentKeyframe();
}

function setAllDirections(dH,dM){
const t=selectedClocks.size>0?selectedClocks:getAllClockIds();
t.forEach(id=>{const[s,c]=id.split('-').map(Number);keyframes[currentKeyframeIndex].clocks[s][c].dirH=dH;keyframes[currentKeyframeIndex].clocks[s][c].dirM=dM});
renderMatrix();
}

function getAllClockIds(){const ids=new Set();for(let s=0;s<8;s++)for(let c=0;c<3;c++)ids.add(s+'-'+c);return ids}

function updatePreview(){
const cv=document.getElementById('previewCanvas');const ctx=cv.getContext('2d');const kf=keyframes[currentKeyframeIndex];
ctx.fillStyle='#1a1a1a';ctx.fillRect(0,0,cv.width,cv.height);
const sz=24,pd=8,sx=35,sy=25;
for(let s=0;s<8;s++){for(let c=0;c<3;c++){
const x=sx+s*(sz*2+pd),y=sy+c*(sz*2+pd),d=kf.clocks[s][c];
ctx.beginPath();ctx.arc(x+sz,y+sz,sz,0,Math.PI*2);ctx.fillStyle='#fff';ctx.fill();ctx.strokeStyle='#333';ctx.lineWidth=2;ctx.stroke();
const hR=(d.angleH-90)*Math.PI/180;ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(hR)*sz*.5,y+sz+Math.sin(hR)*sz*.5);ctx.strokeStyle='#c60a0a';ctx.lineWidth=3;ctx.stroke();
const mR=(d.angleM-90)*Math.PI/180;ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(mR)*sz*.7,y+sz+Math.sin(mR)*sz*.7);ctx.strokeStyle='#3498db';ctx.lineWidth=2;ctx.stroke();
ctx.beginPath();ctx.arc(x+sz,y+sz,2,0,Math.PI*2);ctx.fillStyle='#333';ctx.fill();
}}
ctx.fillStyle='#888';ctx.font='9px sans-serif';ctx.textAlign='center';
for(let s=0;s<8;s++)ctx.fillText('S'+(s+1),sx+s*(sz*2+pd)+sz,12);
ctx.textAlign='right';for(let c=0;c<3;c++)ctx.fillText('C'+c,20,sy+c*(sz*2+pd)+sz+3);
}

function playPreview(){
if(isPlaying){isPlaying=false;document.getElementById('playBtn').textContent='Play';return}
if(keyframes.length<2){alert('Min 2 keyframes');return}
isPlaying=true;document.getElementById('playBtn').textContent='Stop';
let from=0;
function anim(){
if(!isPlaying)return;
let to=from+1;if(to>=keyframes.length)to=0;
animateBetween(from,to,1000,()=>{from=to;if(isPlaying)setTimeout(anim,300)});
}
anim();
}

function animateBetween(fI,tI,dur,cb){
const fK=keyframes[fI],tK=keyframes[tI],st=performance.now();
function a(t){
if(!isPlaying)return;
const p=Math.min((t-st)/dur,1),e=p<.5?4*p*p*p:1-Math.pow(-2*p+2,3)/2;
drawFrame(fK,tK,e);
if(p<1)requestAnimationFrame(a);else if(cb)cb();
}
requestAnimationFrame(a);
}

function drawFrame(fK,tK,p){
const cv=document.getElementById('previewCanvas');const ctx=cv.getContext('2d');
ctx.fillStyle='#1a1a1a';ctx.fillRect(0,0,cv.width,cv.height);
const sz=24,pd=8,sx=35,sy=25;
for(let s=0;s<8;s++){for(let c=0;c<3;c++){
const x=sx+s*(sz*2+pd),y=sy+c*(sz*2+pd);
const f=fK.clocks[s][c],t=tK.clocks[s][c];
const aH=interpAngle(f.angleH,t.angleH,f.dirH,p);
const aM=interpAngle(f.angleM,t.angleM,f.dirM,p);
ctx.beginPath();ctx.arc(x+sz,y+sz,sz,0,Math.PI*2);ctx.fillStyle='#fff';ctx.fill();ctx.strokeStyle='#333';ctx.lineWidth=2;ctx.stroke();
const hR=(aH-90)*Math.PI/180;ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(hR)*sz*.5,y+sz+Math.sin(hR)*sz*.5);ctx.strokeStyle='#c60a0a';ctx.lineWidth=3;ctx.stroke();
const mR=(aM-90)*Math.PI/180;ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(mR)*sz*.7,y+sz+Math.sin(mR)*sz*.7);ctx.strokeStyle='#3498db';ctx.lineWidth=2;ctx.stroke();
ctx.beginPath();ctx.arc(x+sz,y+sz,2,0,Math.PI*2);ctx.fillStyle='#333';ctx.fill();
}}
ctx.fillStyle='#888';ctx.font='9px sans-serif';ctx.textAlign='center';
for(let s=0;s<8;s++)ctx.fillText('S'+(s+1),sx+s*(sz*2+pd)+sz,12);
ctx.textAlign='right';for(let c=0;c<3;c++)ctx.fillText('C'+c,20,sy+c*(sz*2+pd)+sz+3);
}

function interpAngle(f,t,dir,p){
let d;
if(dir==='CW'){d=t-f;if(d<=0)d+=360}
else{d=t-f;if(d>=0)d-=360}
return(f+d*p+360)%360;
}

function previewPrevious(){if(currentKeyframeIndex>0)selectKeyframe(currentKeyframeIndex-1)}
function previewNext(){if(currentKeyframeIndex<keyframes.length-1)selectKeyframe(currentKeyframeIndex+1)}

// Live mode - send to ESP32
function toggleLiveMode(){
liveMode=!liveMode;
const btn=document.getElementById('liveBtn');
btn.classList.toggle('active',liveMode);
btn.textContent=liveMode?'Live ON':'Activer Live';
showNotification(liveMode?'Mode Live active':'Mode Live desactive',liveMode?'success':'info');
}

async function sendCurrentKeyframe(){
const kf=keyframes[currentKeyframeIndex];
const data={keyframes:[{
speed:kf.speed,accel:kf.accel,delayMs:kf.delayMs,
cascadeMode:kf.cascadeMode,cascadeDelayMs:kf.cascadeDelayMs,
clocks:kf.clocks.map(slave=>slave.map(c=>({
angleH:c.angleH,angleM:c.angleM,dirH:c.dirH,dirM:c.dirM
})))
}],name:'_live_preview'};
try{
const res=await fetch('/api/choreo/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)});
if(res.ok){
await fetch('/api/choreo/load?name=_live_preview');
await fetch('/api/choreo/apply',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'keyframe=0'});
showNotification('Keyframe envoyee','success');
}else{showNotification('Erreur envoi','error')}
}catch(e){showNotification('Erreur: '+e,'error')}
}

async function sendAllToStop(){
try{
await fetch('/api/stop',{method:'POST'});
showNotification('Toutes les horloges a 6h','success');
}catch(e){showNotification('Erreur: '+e,'error')}
}

function exportProject(){
const name=document.getElementById('projectName').value||'choreographie';
const data={name:name,version:'1.1',created:new Date().toISOString(),keyframes:keyframes};
const blob=new Blob([JSON.stringify(data,null,2)],{type:'application/json'});
const url=URL.createObjectURL(blob);
const a=document.createElement('a');a.href=url;a.download=name+'.json';a.click();
URL.revokeObjectURL(url);
}

function importProject(){document.getElementById('importFile').click()}

function handleFileImport(e){
const f=e.target.files[0];if(!f)return;
const r=new FileReader();
r.onload=(ev)=>{
try{
const d=JSON.parse(ev.target.result);
if(d.keyframes&&Array.isArray(d.keyframes)){
keyframes=d.keyframes;currentKeyframeIndex=0;
document.getElementById('projectName').value=d.name||'imported';
keyframes.forEach(kf=>{
if(kf.speed===undefined)kf.speed=400;
if(kf.accel===undefined)kf.accel=150;
if(kf.delayMs===undefined)kf.delayMs=0;
if(kf.cascadeMode===undefined)kf.cascadeMode='none';
});
renderTimeline();renderMatrix();updatePreview();
showNotification('Projet importe','success');
}else throw new Error('Format invalide');
}catch(err){showNotification('Erreur: '+err.message,'error')}
};
r.readAsText(f);e.target.value='';
}
</script>
</body>
</html>
)rawliteral";

#endif
