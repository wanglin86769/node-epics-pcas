/**
 * This is a wrapper of the PCAS (Portable Channel Access Server) C++ library to provide a C interface to other programming languages.
 * This file is intended to be placed into the base-3.15.9/src/ca/legacy/pcas/generic directory and compiled into cas.dll, libcas.so or libcas.dylib
 */


#include "wrapper.h"


/** 
 * Declarations of C interfaces to Node.js
 */
extern "C" {
    epicsShareFunc void epicsShareAPI createServer(pvDef *pvs, int count);
    epicsShareFunc void epicsShareAPI createDriver();
    epicsShareFunc void epicsShareAPI installCallback(ReadCallback readCallback, WriteCallback writeCallback);
    epicsShareFunc void epicsShareAPI installReadCallback(ReadCallback readCallback);
    epicsShareFunc void epicsShareAPI installWriteCallback(WriteCallback writeCallback);
    epicsShareFunc void epicsShareAPI createScanThread();
    epicsShareFunc void epicsShareAPI serverProcess(double delay);
    epicsShareFunc void epicsShareAPI setDebugLevel(int level);

    epicsShareFunc void epicsShareAPI updatePVs();
    epicsShareFunc void epicsShareAPI getParam(const char* name, SimpleValue* simpleValue);
    epicsShareFunc void epicsShareAPI setParam(const char* name, SimpleValue* simpleValue);
    epicsShareFunc void epicsShareAPI setParamStatus(const char* name, int alarm, int severity);
    epicsShareFunc void epicsShareAPI getSimpleValue(const char* name, SimpleValue* simpleValue);
}


/** 
 * Global variables
 */
SimpleServer *server = NULL;
Driver *driver = NULL;


/** 
 * Debug level
 * 0: Default level, only print error information.
 * 1: Print PV definition, PV list, parameter library and scan thread.
 * 2: Print PV process information.
 */
int debugLevel = 0;


/** 
 * Get EPICS timestamp
 */
epicsTimeStamp * getEPICSTimeStamp() {
    epicsTimeStamp *ts = new epicsTimeStamp();
    epicsTimeGetCurrent(ts);
    return ts;
}


/** 
 * Value class
 */
Value::Value() {
    type = aitEnumInvalid;
    count = 0;
    buffer = NULL;
}

Value::Value(aitEnum type, int count) {
    this->type = type;
    this->count = count;
    
    // Initialize the buffer field
    int bufferSize = calcBufferSize();
    this->buffer = malloc(bufferSize);
    memset(this->buffer, 0, bufferSize);
}

Value::Value(aitEnum type, int count, void *buffer) {
    this->type = type;
    this->count = count;

    // Initialize the buffer field
    int bufferSize = calcBufferSize();
    this->buffer = malloc(bufferSize);
    memcpy(this->buffer, buffer, bufferSize);
}

Value::Value(Value &obj) {
    this->type = obj.type;
    this->count = obj.count;

    // Initialize the buffer field
    int bufferSize = calcBufferSize();
    this->buffer = malloc(bufferSize);
    memcpy(this->buffer, obj.buffer, bufferSize);
}

void Value::setType(aitEnum type) {
    this->type = type;
}

aitEnum Value::getType() {
    return type;
}

void Value::setCount(int count) {
    this->count = count;
}

int Value::getCount() {
    return count;
}

void Value::copyBuffer(void *buffer) {
    // Copy data to the buffer field
    int bufferSize = calcBufferSize();
    memcpy(this->buffer, buffer, bufferSize);
}

void Value::setBuffer(void *buffer) {
    this->buffer = buffer;
}

void * Value::getBuffer() {
    return buffer;
}

int Value::calcBufferSize() {
    if(type == aitEnumInvalid || count == 0) return 0;
    int bufferSize;
    switch(type) {
        case aitEnumInt32:
            bufferSize = count * sizeof(int);
            break;
        case aitEnumFloat32:
            bufferSize = count * sizeof(float);
            break;
        case aitEnumFloat64:
            bufferSize = count * sizeof(double);
            break;
        case aitEnumString:
            bufferSize = count * MAX_STRING_SIZE;
            break;
        case aitEnumEnum16:
            bufferSize = count * sizeof(int);
            break;
        default:
            bufferSize = 0;
            std::cout << "calcBufferSize(): Unknown PV type " << type << std::endl;
            break;
    }
    return bufferSize;
}

std::ostream & operator << (std::ostream &out, const Value &value) {
    if(value.count > 1)  out << "[";
    for(int i = 0; i < value.count; i++) {
        switch(value.type) {
            case aitEnumInt32:
                out << *((int *)value.buffer + i);
                break;
            case aitEnumFloat32:
                out << *((float *)value.buffer + i);
                break;
            case aitEnumFloat64:
                out << *((double *)value.buffer + i);
                break;
            case aitEnumString:
                out << (char *)value.buffer + i * MAX_STRING_SIZE;
                break;
            case aitEnumEnum16:
                out << *((int *)value.buffer + i);
                break;
            default:
                out << "operator<<(): Unknown PV type " << value.type << std::endl;
                break;
        }
        if(i < value.count - 1) {
            out << ", ";
        }
    }
    if(value.count > 1)  out << "]";

    return out;
}


/** 
 * Data class
 */
Data::Data() {
    value = NULL;
    flag = false;
    alarm = UDF_ALARM;
    severity = INVALID_ALARM;
    udf = true;
    mask = 0;
    time = getEPICSTimeStamp();
}

void Data::initValue(aitEnum type, int count) {
    value = new Value(type, count);
}

void Data::initValue(aitEnum type, int count, void *buffer) {
    value = new Value(type, count, buffer);
}

