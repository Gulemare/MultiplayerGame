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

    config = {
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

    config = {
        key: 'attack',
        frames: scene.anims.generateFrameNumbers('swordsman',
            { start: 20, end: 23 }),
        frameRate: 6,
        repeat: 0
    };
    scene.anims.create(config);

    config = {
        key: 'damaged',
        frames: scene.anims.generateFrameNumbers('swordsman',
            { start: 32, end: 33 }),
        frameRate: 6,
        repeat: 0
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

        const tileKey = `${row},${col}`;

        // Update tile
        if (tileKey in scene.tiles) {
            scene.tiles[tileKey].isOccupied = tile.getOccupied();
            return;
        }

        // Create new tile
        const pos = hex.oddrToScenePos(row, col);
        const sceneTile = scene.add.polygon(pos.x, pos.y, hex.polygon, 0xffffff);
        sceneTile.setDepth(0);
        sceneTile.oddr = hex.Oddr(row, col);

        sceneTile.on('pointerdown', function (pointer) {
            if (pointer.rightButtonDown()) {
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

        scene.tiles[tileKey] = sceneTile;
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
            sceneUnit.moveTo(oddr);
            return;
        }

        // create new unit
        const color = isControllable ? 0xccffcc : 0xffcccc;

        let sceneUnit = scene.add.container(pos.x - hex.WIDTH / 2, pos.y - hex.HEIGHT / 2);

        sceneUnit.oddr = oddr;
        sceneUnit.id = id;

        sceneUnit.sprite = scene.add.sprite(0, 0, 'swordsman').setScale(3);
        sceneUnit.add(sceneUnit.sprite);

        sceneUnit.sprite.setTint(color);

        //sceneUnit.setStrokeStyle(1, 0x000000, 1.0);
        sceneUnit.sprite.setDepth(1000);
        sceneUnit.sprite.setFlipX(true);

        // unit animations ---------------------------------------------
        sceneUnit.sprite.anims.load('idle');
        sceneUnit.sprite.anims.load('attack');
        sceneUnit.sprite.anims.load('idle_selected');
        sceneUnit.sprite.anims.load('walk');

        sceneUnit.playDamaged = function () {
            sceneUnit.sprite.anims.play('damaged');
        };

        sceneUnit.playAttack = function (targetId) {
            const target = scene.units[targetId]

            const targetPos = hex.oddrToScenePos(target.oddr.row, target.oddr.col);
            const attackerPos = hex.oddrToScenePos(sceneUnit.oddr.row, sceneUnit.oddr.col);
            sceneUnit.sprite.setFlipX(targetPos.x > attackerPos.x);
            sceneUnit.sprite.anims.play('attack');
            target.playDamaged();
        };

        sceneUnit.playIdle = function () {
            if (scene.selectedUnit && scene.selectedUnit.id == sceneUnit.id) {
                game.scene.updatePossibleMoves(sceneUnit);
                sceneUnit.sprite.anims.play('idle_selected');
            }
            else
                sceneUnit.sprite.anims.play('idle');
        }

        sceneUnit.moveTo = function (newOddr) {
            const prevPos = hex.oddrToScenePos(sceneUnit.oddr.row, sceneUnit.oddr.col);
            const newPos = hex.oddrToScenePos(newOddr.row, newOddr.col);
            sceneUnit.oddr = newOddr;
            sceneUnit.sprite.setFlipX(newPos.x > prevPos.x);
            sceneUnit.sprite.anims.play('walk');
            scene.tweens.add({
                targets: sceneUnit,
                x: newPos.x - hex.WIDTH / 2,
                y: newPos.y - hex.HEIGHT / 2,
                duration: 300,
                ease: 'Linear',
                onComplete: function (tween, targets) {
                    sceneUnit.playIdle();
                }
            });
        }

        function animComplete(animation, frame) {
            sceneUnit.playIdle();
        }

        sceneUnit.sprite.on('animationcomplete', animComplete, sceneUnit.sprite);
        sceneUnit.playIdle();
        // -------------------------------------------------------------------------------
        

        sceneUnit.sprite.setInteractive();

        if (isControllable) {
            sceneUnit.unselect = function () {
                sceneUnit.sprite.anims.play('idle');
            };

            sceneUnit.select = function () {
                sceneUnit.sprite.anims.play('idle_selected');
            };

            sceneUnit.sprite.on('pointerdown', function (pointer) {
                if (scene.selectedUnit) {
                    if (scene.selectedUnit.id == sceneUnit.id)
                        return;
                    scene.selectedUnit.unselect();
                }
                sceneUnit.select();
                scene.selectedUnit = sceneUnit;
                console.log('unit selected');
                game.scene.updatePossibleMoves(sceneUnit);
            });

            sceneUnit.sprite.on('pointerout', function (pointer) {
            });

            sceneUnit.sprite.on('pointerup', function (pointer) {
            });
        }
        else {
            sceneUnit.sprite.on('pointerdown', function (pointer) {
                if (!pointer.rightButtonDown())
                    return;
                if (!scene.selectedUnit)
                    return;

                var attack = new proto.DirectAttack();
                attack.setUnitId(scene.selectedUnit.id);
                attack.setTargetId(sceneUnit.id);

                var command = new proto.Command();
                command.setDirectAttack(attack);

                scene.sendCommand(command);

            });
        }

        scene.units[id] = sceneUnit;
    });

    const lastCommand = state.getLastCommand()
    if (lastCommand.hasDirectAttack()) {
        const attackCmd = lastCommand.getDirectAttack();
        const attacker = scene.units[attackCmd.getUnitId()]
        attacker.playAttack(attackCmd.getTargetId())
    }
};

game.scene.handleConnectionError = function (error) {
    alert("Error " + error.message);
};