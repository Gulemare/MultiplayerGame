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

    scene.gameStarted = false;

    // UI elements
    scene.endTurnBtn = null;
    scene.waitText = null;
    

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

    if (scene.endTurnBtn)
        scene.endTurnBtn.destroy();
    if (scene.waitText)
        scene.waitText.destroy();
}


game.scene.handleStart = function () {
    //alert("Connected to server");
    scene.waitText = scene.add.text(0, hex.HEIGHT * 1, ["WAIT FOR", "ANOTHER PLAYER"],
        { color: "#000", font: "bold 144px Arial", align: 'center', stroke: "#fff", strokeThickness: 10 });
}

game.scene.updateUI = function (isPlayerActive) {
    if (!scene.gameStarted) {
        const enabledColor = "#0f0";
        const disabledColor = "#aaa";
        scene.endTurnBtn = scene.add.text(hex.WIDTH * 11, 0, ["END", "TURN"],
            { color: enabledColor, font: "bold 72px Arial", align: 'center', stroke: "#fff", strokeThickness: 5 });

        scene.endTurnBtn.setInteractive();
        scene.endTurnBtn.on('pointerdown', function () {
            if (scene.endTurnBtn.enabled) {
                console.log("clicked");
                var end = new proto.EndTurn();
                var command = new proto.Command();
                command.setEndTurn(end);
                scene.sendCommand(command);
                scene.endTurnBtn.setEnabled(false);
                game.scene.clearSelection();
            }
        });
        scene.endTurnBtn.enabled = true;

        scene.endTurnBtn.setEnabled = function (enabledFlag) {
            scene.endTurnBtn.enabled = enabledFlag;
            scene.endTurnBtn.setColor(enabledFlag ? enabledColor : disabledColor);
        }

        scene.waitText.destroy();
        scene.gameStarted = true;
    }

    scene.endTurnBtn.setEnabled(isPlayerActive);
}

game.scene.handleConnectionClosed = function (event) {
    game.scene.clear();
    alert(event.reason);
};

