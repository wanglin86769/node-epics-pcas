const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 1 }
];

PCAS.createServer(pvList);

setInterval(() => {
    const name = 'test:dummy01';
    let value = PCAS.getParam(name);
    if(value >= 80) {
        PCAS.setParamStatus(name, PCAS.Alarm.HIHI_ALARM, PCAS.Severity.MAJOR_ALARM);
    } else if(value <= 20) {
        PCAS.setParamStatus(name, PCAS.Alarm.LOLO_ALARM, PCAS.Severity.MAJOR_ALARM);
    } else if(value >= 60) {
        PCAS.setParamStatus(name, PCAS.Alarm.HIGH_ALARM, PCAS.Severity.MINOR_ALARM);
    } else if(value <= 40) {
        PCAS.setParamStatus(name, PCAS.Alarm.LOW_ALARM, PCAS.Severity.MINOR_ALARM);
    }
    PCAS.updatePVs();
    PCAS.setParam(name, value + 1);
    PCAS.updatePVs();
}, 500);