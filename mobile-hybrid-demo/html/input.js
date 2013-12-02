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

// this file has all the input related functions

var dragging = false;
var dragStartX = 0;
var dragLastX = 0;
var dragStartTime = 0;

function mousedown(e) {
    dragging = true;
    dragLastX = dragStartX = e.clientX;
    var d = new Date();
    dragStartTime = d.getTime();
    e.stopPropagation();
    e.preventDefault();
}

function mousemove(e) {
    if (dragging) activeEffect.drag(dragLastX - e.clientX, e.clientX, e.clientY);
    dragLastX = e.clientX;
    e.stopPropagation();
    e.preventDefault();
}

function mouseup(e) {
    animating = false;
    output("up " + e.clientX + "," + e.clientY);
    dragging = false;
    var d = new Date();
    var t = d.getTime();
    e.stopPropagation();
    e.preventDefault();
    if (e.clientY < 70) {
        if (e.clientX > window.innerWidth - 70) // back button
             goBack();
    } else if (activeEffect != null) {
        activeEffect.releaseDrag();
        if (Math.abs(e.clientX - dragStartX) > 100 && (t - dragStartTime) < 400) { // detect swipes
            if (e.clientX < dragStartX) activeEffect.swipe(-1);
            else activeEffect.swipe(1);
        } else // not a swipe
        if ((t - dragStartTime) < 200) // detect click
               activeEffect.click(e.clientX, e.clientY);

    }
}
