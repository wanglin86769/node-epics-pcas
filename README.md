# EPICS Portable Channel Access Server for Node.js

**node-epics-pcas** is an EPICS PCAS library for Node.js, it is a FFI wrapper that talks to the PCAS shared library using a third-party Node.js FFI package called koffi.

This project is inspired by pcaspy in the following link and the implementation is very similar to pcaspy, the main difference is that most of the logic has been moved from the scripting language to C++.

https://github.com/paulscherrerinstitute/pcaspy

 The implementation of node-epics-pcas is to build a wrapper into the PCAS shared library and provide C interface to Node.js, and then call the C interface from Node.js using koffi.

 ![Alt text](docs/screenshots/architecture.png?raw=true "Title")

# Requirements

## A recent Node.js version is required and the following versions have been tested,

* Node.js 14.21.3

* Node.js 15.14.0

* Node.js 16.19.0

* Node.js 17.9.1

* Node.js 18.13.0

# Supported platforms

* Windows x86_64
* Linux x86_64
* macOS x86_64

# Installation

```bash
npm install node-epics-pcas
```

# Usage

The following APIs are provided for Node.js applications to create PCAS server and interact with the parameter library.

* createServer()
* getParam()
* setParam()
* setParamStatus()
* updatePVs()
* setDebugLevel()

### Create the PCAS server

```javascript
function createServer(pvList, read, write)
```

* pvList: PV list
* read: read function for PV whose **scan** field is greater than 0 and **soft** field is false
* write: write function for PV whose **soft** field is false

Following is the description of PV fields,

| field  | required | default | description |
|--------|----------|---------|-------------|
| name   | Yes      |         |             |
| type   |          | 'float' | 'int', 'float', 'double', 'string' or 'enum'  |
| count  |          | 1       |             |
| scan   |          | 0       |             |
| enums  |          | []      |             |
| states |          | []      |             |
| prec   |          | 0       |             |
| unit   |          | ''      |             |
| hilim  |          | 0       | High limit  |
| lolim  |          | 0       | Low limit   |
| high   |          | 0       |             |
| low    |          | 0       |             |
| hihi   |          | 0       |             |
| lolo   |          | 0       |             |
| mdel   |          | 0       |             |
| adel   |          | 0       |             |
| soft   |          | true    | when set to false, read or write function can be used |
| value  |          | 0 or '' |             |

### Get data from the parameter library

```javascript
function getParam(name)
```

### Set data to the parameter library

```javascript
function setParam(name, data)
```

### Set alarm and severity to the parameter library

```javascript
function setParamStatus(name, alarm, severity)
```

### Post event to monitor clients when value or alarm status changes

```javascript
function updatePVs()
```

### Set debug level to print more information

```javascript
function setDebugLevel(level)
```
* Level 0: Default level, only print error information.
* Level 1: Print PV definition, PV list, parameter library and scan thread.
* Level 2: Print PV process information.

# Examples

### 1. Create a dummy PV

```javascript
const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy' }
];

PCAS.createServer(pvList);
```

### 2. Create dummy scalar PVs

```javascript
const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int' },
    { name: 'test:dummy02', type: 'float' },
    { name: 'test:dummy03', type: 'double' },
    { name: 'test:dummy04', type: 'string' },
    { name: 'test:dummy05', type: 'enum', enums: ['Stop', 'Run'] }
];

PCAS.createServer(pvList);
```

### 3. Create dummy array PVs
```javascript
const PCAS = require('node-epics-pcas');

const pvList = [
    { name: 'test:dummy01', type: 'int', count: 5 },
    { name: 'test:dummy02', type: 'float', count: 5 },
    { name: 'test:dummy03', type: 'double', count: 5 },
    { name: 'test:dummy05', type: 'enum', count: 5, enums: ['Stop', 'Run'] }
];

PCAS.createServer(pvList);
```

### 4. Read data from Node.js

```javascript
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
```

### 5. Write data to Node.js

```javascript
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
```

### 6. Raise alarm automatically

```javascript
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
```

### 7. Raise alarm manually

```javascript
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
```

### 8. Publish Archiver Appliance status as EPICS PV

```javascript
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

        PCAS.setParam('Test:status', status);
        PCAS.setParam('Test:MGMT_uptime', MGMT_uptime);
        PCAS.setParam('Test:pvCount', pvCount);
        PCAS.setParam('Test:connectedPVCount', connectedPVCount);
        PCAS.setParam('Test:disconnectedPVCount', disconnectedPVCount);
        PCAS.setParam('Test:dataRateGBPerDay', dataRateGBPerDay);
        PCAS.updatePVs();
    } catch(error) {
        console.log('axios request failed');
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
        alarmStorageMetrics();
    }
}

setInterval(pollArchiverStatus, REQUEST_TIMEOUT * 1000);
```

# License
MIT license