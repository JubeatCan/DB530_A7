#include "RunOp.h"

RunOp::RunOp(SQLStatement *query, MyDB_BufferManagerPtr buffer,
        map<string, MyDB_TableReaderWriterPtr> tables, MyDB_CatalogPtr catalog)
{
    this->query = query->getSFWQuery();
    this->buffer = buffer;
    this->tables = tables;
    this->schemaOut = make_shared<MyDB_Schema> ();
    // this->schemaSp = make_shared<MyDB_Schema> ();
    this->cata = catalog;
    // this->sp = false;
    // vector<string> projectSp;
    for (auto s : this->query.valuesToSelect) {
        
        // if (s->getAttSchema().first != "sp") {
            schemaOut->appendAtt(s->getAttSchema());
            projection.push_back(s->toString());
        // }
        // schemaSp->appendAtt(s->getAttSchema());
        // cout << s->toString() << endl;

        if (s->getAttSchema().first == "sum") {
            aggsToCompute.push_back(make_pair(MyDB_AggType::sumA, s->toString().substr(3)));
            // projectSp.push_back("sum");
        } else if (s->getAttSchema().first == "avg") {
            aggsToCompute.push_back(make_pair(MyDB_AggType::avgA, s->toString().substr(3)));
            // projectSp.push_back("avg");
        } else {
            groupings.push_back(s->toString());
            // projectSp.push_back(s->toString());
        }

        // if (s->getAttSchema().first == "sp") {
        //     // this->sp = true;
            
        //     string x, n;
        //     cout << s->toString() << endl;
        //     cout << s->getlhs()->getType() << endl;
        //     cout << s->getrhs()->getType() << endl;
        //     if (s->getlhs()->getType() == "string") {
        //         x = s->getrhs()->getType();
        //         n = s->getrhs()->toString();
        //     } else {
        //         x = s->getlhs()->getType();
        //         n = s->getlhs()->toString();
        //     }
            
        //     if (x == "int") {
        //         schemaOut->appendAtt(make_pair(n,make_shared<MyDB_IntAttType>()));
        //     } else if (x == "double") {
        //         schemaOut->appendAtt(make_pair(n,make_shared<MyDB_DoubleAttType>()));
        //     } else {
        //         schemaOut->appendAtt(make_pair(n,make_shared<MyDB_StringAttType>()));
        //     }

        //     groupings.pop_back();
        //     groupings.push_back(n);
        // }

    }
    if (aggsToCompute.empty()) {
        this->isAgg = false;
    } else {
        this->isAgg = true;
    }

    // if (sp) {
    //     projection = projectSp;
    // }

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
        cout << "Join and process" << endl;
    } else {

        input = tables[tablesToProcess[0].first];
        finalInput = copyyyy(input, tablesToProcess[0].second, tablesToProcess[0].first);
        // vector<pair<string, MyDB_AttTypePtr>> sch(input->getTable()->getSchema()->getAtts());
        // auto& tempSch = input->getTable()->getSchema()->getAtts();
        // tempSch.clear();
        // for (auto s : sch) {
        // tempSch.emplace_back(make_pair(tablesToProcess[0].second + "_" + s.first, s.second));
        // }
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
    // if (!sp) {
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
    // } else {
    //     MyDB_TablePtr outputTableSp = make_shared<MyDB_Table>("outputSp", "outputSp.bin", schemaSp);
    //     MyDB_TableReaderWriterPtr outputSp = make_shared<MyDB_TableReaderWriter>(outputTableSp, buffer);
    //     RegularSelection opSp(output, outputSp, "", projection);

    //     int count = 0;
    //     MyDB_RecordPtr recOut = outputSp->getEmptyRecord();
    //     MyDB_RecordIteratorAltPtr it = outputSp->getIteratorAlt();

    //     printf("----------------Results Below---------------\n");
    //     while (it->advance()) {
    //         it->getCurrent(recOut);
    //         ++count;
    //         if (count <= 30) {
    //             cout << recOut << endl;
    //         }
    //     }
    //     printf("Count Result: %d records.\n", count);
    // }
    

}