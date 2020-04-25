
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

	bool checkNumberTypeEqual(ExprTreePtr lhs, ExprTreePtr rhs) {
	    return (lhs->getType() == "double" && rhs->getType() == "int") ||
                (lhs->getType() == "int" && rhs->getType() == "double");
    }

    bool checkNumberStringMatch(ExprTreePtr lhs, ExprTreePtr rhs) {
	    if(lhs->getType() == "string") {
	        if(rhs->getType() == "double" || rhs->getType() == "int"){
	            return true;
	        }
	    }
	    if(rhs->getType() == "string") {
            if(lhs->getType() == "double" || lhs->getType() == "int"){
                return true;
            }
	    }
	    return false;
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

	virtual ExprTreePtr getlhs() = 0;
	virtual ExprTreePtr getrhs() = 0;

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

	 ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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
		return "double";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_DoubleAttType>());
	}
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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
		return "int";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared<MyDB_IntAttType>());
	}
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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
		if (attType == "int")
			return "int";
		if (attType == "double")
		    return "double";
		if (attType == "string")
			return "string";
		return "(Unable to recognize this type)";
		
	}
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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

	    // if (!checkTypeEqual(lhs, rhs, "double")
	    // || !checkTypeEqual(lhs, rhs, "int")
	    // || !checkNumberTypeEqual(lhs, rhs)) {
	    //     errorMessage(lhs, rhs, "-");
	    //     return false;
	    // }
		return true;
	};
	
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		if (getType() == "int") {
			return make_pair("", make_shared<MyDB_IntAttType>());
		} else if (getType() == "double") {
			return make_pair("", make_shared<MyDB_DoubleAttType>());
		} else {
			return make_pair("", make_shared<MyDB_DoubleAttType>());
		}
	}

	string getType() {
        if (checkTypeEqual(lhs, rhs, "int")) {
            return "int";
        }

        if (checkTypeEqual(lhs, rhs, "double") || checkNumberTypeEqual(lhs, rhs)) {
            return "double";
        }

		return "(Unable to recognize this type)";
	}
	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
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

        if (!checkTypeEqual(lhs, rhs, "int")
        && !checkTypeEqual(lhs, rhs, "double")
        && !checkNumberTypeEqual(lhs, rhs)
        && !checkNumberStringMatch(lhs, rhs)
        && !checkTypeEqual(lhs, rhs, "string")) {
            errorMessage(lhs, rhs, "+");
            return false;
        }

		return true;
	};
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		string x = getType();
		if (x == "int") {
			return make_pair("", make_shared<MyDB_IntAttType>());
		} else if (x == "double") {
			return make_pair("", make_shared<MyDB_DoubleAttType>());
		} else {
			return make_pair("sp", make_shared<MyDB_StringAttType>());
		}
	}

	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }

	string getType() {
        if (checkTypeEqual(lhs, rhs, "int")) {
            return "int";
        }

        if (checkTypeEqual(lhs, rhs, "double") || checkNumberTypeEqual(lhs, rhs)) {
            return "double";
        }

        if (checkTypeEqual(lhs, rhs, "string")) {
            return "string";
        }

        if(checkNumberStringMatch(lhs, rhs)) {
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

        // if (!checkTypeEqual(lhs, rhs, "double")
        // || !checkTypeEqual(lhs, rhs, "int")
        // || !checkNumberTypeEqual(lhs, rhs)) {
        //     errorMessage(lhs, rhs, "*");
        //     return false;
        // }

        return true;
	};

	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		string x = getType();
		if (x == "int") {
			return make_pair("", make_shared<MyDB_IntAttType>());
		} else {
			return make_pair("", make_shared<MyDB_DoubleAttType>());
		}
	}

	string getType() {
        if (checkTypeEqual(lhs, rhs, "int")) {
            return "int";
        }

        if (checkTypeEqual(lhs, rhs, "double") || checkNumberTypeEqual(lhs, rhs)) {
            return "double";
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

	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }

	bool check() {
        if (!lhs->check() || !rhs->check()) {
            return false;
        }

        // if (!checkTypeEqual(lhs, rhs, "double")
        // && !checkTypeEqual(lhs, rhs, "int")
        // && !checkNumberTypeEqual(lhs, rhs)) {
        //     errorMessage(lhs, rhs, "/");
        //     return false;
        // }

        return true;
	};

	string getType() {
	    if (checkTypeEqual(lhs, rhs, "int")) {
            return "int";
        }

        if (checkTypeEqual(lhs, rhs, "double") || checkNumberTypeEqual(lhs, rhs)) {
            return "double";
        }

        return "(Unable to recognize this type)";
	}
	pair<string, MyDB_AttTypePtr> getAttSchema () {
		if (getType() == "int") {
			return make_pair("", make_shared<MyDB_IntAttType>());
		} else {
			return make_pair("", make_shared<MyDB_DoubleAttType>());
		}
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }
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

		if (!checkTypeEqual(lhs, rhs, "double")
		&& !checkTypeEqual(lhs, rhs, "int")
		&& !checkNumberTypeEqual(lhs, rhs)
		&& !checkTypeEqual(lhs, rhs, "string")) {
			errorMessage(lhs, rhs, ">");
			return false;
		}

		return true;
	};

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	string getType() {
		if (checkTypeEqual(lhs, rhs, "int")
		|| checkTypeEqual(lhs, rhs, "double")
		|| checkNumberTypeEqual(lhs, rhs)
		|| checkTypeEqual(lhs, rhs, "string")) {
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
	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}

        if (!checkTypeEqual(lhs, rhs, "double")
        && !checkTypeEqual(lhs, rhs, "int")
        && !checkNumberTypeEqual(lhs, rhs)
        && !checkTypeEqual(lhs, rhs, "string")) {
			errorMessage(lhs, rhs, "<");
			return false;
		}

		return true;
	};

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("", make_shared <MyDB_BoolAttType>());
	}

	string getType() {
        if (checkTypeEqual(lhs, rhs, "int")
        || checkTypeEqual(lhs, rhs, "double")
        || checkNumberTypeEqual(lhs, rhs)
        || checkTypeEqual(lhs, rhs, "string")) {
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
	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}
		if (lhs->getType() != rhs->getType()) {
		    if(checkNumberTypeEqual(lhs, rhs)) {
                return true;
            }
			errorMessage(lhs, rhs, "!=");
			return false;
		}

		return true;
	};

	string getType() {
	    if(checkNumberTypeEqual(lhs, rhs)) {
	        return "boolean";
	    }

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
	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
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
	ExprTreePtr getlhs() {
		 return lhs;
	 }
	 ExprTreePtr getrhs() {
		 return rhs;
	 }

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	bool check() {
		if (!lhs->check() || !rhs->check()) {
			return false;
		}
		if (lhs->getType() != rhs->getType()) {
		    if (checkNumberTypeEqual(lhs, rhs)) {
		        return true;
		    }
			errorMessage(lhs, rhs, "==");
			return false;
		}

		return true;
	};

	string getType() {
	    if (checkNumberTypeEqual(lhs, rhs)) {
	        return "boolean";
	    }

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
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
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

	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
	 }
	bool check() {
		if (!child->check()) {
			return false;
		}
		// if(!checkTypeEqual(child, "int") && !checkTypeEqual(child, "double") && !checkNumberTypeEqual(lhs, rhs)) {
        //     errorMessage(child, "SUM");
        //     return false;
		// }

		return true;
	};

	string getType() {
		if (checkTypeEqual(child, "int")) {
			return "int";
		}
        if (checkTypeEqual(child, "double")) {
            return "double";
        }

		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		string x = getType();
		if (x == "int") {
			return make_pair("sum", make_shared <MyDB_IntAttType>());
		} else {
			return make_pair("sum", make_shared <MyDB_DoubleAttType>());
		}
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
	ExprTreePtr getlhs() {
		 return nullptr;
	 }
	 ExprTreePtr getrhs() {
		 return nullptr;
	 }

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	bool check() {
		if (!child->check()) {
			return false;
		}

        if(!checkTypeEqual(child, "int") && !checkTypeEqual(child, "double")) {
            errorMessage(child, "SUM");
            return false;
        }

		return true;
	};

	string getType() {
		if (checkTypeEqual(child, "int")) {
			return "int";
		}
        if (checkTypeEqual(child, "double")) {
            return "double";
        }

		return "(Unable to recognize this type)";
	}

	pair<string, MyDB_AttTypePtr> getAttSchema () {
		return make_pair("avg", make_shared <MyDB_DoubleAttType>());
	}

	~AvgOp () {}
};

#endif
