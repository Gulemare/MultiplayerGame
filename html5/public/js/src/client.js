"use strict";

goog.provide('game.startClient')

goog.require('proto.GameState');
goog.require('hex_utils.oddrToScenePos');

game.startClient = function () {

    var config = {
        type: Phaser.AUTO,
        width: 800,
        height: 600,
        scene: {
            preload: preload,
            create: create
        }
    };

    var game = new Phaser.Game(config);

    function preload() {
        //this.load.image('grass_tile', 'assets/grass_tile.png');
    }

    function create() {
        var self = this;

        this.tiles = this.add.group();

        this.tiles = {}

        const socket = new WebSocket("ws://localhost:1234");
        socket.binaryType = 'arraybuffer'

        socket.onopen = function () {
            alert("Connected to server");
        };

        socket.onclose = function (event) {

            Object.keys(self.tiles).forEach(function (key) {
                self.tiles[key].destroy();
            });
            

            if (event.wasClean) {
                alert('Connection closed nicely');
            } else {
                alert('Connection closed badly');
            }
        };

        socket.onmessage = function (event) {
            let state = proto.GameState.deserializeBinary(event.data);
            console.log("Get data " + state.getPlayer() + " " + state.getActivePlayer());

            state.getTilesList().forEach(function (tile) {
                const row = tile.getPos().getRow();
                const col = tile.getPos().getCol();
                const pos = hex_utils.oddrToScenePos(row, col);
                const HW = hex_utils.WIDTH / 2;
                const HH = hex_utils.HEIGHT / 2;
                const poly = self.add.polygon(pos.x, pos.y, [0, -HH, HW, -HH / 2, HW, HH / 2,
                    0, HH, -HW, HH / 2, -HW, -HH / 2], 0x00aa00);

                self.tiles[`${row},${col}`] = poly;
            });

            
        };

        socket.onerror = function (error) {
            alert("Error " + error.message);
        };
    }

    
}

goog.exportSymbol('game.startClient', game.startClient);