void Data::setValue(Value *value) {
    void *buffer = value->getBuffer();
    this->value->copyBuffer(buffer);
}

Value * Data::getValue() {
    return value;
}

void Data::setAlarm(epicsAlarmCondition alarm) {
    this->alarm = alarm;
}

epicsAlarmCondition Data::getAlarm() {
    return alarm;
}

void Data::setSeverity(epicsAlarmSeverity severity) {
    this->severity = severity;
}
epicsAlarmSeverity Data::getSeverity() {
    return severity;
}

void Data::setMask(unsigned int mask) {
    this->mask = mask;
}

unsigned int Data::getMask() {
    return mask;
}

void Data::setFlag(bool flag) {
    this->flag = flag;
}

bool Data::getFlag() {
    return flag;
}

void Data::setTimeStamp(epicsTimeStamp *time) {
    this->time = time;
}

void Data::setTimeStampToCurrent() {
    epicsTimeGetCurrent(this->time);
}

epicsTimeStamp * Data::getTimeStamp() {
    return time;
}

std::ostream & operator << (std::ostream &out, const Data &data) {
    out << "value=" << *data.value << ", ";
    out << "alarm=" << AlarmStrings[data.alarm] << ", ";
    out << "severity=" << SeverityStrings[data.severity] << ", ";
    out << "flag=" << data.flag << ", ";
    out << "mask=" << data.mask << ", ";
    out << "time=" << data.time;
    return out;
}


/** 
 * Driver class
 */
Driver::Driver() {
    std::map<std::string, casPV*> pvList = server->getPVList();

    if(debugLevel >= 1) {
        std::cout << "\n\n";
        std::cout << "********** Parameter library ***********\n";
    }
    
    for(std::map<std::string, casPV*>::iterator iter = pvList.begin(); iter != pvList.end(); ++iter) {
        // std::cout << "[" << iter->first << ", " << iter->second << "]\n";
        std::string name = iter->first;
        SimplePV *pv = (SimplePV *)iter->second;
        PVInfo *info = pv->getInfo();
        Data *data = new Data();
        data->initValue(info->getValue()->getType(), info->getValue()->getCount(), info->getValue()->getBuffer());
        pvDB.insert(std::pair<std::string, Data*>(name, data));

        if(debugLevel >= 1) {
            std::cout << name << ", {" << *data << "}\n\n";
        }
    }

    if(debugLevel >= 1) {
        std::cout << "****************************************\n";
        std::cout << "\n\n";
    }

    this->readCallback = NULL;
    this->writeCallback = NULL;
}

void Driver::installCallback(ReadCallback readCallback, WriteCallback writeCallback) {
    if(readCallback != NULL)
        this->readCallback = readCallback;
    if(writeCallback != NULL)
        this->writeCallback = writeCallback;
}

void Driver::installReadCallback(ReadCallback readCallback) {
    if(readCallback != NULL)
        this->readCallback = readCallback;
}

void Driver::installWriteCallback(WriteCallback writeCallback) {
    if(writeCallback != NULL)
        this->writeCallback = writeCallback;
}

bool Driver::hasReadCallback() {
    return readCallback != NULL;
}

bool Driver::hasWriteCallback() {
    return writeCallback != NULL;
}

ReadCallback Driver::getReadCallback() {
    return readCallback;
}

WriteCallback Driver::getWriteCallback() {
    return writeCallback;
}

Value * Driver::getParam(std::string name) {
    Value *value = pvDB[name]->getValue();

    if(debugLevel >= 2) {
        std::cout << "getParam(): pv=" << name << ", value=" << *value << std::endl;
    }

    // Clone a new value instance, which will be released later
    Value *cloneValue = new Value(*value);

    return cloneValue;
}

void Driver::setParam(std::string name, Value *value) {
    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];
    PVInfo *info = pv->getInfo();

    if(debugLevel >= 2) {
        // The type and count in value is not guaranteed to be consistent with PV info
        Value *valueToPrint = new Value();
        valueToPrint->setType(info->getValue()->getType());
        valueToPrint->setCount(info->getValue()->getCount());
        valueToPrint->setBuffer(value->getBuffer());
        std::cout << "setParam(): pv=" << name << ", value=" << *valueToPrint << std::endl;
        delete valueToPrint;
    }

    pvDB[name]->setMask(pvDB[name]->getMask() | info->checkValue(value));
    pvDB[name]->setValue(value);
    pvDB[name]->setTimeStampToCurrent();
    if(pvDB[name]->getMask()) {
        pvDB[name]->setFlag(true);
    }
    epicsAlarmCondition alarm;
    epicsAlarmSeverity severity;
    info->checkAlarm(value, &alarm, &severity);
    setParamStatus(name, alarm, severity);
}

void Driver::setParamStatus(std::string name, epicsAlarmCondition alarm, epicsAlarmSeverity severity) {
    if(debugLevel >= 2) {
        if(alarm != pvDB[name]->getAlarm() || severity != pvDB[name]->getSeverity()) {
            std::cout << "setParamStatus(): alarm=" << AlarmStrings[alarm] << ", severity=" << SeverityStrings[severity] << std::endl;
        }
    }

    if(alarm != pvDB[name]->getAlarm()) {
        pvDB[name]->setAlarm(alarm);
        pvDB[name]->setMask(pvDB[name]->getMask() | DBE_ALARM);
        pvDB[name]->setFlag(true);
    }
    if(severity != pvDB[name]->getSeverity()) {
        pvDB[name]->setSeverity(severity);
        pvDB[name]->setMask(pvDB[name]->getMask() | DBE_ALARM);
        pvDB[name]->setFlag(true);
    }
}

