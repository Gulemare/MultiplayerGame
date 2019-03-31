
goog.provide('hex_utils.oddrToScenePos')
goog.provide('hex_utils.WIDTH')
goog.provide('hex_utils.HEIGHT')

const HEX_SIZE = 20;
const WIDTH = Math.sqrt(3) * HEX_SIZE;
const HEIGHT = 2 * HEX_SIZE;
const HALF_WIDTH = WIDTH / 2;
const HALF_HEIGHT = HEIGHT / 2;

hex_utils.WIDTH = WIDTH;
hex_utils.HEIGHT = HEIGHT;

hex_utils.oddrToScenePos = function (row, col) {
    const xOffset = WIDTH + (row % 2) * HALF_WIDTH;
    const yOffset = HEIGHT;
    return { x: col * WIDTH + xOffset, y: row * HEIGHT * 0.75 + yOffset};
}
