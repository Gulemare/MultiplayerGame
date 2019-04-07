"use strict";

goog.provide('game.scene')
goog.provide('game.initScene')

goog.require('proto.GameState');
goog.require('proto.Unit')

goog.require('proto.Command')
goog.require('proto.Move')
goog.require('hex.oddrToScenePos');

var scene = null;
game.initScene = function (newScene, socket) {
    scene = newScene;

    // custom object containers
    scene.tiles = {};
    scene.units = {};
    scene.selectedUnit = null;
    scene.moveTiles = [];

    // disable rmb clicks
    scene.input.mouse.disableContextMenu();

    // create method to send command
    scene.sendCommand = function (concreteCmd) {
        socket.send(concreteCmd.serializeBinary());
    }

    game.initAnimations(scene);
}

game.initAnimations = function (scene) {

    var config = {
        key: 'idle',
        frames: scene.anims.generateFrameNumbers('swordsman',
            { start: 0, end: 3 }),
        frameRate: 6,
        repeat: -1
    };
    scene.anims.create(config);

    var config = {
        key: 'idle_selected',
        frames: scene.anims.generateFrameNumbers('swordsman',
            { start: 4, end: 7 }),
        frameRate: 6,
        repeat: -1
    };
    scene.anims.create(config);

    config = {
        key: 'walk',
        frames: scene.anims.generateFrameNumbers('swordsman',
            { start: 8, end: 15 }),
        frameRate: 6,
        repeat: -1
    };
    scene.anims.create(config);
}

game.scene.clear = function () {
    Object.keys(scene.tiles).forEach(function (key) {
        scene.tiles[key].destroy();
    });

    Object.keys(scene.units).forEach(function (key) {
        scene.units[key].destroy();
    });

    scene.tiles = {};
    scene.moveTiles = [];
    scene.units = {};
}


game.scene.handleStart = function () {
    //alert("Connected to server");
}

game.scene.handleConnectionClosed = function (event) {
    game.scene.clear();

    if (event.wasClean) {
        //alert('Connection closed nicely');
    } else {
        //alert('Connection closed badly');
    }
};

game.scene.updatePossibleMoves = function (sceneUnit) {

    // clear prev move tiles, remove effects from previous move tiles
    scene.moveTiles.forEach(function (tile) {
        tile.clearEffects();
    });
    scene.moveTiles.length = 0;

    // add new move tiles
    hex.oddrNeighbours(sceneUnit.oddr).forEach(function (oddr) {
        const tileKey = `${oddr.row},${oddr.col}`;
        if (tileKey in scene.tiles) {
            let sceneTile = scene.tiles[tileKey];
            if (sceneTile.isOccupied) {
                return;
            }
            scene.moveTiles.push(sceneTile);
        }
    });

    // apply movable effects
    scene.moveTiles.forEach(function (tile) {
        tile.setMovableEffect();
    });
}

