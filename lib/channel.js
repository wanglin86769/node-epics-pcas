const koffi = require('koffi');
const path = require('path');
const os = require('os');
const { aitEnum } = require('./aitTypes');
const { Alarm, Severity } = require('./alarm');


let LIBPCAS_PATH;
switch(os.platform()) {
	case 'win32': 
		// console.log("windows platform");
		LIBPCAS_PATH = path.join(__dirname, 'clibs', 'win64', 'cas.dll');
		break;
	case 'linux': 
		// console.log("Linux Platform");
		LIBPCAS_PATH = path.join(__dirname, 'clibs', 'linux64', 'libcas.so');
		break;
	case 'darwin': 
		// console.log("Darwin platform(MacOS, IOS etc)");
		LIBPCAS_PATH = path.join(__dirname, 'clibs', 'darwin64', 'libcas.dylib');
		break;
	default: 
		console.log("Unknown platform");
		break;
}
if(!LIBPCAS_PATH) {
    throw new Error("Cannot find the PCAS shared library!");
}

const libpcas = koffi.load(LIBPCAS_PATH);


// EPICS maximum string size
const MAX_STRING_SIZE = 40;


// Global function pointer
let driverReadFunc = null;
let driverWriteFunc = null;


// PV definition structure
const pvDef = koffi.struct('pvDef', {
    name: 'char *',
    type: 'int',
    count: 'int',
    scan: 'double',
    enums: 'char **',
    states: 'int *',
    prec: 'int',
    unit: 'char *',
    hilim: 'double',
    lolim: 'double',
    high: 'double',
    low: 'double',
    hihi: 'double',
    lolo: 'double',
    mdel: 'double',
    adel: 'double',
    soft: 'bool',
    value: 'void *'
});


// Data exchange structure between Node.js and C++
const SimpleValue = koffi.struct('SimpleValue', {
    type: 'int',
    count: 'int',
    buffer: 'void *'
});


// Callback prototype to be called by C++
const ReadCallback = koffi.proto('ReadCallback', 'void', ['char *', koffi.out('void *')]);
const WriteCallback = koffi.proto('WriteCallback', 'void', ['char *', 'void *']);


// Functions provided by C++ to create the PCAS server
const _createServer = libpcas.func('createServer', 'void', ['pvDef *', 'int']);
const _createDriver = libpcas.func('createDriver', 'void', []);
const _installCallback = libpcas.func('installCallback', 'void', [koffi.pointer(ReadCallback), koffi.pointer(WriteCallback)]);
const _installReadCallback = libpcas.func('installReadCallback', 'void', [koffi.pointer(ReadCallback)]);
const _installWriteCallback = libpcas.func('installWriteCallback', 'void', [koffi.pointer(WriteCallback)]);
const _createScanThread = libpcas.func('createScanThread', 'void', []);
const _serverProcess = libpcas.func('serverProcess', 'void', ['double']);
const _setDebugLevel = libpcas.func('setDebugLevel', 'void', ['int']);


// Functions provided by C++ to exchange data with the parameter library in C++
const _getParam = libpcas.func('getParam', 'void', ['char *', koffi.out('SimpleValue *')]);
const _setParam = libpcas.func('setParam', 'void', ['char *', 'SimpleValue *']);
const _setParamStatus = libpcas.func('setParamStatus', 'void', ['char *', 'int', 'int']);
const _updatePVs = libpcas.func('updatePVs', 'void', []);
const _getSimpleValue = libpcas.func('getSimpleValue', 'void', ['char *', koffi.out('SimpleValue *')]);


// Convert string array to Node.js buffer
function stringArrayToBuffer(array) {
    let count = array.length;
    let buf = Buffer.alloc(count * MAX_STRING_SIZE);
    for(let i = 0; i < count; i++) {
        buf.write(array[i], i * MAX_STRING_SIZE, MAX_STRING_SIZE);
    }
    return buf;
}


// Convert Node.js buffer to string array
function bufferToStringArray(buffer, count) {
    let array = [];
    for(let i = 0; i < count; i++) {
        let str = koffi.decode(buffer, i * MAX_STRING_SIZE, 'char', MAX_STRING_SIZE);
        array.push(str);
    }
    return array;
}


