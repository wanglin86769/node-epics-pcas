#ifndef WRAPPER_H
#define WRAPPER_H

#include <fdManager.h>
#include <gddApps.h>
#include <gddAppFuncTable.h>
#include <casdef.h>
#include <caeventmask.h>
#include <epicsThread.h>

#include <string>
#include <map>
#include <iostream>
#include <vector>


// Map between PV data type and PCAS architecture­-independent type
// std::map<std::string, aitEnum> pvTypeToAit = { 
//     { "int", aitEnumInt32 },
//     { "float", aitEnumFloat32 },
//     { "double", aitEnumFloat64 },
//     { "string", aitEnumString },
//     { "enum", aitEnumEnum16 }
// };


// PV definition
typedef struct pvDef {
    char *name;
    int type;
    int count;
    double scan;
    char **enums;
    int *states;
    int prec;
    char *unit;
    double hilim;
    double lolim;
    double high;
    double low;
    double hihi;
    double lolo;
    double mdel;
    double adel;
    bool soft;
    void *value;
} pvDef;


// Data structure to exchange data between C++ and Node.js
typedef struct SimpleValue {
    int type;  // Needs to convert to or from aitEnum data type
    int count;
    void *buffer;
} SimpleValue;


// The callback to read data from Node.js to C++
typedef void (*ReadCallback)(const char*, SimpleValue*);


// The callback to write data from C++ to Node.js
typedef void (*WriteCallback)(const char*, SimpleValue*);


// Get EPICS timestamp
epicsTimeStamp * getEPICSTimeStamp();


// Alarm condition strings
const std::string AlarmStrings[] = {
    "NO_ALARM",
    "READ",
    "WRITE",
    "HIHI",
    "HIGH",
    "LOLO",
    "LOW",
    "STATE",
    "COS",
    "COMM",
    "TIMEOUT",
    "HWLIMIT",
    "CALC",
    "SCAN",
    "LINK",
    "SOFT",
    "BAD_SUB",
    "UDF",
    "DISABLE",
    "SIMM",
    "READ_ACCESS",
    "WRITE_ACCESS"
};


// Alarm severity strings
const std::string SeverityStrings[] = {
    "NO_ALARM",
    "MINOR",
    "MAJOR",
    "INVALID"
};


// Data structure for value
class Value {
public:
    Value();
    Value(aitEnum type, int count);
    Value(aitEnum type, int count, void *buffer);
    Value(Value &obj);
    void setType(aitEnum type);
    aitEnum getType();
    void setCount(int count);
    int getCount();
    void copyBuffer(void *buffer);
    void setBuffer(void *buffer);
    void *getBuffer();
    int calcBufferSize();
    friend std::ostream & operator << (std::ostream &out, const Value &value);
private:
    aitEnum type;
    int count;
    void *buffer;
};


// Data structure for parameter library
class Data {
public:
    Data();
    void initValue(aitEnum type, int count);
    void initValue(aitEnum type, int count, void *buffer);
    void setValue(Value *value);
    Value * getValue();
    void setAlarm(epicsAlarmCondition alarm);
    epicsAlarmCondition getAlarm();
    void setSeverity(epicsAlarmSeverity severity);
    epicsAlarmSeverity getSeverity();
    void setMask(unsigned int mask);
    unsigned int getMask();
    void setFlag(bool flag);
    bool getFlag();
    void setTimeStamp(epicsTimeStamp *time);
    epicsTimeStamp * getTimeStamp();
    friend std::ostream & operator << (std::ostream &out, const Data &data);
private:
    Value *value;
    bool flag;
    epicsAlarmCondition alarm;
    epicsAlarmSeverity severity;
    bool udf;
    unsigned int mask;
    epicsTimeStamp *time;
};


