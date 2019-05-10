"use strict";

goog.provide('game.startClient')

goog.require('game.initScene')

game.startClient = function () {

    var config = {
        type: Phaser.AUTO,
        width: 1366,
        height: 768,
        scale: {
            mode: Phaser.Scale.FIT,
            autoCenter: Phaser.Scale.CENTER_BOTH
        },
        backgroundColor: 0xBBBBBB,
        scene: {
            preload: preload,
            create: create
        }
    };

    function preload() {
        //this.load.image('grass_tile', 'assets/grass_tile.png');
        this.load.spritesheet('swordsman', 'assets/swordsman.png', { frameWidth: 48, frameHeight: 48 });
    }

    function create() {
        const socket = new WebSocket("ws://95.161.156.126:1234");
        socket.binaryType = 'arraybuffer'

        game.initScene(this, socket);

        socket.onopen = game.scene.handleStart
        socket.onclose = game.scene.handleConnectionClosed
        socket.onmessage = game.scene.handleGameStateRecieved
        socket.onerror = game.scene.handleConnectionError
    }

    var gameObject = new Phaser.Game(config);
}

goog.exportSymbol('game.startClient', game.startClient);

