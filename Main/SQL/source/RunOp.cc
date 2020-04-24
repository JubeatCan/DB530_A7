#include "RunOp.h"

RunOp::RunOp(SQLStatement *query, MyDB_BufferManagerPtr buffer,
        map<string, MyDB_TableReaderWriterPtr> tables, MyDB_CatalogPtr catalog)
{
    this->query = query->getSFWQuery();
    this->buffer = buffer;
    this->tables = tables;
    this->schemaOut = make_shared<MyDB_Schema> ();
    this->cata = catalog;

    for (auto s : this->query.valuesToSelect) {
        schemaOut->appendAtt(s->getAttSchema());
        projection.push_back(s->toString());
        if (s->getAttSchema().first == "sum") {
            aggsToCompute.push_back(make_pair(MyDB_AggType::sumA, s->toString().substr(3)));
        } else if (s->getAttSchema().first == "avg") {
            aggsToCompute.push_back(make_pair(MyDB_AggType::avgA, s->toString().substr(3)));
        } else {
            groupings.push_back(s->toString());
        }
    }
    if (aggsToCompute.empty()) {
        this->isAgg = false;
    } else {
        this->isAgg = true;
    }

}

MyDB_TableReaderWriterPtr RunOp::copyyyy(MyDB_TableReaderWriterPtr input, string alias, string name) {
    MyDB_TablePtr temp = make_shared<MyDB_Table>();
    temp->fromCatalog(name, cata);
    vector<pair<string, MyDB_AttTypePtr>> sch(input->getTable()->getSchema()->getAtts());
    auto& tempSch = temp->getSchema()->getAtts();
    tempSch.clear();
    for (auto s : sch) {
        tempSch.emplace_back(make_pair(alias + "_" + s.first, s.second));
    }
    cout << temp->getTupleCount() << endl;
    return make_shared<MyDB_TableReaderWriter>(temp, buffer);
}

void RunOp::run() {
    vector<pair<string, string> >& tablesToProcess = query.tablesToProcess;
    map<string, MyDB_TableReaderWriterPtr> inputTemp;
    MyDB_TableReaderWriterPtr input;
    MyDB_TableReaderWriterPtr finalInput;
    MyDB_TablePtr outputTable = make_shared<MyDB_Table>("output", "output.bin", schemaOut);
    MyDB_TableReaderWriterPtr output = make_shared<MyDB_TableReaderWriter>(outputTable, buffer);
    if (tablesToProcess.size() != 1) {
        // TODO: join and result in finalInput.

    } else {

        input = tables[tablesToProcess[0].first];
        finalInput = copyyyy(input, tablesToProcess[0].second, tablesToProcess[0].first);
    }

    string predicates = "";
    int i = 0;
    for (auto p : query.allDisjunctions) {
        if (i == 0) {
            predicates += p->toString();
        } else {
            predicates = "&& (" + predicates + "," + p->toString() + ")";
        }
        ++i;
    }

    if (isAgg) {
        Aggregate op(finalInput, output, aggsToCompute, groupings, predicates);
        op.run();
    } else {
        RegularSelection op(finalInput, output, predicates, projection);
        op.run();
    }

    //Output
    int count = 0;
    MyDB_RecordPtr recOut = output->getEmptyRecord();
    MyDB_RecordIteratorAltPtr it = output->getIteratorAlt();

    printf("----------------Results Below---------------\n");
    while (it->advance()) {
        it->getCurrent(recOut);
        ++count;
        if (count <= 30) {
            cout << recOut << endl;
        }
    }
    printf("Count Result: %d records.\n", count);

}