// Convert PV data type to PCAS architecture­-independent type
function convertPVTypeToAitType(pvType) {
    let aitType;
    switch(pvType) {
        case 'int':
            aitType = aitEnum.aitEnumInt32;
            break;
        case 'float':
            aitType = aitEnum.aitEnumFloat32;
            break;
        case 'double':
            aitType = aitEnum.aitEnumFloat64;
            break;
        case 'string':
            aitType = aitEnum.aitEnumString;
            break;
        case 'enum':
            aitType = aitEnum.aitEnumEnum16;
            break;
        default:
            aitType = aitEnum.aitEnumInvalid;
    }
    return aitType;
}


// Check if PV fields are valid
function validatePVField(pvList) {
    const availableFields = ['name', 'type', 'count', 'scan', 'enums', 'states',
                            'prec', 'unit', 'hilim', 'lolim', 'high', 'low',
                            'hihi', 'lolo', 'mdel', 'adel', 'soft', 'value'];

    for(let pv of pvList) {
        if(pv.name === undefined) {
            throw new Error("PV name is not specified");
        }

        for(const [key, value] of Object.entries(pv)) {
            if(!availableFields.includes(key)) {
                throw new Error(`PV field ${key} is not supported`);
            }
            
            let valid = true;
            let message = `Field ${key} for PV ${pv.name} is invalid`;
            switch(key) {
                case 'name':
                    if(typeof value !== "string") valid = false;
                    break;
                case 'type':
                    if(typeof value !== "string") valid = false;
                    break;
                case 'count':
                    if(!Number.isInteger(value) || value < 1) valid = false;
                    break;
                case 'scan':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'enums':
                    if(!Array.isArray(value)) valid = false;
                    break;
                case 'states':
                    if(!Array.isArray(value)) valid = false;
                    break;
                case 'prec':
                    if(!Number.isInteger(value)) valid = false;
                    break;
                case 'unit':
                    if(typeof value !== "string") valid = false;
                    break;
                case 'hilim':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'lolim':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'high':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'low':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'hihi':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'lolo':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'mdel':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'adel':
                    if(typeof(value) !== 'number') valid = false;
                    break;
                case 'soft':
                    if(typeof value !== 'boolean') valid = false;
                    break;
                case 'value':
                    if(pv.count === undefined || pv.count === 1) {
                        if(Array.isArray(value)) {
                            valid = false;
                        }
                    } else if(pv.count > 1) {
                        if(!Array.isArray(value) || value.length !== pv.count) {
                            valid = false;
                        }
                    }
                    break;
                default:
                    console.log(`Unsupported field name ${key} for PV ${pv.name}`);
                    valid = false;
            }
            if(!valid) {
                throw new Error(message);
            }
        }
    }
}