Data * Driver::getParamDB(std::string name) {
    return pvDB[name];
}

Value * Driver::read(std::string name) {
    if(!hasReadCallback()) return NULL;

    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];
    PVInfo *info = pv->getInfo();
    Value *value = new Value(info->getValue()->getType(), info->getValue()->getCount());

    SimpleValue *simpleValue = new SimpleValue();
    simpleValue->type = value->getType();
    simpleValue->count = value->getCount();
    simpleValue->buffer = value->getBuffer();

    // Read data from Node.js
    readCallback(name.c_str(), simpleValue);

    if(debugLevel >= 2) {
        std::cout << "Driver::read(): pv=" << name << ", value=" << *value << std::endl;
    }

    return value;
}

bool Driver::write(std::string name, Value *value) {
    if(!hasWriteCallback()) return false;
    if(!value) return false;

    if(debugLevel >= 2) {
        std::cout << "Driver::write(): pv=" << name << ", value=" << *value << std::endl;
    }

    SimpleValue *simpleValue = new SimpleValue();
    simpleValue->type = value->getType();
    simpleValue->count = value->getCount();
    simpleValue->buffer = value->getBuffer();

    // Write data to Node.js
    writeCallback(name.c_str(), simpleValue);
    return true;
}

void Driver::updatePVs() {
    std::map<std::string, casPV*> pvList = server->getPVList();
    for(std::map<std::string, casPV*>::iterator iter = pvList.begin(); iter != pvList.end(); ++iter) {
        std::string name = iter->first;
        updatePV(name);
    }
}

void Driver::updatePV(std::string name) {
    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];
    PVInfo *info = pv->getInfo();
    if(pvDB[name]->getFlag() == true && info->getScan() == 0) {
        pvDB[name]->setFlag(false);
        pv->updateValue(pvDB[name]);
        pvDB[name]->setMask(0);

        if(debugLevel >= 2) {
            std::cout << "Driver::updatePV(): PV " << name << " updated" << std::endl;
        }
    }
}


/** 
 * PVInfo class
 */
PVInfo::PVInfo(pvDef *pv) {
    name = pv->name;
    scan = pv->scan;

    // Initialize enums, the terminator is null
    for(char **ptr = pv->enums; *ptr; ptr++) {
        std::string str = *ptr;
        enums.push_back(str);
    }

    // Initialize states, the terminator is -1
    for(int *ptr = pv->states; *ptr != -1; ptr++) {
        epicsAlarmSeverity severity = (epicsAlarmSeverity)(*ptr);
        states.push_back(severity);
    }

    prec = pv->prec;
    unit = pv->unit;
    hilim = pv->hilim;
    lolim = pv->lolim;
    high = pv->high;
    low = pv->low;
    hihi = pv->hihi;
    lolo = pv->lolo;
    mdel = pv->mdel;
    adel = pv->adel;
    soft = pv->soft;

    valid_low_high = false;
    valid_lolo_hihi = false;
    validateLimit();

    aitEnum type = (aitEnum)pv->type;
    value = new Value(type, pv->count, pv->value);
    mlst = new Value(type, pv->count, pv->value);
    alst = new Value(type, pv->count, pv->value);
}

std::string PVInfo::getName() { 
    return name;
}

double PVInfo::getHopr() {
    return hilim;
}

double PVInfo::getLopr() {
    return lolim;
}

std::string PVInfo::getUnits() { 
    return unit;
}

double PVInfo::getHighWarning() {
    return high;
}

double PVInfo::getLowWarning() {
    return low;
}

double PVInfo::getHighAlarm() {
    return hihi;
}

double PVInfo::getLowAlarm() {
    return lolo;
}

int PVInfo::getPrecision() {
    return prec;
}

Value* PVInfo::getValue() {
    return value;
}

double PVInfo::getScan() {
    return scan;
}

bool PVInfo::getSoft() {
    return soft;
}

std::vector<std::string> PVInfo::getEnums() {
    return enums;
}

// Validate alarm limit
void PVInfo::validateLimit() {
    if(low >= high) {
        valid_low_high = false;
    } else {
        valid_low_high = true;
    }

    if(lolo >= hihi) {
        valid_lolo_hihi = false;
    } else {
        valid_lolo_hihi = true;
    }
}

// Check value change event
unsigned int PVInfo::checkValue(Value *newValue) {
    unsigned int mask = 0;
    void *buffer = newValue->getBuffer();
    
    // Array type always gets notified
    if(value->getCount() > 1) {
        mask = DBE_VALUE | DBE_LOG;
        return mask;
    } 
    
    aitEnum type = value->getType();
    switch(type) {
        case aitEnumInt32:
            if(abs(*(int *)mlst->getBuffer() - *(int *)buffer) > mdel) {
                mask |= DBE_VALUE;
                *(int *)mlst->getBuffer() = *(int *)buffer;
            }
            if(abs(*(int *)alst->getBuffer() - *(int *)buffer) > adel) {
                mask |= DBE_LOG;
                *(int *)alst->getBuffer() = *(int *)buffer;
            }
            break;
        case aitEnumFloat32:
            if(fabs(*(float *)mlst->getBuffer() - *(float *)buffer) > mdel) {
                mask |= DBE_VALUE;
                *(float *)mlst->getBuffer() = *(float *)buffer;
            }
            if(fabs(*(float *)alst->getBuffer() - *(float *)buffer) > adel) {
                mask |= DBE_LOG;
                *(float *)alst->getBuffer() = *(float *)buffer;
            }
            break;
        case aitEnumFloat64:
            if(fabs(*(double *)mlst->getBuffer() - *(double *)buffer) > mdel) {
                mask |= DBE_VALUE;
                *(double *)mlst->getBuffer() = *(double *)buffer;
            }
            if(fabs(*(double *)alst->getBuffer() - *(double *)buffer) > adel) {
                mask |= DBE_LOG;
                *(double *)alst->getBuffer() = *(double *)buffer;
            }
            break;
        case aitEnumString:
            if(strcmp((char *)mlst->getBuffer(), (char *)buffer) != 0) {
                mask |= DBE_VALUE;
                memcpy(mlst->getBuffer(), buffer, MAX_STRING_SIZE);
            }
            if(strcmp((char *)alst->getBuffer(), (char *)buffer) != 0) {
                mask |= DBE_VALUE;
                memcpy(alst->getBuffer(), buffer, MAX_STRING_SIZE);
            }
            break;
        case aitEnumEnum16:
            if(*(int *)mlst->getBuffer() != *(int *)buffer) {
                mask |= DBE_VALUE;
                *(int *)mlst->getBuffer() = *(int *)buffer;
            }
            if(*(int *)alst->getBuffer() != *(int *)buffer) {
                mask |= DBE_LOG;
                *(int *)alst->getBuffer() = *(int *)buffer;
            }
            break;
        default:
            std::cout << "checkValue(): Unknown PV type " << type << std::endl;
            break;
    }

    return mask;
}

