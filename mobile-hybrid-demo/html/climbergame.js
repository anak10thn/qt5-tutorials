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

// a simple game with multiple layers of background

var climbergameEffect = {
    timer: false,
    dimensions: Array(),
    maxWidth: 360,
    maxHeight: 640,
    screenYpos: 0,
    heroX: 180,
    heroY: 20,
    heroVX: 0,
    heroVY: 0,
    platforms: Array(),
    clouds: Array(),
    click: function (x, y) {},
    swipe: function (dir) {},
    drag: function (amt, x, y) {},
    releaseDrag: function (amt) {},
    init: function () {
        console.log("climber init.");
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);
        gete("content").style.backgroundPosition = "0px 100px";
        climbergameEffect.resize();
        this.hero = document.createElement("div");
        this.hero.setAttribute("id", "hero");
        gete("content").appendChild(this.hero);
        this.cliff = document.createElement("div");
        this.cliff.setAttribute("id", "cliff");
        gete("content").appendChild(this.cliff);
        this.platforms = new Array();
        this.screenYpos = 0;
        this.heroVY = 0;
        this.heroVX = 0;
        // generate clouds
        for (var n = 0; n < 6; n++) {
            var temp = document.createElement("div");
            temp.xp = Math.round(Math.random() * 270);
            temp.style.left = temp.xp + "px";
            temp.yp = Math.round(Math.random() * 600);
            temp.ptype = Math.round(Math.random() * 2);
            if (temp.ptype == 1) {
                temp.setAttribute("class", "cloudlow");
            } else {
                temp.setAttribute("class", "cloudhigh");
            }
            this.clouds.push(temp);
            gete("content").appendChild(temp);
        }
        // generate platforms
        for (var n = 0; n < 30; n++) {
            var temp = document.createElement("div");
            temp.setAttribute("class", "platform");
            temp.xp = Math.round(Math.random() * 270);
            temp.style.left = temp.xp + "px";
            temp.yp = 600 - n * (145 + Math.random() * 10);
            temp.ptype = Math.round(Math.random() * 2);
            temp.style.backgroundImage = "url(images/climber_platform_" + temp.ptype + ".png)";
            this.platforms.push(temp);
            gete("content").appendChild(temp);
        }
        this.heroY = 220;
        this.heroX = 180;
    },
    start: function () {
        this.timer = setTimeout("climbergameEffect.renderframe()", 200);
    },
    deinit: function () {
        gete("content").style.background = "white";
        gete("content").style.backgroundPosition = "0px 0px";
        if (gete("content").hasChildNodes()) while (gete("content").childNodes.length >= 1)
        gete("content").removeChild(gete("content").firstChild);
        delete this.timer;
    },
    resize: function () {
        gete("content").style.background = "url(images/climber_bg.jpg)";
        gete("content").style.backgroundPosition = "0px -640px";
        climbergameEffect.dimensions = Array(0, 360, 0, 640);
    },
    images: function () {
        // images to preload
        return new Array("images/climber_bg.jpg", "images/climber_cliff_0.png", "images/climber_cloud_0.png", "images/climber_cloud_1.png", "images/climber_hero_l_0.png", "images/climber_hero_l_1.png", "images/climber_hero_l_2.png", "images/climber_hero_l_3.png", "images/climber_hero_l_4.png", "images/climber_hero_l_5.png", "images/climber_hero_r_0.png", "images/climber_hero_r_1.png", "images/climber_hero_r_2.png", "images/climber_hero_r_3.png", "images/climber_hero_r_4.png", "images/climber_hero_r_5.png", "images/climber_platform_0.png", "images/climber_platform_1.png", "images/climber_platform_2.png");
    },
    renderframe: function () {
        // scroll background if the hero is high enough on the screen    
        if (this.heroY - this.screenYpos < 100) this.screenYpos -= (100 - (this.heroY - this.screenYpos));
        // set the positions for the background and the cliff appearing on the side
        gete("cliff").style.backgroundPosition = "0px " + (((-this.screenYpos) % 1200) - 584) + "px";
        gete("content").style.backgroundPosition = "0px " + (-640 + ((-this.screenYpos) >> 3)) + "px";
        for (p in this.platforms) {
            // only need to check for platform collision if hero is falling
            if (this.heroVY > 0) if (this.platforms[p].yp > this.heroY + 75 && this.platforms[p].yp < this.heroY + 95) if (this.heroX > this.platforms[p].xp - 40 && this.heroX < this.platforms[p].xp + 85) this.heroVY = -20 - (this.platforms[p].ptype * 3);
            // decide whether to draw this, display = none makes things a lot faster for things that dont need to be drawn                  
            if (this.platforms[p].yp - this.screenYpos < 0 || this.platforms[p].yp - this.screenYpos > 640) {
                this.platforms[p].style.display = "none";
            } else {
                this.platforms[p].style.display = "inline";
                this.platforms[p].style.top = this.platforms[p].yp - this.screenYpos + "px";
            }

        }
        // move clouds
        for (p in this.clouds) {
            this.clouds[p].xp += this.clouds[p].ptype + 1;
            this.clouds[p].style.left = ((this.clouds[p].xp % 560) - 100) + "px";
            this.clouds[p].style.top = (((this.clouds[p].yp - (this.screenYpos >> (this.clouds[p].ptype))) % 740) - 50) + "px";
        }

        this.heroVY += 0.7;
        this.heroVX = ((this.heroVX * 3) + (xAcc / -6)) / 4;
        this.heroX += this.heroVX;
        this.heroY += this.heroVY;
        // figure out correct sprite to display for the hero
        this.hero.style.backgroundImage = "url(images/climber_hero_" + (this.heroVX < 0 ? "l" : "r") + "_" + (5 - Math.floor(Math.abs(this.heroVY) / 6)) + ".png)";
        // make sure the hero doesnt leave the screen
        if (this.heroX < 0) this.heroX = 0;
        if (this.heroX > 300) this.heroX = 300;
        gete("hero").style.top = this.heroY - this.screenYpos + "px";
        gete("hero").style.left = this.heroX + "px";
        // if hero falls off screen, reinitialize
        if (this.heroY - this.screenYpos > 600) this.init();
        if (this.timer) this.timer = setTimeout("climbergameEffect.renderframe()", 30);
    }
}
