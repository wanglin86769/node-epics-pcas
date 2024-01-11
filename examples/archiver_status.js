const PCAS = require('node-epics-pcas');
const axios = require('axios');

const ARCHIVER_URL = 'http://192.168.1.100:17665'
const GET_INSTANCE_METRICS_URL = ARCHIVER_URL + '/mgmt/bpl/getInstanceMetrics'
const GET_STORAGE_METRICS_FOR_APPLIANCE_URL = ARCHIVER_URL + '/mgmt/bpl/getStorageMetricsForAppliance?appliance=appliance0'
const REQUEST_TIMEOUT = 5

const pvList = [
    { name: 'Test:status',                      type: 'string', value: '' },
    { name: 'Test:MGMT_uptime',                 type: 'string', value: '' },
    { name: 'Test:pvCount',                     type: 'int',    value: 0 },
    { name: 'Test:connectedPVCount',            type: 'int',    value: 0 },
    { name: 'Test:disconnectedPVCount',         type: 'int',    value: 0 },
    { name: 'Test:dataRateGBPerDay',            type: 'float',  prec: 2, unit: 'GB/day', value: 0 },
    { name: 'Test:sts_total_space',             type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:sts_available_space',         type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:sts_available_space_percent', type: 'float',  prec: 2, unit: '%',      value: 0 },
    { name: 'Test:mts_total_space',             type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:mts_available_space',         type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:mts_available_space_percent', type: 'float',  prec: 2, unit: '%',      value: 0 },
    { name: 'Test:lts_total_space',             type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:lts_available_space',         type: 'float',  prec: 2, unit: 'GB',     value: 0 },
    { name: 'Test:lts_available_space_percent', type: 'float',  prec: 2, unit: '%',      value: 0 },
];

PCAS.createServer(pvList);

function alarmInstanceMetrics() {
    PCAS.setParamStatus('Test:status', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:MGMT_uptime', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:pvCount', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:connectedPVCount', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:disconnectedPVCount', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:dataRateGBPerDay', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.updatePVs();
}

function alarmStorageMetrics() {
    PCAS.setParamStatus('Test:sts_total_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:sts_available_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:sts_available_space_percent', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:mts_total_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:mts_available_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:mts_available_space_percent', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:lts_total_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:lts_available_space', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.setParamStatus('Test:lts_available_space_percent', PCAS.Alarm.TIMEOUT_ALARM, PCAS.Severity.MINOR_ALARM);
    PCAS.updatePVs();
}

// Poll archiver appliance status
async function pollArchiverStatus() {

    // Get instance metrics data
    try {
        let response = await axios.get(GET_INSTANCE_METRICS_URL);
        if(!response || !response.data || !response.status) {
            console.log('Instance metrics data is not obtained from the Archiver Appliance server');
            alarmInstanceMetrics();
            return;
        }
        let data = response.data[0];
        console.log(data);

        let status = data['status'];  // string
        let MGMT_uptime = data['MGMT_uptime'];  // string
        let pvCount = Number(data['pvCount']);  // int
        let connectedPVCount = Number(data['connectedPVCount']);  // int
        let disconnectedPVCount = Number(data['disconnectedPVCount']);  // int
        let dataRateGBPerDay = Number(data['dataRateGBPerDay']);  // float

        // console.log(status);
        // console.log(MGMT_uptime);
        // console.log(pvCount);
        // console.log(connectedPVCount);
        // console.log(disconnectedPVCount);
        // console.log(dataRateGBPerDay);

        PCAS.setParam('Test:status', status);
        PCAS.setParam('Test:MGMT_uptime', MGMT_uptime);
        PCAS.setParam('Test:pvCount', pvCount);
        PCAS.setParam('Test:connectedPVCount', connectedPVCount);
        PCAS.setParam('Test:disconnectedPVCount', disconnectedPVCount);
        PCAS.setParam('Test:dataRateGBPerDay', dataRateGBPerDay);
        PCAS.updatePVs();
    } catch(error) {
        console.log('axios request failed');
        console.log(error.code);
        console.log(error.message);
        console.log(error.stack);

        alarmInstanceMetrics();
    }

    // Get storage metrics data
    try {
        let response = await axios.get(GET_STORAGE_METRICS_FOR_APPLIANCE_URL);
        if(!response || !response.data || !response.status) {
            console.log('Storage metrics data is not obtained from the Archiver Appliance server');
            alarmStorageMetrics();
            return;
        }
        let data = response.data;
        console.log(data);

        let sts_total_space;
        let sts_available_space;
        let sts_available_space_percent;
        let mts_total_space;
        let mts_available_space;
        let mts_available_space_percent;
        let lts_total_space;
        let lts_available_space;
        let lts_available_space_percent;
        for(let item of data) {
            if(item['name'] === 'STS') {
                sts_total_space = Number(item['total_space'].replace(',', ''));
                sts_available_space = Number(item['available_space'].replace(',', ''));
                sts_available_space_percent = Number(item['available_space_percent'].replace(',', ''));
            }
            if(item['name'] === 'MTS') {
                mts_total_space = Number(item['total_space'].replace(',', ''));
                mts_available_space = Number(item['available_space'].replace(',', ''));
                mts_available_space_percent = Number(item['available_space_percent'].replace(',', ''));
            }
            if(item['name'] === 'LTS') {
                lts_total_space = Number(item['total_space'].replace(',', ''));
                lts_available_space = Number(item['available_space'].replace(',', ''));
                lts_available_space_percent = Number(item['available_space_percent'].replace(',', ''));
            }
        }

        // console.log(sts_total_space);
        // console.log(sts_available_space);
        // console.log(sts_available_space_percent);
        // console.log(mts_total_space);
        // console.log(mts_available_space);
        // console.log(mts_available_space_percent);
        // console.log(lts_total_space);
        // console.log(lts_available_space);
        // console.log(lts_available_space_percent);

        PCAS.setParam('Test:sts_total_space', sts_total_space);
        PCAS.setParam('Test:sts_available_space', sts_available_space);
        PCAS.setParam('Test:sts_available_space_percent', sts_available_space_percent);
        PCAS.setParam('Test:mts_total_space', mts_total_space);
        PCAS.setParam('Test:mts_available_space', mts_available_space);
        PCAS.setParam('Test:mts_available_space_percent', mts_available_space_percent);
        PCAS.setParam('Test:lts_total_space', lts_total_space);
        PCAS.setParam('Test:lts_available_space', lts_available_space);
        PCAS.setParam('Test:lts_available_space_percent', lts_available_space_percent);
        PCAS.updatePVs();
    } catch(error) {
        console.log('axios request failed');
        console.log(error.code);
        console.log(error.message);
        console.log(error.stack);
        
        alarmStorageMetrics();
    }
}

setInterval(pollArchiverStatus, REQUEST_TIMEOUT * 1000);