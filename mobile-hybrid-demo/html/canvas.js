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

// this effect demonstrates a fullscreen canvas and the drawing speed of it
// it simulates a gravity system where the center of gravity is moved by touching the screen

var canvasEffect = {
    timer: null,
    context: null,
    cogravitypos: new Array(180, 320, 500),
    gravityballpos: new Array(),
    gravityballvelocity: new Array(),
    gravityballcolor: new Array(),
    tick: 0,
    click: function (x, y) {},
    swipe: function (dir) {},
    drag: function (amt, x, y) {
        // move the center of gravity
        canvasEffect.cogravitypos[0] = x;
        canvasEffect.cogravitypos[1] = y;
    },
    releaseDrag: function (amt) {},
    randomcolor: function () {
        // helper function to generate a random color fitting the color scheme.
        var c = (127 + Math.round(Math.random() * 127)) + (Math.round(Math.random() * 100) << 8) + (Math.round(Math.random() * 25) << 16);
        return c.toString(16);
    },
    init: function () {
        console.log("canvasEffect init.");
        gete("content").style.background = "black";
        canvasEffect.canvas = document.createElement("canvas");
        canvasEffect.canvas.setAttribute("id", "canvas");
        gete("content").appendChild(canvasEffect.canvas);
        gete("canvas").style.width = "360px";
        gete("canvas").style.height = "640px";
        canvasEffect.canvas.width = 360;
        canvasEffect.canvas.height = 640;
        if (canvasEffect.canvas && canvasEffect.canvas.getContext) {
            canvasEffect.context = canvasEffect.canvas.getContext("2d");
        }
        for (n = 0; n < 5; n++) {
            // generate initial gravity balls
            // gravityballpos also includes mass as the third element
            canvasEffect.gravityballpos.push(new Array(Math.random() * 360, Math.random() * 640, Math.random() * 20 + 1));
            canvasEffect.gravityballvelocity.push(new Array(Math.random() * 10 - 5, Math.random() * 10 - 5));
            canvasEffect.gravityballcolor.push("#" + canvasEffect.randomcolor());
        }
    },
    start: function () {
        output("start gravity.");
        canvasEffect.context.lineWidth = 1.0;
        canvasEffect.context.strokeStyle = "#103040";
        canvasEffect.context.globalCompositeOperation = "lighter";
        canvasEffect.timer = setTimeout("canvasEffect.draw()", 1);
    },
    deinit: function () {
        delete canvasEffect.context;
        gete("content").style.background = "white";
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);

    },
    resize: function () {},
    images: function () {
        // no images to preload in this effect
        return new Array();
    },
    gravity: function (source, dest) {
        // calculates the gravity between two objects
        out = new Array();
        dx = (dest[0] - source[0]);
        dy = (dest[1] - source[1]);
        d = Math.sqrt(dx * dx + dy * dy);
        dx = dx / d;
        dy = dy / d;
        var f = Math.min(6, (source[2] * dest[2]) / (d * d));
        out.push(dx * f);
        out.push(dy * f);
        return out;
    },
    draw: function () {
        canvasEffect.tick++;
        if (canvasEffect.tick % 10 == 0) {
            // darken the screen every 10 ticks to fade out the old image
            canvasEffect.context.globalCompositeOperation = "source-over";
            canvasEffect.context.fillStyle = "#000000";
            canvasEffect.context.globalAlpha = 0.025;
            canvasEffect.context.fillRect(0, 0, 360, 640);
            canvasEffect.context.globalCompositeOperation = "lighter";
            canvasEffect.context.globalAlpha = 1.0;
        }
        for (b = 0; b < 5; b++) {
            canvasEffect.context.strokeStyle = canvasEffect.gravityballcolor[b];
            // draw lines from the old position to the new position and to the center of gravity
            canvasEffect.context.beginPath();
            canvasEffect.context.moveTo(canvasEffect.gravityballpos[b][0], canvasEffect.gravityballpos[b][1]);
            // move the gravity ball to the new position
            canvasEffect.gravityballpos[b][0] += (canvasEffect.gravityballvelocity[b][0]);
            canvasEffect.gravityballpos[b][1] += (canvasEffect.gravityballvelocity[b][1]);
            canvasEffect.context.lineTo(canvasEffect.gravityballpos[b][0], canvasEffect.gravityballpos[b][1]);
            canvasEffect.context.lineTo(canvasEffect.cogravitypos[0], canvasEffect.cogravitypos[1]);
            canvasEffect.context.stroke();
            // calculate new velocity for each gravity ball, starting with the center of gravity
            var vv = canvasEffect.gravity(canvasEffect.gravityballpos[b], canvasEffect.cogravitypos);
            canvasEffect.gravityballvelocity[b][0] += vv[0];
            canvasEffect.gravityballvelocity[b][1] += vv[1];
            // and then to all the other gravity balls
            for (c = 0; c < 5; c++) {
                if (c != b) {
                    vv = canvasEffect.gravity(canvasEffect.gravityballpos[b], canvasEffect.gravityballpos[c]);
                    canvasEffect.gravityballvelocity[b][0] += vv[0];
                    canvasEffect.gravityballvelocity[b][1] += vv[1];
                }
            }
            // cap velocity to 100 
            canvasEffect.gravityballvelocity[b][0] = Math.max(-100, Math.min(100, canvasEffect.gravityballvelocity[b][0]));
            canvasEffect.gravityballvelocity[b][1] = Math.max(-100, Math.min(100, canvasEffect.gravityballvelocity[b][1]));
            // if the ball is too far off the screen, generate a new ball in its place
            if (canvasEffect.gravityballpos[b][0] < -300 || canvasEffect.gravityballpos[b][0] > 660 || canvasEffect.gravityballpos[b][1] < -300 || canvasEffect.gravityballpos[b][1] > 940) {
                canvasEffect.gravityballpos[b] = new Array(Math.random() * 360, Math.random() * 640, Math.random() * 40 + 1);
                canvasEffect.gravityballvelocity[b] = new Array(Math.random() * 10 - 5, Math.random() * 10 - 5);

            } else
            // if the gravity ball is too close to the center of gravity, also generate a new ball. 
            if (Math.abs(canvasEffect.gravityballpos[b][0] - canvasEffect.cogravitypos[0]) < 1 && Math.abs(canvasEffect.gravityballpos[b][1] - canvasEffect.cogravitypos[1]) < 1) {
                canvasEffect.gravityballpos[b] = new Array(Math.random() * 360, Math.random() * 640, Math.random() * 40 + 1);
                canvasEffect.gravityballvelocity[b] = new Array(Math.random() * 10 - 5, Math.random() * 10 - 5);
            }
        }
        canvasEffect.timer = setTimeout("canvasEffect.draw()", 50);
    }


}
