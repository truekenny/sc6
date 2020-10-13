let canvas = document.getElementById("map");
let canvasWidth = canvas.width;
let canvasHeight = canvas.height;
let ctx = canvas.getContext("2d");
let canvasData = ctx.getImageData(0, 0, canvasWidth, canvasHeight);

let mapOffsetX = 0;
let mapOffsetY = 1;

let image = new Image();
image.src = "images/world.jpg";

image.onload = function () {
    ctx.drawImage(image, mapOffsetX, mapOffsetY, 360, 180);
    canvasData = ctx.getImageData(0, 0, canvasWidth, canvasHeight);

    // Три тестовые точки
    [
        {x: 1, y: 1, r: 255, g: 0, b: 0, a: 255},
        {x: 2, y: 2, r: 0, g: 255, b: 0, a: 255},
        {x: 3, y: 3, r: 0, g: 0, b: 255, a: 255}
    ].forEach(function ({x, y, r, g, b, a}) {
        drawPixel(x, y, r, g, b, a);
    });

    // Тестовые точки по краям мира
    [
        {lat: 0, lon: 0},
        {lat: 57, lon: 160}, // Камчатка
        {lat: -33, lon: 151}, // Сидней
        {lat: 40, lon: -73} // Нью Йорк
    ].forEach(function ({lat, lon}) {
        let x = Math.round(lon + 180);
        let y = 180 - Math.round(lat + 90);
        drawPixel(x, y, 0, 0, 255, 255);
    });
};


/**
 * Занести точку во временный массив данных (картунка)
 */
function drawPixel(x, y, r, g, b, a) {
    let index = (x + y * canvasWidth) * 4;

    canvasData.data[index] = r;
    canvasData.data[index + 1] = g;
    canvasData.data[index + 2] = b;
    canvasData.data[index + 3] = a;
}

/**
 * Отобразить накопленные данные
 */
function updateCanvas() {
    ctx.putImageData(canvasData, 0, 0);

    ctx.globalAlpha = 0.01;
    ctx.drawImage(image, mapOffsetX, mapOffsetY, 360, 180);
    ctx.globalAlpha = 1;
    canvasData = ctx.getImageData(0, 0, canvasWidth, canvasHeight);

    setTimeout(function () {
        updateCanvas();
    }, 100);
}

/**
 * Поддержка постоянного подключения
 * @param socket
 */
function ping(socket) {
    socket.send('p');

    setTimeout(function () {
        ping(socket);
    }, 1000);
}

let link = "ws://" + location.host + "/websocket";
let socket = new WebSocket(link, "chat");
socket.binaryType = 'arraybuffer';

// Соединение открыто
socket.addEventListener('open', function (event) {
    ping(socket);
    updateCanvas();
});

// Наблюдает за сообщениями
socket.addEventListener('message', function (event) {
    let data = new DataView(event.data);
    let lat = data.getInt16(0, true);
    let lon = data.getInt16(2, true);
    let udp = data.getUint8(4);

    //             ^ lat 90
    //        US   |   RU
    //             |      JP
    // -180 -------+------> lon 180
    //             |
    //        BR   |   AUS
    //             | -90

    drawPixel(Math.round(lon + 180), 180 - Math.round(lat + 90), udp ? 0 : 255, udp ? 255 : 0, 0, 255);
});

// Наблюдает за ошибками
socket.addEventListener('error', function (event) {
    console.log('Websocket Error ', event);
    alert('Websocket Error')
});