// Assign default value to PV fields and convert field format
function convertPVFieldFormat(pvList) {
    if(!pvList || !pvList.length) {
        console.log('convertPVFieldFormat(): PV list is empty');
        return;
    }

    for(let pv of pvList) {
        if(!pv) break;

        if(pv.type === undefined) pv.type = 'float';
        pv.type = convertPVTypeToAitType(pv.type);

        if(pv.count === undefined) pv.count = 1;
        if(pv.scan === undefined) pv.scan = 0;

        if(pv.enums === undefined) pv.enums = [];

        if(pv.states === undefined) pv.states = [];
        if(pv.enums && pv.enums.length) pv.states = Array(pv.enums.length).fill(Severity.NO_ALARM);
        
        // Terminator for enums is null
        pv.enums.push(null);

        // Terminator for states is -1
        pv.states.push(-1);

        if(pv.prec === undefined) pv.prec = 0;
        if(pv.unit === undefined) pv.unit = '';
        if(pv.hilim === undefined) pv.hilim = 0;
        if(pv.lolim === undefined) pv.lolim = 0;
        if(pv.high === undefined) pv.high = 0;
        if(pv.low === undefined) pv.low = 0;
        if(pv.hihi === undefined) pv.hihi = 0;
        if(pv.lolo === undefined) pv.lolo = 0;
        if(pv.mdel === undefined) pv.mdel = 0;
        if(pv.adel === undefined) pv.adel = 0;
        if(pv.soft === undefined) pv.soft = true;
        if(pv.value === undefined) {
            if(pv.type === aitEnum.aitEnumString) {
                if(pv.count > 1) {
                    pv.value = Array(pv.count).fill('');
                } else {
                    pv.value = '';
                }
            } else {
                if(pv.count > 1) {
                    pv.value = Array(pv.count).fill(0);
                } else {
                    pv.value = 0;
                }
            }
        }
    
        if(!Array.isArray(pv.value)) {
            pv.value = [ pv.value ];
        }
    
        switch(pv.type) {
            case aitEnum.aitEnumInt32:
                pv.value = koffi.as(pv.value, 'int *');
                break;
            case aitEnum.aitEnumFloat32:
                pv.value = koffi.as(pv.value, 'float *');
                break;
            case aitEnum.aitEnumFloat64:
                pv.value = koffi.as(pv.value, 'double *');
                break;
            case aitEnum.aitEnumString:
                pv.value = stringArrayToBuffer(pv.value);
                break;
            case aitEnum.aitEnumEnum16:
                pv.value = koffi.as(pv.value, 'int *');
                break;
            default:
                console.log(`convertPVFieldFormat(): Unknown PV type ${pv.type} for PV ${pv.name}`);
        }
    }
}


// The read callback to be called by C++
const readCallbackPtr = koffi.register((name, result) => {
    let data = driverReadFunc(name);
    if(data === null || data === undefined) {
        console.log(`readCallbackPtr(): no data return from driverReadFunc() for PV ${name}`);
        return;
    }
    if(!Array.isArray(data)) {
        data = [data];
    }

    let simpleValue = koffi.decode(result, 'SimpleValue');
    let buffer = simpleValue.buffer;

    if(simpleValue.count !== data.length) {
        console.log(`readCallbackPtr(): returned data length ${data.length} is not consistent with PV count ${simpleValue.count} for PV ${name}`);
        return;
    }

    switch(simpleValue.type) {
        case aitEnum.aitEnumInt32:
            koffi.encode(buffer, 'int', data, data.length);
            break;
        case aitEnum.aitEnumFloat32:
            koffi.encode(buffer, 'float', data, data.length);
            break;
        case aitEnum.aitEnumFloat64:
            koffi.encode(buffer, 'double', data, data.length);
            break;
        case aitEnum.aitEnumString:
            for(let i = 0; i< data.length; i++) {
                let item = data[i];
                koffi.encode(buffer, MAX_STRING_SIZE * i, 'char', item, item.length + 1);
            }
            break;
        case aitEnum.aitEnumEnum16:
            koffi.encode(buffer, 'int', data, data.length);
            break;
        default:
            console.log(`readCallbackPtr(): Unknown PV type ${simpleValue.type}.`);
    }
}, koffi.pointer(ReadCallback));


// The write callback to be called by C++
const writeCallbackPtr = koffi.register((name, value) => {
    let simpleValue = koffi.decode(value, 'SimpleValue');
    let array;
    switch(simpleValue.type) {
        case aitEnum.aitEnumInt32:
            array = koffi.decode(simpleValue.buffer, 'int', simpleValue.count);
            break;
        case aitEnum.aitEnumFloat32:
            array = koffi.decode(simpleValue.buffer, 'float', simpleValue.count);
            break;
        case aitEnum.aitEnumFloat64:
            array = koffi.decode(simpleValue.buffer, 'double', simpleValue.count);
            break;
        case aitEnum.aitEnumString:
            array = bufferToStringArray(simpleValue.buffer, simpleValue.count);
            break;
        case aitEnum.aitEnumEnum16:
            array = koffi.decode(simpleValue.buffer, 'int', simpleValue.count);
            break;
        default:
            console.log(`writeCallbackPtr(): Unknown PV type ${simpleValue.type}`);
            return;
    }
    let data = simpleValue.count === 1 ? array[0] : array;
    driverWriteFunc(name, data);
}, koffi.pointer(WriteCallback));