void PVInfo::checkAlarm(Value *newValue, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity) {
    aitEnum type = value->getType();
    void *buffer = newValue->getBuffer();

    // Array type does not raise alarm
    if(value->getCount() > 1) {
        *alarm = epicsAlarmNone;
        *severity = epicsSevNone;
        return;
    } 

    switch(type) {
        case aitEnumInt32:
            _checkNumericAlarm(*(int *)buffer, alarm, severity);
            break;
        case aitEnumFloat32:
            _checkNumericAlarm(*(float *)buffer, alarm, severity);
            break;
        case aitEnumFloat64:
            _checkNumericAlarm(*(double *)buffer, alarm, severity);
            break;
        case aitEnumString:
            *alarm = epicsAlarmNone;
            *severity = epicsSevNone;
            break;
        case aitEnumEnum16:
            _checkEnumAlarm(*(int *)buffer, alarm, severity);
            break;
        default:
            std::cout << "checkAlarm(): Unknown PV type " << type << std::endl;
            break;
    }
}

void PVInfo::_checkNumericAlarm(double value, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity) {
    *alarm = epicsAlarmNone;
    *severity = epicsSevNone;

    if(valid_low_high) {
        if(value <= low) {
            *alarm = epicsAlarmLow;
            *severity = epicsSevMinor;
        } else if(value >= high) {
            *alarm = epicsAlarmHigh;
            *severity = epicsSevMinor;
        }
    }
    if(valid_lolo_hihi) {
        if(value <= lolo) {
            *alarm = epicsAlarmLoLo;
            *severity = epicsSevMajor;
        } else if(value >= hihi) {
            *alarm = epicsAlarmHiHi;
            *severity = epicsSevMajor;
        }
    }
}

void PVInfo::_checkEnumAlarm(int value, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity) {
    if(value >= 0 && value < states.size()) {
        *severity = states[value];
        if(*severity == epicsSevNone) {
            *alarm = epicsAlarmNone;
        } else {
            *alarm = epicsAlarmState;
        }
    } else {
        *alarm = epicsAlarmState;
        *severity = epicsSevMajor;
    }
}

std::ostream & operator << (std::ostream &out, const PVInfo &info) {
    out << "name=" << info.name << ", ";
    out << "type=" << info.value->getType() << ", ";
    out << "count=" << info.value->getCount() << ", ";
    out << "scan=" << info.scan << ", ";

    out << "enums=[";
    for(int i = 0; i < info.enums.size(); i++) {
        out << info.enums[i];
        if(i < info.enums.size() - 1) {
            out << ",";
        }
    }
    out << "]" << ", ";

    out << "states=[";
    for(int i = 0; i < info.states.size(); i++) {
        out << SeverityStrings[info.states[i]];
        if(i < info.states.size() - 1) {
            out << ",";
        }
    }
    out << "]" << ", ";

    out << "prec=" << info.prec << ", ";
    out << "unit=" << info.unit << ", ";
    out << "hilim=" << info.hilim << ", ";
    out << "lolim=" << info.lolim << ", ";
    out << "high=" << info.high << ", ";
    out << "low=" << info.low << ", ";
    out << "hihi=" << info.hihi << ", ";
    out << "lolo=" << info.lolo << ", ";
    out << "mdel=" << info.mdel << ", ";
    out << "adel=" << info.adel << ", ";
    out << "soft=" << info.soft << ", ";
    out << "value=" << *info.value;
    return out;
}


/** 
 * SimplePV class
 */
SimplePV::SimplePV() {

}

SimplePV::SimplePV(char *name, PVInfo *info) {
    this->name = name;
    this->info = info;
    this->interest = false;

    // if(info->getScan() > 0) {
    //     epicsThreadCreate("scanThread",
    //         epicsThreadPriorityMedium,
    //         epicsThreadGetStackSize(epicsThreadStackMedium),
    //         scanThread,
    //         info);
    // }

    if(!initialized) {
        initFT();
    }
}

SimplePV::~SimplePV() {

}

caStatus SimplePV::interestRegister() {
    interest = true;
    return S_casApp_success;
}

void SimplePV::interestDelete() {
    interest = false;
}

caStatus SimplePV::writeValue(const gdd &dd) {
    Value *value = getValueFromGDD(&dd);
    if(!info->getSoft() && driver->hasWriteCallback()) {
        bool success = driver->write(this->name, value);
        if(!success) {
            driver->setParamStatus(this->name, epicsAlarmWrite, epicsSevInvalid);
        }
    }
    driver->setParam(this->name, value);
    driver->updatePV(this->name);

    return S_casApp_success;
}

