const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 1, scan: 1, soft: false },
    { name: 'test:dummy02', type: 'float', count: 1, scan: 1, soft: false },
    { name: 'test:dummy03', type: 'double', count: 1, scan: 1, soft: false },
    { name: 'test:dummy04', type: 'int', count: 5, scan: 1, soft: false },
    { name: 'test:dummy05', type: 'float', count: 5, scan: 1, soft: false },
    { name: 'test:dummy06', type: 'double', count: 5, scan: 1, soft: false },
];

function read(name) {
    let value;
    if(name === 'test:dummy01') {
        value = Math.random() * 100;
    }
    if(name === 'test:dummy02') {
        value = Math.random();
    }
    if(name === 'test:dummy03') {
        value = Math.random() * 0.01;
    }
    if(name === 'test:dummy04') {
        value = [Math.random() * 100, Math.random() * 100, Math.random() * 100, Math.random() * 100, Math.random() * 100];
    }
    if(name === 'test:dummy05') {
        value = [Math.random(), Math.random(), Math.random(), Math.random(), Math.random()];
    }
    if(name === 'test:dummy06') {
        value = [Math.random() * 0.01, Math.random() * 0.01, Math.random() * 0.01, Math.random() * 0.01, Math.random() * 0.01];
    }
    return value;
}

PCAS.createServer(pvList, read, null);