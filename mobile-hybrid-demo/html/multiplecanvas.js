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

// this demonstrates multiple canvases and simple drawing on them

var multiplecanvasEffect = {
    maxtick: 0,
    gridSize: 18,
    click: function (x, y) {
        // redraw on click
        multiplecanvasEffect.drawgraph("USD");
    },
    swipe: function (dir) {},
    drag: function (amt) {},
    releaseDrag: function (amt) {},
    updateLayout: function (redraw) {
        multiplecanvasEffect.maxtick = 61;
        multiplecanvasEffect.gridSize = 18;
        multiplecanvasEffect.fillimage.src = "images/stocks_fill_color.png";
        gete("content").style.background = "url(images/stocks_background.png)";
        gete("title").style.top = "70px";
        gete("title").style.left = "10px";
        gete("biggraph").style.width = "305px";
        gete("biggraph").style.height = "235px";
        gete("biggraph").style.top = "101px";
        gete("biggraph").style.left = "2px";
        gete("smallgraph").style.width = "305px";
        gete("smallgraph").style.height = "125px";
        gete("smallgraph").style.top = "427px";
        gete("smallgraph").style.left = "2px";
        gete("volumegraph").style.width = "305px";
        gete("volumegraph").style.height = "55px";
        gete("volumegraph").style.top = "367px";
        gete("volumegraph").style.left = "2px";
        gete("volumegraph2").style.width = "305px";
        gete("volumegraph2").style.height = "55px";
        gete("volumegraph2").style.top = "583px";
        gete("volumegraph2").style.left = "2px";
        multiplecanvasEffect.biggraph.width = 304;
        multiplecanvasEffect.biggraph.height = 235;
        multiplecanvasEffect.smallgraph.width = 304;
        multiplecanvasEffect.smallgraph.height = 125;
        multiplecanvasEffect.volumegraph.width = 304;
        multiplecanvasEffect.volumegraph.height = 55;
        multiplecanvasEffect.volumegraph2.width = 304;
        multiplecanvasEffect.volumegraph2.height = 55;
        multiplecanvasEffect.heights[0] = 235;
        multiplecanvasEffect.heights[1] = 126;
        multiplecanvasEffect.heights[2] = 54;
        multiplecanvasEffect.heights[3] = 54;
        // get canvas contexts 		
        if (multiplecanvasEffect.biggraph && multiplecanvasEffect.biggraph.getContext) {
            multiplecanvasEffect.context[0] = multiplecanvasEffect.biggraph.getContext("2d");
        }
        if (multiplecanvasEffect.smallgraph && multiplecanvasEffect.smallgraph.getContext) {
            multiplecanvasEffect.context[1] = multiplecanvasEffect.smallgraph.getContext("2d");
        }
        if (multiplecanvasEffect.volumegraph && multiplecanvasEffect.volumegraph.getContext) {
            multiplecanvasEffect.context[2] = multiplecanvasEffect.volumegraph.getContext("2d");
        }
        if (multiplecanvasEffect.volumegraph2 && multiplecanvasEffect.volumegraph2.getContext) {
            multiplecanvasEffect.context[3] = multiplecanvasEffect.volumegraph2.getContext("2d");
        }
        if (redraw) multiplecanvasEffect.timer = setTimeout("multiplecanvasEffect.drawgraph('USD')", 1);

    },
    init: function () {
        console.log("multiplecanvasEffect init.");
        multiplecanvasEffect.fillimage = new Image();
        multiplecanvasEffect.fillimage.src = "images/stocks_fill_color.png";
        title = document.createElement("div");
        title.setAttribute("id", "title");
        gete("content").appendChild(title);
        multiplecanvasEffect.biggraph = document.createElement("canvas");
        multiplecanvasEffect.biggraph.setAttribute("id", "biggraph");
        multiplecanvasEffect.smallgraph = document.createElement("canvas");
        multiplecanvasEffect.smallgraph.setAttribute("id", "smallgraph");
        multiplecanvasEffect.volumegraph = document.createElement("canvas");
        multiplecanvasEffect.volumegraph.setAttribute("id", "volumegraph");
        multiplecanvasEffect.volumegraph2 = document.createElement("canvas");
        multiplecanvasEffect.volumegraph2.setAttribute("id", "volumegraph2");
        gete("content").appendChild(multiplecanvasEffect.biggraph);
        gete("content").appendChild(multiplecanvasEffect.smallgraph);
        gete("content").appendChild(multiplecanvasEffect.volumegraph);
        gete("content").appendChild(multiplecanvasEffect.volumegraph2);
        multiplecanvasEffect.updateLayout(false);


    },
    start: function () {
        multiplecanvasEffect.timer = setTimeout("multiplecanvasEffect.drawgraph('USD')", 1);
    },
    deinit: function () {
        for (n = 0; n < 4; n++)
        delete multiplecanvasEffect.context[n];
        gete("content").style.background = "white";
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);

    },
    resize: function () {
        if (multiplecanvasEffect.timer) multiplecanvasEffect.timer = false;
        multiplecanvasEffect.updateLayout(true);
        //multiplecanvasEffect.drawGraph();
    },
    images: function () {
        return new Array("images/stocks_fill_color.png", "images/stocks_fill_color_big.png", "images/stocks_background.png", "images/stocks_buttons.png");
    },
    data: function (x) {
        return 60 + x / 4 + (Math.sin(x / 4)) + (Math.sin(x / 7)) * 2 + (Math.sin(x / 6));
    },
    drawphase: -1,
    timer: null,
    gdata: null,
    filled: null,
    gdatapertick: null,
    context: Array(),
    fillimage: null,
    heights: Array(),
    draw: function () {
        // main drawing happens here in different phases
        // phase 0 draws ticklines
        // phase 1 draws the data
        // phase 2 is the end phase
        if (multiplecanvasEffect.timer == false) {
            multiplecanvasEffect.drawphase = -1;
            multiplecanvasEffect.drawgraph("USD");
            return;
        }
        multiplecanvasEffect.timer = false;
        if (multiplecanvasEffect.drawphase == 2) {
            multiplecanvasEffect.drawphase = -1;
            return;
        }
        if (multiplecanvasEffect.drawphase == 1) {
            // draw the charts
            var val;
            for (n = 0; n < 2; n++) {
                multiplecanvasEffect.context[n].lineWidth = 1.0;
                multiplecanvasEffect.context[n].strokeStyle = "#30FFFF";
                multiplecanvasEffect.context[n].beginPath();
                multiplecanvasEffect.context[n].moveTo(5 * multiplecanvasEffect.tick, multiplecanvasEffect.heights[n] - (multiplecanvasEffect.gdata[5 * multiplecanvasEffect.tick] / (n + 1)));
                // charts get drawn in 5 pixel steps
                for (m = 0; m < 6; m++) {
                    // get the data
                    val = multiplecanvasEffect.gdata[5 * multiplecanvasEffect.tick + m] / (n + 1);
                    if (m < 5) {
                        multiplecanvasEffect.context[n].globalAlpha = 0.4;
                        // check that val is a number, otherwise this crashes on the simulator
                        if (!isNaN(val)) multiplecanvasEffect.context[n].drawImage(
                        multiplecanvasEffect.fillimage, 5 * multiplecanvasEffect.tick + m, 0, 1, 1, 5 * multiplecanvasEffect.tick + m, multiplecanvasEffect.heights[n] - val, 1, val);
                    }
                    multiplecanvasEffect.context[n].globalAlpha = 1.0;
                    multiplecanvasEffect.context[n].lineTo(5 * multiplecanvasEffect.tick + m, multiplecanvasEffect.heights[n] - (multiplecanvasEffect.gdata[5 * multiplecanvasEffect.tick + m] / (n + 1)));
                }
                multiplecanvasEffect.context[n].stroke();
            }
            // draw the volume graphs
            multiplecanvasEffect.context[2].fillStyle = "#6090A0";
            multiplecanvasEffect.context[3].fillStyle = "#6090A0";
            for (n = 0; n < 5; n++) {
                val = 10 + Math.floor((Math.random() * 20));
                multiplecanvasEffect.context[2].fillRect(5 * multiplecanvasEffect.tick + n, multiplecanvasEffect.volumegraph.height - val, 1, val);
                multiplecanvasEffect.context[3].fillRect(5 * multiplecanvasEffect.tick + n, multiplecanvasEffect.volumegraph.height - val, 1, val);
            }
            multiplecanvasEffect.tick++;
            // check if we have drawn enough
            if (5 * multiplecanvasEffect.tick >= multiplecanvasEffect.biggraph.width) {
                multiplecanvasEffect.drawphase = 2;
                multiplecanvasEffect.tick = 0;
            }

        }
        // draw tick lines
        if (multiplecanvasEffect.drawphase == 0) {
            for (n = 0; n < 4; n++) {
                multiplecanvasEffect.context[n].strokeStyle = "#9090D0";
                multiplecanvasEffect.context[n].lineWidth = 0.3;
                if (multiplecanvasEffect.tick < 16) {
                    multiplecanvasEffect.context[n].beginPath();
                    multiplecanvasEffect.context[n].moveTo(multiplecanvasEffect.gridSize + multiplecanvasEffect.tick * multiplecanvasEffect.gridSize, 1);
                    multiplecanvasEffect.context[n].lineTo(multiplecanvasEffect.gridSize + multiplecanvasEffect.tick * multiplecanvasEffect.gridSize, multiplecanvasEffect.heights[n]);
                    multiplecanvasEffect.context[n].stroke();
                } else {
                    multiplecanvasEffect.context[n].beginPath();
                    multiplecanvasEffect.context[n].moveTo(1, multiplecanvasEffect.gridSize + (multiplecanvasEffect.tick - 16) * multiplecanvasEffect.gridSize);
                    multiplecanvasEffect.context[n].lineTo(multiplecanvasEffect.biggraph.width, multiplecanvasEffect.gridSize + (multiplecanvasEffect.tick - 16) * multiplecanvasEffect.gridSize);
                    multiplecanvasEffect.context[n].stroke();
                }
            }
            multiplecanvasEffect.tick++;
            if (multiplecanvasEffect.tick == 33) {
                multiplecanvasEffect.drawphase = 1;
                multiplecanvasEffect.tick = 0;
            }
        }
        multiplecanvasEffect.timer = setTimeout("multiplecanvasEffect.draw()", 30);
    },

    drawgraph: function (cur) {
        output("draw graph." + multiplecanvasEffect.drawphase);
        if (multiplecanvasEffect.drawphase != -1) return;
        gete("title").innerHTML = cur + ":EUR 30-day +0.24";
        multiplecanvasEffect.gdata = new Array();
        multiplecanvasEffect.gdatapertick = new Array();
        multiplecanvasEffect.filled = new Array();
        // generate data for the graphs
        for (n = 0; n < multiplecanvasEffect.biggraph.width; n++) {
            multiplecanvasEffect.gdata[n] = multiplecanvasEffect.data(n);
            multiplecanvasEffect.gdatapertick[n] = multiplecanvasEffect.data(n) / 60;
            multiplecanvasEffect.filled[n] = 0;
        }
        multiplecanvasEffect.drawphase = 0;
        multiplecanvasEffect.tick = 0;
        // clear the canvases before drawing
        multiplecanvasEffect.context[2].clearRect(0, 0, 305, multiplecanvasEffect.heights[2]);
        multiplecanvasEffect.context[3].clearRect(0, 0, 305, multiplecanvasEffect.heights[2]);
        multiplecanvasEffect.context[1].clearRect(0, 0, 305, multiplecanvasEffect.heights[1]);
        multiplecanvasEffect.context[0].clearRect(0, 0, 305, multiplecanvasEffect.heights[0]);
        multiplecanvasEffect.timer = setTimeout("multiplecanvasEffect.draw()", 1);
    }


}