// Register driver's read function and install the read callback to C++
function registerDriverReadFunc(read) {
    if(!read) {
        console.log('registerDriverReadFunc(): read is empty');
        return;
    }
    driverReadFunc = read;
    _installReadCallback(readCallbackPtr);
}


// Register driver's write function and install the write callback to C++
function registerDriverWriteFunc(write) {
    if(!write) {
        console.log('registerDriverWriteFunc(): write is empty');
        return;
    }
    driverWriteFunc = write;
    _installWriteCallback(writeCallbackPtr);
}


// Prevent the Node.js main thread from exiting
function waitForever(interval) {
    setInterval(() => {}, interval);
}


/*************************************************************************
*                                                                        *
*                     API for Node.js applications                       *
*                                                                        *
*************************************************************************/


// Create the PCAS server
function createServer(pvList, read, write) {
    validatePVField(pvList);
    convertPVFieldFormat(pvList);

    _createServer(pvList, pvList.length);
    _createDriver();
    if(read) {
        registerDriverReadFunc(read);
    }
    if(write) {
        registerDriverWriteFunc(write);
    }
    _createScanThread();
    _serverProcess(0.2);

    waitForever(1000);
}


// Get data from the parameter library
function getParam(name) {
    let simpleValue = {};
    _getParam(name, simpleValue);
    if(simpleValue.count < 1) {
        console.log(`getParam(): Invalid PV count ${simpleValue.count} for PV ${name}`);
        return null;
    }
    let array;
    switch(simpleValue.type) {
        case aitEnum.aitEnumInt32:
            array = koffi.decode(simpleValue.buffer, 'int', simpleValue.count);
            break;
        case aitEnum.aitEnumFloat32:
            array = koffi.decode(simpleValue.buffer, 'float', simpleValue.count);
            break;
        case aitEnum.aitEnumFloat64:
            array = koffi.decode(simpleValue.buffer, 'double', simpleValue.count);
            break;
        case aitEnum.aitEnumString:
            array = bufferToStringArray(simpleValue.buffer, simpleValue.count);
            break;
        case aitEnum.aitEnumEnum16:
            array = koffi.decode(simpleValue.buffer, 'int', simpleValue.count);
            break;
        default:
            console.log(`getParam(): Unknown PV type ${simpleValue.type} for PV ${name}`);
            return null;
    }

    // Release the memory dynamically allocated in C++
    koffi.free(simpleValue.buffer);

    return simpleValue.count === 1 ? array[0] : array;
}


// Set data to the parameter library
function setParam(name, data) {
    if(data === null || data === undefined) {
        console.log(`setParam(): empty data for PV ${name}`);
        return;
    }
    if(!Array.isArray(data)) data = [data];

    let simpleValue = {};
    _getSimpleValue(name, simpleValue);

    if(simpleValue.count !== data.length) {
        console.log(`setParam(): data length ${data.length} is not consistent with PV count ${simpleValue.count} for PV ${name}`);
        return;
    }

    let value = { type: 0, count: 0, buffer: null };
    switch(simpleValue.type) {
        case aitEnum.aitEnumInt32:
            value.buffer = koffi.as(data, 'int *');
            break;
        case aitEnum.aitEnumFloat32:
            value.buffer = koffi.as(data, 'float *');
            break;
        case aitEnum.aitEnumFloat64:
            value.buffer = koffi.as(data, 'double *');
            break;
        case aitEnum.aitEnumString:
            value.buffer = stringArrayToBuffer(data);
            break;
        case aitEnum.aitEnumEnum16:
            value.buffer = koffi.as(data, 'int *');
            break;
        default:
            console.log(`setParam(): Unknown PV type ${simpleValue.type} for PV ${name}`);
            return;
    }
    _setParam(name, value);
}


// Set alarm and severity to the parameter library
function setParamStatus(name, alarm, severity) {
    _setParamStatus(name, alarm, severity);
}


// Post event to monitor clients
function updatePVs() {
    _updatePVs();
}


// Set debug level to print debug information
function setDebugLevel(level) {
    _setDebugLevel(level);
}


module.exports = {
    createServer,
    getParam,
    setParam,
    setParamStatus,
    updatePVs,
    setDebugLevel,
};