/* This file is part of mobile_hybrid_demo (HTML5 demonstration)

Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
All rights reserved.

Contact:  Nokia Corporation qt-info@nokia.com

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: *
Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer. * Redistributions in binary
form must reproduce the above copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other materials provided with
the distribution. * Neither the name of Nokia Corporation and its
Subsidiary(-ies) nor the names of its contributors may be used to endorse or
promote products derived from this software without specific prior written
permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
*/

// this file contains global functions and variables

// helper function to cut down on typing

function gete(id) {
    return document.getElementById(id);
}

// the effect we are currently in
var activeEffect = null;
// the name of the effect we are loading
var targetEffect = "";
var log = new Array();
var animating = false;

// Accelerometer variables
var xAcc = 0;
var yAcc = 0;
var zAcc = 0;

function accelerometer(xx, yy, zz) {
    // the qt app calls this to pass us the details
    xAcc = xx;
    yAcc = yy;
    zAcc = zz;
}

function output(s) {
    // log function, uncomment return to enable it.
    return;
    log.unshift((new Date()).getTime() + ":" + s);
    if (log.length > 3) log.pop();
    gete("output").innerHTML = log.join("<br>");
    console.log(s);
}


function resize() {
    output("resize: " + window.innerWidth + "x" + window.innerHeight);
    if (activeEffect) activeEffect.resize();
    gete("close").style.left = (window.innerWidth - 60) + "px";
    loader.setLoadingScreen();
}

function initialize() {
    // first function that gets called
    output("initializing..");
    loader.setLoadingScreen();
    loader.unload("loader");
    window.onresize = resize;
    loader.JSLoaded = start;
    loader.loadJS("menu", true);
    loader.loadCSS("menu");
    resize();
}

function start() {
    gete("loadingScreen").style.opacity = 0;
    output("starting..");
    document.addEventListener('mousedown', mousedown, true);
    document.addEventListener('mousemove', mousemove, true);
    document.addEventListener('mouseup', mouseup, true);
    menuEffect.init();
    menuEffect.start();
    activeEffect = menuEffect;
    gete("close").style.opacity = 1;
}

function startEffect(name) {
    output("starting " + name);
    targetEffect = name;
    loader.fadedIn = loadEffect;
    loader.fadeIn();
}

function loadEffect() {
    output("loaded.");
    menuEffect.deinit();
    loader.JSLoaded = effectLoaded;
    loader.loadJS(targetEffect, true);
    loader.loadCSS(targetEffect);
}

function effectLoaded() {
    eval("activeEffect = " + targetEffect + "Effect;");
    activeEffect.init();
}

function goBack() {
    output("GO BACK");
    imageCache = new Array();
    loader.loadTimer = false;

    if (targetEffect == "") {
        output("CLOSING");
        // this is bound to QT object 
        testObject.close();
        window.location = "index.html";
    } else {
        loader.fadedIn = deinitEffect;
        loader.fadeIn();
    }
    //setTimeout("deiniteffect()",500);
}

function deinitEffect() {
    output("DEINIT");
    gete("mask").display = "inline";
    menuEffect.setBG();
    loader.setLoadingProgress(1.0);
    activeEffect.deinit();
    eval("delete " + targetEffect + "Effect;"); //" = false;");
    loader.unload(targetEffect);
    menuEffect.start();
    activeEffect = menuEffect;
    targetEffect = "";
    setTimeout("loader.fadeOut()", 500);
}
