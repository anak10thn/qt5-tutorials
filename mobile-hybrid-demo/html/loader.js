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

// loader class to dynamically load the effects

var loader = {
    loadingBarWidth: 0,
    fadeTimer: false,
    setLoadingScreen: function () {
        // sets the basic loading screen up
        loader.loadingBarWidth = window.innerWidth - (window.innerWidth / 10);
        gete("toLoadBar").style.width = loader.loadingBarWidth + "px";
        gete("toLoadBar").style.height = "10px";
        gete("toLoadBar").style.left = (window.innerWidth / 20) + "px";
        gete("toLoadBar").style.top = (window.innerHeight / 2) + "px";

        gete("loadedBar").style.top = (window.innerHeight / 2) + "px";
        gete("loadedBar").style.left = (window.innerWidth / 20) + "px";
        gete("loadedBar").style.width = "0px";
        gete("loadedBar").style.height = "10px";

        gete("splashlogo").style.top = (window.innerHeight / 2) - 20 - (gete("splashlogo").height) + "px";
        gete("splashlogo").style.left = (window.innerWidth / 2) - (gete("splashlogo").width / 2) + "px";
    },
    setLoadingProgress: function (val) {
        gete("loadedBar").style.width = Math.floor(val * loader.loadingBarWidth) + "px";
    },
    fadeIn: function () {
        // fade in loading screen
        // this has to be done in two steps, otherwise the transition would never fire as visibility = hidden
        gete("loadingScreen").style.visibility = "visible";
        loader.setLoadingScreen();
        fadeTimer = setTimeout("loader.realFadeIn()", 1);
    },
    realFadeIn: function () {
        gete("loadingScreen").style.opacity = 1;
        fadeTimer = setTimeout("loader.fadedIn()", 500);
    },
    fadedIn: function () {},
    fadeOut: function () {
        gete("loadingScreen").style.opacity = 0;
        fadeTimer = setTimeout("loader.fadedOut()", 1000);
    },
    fadedOut: function () {
        gete("loadingScreen").style.visibility = "hidden";
    },
    loadJS: function (title, hasimages) {
        // load an effect
        output("loading " + title);
        var tempjs = document.createElement("script");
        tempjs.setAttribute("id", "js_" + title)
        tempjs.setAttribute("type", "text/javascript")
        // this parameter is to make sure this is not cached, for dev purposes
        var d = new Date();
        var t = d.getTime();
        tempjs.setAttribute("src", title + ".js?t=" + t);
        if (hasimages)
        // if images need to be preloaded, do it here.
        tempjs.onload = function () {
            output("js loaded.");
            eval("var imagesToLoad = " + title + "Effect.images();");
            loader.loadImages(imagesToLoad);
            loader.JSLoaded();
        }
        gete("head").appendChild(tempjs);
    },

    JSLoaded: function () {},

    loadCSS: function (title) {
        // load css
        var d = new Date();
        var t = d.getTime();
        var tempcss = document.createElement("link");
        tempcss.setAttribute("id", "css_" + title)
        tempcss.setAttribute("rel", "stylesheet")
        tempcss.setAttribute("type", "text/css")
        tempcss.setAttribute("href", title + ".css?t=" + t);
        gete("head").appendChild(tempcss);
    },
    loadTimer: false,
    imageCache: null,
    loadImages: function (imglist) {
        output("loading images");
        loader.imageCache = new Array();
        for (n = 0; n < imglist.length; n++) {
            var i = new Image();
            i.src = imglist[n];
            loader.imageCache.push(i);
        }
        loadTimer = setTimeout("loader.progressLoad()", 50);
        loader.progressCount = 0;
    },
    progressCount: 0,
    progressLoad: function () {
        // load next item on the preload list
        if (loader.progressCount == 100) loader.loadComplete();
        if (loader.imageCache.length == 0) loader.loadComplete();
        var loaded = 0;
        for (n = 0; n < loader.imageCache.length; n++)
        if (loader.imageCache[n].complete) loaded++;
        loader.setLoadingProgress(loaded / loader.imageCache.length);
        if (loaded == loader.imageCache.length) loader.loadComplete();
        else loadTimer = setTimeout("loader.progressLoad()", 50);
    },
    loadComplete: function () {
        output("load complete!");
        activeEffect.start();
        loader.fadeOut();

    },
    unload: function (title) {
        console.log("unloading " + title);
        try {
            gete("head").removeChild(gete("css_" + title));
            gete("head").removeChild(gete("js_" + title));
        } catch (e) {
            console.log("unload " + title + ":" + e.message);
        }
    }
}