caStatus SimplePV::read(const casCtx &ctx, gdd &prototype) {
    return SimplePV::ft.read(*this, prototype);
}

caStatus SimplePV::write(const casCtx &ctx, const gdd &value) {
    return writeValue(value);
}

/* application function table */
void SimplePV::initFT() {
    if (!SimplePV::initialized)
    {
        SimplePV::ft.installReadFunc ("value", &SimplePV::getValue);
        SimplePV::ft.installReadFunc ("precision", &SimplePV::getPrecision);
        SimplePV::ft.installReadFunc ("graphicHigh", &SimplePV::getHighLimit);
        SimplePV::ft.installReadFunc ("graphicLow", &SimplePV::getLowLimit);
        SimplePV::ft.installReadFunc ("controlHigh", &SimplePV::getHighLimit);
        SimplePV::ft.installReadFunc ("controlLow", &SimplePV::getLowLimit);
        SimplePV::ft.installReadFunc ("alarmHighWarning", &SimplePV::getHighWarnLimit);
        SimplePV::ft.installReadFunc ("alarmLowWarning", &SimplePV::getLowWarnLimit);
        SimplePV::ft.installReadFunc ("alarmHigh", &SimplePV::getHighAlarmLimit);
        SimplePV::ft.installReadFunc ("alarmLow", &SimplePV::getLowAlarmLimit);
        SimplePV::ft.installReadFunc ("units", &SimplePV::getUnits);
        SimplePV::ft.installReadFunc ("enums", &SimplePV::getEnums);
        SimplePV::initialized = true;
    }
}

caStatus SimplePV::getValue(gdd &value) {
    aitEnum type = info->getValue()->getType();
    if(value.primitiveType() == aitEnumInvalid) {
        value.setPrimType(type);
    }

    Value *newValue = NULL;
    if(info->getScan() > 0 || info->getSoft() || !driver->hasReadCallback()) {
        newValue = driver->getParam(this->name);
    } else if(driver->hasReadCallback()) {
        newValue = driver->read(this->name);
    }

    if(newValue == NULL) {
        return S_casApp_undefined;
    }

    putValueToGDD(&value, newValue);
    
    Data *data = driver->getParamDB(this->name);
    value.setStatSevr(data->getAlarm(), data->getSeverity());
    value.setTimeStamp(data->getTimeStamp());
    
    return S_casApp_success;
};

caStatus SimplePV::getPrecision(gdd &prec) {
    prec.putConvert(info->getPrecision());
    return S_casApp_success;
};

caStatus SimplePV::getHighLimit(gdd &hilim) {
    hilim.putConvert(info->getHopr());
    return S_casApp_success;
};

caStatus SimplePV::getLowLimit(gdd &lolim) {
    lolim.putConvert(info->getLopr());
    return S_casApp_success;
};

caStatus SimplePV::getHighWarnLimit(gdd &hilim) {
    hilim.putConvert(info->getHighWarning());
    return S_casApp_success;
};

caStatus SimplePV::getLowWarnLimit(gdd &lolim) {
    lolim.putConvert(info->getLowWarning());
    return S_casApp_success;
};

caStatus SimplePV::getHighAlarmLimit(gdd &hilim) {
    hilim.putConvert(info->getHighAlarm());
    return S_casApp_success;
};

caStatus SimplePV::getLowAlarmLimit(gdd &lolim) {
    lolim.putConvert(info->getLowAlarm());
    return S_casApp_success;
};

caStatus SimplePV::getUnits(gdd &units) {
    units.putConvert(aitString(info->getUnits().c_str()));
    return S_casApp_success;
};

caStatus SimplePV::getEnums(gdd &enums) {
    int count = info->getEnums().size();
    enums.setDimension(1);
    enums.setBound(0, 0, count);

    aitString *d = new aitString[count];
    for(int i = 0; i < count; i++) {
        *(d + i) = aitString(info->getEnums()[i].c_str());
    }
    enums.putRef(d);

    return S_casApp_success;
};

const char * SimplePV::getName() const {
    return this->name.c_str();
}

aitEnum SimplePV::bestExternalType() const { 
    return info->getValue()->getType();
}

unsigned SimplePV::maxDimension() const {
    int count = info->getValue()->getCount();
    return count > 1 ? 1 : 0; 
}

aitIndex SimplePV::maxBound(unsigned dimension) const {
    int count = info->getValue()->getCount();
    return dimension == 0 ? count : 0; 
}

void SimplePV::destroy() {}

PVInfo * SimplePV::getInfo() {
    return info;
}

