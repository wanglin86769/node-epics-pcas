const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy' }
];

PCAS.createServer(pvList);