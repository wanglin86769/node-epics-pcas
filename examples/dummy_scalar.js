const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int' },
    { name: 'test:dummy02', type: 'float' },
    { name: 'test:dummy03', type: 'double' },
    { name: 'test:dummy04', type: 'string' },
    { name: 'test:dummy05', type: 'enum', enums: ['Stop', 'Run'] }
];

PCAS.createServer(pvList);