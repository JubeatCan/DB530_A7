#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include "RegularSelection.h"
#include "ScanJoin.h"

class RunOp
{
private:
    /* data */
    SFWQuery query;
    map<string, MyDB_TableReaderWriterPtr> tables;
    MyDB_BufferManagerPtr buffer;
    vector<string> groupings;
    vector<pair<MyDB_AggType, string>> aggsToCompute;
    vector<string> projection;
    MyDB_SchemaPtr schemaOut;
    MyDB_CatalogPtr cata;
    bool isAgg;

public:
    RunOp(SQLStatement *query, MyDB_BufferManagerPtr buffer,
        map<string, MyDB_TableReaderWriterPtr> tables, MyDB_CatalogPtr catalog);
    void run();
    MyDB_TableReaderWriterPtr copyyyy(MyDB_TableReaderWriterPtr input, string alias, string name);
};
