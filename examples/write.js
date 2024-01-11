const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 1, soft: false },
    { name: 'test:dummy02', type: 'float', count: 1, soft: false },
    { name: 'test:dummy03', type: 'double', count: 1, soft: false },
    { name: 'test:dummy04', type: 'int', count: 5, soft: false },
    { name: 'test:dummy05', type: 'float', count: 5, soft: false },
    { name: 'test:dummy06', type: 'double', count: 5, soft: false},
];

let data = {};

function write(name, value) {
    if(name === 'test:dummy01') {
        data['dummy01'] = value;
    }
    if(name === 'test:dummy02') {
        data['dummy02'] = value;
    }
    if(name === 'test:dummy03') {
        data['dummy03'] = value;
    }
    if(name === 'test:dummy04') {
        data['dummy04'] = value;
    }
    if(name === 'test:dummy05') {
        data['dummy05'] = value;
    }
    if(name === 'test:dummy06') {
        data['dummy06'] = value;
    }
}

PCAS.createServer(pvList, null, write);

setInterval(() => {
    console.log(data);
}, 10000);