// Driver for the server tool
class Driver {
public:
    Driver();
    void installCallback(ReadCallback readCallback, WriteCallback writeCallback);
    void installReadCallback(ReadCallback readCallback);
    void installWriteCallback(WriteCallback writeCallback);
    bool hasReadCallback();
    bool hasWriteCallback();
    ReadCallback getReadCallback();
    WriteCallback getWriteCallback();
    Value * getParam(std::string name);
    void setParam(std::string name, Value *value);
    void setParamStatus(std::string name, epicsAlarmCondition alarm, epicsAlarmSeverity severity);
    Data * getParamDB(std::string name);
    Value * read(std::string name);
    bool write(std::string name, Value *value);
    void updatePVs();
    void updatePV(std::string name);
private:
    std::map<std::string, Data*> pvDB;
    ReadCallback readCallback;
    WriteCallback writeCallback;
};


// PV info for the server tool
class PVInfo {
public:
    PVInfo(pvDef *pv);
    std::string getName();
    double getHopr();
    double getLopr();
    std::string getUnits();
    double getHighWarning();
    double getLowWarning();
    double getHighAlarm();
    double getLowAlarm();
    int getPrecision();
    Value* getValue();
    double getScan();
    bool getSoft();
    std::vector<std::string> getEnums();
    void validateLimit();
    unsigned int checkValue(Value *newValue);
    void checkAlarm(Value *newValue, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity);
    void _checkNumericAlarm(double value, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity);
    void _checkEnumAlarm(int value, epicsAlarmCondition *alarm, epicsAlarmSeverity *severity);
    friend std::ostream & operator << (std::ostream &out, const PVInfo &pvinfo);
private:
    std::string name;
    double scan;
    std::vector<std::string> enums;
    std::vector<epicsAlarmSeverity> states;
    int prec;
    std::string unit;
    double hilim;
    double lolim;
    double high;
    double low;
    double hihi;
    double lolo;
    double mdel;
    double adel;
    bool soft;
    bool valid_low_high;
    bool valid_lolo_hihi;
    Value *value;
    Value *mlst;
    Value *alst;
};


// PV instance for the server tool
class SimplePV: public casPV
{
    public:
        SimplePV();
        SimplePV(char *name, PVInfo *info);
        virtual ~SimplePV();
        virtual caStatus interestRegister();
        virtual void interestDelete();
        virtual caStatus writeValue(const gdd &value);
        virtual caStatus read(const casCtx &ctx, gdd &prototype);
        virtual caStatus write (const casCtx &ctx, const gdd &value);
        static void initFT();
        virtual caStatus getValue(gdd &value);
        virtual caStatus getPrecision(gdd &prec);
        virtual caStatus getHighLimit(gdd &hilim);
        virtual caStatus getLowLimit(gdd &lolim);
        virtual caStatus getHighWarnLimit(gdd &hilim);
        virtual caStatus getLowWarnLimit(gdd &lolim);
        virtual caStatus getHighAlarmLimit(gdd &hilim);
        virtual caStatus getLowAlarmLimit(gdd &lolim);
        virtual caStatus getUnits(gdd &units);
        virtual caStatus getEnums(gdd &enums);
        virtual const char *getName() const;
        virtual aitEnum bestExternalType() const;
        virtual unsigned maxDimension() const;
        virtual aitIndex maxBound(unsigned dimension) const;
        virtual void destroy();
        PVInfo *getInfo();
        void updateValue(Data *data);
        void myPostEvent(int mask, gdd &value);
        Value * getValueFromGDD(const gdd *pGDD);
        void putValueToGDD(gdd *pGDD, Value *value);
        void putGDDToGDD(gdd *pGDD, gdd *value);
    private:
        static gddAppFuncTable<SimplePV> ft;
        static bool initialized;
        bool interest;
        std::string name;
        PVInfo *info;
};


// The server instance
class SimpleServer: public caServer {
public:
    SimpleServer();
    virtual ~SimpleServer();
    virtual pvExistReturn pvExistTest(const casCtx &ctx, const caNetAddr &clientAddress, const char *pPVAliasName);
    virtual pvAttachReturn pvAttach(const casCtx &ctx, const char *pPVAliasName);
    void addPV(const char *name, casPV *pv);
    void createSinglePV(pvDef *pvdef);
    void process(double delay);
    std::map<std::string, casPV*> getPVList();
private:
    std::map<std::string, casPV*> pvList;
};


#endif