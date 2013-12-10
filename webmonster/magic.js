
var context = {
    width: 800,
    height: 600,
    globalAlpha: 1.0,
    strokeStyle: "rgba(0, 0, 0, 1)",
    fillStyle: "rgba(0, 0, 0, 1)",
    pointCount: 0,
    pointArray: [0, 0, 0, 0, 0, 0, 0, 0],
    polygonCount: 0,
    polygonBuffer: [],

    beginPath: function () {
        this.pointCount = 0;
    },

    moveTo: function (x, y) {
        this.pointArray[this.pointCount++] = x;
        this.pointArray[this.pointCount++] = y;
    },

    lineTo: function (x, y) {
        this.pointArray[this.pointCount++] = x;
        this.pointArray[this.pointCount++] = y;
    },

    closePath: function () {
        // ignore
    },

    stroke: function () {
        this.polygonBuffer[this.polygonCount++] = [
            this.globalAlpha, this.strokeStyle, "none",
            this.pointCount, this.pointArray.slice()
        ];
    },

    fill: function () {
        this.polygonBuffer[this.polygonCount++] = [
            this.globalAlpha, "none", this.fillStyle,
            this.pointCount, this.pointArray.slice()
        ];
    },

    fillRect: function (x, y, w, h) {
        this.polygonBuffer[this.polygonCount++] = [
            1.0, "none", this.fillStyle,
            this.pointCount, [x, y, x + w, y, x + w, y + h, x, y + h]
        ];
    },

    state: [],

    save: function () {
        this.state.push(this.globalAlpha);
        this.state.push(this.strokeStyle);
        this.state.push(this.fillStyle);
        this.globalAlpha = 1.0;
        this.strokeStyle = "rgba(0, 0, 0, 1)";
        this.fillStyle = "rgba(0, 0, 0, 1)";
    },

    restore: function () {
        this.fillStyle = this.state.pop();
        this.strokeStyle = this.state.pop();
        this.globalAlpha = this.state.pop();
    }

};

var canvas = {
    width: 800,
    height: 600,

    getContext: function (id) {
        if (id === "2d") {
            return context;
        }
    },

    addEventListener: function (t, f, o) {
        // ignore
    }
};

document.getElementById = function (id) {
    if (id === "canvas") {
        return canvas;
    }
};

document.addEventListener = function(t, f, o) {
        // ignore
};

window.addEventListener = function (t, f, o) {
    if (t === "load") {
        this.onLoad = f;
    }
};

window.onTimer = function() {
    this.scriptTimer();
    window.webmonster.drawPolygons(context.polygonCount, context.polygonBuffer);
    context.polygonCount = 0;
};

window.originalSetInterval = window.setInterval;

this.setInterval = function(f, i) {
    window.scriptTimer = f;
    this.originalSetInterval(window.onTimer, i);
};