game.scene.clearSelection = function () {
    if (scene.selectedUnit) {
        scene.selectedUnit.unselect();
        scene.selectedUnit = null;
    }

    scene.moveTiles.forEach(function (tile) {
        tile.clearEffects();
    });
    scene.moveTiles.length = 0;
}

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
            if (sceneTile.occupiedBy > 0) {
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
    const player = state.getPlayer();
    const team = state.getTeam();

    console.log(state.getActivePlayersList())

    let isPlayerActive = false;
    state.getActivePlayersList().forEach(function (id) {
        if (id == player)
            isPlayerActive = true;
    });
    

    game.scene.updateUI(isPlayerActive);

    state.getTilesList().forEach(function (tile) {
        const row = tile.getPos().getRow();
        const col = tile.getPos().getCol();

        const tileKey = `${row},${col}`;

        // Update tile
        if (tileKey in scene.tiles) {
            scene.tiles[tileKey].occupiedBy = tile.getOccupied();
            return;
        }

        // Create new tile
        const pos = hex.oddrToScenePos(row, col);
        const sceneTile = scene.add.polygon(pos.x, pos.y, hex.polygon, 0xffffff);
        sceneTile.setDepth(0);
        sceneTile.oddr = hex.Oddr(row, col);

        var shape = new Phaser.Geom.Polygon(hex.polygon);
        sceneTile.setInteractive(shape, Phaser.Geom.Polygon.Contains);

        sceneTile.on('pointerdown', function (pointer) {
            //if (pointer.rightButtonDown()) {

            if (sceneTile.occupiedBy > 0) {
                // select unit
                if (!scene.endTurnBtn.enabled)
                    return;
                let unitOnTile = scene.units[sceneTile.occupiedBy];
                if (unitOnTile.player == player) {
                    unitOnTile.select();
                    return;
                }
            }

            if (scene.selectedUnit) {
                if (sceneTile.occupiedBy == 0) {
                    // Move selected unit
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
                else {
                    // Attack unit on tile
                    let unitOnTile = scene.units[sceneTile.occupiedBy];
                    if (unitOnTile.team == team)
                        return;

                    var attack = new proto.DirectAttack();
                    attack.setUnitId(scene.selectedUnit.id);
                    attack.setTargetId(unitOnTile.id);

                    var command = new proto.Command();
                    command.setDirectAttack(attack);

                    scene.sendCommand(command);

                }
            }
        });

        sceneTile.setStrokeStyle(3, 0x000000, 1.0);
        sceneTile.occupiedBy = tile.getOccupied();
        sceneTile.clearEffects = function () {
            sceneTile.setStrokeStyle(3, 0x000000, 1.0);
            sceneTile.setFillStyle(0xffffff, 1.0);
        }
        sceneTile.setMovableEffect = function () {
            sceneTile.clearEffects();
            sceneTile.setFillStyle(0xaaffaa, 1.0);
        }

        scene.tiles[tileKey] = sceneTile;
    });

    state.getUnitsMap().forEach(function (unit, id) {
        const row = unit.getPosition().getRow();
        const col = unit.getPosition().getCol();
        const oddr = hex.Oddr(row, col);
        const pos = hex.oddrToScenePos(row, col);
        const isEnemy = unit.getTeam() != team;
        const isControllable = unit.getPlayer() == player;
        
        // update unit
        if (id in scene.units) {
            let sceneUnit = scene.units[id];

            if (oddr.row != sceneUnit.oddr.row || oddr.col != sceneUnit.oddr.col) {
                sceneUnit.moveTo(oddr);
            }

            if (sceneUnit.health != unit.getHealth()) {
                sceneUnit.health = unit.getHealth();
                sceneUnit.healthBar.setPercent(sceneUnit.health * 100.0 / sceneUnit.maxHealth);
            }

            return;
        }

        // create new unit
        let color = isControllable ? 0xccffcc : 0xccccff;
        color = isEnemy ? 0xffcccc : color;

        let sceneUnit = scene.add.container(pos.x - hex.WIDTH / 2, pos.y - hex.HEIGHT / 2);
        sceneUnit.sprite = scene.add.sprite(0, -20, 'swordsman').setScale(3);
        sceneUnit.add(sceneUnit.sprite);

        const maxBarWidth = hex.WIDTH - 20;
        sceneUnit.healthBar = scene.add.rectangle(0, -sceneUnit.sprite.displayHeight / 2, maxBarWidth, 10, 0x00ff00)
        let healthBarDecorator = scene.add.rectangle(0, -sceneUnit.sprite.displayHeight / 2, maxBarWidth, 10, 0xffffff, 0.0)
        sceneUnit.healthBar.setPercent = function (percent) {
            if (percent > 100)
                return;
            if (percent < 0)
                percent = 0;
            sceneUnit.healthBar.setScale(percent / 100.0, 1.0);
            sceneUnit.healthBar.setX(sceneUnit.healthBar.displayWidth / 2.0 - maxBarWidth / 2.0);
            let color = percent > 75 ? 0x00ff00 : 0xffff00;
            color = percent < 25 ? 0xff0000 : color;
            sceneUnit.healthBar.setFillStyle(color);
        };
        
        healthBarDecorator.setStrokeStyle(2, 0x000000, 1.0);
        
        sceneUnit.add(sceneUnit.healthBar);
        sceneUnit.add(healthBarDecorator);

        sceneUnit.oddr = oddr;
        sceneUnit.id = id;
        sceneUnit.player = unit.getPlayer();
        sceneUnit.team = unit.getTeam();
        sceneUnit.health = unit.getHealth();
        sceneUnit.maxHealth = unit.getHealth();

        sceneUnit.sprite.setTint(color);
        //sceneUnit.setStrokeStyle(1, 0x000000, 1.0);
        //sceneUnit.sprite.setDepth(10);
        //sceneUnit.healthBar.setDepth(50);
        //healthBarDecorator.setDepth(50);

        sceneUnit.updateDepth = function () {
            sceneUnit.setDepth(sceneUnit.oddr.row);
        };
        sceneUnit.updateDepth();

        sceneUnit.sprite.setFlipX(true);

        // unit animations ---------------------------------------------
        sceneUnit.sprite.anims.load('idle');
        sceneUnit.sprite.anims.load('attack');
        sceneUnit.sprite.anims.load('idle_selected');
        sceneUnit.sprite.anims.load('walk');

        sceneUnit.playDamaged = function () {
            sceneUnit.sprite.anims.play('damaged');
            if (sceneUnit.health <= 0) {
                sceneUnit.destroy();
                if (scene.selectedUnit)
                    game.scene.updatePossibleMoves(scene.selectedUnit);
                return;
            }
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
            if (scene.selectedUnit && scene.selectedUnit.id == sceneUnit.id)
                sceneUnit.sprite.anims.play('idle_selected');
            else
                sceneUnit.sprite.anims.play('idle');
        }

        sceneUnit.moveTo = function (newOddr) {
            const prevPos = hex.oddrToScenePos(sceneUnit.oddr.row, sceneUnit.oddr.col);
            const newPos = hex.oddrToScenePos(newOddr.row, newOddr.col);
            sceneUnit.oddr = newOddr;
            sceneUnit.updateDepth();
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
                    if (scene.selectedUnit)
                        game.scene.updatePossibleMoves(scene.selectedUnit);
                }
            });
        }

        function animComplete(animation, frame) {
            sceneUnit.playIdle();
        }

        sceneUnit.select = function () {
            if (scene.selectedUnit) {
                if (scene.selectedUnit.id == sceneUnit.id)
                    return;
                scene.selectedUnit.unselect();
            }
            scene.selectedUnit = sceneUnit;
            game.scene.updatePossibleMoves(sceneUnit);
            sceneUnit.sprite.anims.play('idle_selected');
            
        };

        sceneUnit.unselect = function () {
            sceneUnit.sprite.anims.play('idle');
        };

        sceneUnit.sprite.on('animationcomplete', animComplete, sceneUnit.sprite);
        sceneUnit.playIdle();
        // -------------------------------------------------------------------------------

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