void SimplePV::updateValue(Data *data) {
    if(!interest) return;

    int count = info->getValue()->getCount();
    aitEnum type = info->getValue()->getType();

    gdd * gddValue = new gdd(gddAppType_value, type);
    Value *value = data->getValue();
    void *buffer = value->getBuffer();

    if(count > 1) {
        gddValue->setDimension(1);
        gddValue->setBound(0, 0, count);
    }

    putValueToGDD(gddValue, value);
    gddValue->setTimeStamp(data->getTimeStamp());
    gddValue->setStatSevr(data->getAlarm(), data->getSeverity());

    gdd *gddCtrl;
    switch(type) {
        case aitEnumInt32:
            gddCtrl = gddApplicationTypeTable::AppTable().getDD(gddAppType_dbr_ctrl_long);
            gddCtrl[1].putConvert(aitString(info->getUnits().c_str()));
            gddCtrl[2].putConvert(info->getLowWarning());
            gddCtrl[3].putConvert(info->getHighWarning());
            gddCtrl[4].putConvert(info->getLowAlarm());
            gddCtrl[5].putConvert(info->getHighAlarm());
            gddCtrl[6].putConvert(info->getLopr());
            gddCtrl[7].putConvert(info->getHopr());
            gddCtrl[8].putConvert(info->getLopr());
            gddCtrl[9].putConvert(info->getHopr());
            putGDDToGDD(&gddCtrl[10], gddValue);
            break;
        case aitEnumFloat32:
            gddCtrl = gddApplicationTypeTable::AppTable().getDD(gddAppType_dbr_ctrl_float);
            gddCtrl[1].putConvert(aitString(info->getUnits().c_str()));
            gddCtrl[2].putConvert(info->getLowWarning());
            gddCtrl[3].putConvert(info->getHighWarning());
            gddCtrl[4].putConvert(info->getLowAlarm());
            gddCtrl[5].putConvert(info->getHighAlarm());
            gddCtrl[6].putConvert(info->getLopr());
            gddCtrl[7].putConvert(info->getHopr());
            gddCtrl[8].putConvert(info->getLopr());
            gddCtrl[9].putConvert(info->getHopr());
            gddCtrl[10].putConvert(info->getPrecision());
            putGDDToGDD(&gddCtrl[11], gddValue);
            break;
        case aitEnumFloat64:
            gddCtrl = gddApplicationTypeTable::AppTable().getDD(gddAppType_dbr_ctrl_double);
            gddCtrl[1].putConvert(aitString(info->getUnits().c_str()));
            gddCtrl[2].putConvert(info->getLowWarning());
            gddCtrl[3].putConvert(info->getHighWarning());
            gddCtrl[4].putConvert(info->getLowAlarm());
            gddCtrl[5].putConvert(info->getHighAlarm());
            gddCtrl[6].putConvert(info->getLopr());
            gddCtrl[7].putConvert(info->getHopr());
            gddCtrl[8].putConvert(info->getLopr());
            gddCtrl[9].putConvert(info->getHopr());
            gddCtrl[10].putConvert(info->getPrecision());
            putGDDToGDD(&gddCtrl[11], gddValue);
            break;
        case aitEnumString:
            gddCtrl = gddValue;
            break;
        case aitEnumEnum16:
            {
                gddCtrl = gddApplicationTypeTable::AppTable().getDD(gddAppType_dbr_ctrl_enum);

                putGDDToGDD(&gddCtrl[1], gddValue);

                aitString *d = new aitString[count];
                for(int i = 0; i < count; i++) {
                    *(d + i) = aitString((char *)buffer + i * MAX_STRING_SIZE);
                }
                gddCtrl[2].putRef(d);
            }
            break;
        default:
            std::cout << "updateValue(): Unknown PV type " << type << std::endl;
            gddCtrl = NULL;
            break;
    }

    if(gddCtrl != NULL) {
        myPostEvent(data->getMask(), *gddCtrl);
    }
}

// Get Value from GDD
Value * SimplePV::getValueFromGDD(const gdd *pGDD) {
    aitEnum type = info->getValue()->getType();
    int count = info->getValue()->getCount();

    Value *value = new Value(type, count);
    void *buffer = value->getBuffer();

    aitEnum primitiveType = pGDD->primitiveType();
    if(pGDD->isScalar()) {
        switch(primitiveType) {
            case aitEnumInt32:
                {
                    int d;
                    pGDD->getConvert(d);
                    *(int *)buffer = d;
                }
                break;
            case aitEnumFloat32:
                {
                    float d;
                    pGDD->getConvert(d);
                    *(float *)buffer = d;
                }
                break;
            case aitEnumFloat64:
                {
                    double d;
                    pGDD->getConvert(d);
                    *(double *)buffer = d;
                }
                break;
            case aitEnumString:
                {
                    aitString d;
                    pGDD->getConvert(d);
                    memcpy(buffer, d.string(), MAX_STRING_SIZE);
                }
                break;
            case aitEnumEnum16:
                {
                    int d;
                    pGDD->getConvert(d);
                    *(int *)buffer = d;
                }
                break;
            default:
                std::cout << "getValueFromGDD(): Unknown primitiveType " << primitiveType << std::endl;
                break;
        }
    } else {
        aitUint32 elementCount = pGDD->getDataSizeElements();
        switch(primitiveType) {
            case aitEnumInt32:
                pGDD->get((int *)buffer);
                break;
            case aitEnumFloat32:
                pGDD->get((float *)buffer);
                break;
            case aitEnumFloat64:
                pGDD->get((double *)buffer);
                break;
            case aitEnumString:
                {
                    aitString *d = new aitString[elementCount];
                    pGDD->get(d);
                    for(int i = 0; i < elementCount; i++) {
                        memcpy((char *)buffer + MAX_STRING_SIZE * i, (d + i)->string(), MAX_STRING_SIZE);
                    }   
                }
                break;
            case aitEnumEnum16:
                pGDD->get((int *)buffer);
                break;
            default:
                std::cout << "getValueFromGDD(): Unknown primitiveType " << primitiveType << std::endl;
                break;
        }
    }

    return value;
}

