"use strict";

goog.provide('game.startClient')
goog.require('proto.GameState');

//var canvas = document.getElementById("canvas");
//var scene = canvas.getContext("2d");
//scene.fillRect(50, 25, 150, 100);

var canvas = new fabric.Canvas('canvas');

/*
var poly = new fabric.Polyline([
    { x: 10, y: 10 },
    { x: 50, y: 30 },
    { x: 40, y: 70 },
    { x: 60, y: 50 },
    { x: 100, y: 150 },
    { x: 40, y: 100 }
],
    {
        stroke: 'red',
        left: 100,
        top: 100
    });
canvas.add(poly);
*/

game.startClient = function () {

    let socket = new WebSocket("ws://localhost:1234");
    socket.binaryType = 'arraybuffer'

    socket.onopen = function () {
        alert("Connected to server");
    };

    socket.onclose = function (event) {
        if (event.wasClean) {
            alert('Connection closed nicely');
        } else {
            alert('Connection closed badly');
        }
    };

    socket.onmessage = function (event) {
        let state = proto.GameState.deserializeBinary(event.data);
        alert("Get data " + state.getPlayer() + " " + state.getActivePlayer());
    };

    socket.onerror = function (error) {
        alert("Error " + error.message);
    };
}

goog.exportSymbol('game.startClient', game.startClient);

