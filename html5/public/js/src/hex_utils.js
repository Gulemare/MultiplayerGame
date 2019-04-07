goog.provide('hex.SIZE')
goog.provide('hex.WIDTH')
goog.provide('hex.HEIGHT')

goog.provide('hex.Oddr')
goog.provide('hex.Cube')

goog.provide('hex.oddrToScenePos')
goog.provide('hex.oddrNeighbours')
goog.provide('hex.polygon')

const SIZE = 60; // pixels from center to corner
const WIDTH = Math.sqrt(3) * SIZE;
const HEIGHT = 2 * SIZE;
const HWIDTH = WIDTH / 2;
const HHEIGHT = HEIGHT / 2;
hex.WIDTH = WIDTH;
hex.HEIGHT = HEIGHT;

// col and row
const oddrDirections = [
    [[+1, 0], [0, -1], [-1, -1],
    [-1, 0], [-1, +1], [0, +1]],
    [[+1, 0], [+1, -1], [0, -1],
    [-1, 0], [0, +1], [+1, +1]],
]

hex.Oddr = function () {
    return {
        row: arguments[0],
        col: arguments[1]
    }
};

hex.Cube = function () {
    return {
        x: arguments[0],
        y: arguments[1],
        z: arguments[2],
    }
};

hex.oddrToScenePos = function (row, col) {
    const xOffset = WIDTH + (row % 2) * HWIDTH;
    const yOffset = HEIGHT;
    return { x: col * WIDTH + xOffset, y: row * HEIGHT * 0.75 + yOffset };
};

hex.polygon = [0, -HHEIGHT, HWIDTH, -HHEIGHT / 2, HWIDTH, HHEIGHT / 2, 0,
    HHEIGHT, -HWIDTH, HHEIGHT / 2, -HWIDTH, -HHEIGHT / 2];



hex.oddrNeighbours = function (oddr) {
    var parity = oddr.row & 1;
    neighbours = [];
    oddrDirections[parity].forEach(function (dir) {
        neighbours.push(hex.Oddr(oddr.row + dir[1], oddr.col + dir[0]));
    });
    return neighbours;
}
