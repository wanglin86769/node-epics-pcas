const PCAS = require('node-epics-pcas');

const pvList = [
    {
        name: 'test:dummy01',
        type: 'int',
        count: 1,
        scan: 1,
        high: 60,
        low: 40,
        hihi: 80,
        lolo: 20,
        soft: false
    }
];

function read(name) {
    let value;
    if(name === 'test:dummy01') {
        value = Math.random() * 100;
    }
    return value;
}

PCAS.createServer(pvList, read, null);