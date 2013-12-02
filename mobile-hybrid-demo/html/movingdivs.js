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

// Effect demonstrating moving divs 

var movingdivsEffect = {
    bubbles: Array(),
    dimensions: Array(),
    timer: false,
    spawntime: 0,
    click: function (x, y) {},
    swipe: function (dir) {},
    drag: function (amt, x, y) {
        // limit spawning new bubbles if there are too many on screen already
        if ((new Date()).getTime() - this.spawntime > this.bubbles.length) {
            temp = document.createElement("div");
            temp.xp = x;
            temp.yp = y;
            temp.zp = Math.floor(Math.random() * 3) + 3;
            // generate initial acceleration according to the current accelerometer data
            temp.xa = temp.zp / 4 * xAcc / 6;
            temp.ya = temp.zp / 4 * yAcc / -6;
            temp.setAttribute("class", "bubble");
            temp.style.top = temp.yp + "px";
            temp.style.left = temp.xp + "px";
            temp.style.backgroundImage = "url(images/bubble_" + Math.floor(Math.random() * 3) + ".png)";
            gete("bubbles").appendChild(temp);
            this.bubbles.push(temp);
            this.spawntime = (new Date()).getTime();
        }
    },
    releaseDrag: function (amt) {},
    init: function () {
        console.log("bubbles init.");
        bubs = document.createElement("div");
        bubs.setAttribute("id", "bubbles");
        gete("content").appendChild(bubs);
        movingdivsEffect.resize();
    },
    start: function () {
        this.timer = setTimeout("movingdivsEffect.renderframe()", 200);
    },
    deinit: function () {
        // remove all content
        gete("content").style.background = "white";
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);
        // stop timer
        delete this.timer;
    },
    resize: function () {
        output("window:" + window.innerWidth + "," + window.innerHeight);
        gete("content").style.background = "url(images/bubbles_bg.jpg)";
        movingdivsEffect.dimensions = Array(0, 360, 0, 640);
    },
    images: function () {
        // returns images to preload
        return new Array("images/bubbles_bg.jpg", "images/bubble_0.png", "images/bubble_1.png", "images/bubble_2.png");
    },
    renderframe: function () {
        // get current accelerometer data and scale it
        xaa = xAcc / 15;
        yaa = yAcc / -15;
        for (var b in this.bubbles) {
            this.bubbles[b].xp += this.bubbles[b].xa + xaa;
            this.bubbles[b].yp += this.bubbles[b].ya + yaa;
            // check to see if this bubble needs to be removed.
            if (this.bubbles[b].xp < movingdivsEffect.dimensions[0] || this.bubbles[b].xp > movingdivsEffect.dimensions[1] || this.bubbles[b].yp < movingdivsEffect.dimensions[2] || this.bubbles[b].yp > movingdivsEffect.dimensions[3]) {
                gete("bubbles").removeChild(this.bubbles[b]);
                this.bubbles.splice(b, 1);
            } else {
                this.bubbles[b].style.left = Math.round(this.bubbles[b].xp) + "px";
                this.bubbles[b].style.top = Math.round(this.bubbles[b].yp) + "px";
            }
        }
        // trigger repaint as soon as possible
        if (this.timer) this.timer = setTimeout("movingdivsEffect.renderframe()", 1);
    }
}
