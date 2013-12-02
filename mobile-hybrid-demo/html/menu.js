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

// the menu system

function menuItem(name, imagename, layercount) {
    this.destination = name;
    this.div = document.createElement("div");
    this.div.setAttribute("id", name);
    this.div.setAttribute("name", imagename);
    this.div.setAttribute("class", "menuItem");
    this.layers = new Array();
    for (n = 0; n < layercount; n++) {
        im = document.createElement("img");
        im.setAttribute("class", "menuLayer");
        im.src = ("images/menu_" + imagename + "_" + n + ".png");
        this.layers.push(im);
        this.div.appendChild(im);
    }

}

menuItem.prototype.moveLayers = function (pos, dif) {
    for (m = 0; m < this.layers.length; m++)
    this.layers[m].style.left = (pos + dif * m) + "px";
}

var menuEffect = {
    itemWidth: 210,
    drawOffset: 0,
    centerOffset: 0,
    menuWidth: 0,
    selected: 4,
    scrollTimer: false,
    updateTimer: false,
    inertia: 0,
    diffOffset: 0,
    menuY: 200,
    menuHeight: 200,
    items: Array(
    new menuItem("multiplecanvas", "stocks", 3), new menuItem("csstransform", "flikr", 3), new menuItem("canvas", "gravity", 3), new menuItem("movingdivs", "bubbles", 3), new menuItem("climbergame", "climber", 3)),
    click: function (x, y) {
        // clicked on the sides means scroll
        if (x > window.innerWidth - 70) {
            this.selected++;
            this.drawOffset += this.itemWidth;
            scrollTimer = setTimeout("menuEffect.scroll(" + this.drawOffset + ")", 5);
        }
        if (x < 70) {
            this.selected--;
            this.drawOffset -= this.itemWidth;
            scrollTimer = setTimeout("menuEffect.scroll(" + this.drawOffset + ")", 5);
        }
        // in the center, select the current effect and start it up
        if (x >= 70 && x <= window.innerWidth - 70 && y >= this.menuY && y <= this.menuY + this.menuHeight) {
            console.log("go to " + this.items[this.selected].destination);
            startEffect(this.items[this.selected].destination);
        }
        this.selected = (this.selected + this.items.length) % this.items.length;
    },
    swipe: function (dir) {
        console.log("swipe" + this.drawOffset);
    },
    drag: function (amt) {
        this.drawOffset -= amt;
        if (updateTimer == false) // only update if there is not an update queued up
        updateTimer = setTimeout("menuEffect.updateLayout();", 8);
    },
    scroll: function (amt) {
        scrollTimer = false;
        this.drawOffset -= Math.floor(amt / 4);
        if (updateTimer == false) // only update if there is not an update queued up
        updateTimer = setTimeout("menuEffect.updateLayout();", 8);
        if (Math.abs(this.drawOffset) > 4) {
            // we need to scroll more
            scrollTimer = setTimeout("menuEffect.scroll(" + this.drawOffset + ")", 5);
        } else {
            this.drawOffset = 0;
            scrollTimer = false;
        }

    },
    releaseDrag: function (amt) {
        if (this.drawOffset > 0) while (this.drawOffset > this.itemWidth / 2) {
            this.drawOffset -= this.itemWidth;
            this.selected--;
        }
        if (this.drawOffset < 0) while (this.drawOffset < -this.itemWidth / 2) {
            this.drawOffset += this.itemWidth;
            this.selected++;
        }
        this.selected = (this.selected + this.items.length) % this.items.length;
        if (this.drawOffset != 0) scrollTimer = setTimeout("menuEffect.scroll(" + this.drawOffset + ")", 5);
        if (updateTimer == false) updateTimer = setTimeout("menuEffect.updateLayout();", 5);
    },
    init: function () {
        console.log("menuEffect init.");
    },

    resize: function () {
        this.setLayout();
    },
    setBG: function () {
        gete("content").style.background = "url(images/menu_bg.jpg)";
        gete("mask").style.background = "url(images/menu_bg_mask.png)";

    },
    setLayout: function () {
        gete("mask").style.display = "inline";
        this.itemWidth = 280;
        this.setBG();
        this.menuWidth = this.itemWidth * this.items.length;
        this.drawOffset = 0;
        this.centerOffset = (window.innerWidth / 2) - (this.itemWidth / 2);
        this.diffOffset = this.centerOffset;
        ypos = (window.innerHeight / 2) - 80;
        this.menuY = ypos;
        for (n = 0; n < this.items.length; n++)
        for (m = 0; m < this.items[n].layers.length; m++)
        this.items[n].layers[m].style.top = ypos + "px";
        this.updateLayout();
    },

    updateLayout: function () {
        // this function actually updates the items on the menu
        updateTimer = false;
        var xpos = (this.itemWidth * (-this.selected) + this.drawOffset + this.centerOffset);
        for (n = 0; n < this.items.length; n++) {
            // place each item to where it should be
            if (xpos > window.innerWidth) xpos -= this.menuWidth;
            if (xpos < 0 - this.itemWidth) xpos += this.menuWidth;
            if (xpos < 0 - this.itemWidth || xpos > window.innerWidth) {
                // this menuitem is not visible, but we have to leave them on the screen to avoid 
                // webkit thinking they are not needed and uncache them
                for (m = 0; m < this.items[n].layers.length; m++)
                this.items[n].layers[m].style.left = (window.innerWidth - 1) + "px";
            } else {
                dif = (xpos - this.diffOffset) >> 2;
                this.items[n].moveLayers(xpos, dif);
            }
            xpos += this.itemWidth;
        }
    },
    start: function () {
        this.setLayout();
        for (n = 0; n < this.items.length; n++) {
            gete("content").appendChild(this.items[n].div);
        }
        output("started menu.");
    },
    hide: function () {
        output("hiding menu");
        this.deinit();
    },
    deinit: function () {
        //		gete("content").style.background = "white";
        gete("mask").style.display = "none";
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);

    },
    images: function () {
        var temp = new Array("images/menu_bg.jpg", "images/menu_bg_mask.png");
        for (n = 0; n < this.items.length; n++)
        for (m = 0; m < this.items[n].layers.length; m++)
        temp.push(this.items[n].layers[m].src);
        return temp;
    }
}
