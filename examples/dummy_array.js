const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 5 },
    { name: 'test:dummy02', type: 'float', count: 5 },
    { name: 'test:dummy03', type: 'double', count: 5 },
    { name: 'test:dummy05', type: 'enum', count: 5, enums: ['Stop', 'Run'] }
];

PCAS.createServer(pvList);