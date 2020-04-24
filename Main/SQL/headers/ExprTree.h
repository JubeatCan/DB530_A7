
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <string>
#include <vector>
#include <set>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {
public:
	static MyDB_CatalogPtr catalogPtr;
	static vector<pair<string, string>> tables;
	static vector<ExprTreePtr> groups;

public:

	virtual string toString () = 0;
	virtual ~ExprTree () {}
	// For checking.
	virtual bool check() = 0;
	// To help with type check
	// Lookup Table: "number" "boolean" "string" "(Unable to recognize this type)"
	virtual string getType() = 0;
	virtual pair<string, MyDB_AttTypePtr> getAttSchema () = 0;

	bool checkTypeEqual(ExprTreePtr lhs, ExprTreePtr rhs, string type) {
		return (lhs->getType() == type && rhs->getType() == type) ? true : false;
	}

	bool checkTypeEqual(ExprTreePtr opn, string type) {
		return (opn->getType() == type) ? true : false;
	}

	void errorMessage(ExprTreePtr lhs, ExprTreePtr rhs, string opr) {
		cerr << "This operator \'" << opr << "\' cannot be used between "
			<< lhs->toString() << " and " << rhs->toString() << "." << endl;
	}

	void errorMessage(ExprTreePtr opn, string opr) {
		cerr << "This operator \'" << opr << "\' cannot be used on "
			<< opn->toString() << "." << endl;
	}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}	

	bool check() {
		return true;
	};

	string getType() {
		return "boolean";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_BoolAttType>());
	}
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	bool check() {
		return true;
	};

	string getType() {
		return "number";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_DoubleAttType>());
	}

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	bool check() {
		return true;
	};

	string getType() {
		return "number";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_IntAttType>());
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	bool check() {
		return true;
	};

	string getType() {
		return "string";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_StringAttType>());
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	string attType = "";
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}	

	bool check() {
	    // check table name
	    bool tableExist = false;
	    string tableFullName;
	    for (auto table : tables) {
	        if (tableName == table.second) {
                tableFullName = table.first;
                tableExist = true;
	        }
	    }

	    if (!tableExist) {
	        cout << "The table [" << tableName << "] doesn't exist, please check" << endl;
	        return false;
	    }

	    // check attribute
	    bool attExist = false;
	    vector<string> attributes;
	    catalogPtr->getStringList(tableFullName + ".attList", attributes);
	    set<string> attSet(attributes.begin(), attributes.end());
	    if (attSet.find(attName) != attSet.end()) {
	        attExist = true;
	    }

	    if (!attExist) {
            cout << "The attribute [" << attName << "] doesn't exist, please check" << endl;
            return false;
	    }

	    // get attribute type
        catalogPtr->getString(tableFullName + "." + attName + ".type", attType);

		return true;
	};

	string getType() {
	    // ?
		if (attType == "") {
			check();
		}
		if (attType == "bool")
			return "boolean";
		if (attType == "int" || attType == "double")
			return "number";
		if (attType == "string")
			return "string";
		return "(Unable to recognize this type)";
		
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		if (attType == "bool") {
			return make_pair("[" + attName + "]", make_shared<MyDB_BoolAttType>());
		} else if (attType == "string") {
			return make_pair("[" + attName + "]", make_shared<MyDB_StringAttType>());
		} else if (attType == "int") {
			return make_pair("[" + attName + "]", make_shared<MyDB_IntAttType>());
		} else if (attType == "double") {
			return make_pair("[" + attName + "]", make_shared<MyDB_DoubleAttType>());
		}
		return make_pair("[" + attName + "]", make_shared<MyDB_BoolAttType>());
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
	    if (!lhs->check() || !rhs->check()) {
	        return false;
	    }

	    if (!checkTypeEqual(lhs, rhs, "number")) {
	        errorMessage(lhs, rhs, "-");
	        return false;
	    }
		return true;
	};
	
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return lhs->getAttSchema();
	}

	string getType() {
	    if (checkTypeEqual(lhs, rhs, "number")) {
	        return "number";
	    }
		return "(Unable to recognize this type)";
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
        if (!lhs->check() || !rhs->check()) {
            return false;
        }

        if (!checkTypeEqual(lhs, rhs, "number") && !checkTypeEqual(lhs, rhs, "string")) {
            errorMessage(lhs, rhs, "+");
            return false;
        }

		return true;
	};
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_DoubleAttType>());
	}

	string getType() {
        if (checkTypeEqual(lhs, rhs, "number")) {
            return "number";
        }
        if (checkTypeEqual(lhs, rhs, "string")) {
            return "string";
        }
        return "(Unable to recognize this type)";
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
        if (!lhs->check() || !rhs->check()) {
            return false;
        }

        if (!checkTypeEqual(lhs, rhs, "number")) {
            errorMessage(lhs, rhs, "*");
            return false;
        }

        return true;
	};
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return lhs->getAttSchema();
	}

	string getType() {
        if (checkTypeEqual(lhs, rhs, "number")) {
            return "number";
        }
        return "(Unable to recognize this type)";
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
        if (!lhs->check() || !rhs->check()) {
            return false;
        }

        if (!checkTypeEqual(lhs, rhs, "number")) {
            errorMessage(lhs, rhs, "/");
            return false;
        }

        return true;
	};

	string getType() {
        if (checkTypeEqual(lhs, rhs, "number")) {
            return "number";
        }
        return "(Unable to recognize this type)";
	}
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return lhs->getAttSchema();
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}

		if (!checkTypeEqual(lhs, rhs, "number") && !checkTypeEqual(lhs, rhs, "string")) {
			errorMessage(lhs, rhs, ">");
			return false;
		}

		return true;
	};

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	string getType() {
		if (checkTypeEqual(lhs, rhs, "number") || checkTypeEqual(lhs, rhs, "string")) {
			return "boolean";
		}
		return "(Unable to recognize this type)";
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}

		if (!checkTypeEqual(lhs, rhs, "number") && !checkTypeEqual(lhs, rhs, "string")) {
			errorMessage(lhs, rhs, "<");
			return false;
		}

		return true;
	};

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	string getType() {
		if (checkTypeEqual(lhs, rhs, "number") || checkTypeEqual(lhs, rhs, "string")) {
			return "boolean";
		}
		return "(Unable to recognize this type)";
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}
		if (lhs->getType() != rhs->getType()) {
			errorMessage(lhs, rhs, "!=");
			return false;
		}

		return true;
	};

	string getType() {
		if (lhs->getType() == rhs->getType()) {
			return "boolean";
		}

		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}

		if (!checkTypeEqual(lhs, rhs, "boolean")) {
			errorMessage(lhs, rhs, "||");
			return false;
		}

		return true;
	};

	string getType() {
		if (checkTypeEqual(lhs, rhs, "boolean")) {
			return "boolean";
		}
		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}
		if (lhs->getType() != rhs->getType()) {
			errorMessage(lhs, rhs, "==");
			return false;
		}

		return true;
	};

	string getType() {
		if (lhs->getType() == rhs->getType()) {
			return "boolean";
		}

		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	bool check() {
		if (!child->check()) {
			return false;
		}

		if (!checkTypeEqual(child, "boolean")) {
			errorMessage(child, "!");
			return false;
		}

		return true;
	};

	string getType() {
		if (checkTypeEqual(child, "boolean")) {
			return "boolean";
		}
		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	bool check() {
		if (!child->check()) {
			return false;
		}
		if (!checkTypeEqual(child, "number")) {
			errorMessage(child, "SUM");
			return false;
		}

		return true;
	};

	string getType() {
		if (checkTypeEqual(child, "number")) {
			return "number";
		}
		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("sum", make_shared <MyDB_DoubleAttType>());
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	bool check() {
		if (!child->check()) {
			return false;
		}
		if (!checkTypeEqual(child, "number")) {
			errorMessage(child, "SUM");
			return false;
		}

		return true;
	};

	string getType() {
		if (checkTypeEqual(child, "number")) {
			return "number";
		}
		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("avg", make_shared <MyDB_DoubleAttType>());
	}

	~AvgOp () {}
};

#endif
