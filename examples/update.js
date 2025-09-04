const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 1 },
    { name: 'test:dummy02', type: 'float', count: 1 },
    { name: 'test:dummy03', type: 'double', count: 1 },
    { name: 'test:dummy04', type: 'int', count: 5 },
    { name: 'test:dummy05', type: 'float', count: 5 },
    { name: 'test:dummy06', type: 'double', count: 5 },
];

PCAS.createServer(pvList);

let isFirstUpdate = true;

const initialScalar = {
    'test:dummy01': 100,
    'test:dummy02': 10.5,
    'test:dummy03': 3.14159,
};

const initialArray = {
    'test:dummy04': [10, 20, 30, 40, 50],
    'test:dummy05': [1.1, 2.2, 3.3, 4.4, 5.5],
    'test:dummy06': [0.01, 0.02, 0.03, 0.04, 0.05],
};

function updatePVs() {
    for (const pv of pvList) {
        const name = pv.name;

        if (isFirstUpdate) {
            if (pv.count === 1) {
                PCAS.setParam(name, initialScalar[name]);
            } else {
                PCAS.setParam(name, initialArray[name]);
            }
        } else {
            const currentValue = PCAS.getParam(name);

            let newValue;
            if (pv.count === 1) {
                newValue = currentValue + 1;
            } else {
                // Convert TypedArray to plain array, then map
                newValue = [...currentValue].map(x => x + 1);
            }

            PCAS.setParam(name, newValue);
        }
    }

    PCAS.updatePVs();

    if (isFirstUpdate) {
        isFirstUpdate = false;
    }
}

setInterval(updatePVs, 1000);