// Put Value to GDD
void SimplePV::putValueToGDD(gdd *pGDD, Value *value) {
    int count = value->getCount();
    void *buffer = value->getBuffer();

    aitEnum primitiveType = pGDD->primitiveType();
    if(pGDD->isScalar()) {
        switch(primitiveType) {
            case aitEnumInt32:
                pGDD->putConvert(*(int *)buffer);
                break;
            case aitEnumFloat32:
                pGDD->putConvert(*(float *)buffer);
                break;
            case aitEnumFloat64:
                pGDD->putConvert(*(double *)buffer);
                break;
            case aitEnumString:
                pGDD->putConvert(aitString((char *)buffer));
                break;
            case aitEnumEnum16:
                pGDD->putConvert(*(int *)buffer);
                break;
            default:
                std::cout << "getValueFromGDD(): Unknown primitiveType " << primitiveType << std::endl;
                break;
        }
    } else {
        pGDD->setDimension(1);
        pGDD->setBound(0, 0, count);
        switch(primitiveType) {
            case aitEnumInt32:
                pGDD->putRef((int *)buffer);
                break;
            case aitEnumFloat32:
                pGDD->putRef((float *)buffer);
                break;
            case aitEnumFloat64:
                pGDD->putRef((double *)buffer);
                break;
            case aitEnumString:
                {
                    aitString *d = new aitString[count];
                    for(int i = 0; i < count; i++) {
                        *(d + i) = aitString((char *)buffer + i * MAX_STRING_SIZE);
                    }
                    pGDD->putRef(d);
                }
                break;
            case aitEnumEnum16:
                pGDD->putRef((int *)buffer);
                break;
            default:
                std::cout << "getValueFromGDD(): Unknown primitiveType " << primitiveType << std::endl;
                break;
        }
    }
}

// Put GDD to GDD
void SimplePV::putGDDToGDD(gdd *pGDD, gdd *value) {
    if(value->isAtomic()) {
        pGDD->setDimension(1);
        aitIndex first, count;
        value->getBound(0, first, count);
        pGDD->setBound(0, first, count);
    }
    
    pGDD->put(value);
}

void SimplePV::myPostEvent(int mask, gdd &value)
{
    caServer *pCAS = this->getCAS();
    if(pCAS != NULL) {
        casEventMask select;
        if(mask & DBE_VALUE)
            select |= pCAS->valueEventMask();
        if(mask & DBE_LOG)
            select |= pCAS->logEventMask();
        if(mask & DBE_ALARM)
            select |= pCAS->alarmEventMask();
        if(mask & DBE_PROPERTY)
            select |= pCAS->propertyEventMask();
        casPV::postEvent(select, value);
    }
}

gddAppFuncTable<SimplePV> SimplePV::ft;
bool SimplePV::initialized = false;


/** 
 * SimpleServer class
 */
SimpleServer::SimpleServer() {

}

SimpleServer::~SimpleServer() {

};

pvExistReturn SimpleServer::pvExistTest(const casCtx &ctx, const caNetAddr &clientAddress, const char *pPVAliasName) {
    if(pvList.find(pPVAliasName) != pvList.end())
        return pverExistsHere;
    else
        return pverDoesNotExistHere;
}

pvAttachReturn SimpleServer::pvAttach(const casCtx &ctx, const char *pPVAliasName) {
    if(pvList.find(pPVAliasName) != pvList.end())
        return pvAttachReturn(*pvList[pPVAliasName]);
    else
        return S_casApp_pvNotFound;
}

void SimpleServer::addPV(const char *name, casPV *pv) { 
    pvList.insert(std::pair<std::string, casPV*>(name, pv));
}

void SimpleServer::createSinglePV(pvDef *pvdef) {
    char *name = pvdef->name;
    PVInfo *info = new PVInfo(pvdef);
    casPV *pv = new SimplePV(name, info);
    addPV(name, pv);
}

void SimpleServer::process(double delay) {
    fileDescriptorManager.process(delay);
}

std::map<std::string, casPV*> SimpleServer::getPVList() {
    return pvList;
}


/** 
 * Print PV definition
 */
