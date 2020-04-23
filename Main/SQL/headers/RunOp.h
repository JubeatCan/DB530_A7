#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"

class RunOp
{
private:
    /* data */
    SFWQuery query;
    map<string, MyDB_TableReaderWriterPtr> tables;
    MyDB_BufferManagerPtr buffer;
    vector<string> groupings;
    vector<pair<MyDB_AggType, string>> aggsToCompute;

public:
    RunOp(/* args */SQLStatement *query);
    ~RunOp();
};
