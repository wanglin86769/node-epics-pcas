const PCAS = require('node-epics-pcas');

const pvList = [
    // --- Scalar PVs ---
    { name: 'test:char',     type: 'char',   count: 1, scan: 1, soft: false },
    { name: 'test:short',    type: 'short',  count: 1, scan: 1, soft: false },
    { name: 'test:int',      type: 'int',    count: 1, scan: 1, soft: false },
    { name: 'test:float',    type: 'float',  count: 1, scan: 1, soft: false },
    { name: 'test:double',   type: 'double', count: 1, scan: 1, soft: false },
    { name: 'test:string',   type: 'string', count: 1, scan: 1, soft: false },
    { name: 'test:enum',     type: 'enum',   count: 1, scan: 1, soft: false, 
      enums: ['OFF', 'ON', 'TRIPPED', 'DELAY', 'FAULT'] },

    // --- Array PVs (all count: 5) ---
    { name: 'test:char_array',     type: 'char',   count: 5, scan: 1, soft: false },
    { name: 'test:short_array',    type: 'short',  count: 5, scan: 1, soft: false },
    { name: 'test:int_array',      type: 'int',    count: 5, scan: 1, soft: false },
    { name: 'test:float_array',    type: 'float',  count: 5, scan: 1, soft: false },
    { name: 'test:double_array',   type: 'double', count: 5, scan: 1, soft: false },
    { name: 'test:string_array',   type: 'string', count: 5, scan: 1, soft: false },
    { name: 'test:enum_array',     type: 'enum',   count: 5, scan: 1, soft: false, 
      enums: ['INIT', 'RUN', 'PAUSE', 'STOP', 'ERROR'] },
];

function read(name) {
    let value;

    // Current time for dynamic strings
    const time = new Date().toLocaleTimeString();

    // --- Scalar values ---
    if (name === 'test:char') {
        value = Math.floor(Math.random() * 256) - 128;  // -128 to 127
    }
    else if (name === 'test:short') {
        value = Math.floor(Math.random() * 65536) - 32768;  // -32768 to 32767
    }
    else if (name === 'test:int') {
        value = Math.floor(Math.random() * 1000);  // 0 to 999
    }
    else if (name === 'test:float') {
        value = parseFloat((Math.random() * 10).toFixed(6));  // 0.0 to 10.0
    }
    else if (name === 'test:double') {
        value = Math.random() * 0.01;  // 0.0 to 0.01
    }
    else if (name === 'test:string') {
        value = `Live @ ${time}`;  // Dynamic string
    }
    else if (name === 'test:enum') {
        value = Math.floor(Math.random() * 5);  // 0 to 4
    }

    // --- Array values (all length 5) ---
    else if (name === 'test:char_array') {
        value = [
            Math.floor(Math.random() * 256) - 128,
            Math.floor(Math.random() * 256) - 128,
            Math.floor(Math.random() * 256) - 128,
            Math.floor(Math.random() * 256) - 128,
            Math.floor(Math.random() * 256) - 128
        ];
    }
    else if (name === 'test:short_array') {
        value = [
            Math.floor(Math.random() * 65536) - 32768,
            Math.floor(Math.random() * 65536) - 32768,
            Math.floor(Math.random() * 65536) - 32768,
            Math.floor(Math.random() * 65536) - 32768,
            Math.floor(Math.random() * 65536) - 32768
        ];
    }
    else if (name === 'test:int_array') {
        value = [
            Math.floor(Math.random() * 1000),
            Math.floor(Math.random() * 1000),
            Math.floor(Math.random() * 1000),
            Math.floor(Math.random() * 1000),
            Math.floor(Math.random() * 1000)
        ];
    }
    else if (name === 'test:float_array') {
        value = [
            parseFloat((Math.random() * 10).toFixed(6)),
            parseFloat((Math.random() * 10).toFixed(6)),
            parseFloat((Math.random() * 10).toFixed(6)),
            parseFloat((Math.random() * 10).toFixed(6)),
            parseFloat((Math.random() * 10).toFixed(6))
        ];
    }
    else if (name === 'test:double_array') {
        value = [
            Math.random() * 0.01,
            Math.random() * 0.01,
            Math.random() * 0.01,
            Math.random() * 0.01,
            Math.random() * 0.01
        ];
    }
    else if (name === 'test:string_array') {
        value = [
            `Str1 ${time}`,
            `Str2 ${time}`,
            `Str3 ${time}`,
            `Str4 ${time}`,
            `Str5 ${time}`
        ];
    }
    else if (name === 'test:enum_array') {
        value = [
            Math.floor(Math.random() * 5),
            Math.floor(Math.random() * 5),
            Math.floor(Math.random() * 5),
            Math.floor(Math.random() * 5),
            Math.floor(Math.random() * 5)
        ];
    }

    return value;
}

// Start the server
PCAS.createServer(pvList, read, null);