void printPVDef(pvDef *pv) {
    std::cout << "name=" << pv->name << ", ";
    std::cout << "type=" << pv->type << ", ";
    std::cout << "count=" << pv->count << ", ";
    std::cout << "scan=" << pv->scan << ", ";

    std::cout << "enums=[";
    for(char **ptr = pv->enums; *ptr; ptr++) {
        std::cout << *ptr;
        if(*(ptr + 1)) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << ", ";

    std::cout << "states=[";
    for(int *ptr = pv->states; *ptr != -1; ptr++) {
        std::cout << SeverityStrings[*ptr];
        if(*(ptr + 1) != -1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << ", ";

    std::cout << "prec=" << pv->prec << ", ";
    std::cout << "unit=" << pv->unit << ", ";
    std::cout << "hilim=" << pv->hilim << ", ";
    std::cout << "lolim=" << pv->lolim << ", ";
    std::cout << "high=" << pv->high << ", ";
    std::cout << "low=" << pv->low << ", ";
    std::cout << "hihi=" << pv->hihi << ", ";
    std::cout << "lolo=" << pv->lolo << ", ";
    std::cout << "adel=" << pv->adel << ", ";
    std::cout << "mdel=" << pv->mdel << ", ";
    std::cout << "soft=" << pv->soft << ", ";

    std::cout << "value=";
    if(pv->count > 1) {
        std::cout << "[";
    }
    for(int i = 0; i < pv->count; i++) {
        switch(aitEnum(pv->type)) {
            case aitEnumInt32:
                std::cout << *((int *)pv->value + i);
                break;
            case aitEnumFloat32:
                std::cout << *((float *)pv->value + i);
                break;
            case aitEnumFloat64:
                std::cout << *((double *)pv->value + i);
                break;
            case aitEnumString:
                std::cout << (char *)pv->value + i * MAX_STRING_SIZE;
                break;
            case aitEnumEnum16:
                std::cout << *((int *)pv->value + i);
                break;
            default:
                std::cout << "printPVDef(): Unknown PV type " << pv->type << std::endl;
                break;
        }
        if(i < pv->count - 1) {
            std::cout << ", ";
        }
    }
    if(pv->count > 1) {
        std::cout << "]";
    }
}


/** 
 * Create server instance
 */
void createServer(pvDef *pvs, int count) {
    server = new SimpleServer();
    pvDef *pv;

    // Print PV definition
    if(debugLevel >= 1) {
        std::cout << "\n\n";
        std::cout << "************ PV definition *************\n";
        for(int i = 0; i < count; i++) {
            pv = pvs + i;
            printPVDef(pv);
            std::cout << "\n\n";
        }
        std::cout << "****************************************\n";
        std::cout << "\n\n";
    }

    // Create PVs for the server tool
    for(int i = 0; i < count; i++) {
        pv = pvs + i;
        server->createSinglePV(pv);
    }

    // Print PV list in the server tool
    if(debugLevel >= 1) {
        std::cout << "\n\n";
        std::cout << "*************** PV list ****************\n";
        std::map<std::string, casPV*> pvList = server->getPVList();
        for(std::map<std::string, casPV*>::iterator iter = pvList.begin(); iter != pvList.end(); ++iter) {
            std::string name = iter->first;
            SimplePV *pv = (SimplePV *)iter->second;
            PVInfo *info = pv->getInfo();
            std::cout << name << ", {" << *info << "}\n\n";
        }
        std::cout << "****************************************\n\n";
        std::cout << "\n\n";
    }
}


/** 
 * Create driver instance
 */
void createDriver() {
    driver = new Driver();
}


/** 
 * Install read and write callbacks
 */
void installCallback(ReadCallback readCallback, WriteCallback writeCallback) {
    driver->installCallback(readCallback, writeCallback);
}


/** 
 * Install read callback
 */
void installReadCallback(ReadCallback readCallback) {
    driver->installReadCallback(readCallback);
}


/** 
 * Install write callback
 */
void installWriteCallback(WriteCallback writeCallback) {
    driver->installWriteCallback(writeCallback);
}


/** 
 * The scan thread for PVs whose scan field is greater than zero
 */
void scanThread(void *arg) {
    PVInfo *info = (PVInfo *) arg;
    std::string name = info->getName();
    double scan = info->getScan();

    if(debugLevel >= 1) {
        std::cout << "Starting scan thread: pv=" << name << ", scan=" << scan << std::endl;
    }

    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];

    while(true) {        
        if(!info->getSoft() && driver->hasReadCallback()) {
            Value *newValue = driver->read(name);
            driver->setParam(name, newValue);

            // post update events if necessary
            Data *data = driver->getParamDB(name);
            if(data->getFlag() == true) {
                data->setFlag(false);
                pv->updateValue(data);
                data->setMask(0);
            }
        }
        epicsThreadSleep(scan);
    }
}


/** 
 * Create scan thread for PVs whose scan field is greater than zero
 */
void createScanThread() {
    std::map<std::string, casPV*> pvList = server->getPVList();
    for(std::map<std::string, casPV*>::iterator iter = pvList.begin(); iter != pvList.end(); ++iter) {
        std::string name = iter->first;
        SimplePV *pv = (SimplePV *)iter->second;
        PVInfo *info = pv->getInfo();
        
        if(info->getScan() > 0) {
            epicsThreadCreate("scanThread",
                epicsThreadPriorityMedium,
                epicsThreadGetStackSize(epicsThreadStackMedium),
                scanThread,
                info);
        }
    }
}


/** 
 * The thread for server process
 */
void serverProcessThread(void *arg)
{
    double delay = *(double *)arg;

    if(debugLevel >= 1) {
        std::cout << "serverProcessThread(): delay=" << delay << " second" << std::endl;
    }

    while(true) {
        server->process(delay);
    }
}


/** 
 * Create the thread for server process
 */
void serverProcess(double delay)
{
    double *d = new double;
    *d = delay;
    epicsThreadCreate("serverProcessThread",
        epicsThreadPriorityMedium,
        epicsThreadGetStackSize(epicsThreadStackMedium),
        serverProcessThread,
        d);
}


/** 
 * Set debug level
 */
void setDebugLevel(int level) {
    std::cout << "Setting debug level to " << level << std::endl;
    debugLevel = level;
}


/** 
 * Post update events on all PVs with value or alarm status changed
 */
void updatePVs() {
    driver->updatePVs();
}


/** 
 * Get data from parameter library
 */
void getParam(const char* name, SimpleValue* simpleValue) {
    Value *value = driver->getParam(name);

    simpleValue->type = value->getType();
    simpleValue->count = value->getCount();
    simpleValue->buffer = value->getBuffer();
}


/** 
 * Set data to parameter library
 */
void setParam(const char* name, SimpleValue* simpleValue) {
    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];
    PVInfo *info = pv->getInfo();

    aitEnum type = info->getValue()->getType();
    int count = info->getValue()->getCount();
    Value *value = new Value(type, count);
    value->copyBuffer(simpleValue->buffer);

    driver->setParam(name, value);
}


/** 
 * Set alarm and severity to parameter library
 */
void setParamStatus(const char* name, int alarm, int severity) {
    driver->setParamStatus(name, (epicsAlarmCondition)alarm, (epicsAlarmSeverity)severity);
}


/** 
 * Get type and count for a specific PV
 */
void getSimpleValue(const char* name, SimpleValue* simpleValue) {
    std::map<std::string, casPV*> pvList = server->getPVList();
    SimplePV *pv = (SimplePV *)pvList[name];
    PVInfo *info = pv->getInfo();

    simpleValue->type = info->getValue()->getType();
    simpleValue->count = info->getValue()->getCount();
    simpleValue->buffer = NULL;
}