game.scene.handleGameStateRecieved = function (event) {
    const state = proto.GameState.deserializeBinary(event.data);
    console.log("Get data " + state.getPlayer() + " " + state.getActivePlayer());

    const player = state.getPlayer();

    state.getTilesList().forEach(function (tile) {
        const row = tile.getPos().getRow();
        const col = tile.getPos().getCol();
        const pos = hex.oddrToScenePos(row, col);
        const sceneTile = scene.add.polygon(pos.x, pos.y, hex.polygon, 0xffffff);
        sceneTile.setDepth(0);
        sceneTile.oddr = hex.Oddr(row, col);

        sceneTile.on('pointerdown', function (pointer) {
            if (pointer.rightButtonDown()) {
                console.log('move to tile: ' + sceneTile.oddr.row + ', ' + sceneTile.oddr.col);
                
                var pos = new proto.Position();
                pos.setRow(sceneTile.oddr.row);
                pos.setCol(sceneTile.oddr.col);

                var move = new proto.Move();
                move.setPosition(pos);
                move.setUnitId(scene.selectedUnit.id);

                var command = new proto.Command();
                command.setMove(move)

                scene.sendCommand(command);
            }
        });

        sceneTile.setStrokeStyle(3, 0x000000, 1.0);
        sceneTile.isOccupied = tile.getOccupied();
        sceneTile.clearEffects = function () {
            sceneTile.disableInteractive();
            sceneTile.setStrokeStyle(3, 0x000000, 1.0);
            sceneTile.setFillStyle(0xffffff, 1.0);
        }
        sceneTile.setMovableEffect = function () {
            sceneTile.clearEffects();
            var shape = new Phaser.Geom.Polygon(hex.polygon);
            sceneTile.setInteractive(shape, Phaser.Geom.Polygon.Contains);
            sceneTile.setFillStyle(0xaaffaa, 1.0);
        }

        scene.tiles[`${row},${col}`] = sceneTile;
    });

    state.getUnitsMap().forEach(function (unit, id) {
        const row = unit.getPosition().getRow();
        const col = unit.getPosition().getCol();
        const oddr = hex.Oddr(row, col);
        const pos = hex.oddrToScenePos(row, col);
        const isControllable = unit.getPlayer() == player;

        // update unit
        if (id in scene.units) {
            let sceneUnit = scene.units[id];

            if (oddr.row == sceneUnit.oddr.row && oddr.col == sceneUnit.oddr.col) {
                return;
            }
            const prevPos = hex.oddrToScenePos(sceneUnit.oddr.row, sceneUnit.oddr.col);
            sceneUnit.oddr = oddr;
            sceneUnit.setFlipX(pos.x > prevPos.x);
            sceneUnit.anims.play('walk');
            scene.tweens.add({
                targets: sceneUnit,
                x: pos.x - hex.WIDTH / 2,
                y: pos.y - hex.HEIGHT / 2,
                duration: 300,
                ease: 'Linear',
                onComplete: function (tween, targets) {
                    if (scene.selectedUnit && scene.selectedUnit.id == id) {
                        game.scene.updatePossibleMoves(sceneUnit);
                        sceneUnit.anims.play('idle_selected');
                    }
                    else {
                        sceneUnit.anims.play('idle');
                    }
                }
            });
            return;
        }

        // create new unit
        const color = isControllable ? 0xccffcc : 0xffcccc;
        //let sceneUnit = scene.add.ellipse(pos.x - hex.WIDTH / 2, pos.y - hex.HEIGHT / 2, hex.HEIGHT / 2, hex.HEIGHT / 2, color)

        let sceneUnit = scene.add.sprite(pos.x - hex.WIDTH / 2, pos.y - hex.HEIGHT / 2, 'swordsman').setScale(3);
        sceneUnit.setTint(color);

        //sceneUnit.setStrokeStyle(1, 0x000000, 1.0);
        sceneUnit.setDepth(1000);

        sceneUnit.setFlipX(true);
        sceneUnit.anims.load('idle');
        sceneUnit.anims.load('walk');
        sceneUnit.anims.play('idle');

        sceneUnit.oddr = oddr;
        sceneUnit.id = id;

        if (isControllable) {
            sceneUnit.setInteractive();

            sceneUnit.unselect = function () {
                //sceneUnit.setStrokeStyle(1, 0x000000, 1.0);
                sceneUnit.anims.play('idle');
            };

            sceneUnit.select = function () {
                //sceneUnit.setStrokeStyle(5, 0x000000, 1.0);
                sceneUnit.anims.play('idle_selected');
            };

            sceneUnit.on('pointerdown', function (pointer) {
                if (scene.selectedUnit) {
                    scene.selectedUnit.unselect();
                }
                sceneUnit.select();
                scene.selectedUnit = sceneUnit;
                console.log('unit selected');
                game.scene.updatePossibleMoves(sceneUnit);
            });

            sceneUnit.on('pointerout', function (pointer) {
            });

            sceneUnit.on('pointerup', function (pointer) {
            });
        }

        scene.units[id] = sceneUnit;
    });
};

game.scene.handleConnectionError = function (error) {
    alert("Error " + error.message);
};