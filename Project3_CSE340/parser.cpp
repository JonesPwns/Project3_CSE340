#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>
#include "parser.h"

using namespace std;

void syntax_error()
{
	cout << "Syntax Error\n";
	exit(1);
}

Token Parser::peek()
{
	Token t = lexer.GetToken();
	lexer.UngetToken(t);
	return t;
}

struct scope {

	// map to hold variable name and its type
	map<string, int> var__type_table;
	map<string, bool> var__reference_table; // used to set variable to true that have been reference after its declaration
	map<string, bool> var_is_defined;		// set to true when var is lhs of assignment statement
	map<string, int> var__declared_at;      // keep track of line numbers
	scope* parent = NULL;

};


// funtion forward declarations
void parse_program();
void parse_scope();
void parse_scope_list();
void parse_var_decl();
void parse_id_list();
int parse_type_name();
void parse_stmt_list();
void parse_stmt();
int parse_assign_stmt();
void parse_while_stmt();
int parse_expr();
void parse_condition();
void syntax_error();
void insert(string, int);
int lookup_type(string, scope * scope_ptr);
int type_check(int op, int type1, int type2, int line_number);
void add_reference(string, int);
void print_semantic_error();



scope * current_scope = new scope();
vector<string> idlist;

string semantic_error = "";
bool semantic_error_flag = false;

string uninitialized_error = "";
bool uninitialized_error_flag = false;

string unreferenced_error = "";
bool unreferenced_error_flag = false;

string declaration_error = "";
bool declaration_error_flag = false;

int line_number = 1;

LexicalAnalyzer lexer;
const string ERROR_MESSAGE = "Syntax Error";
string reference_string = "";


void print_declaration_error()
{
	//cout << "Printing varible info!\n";
	cout << declaration_error << endl;
}

void print_variable_information()
{
	//cout << "Printing varible info!\n";
	cout << reference_string << endl;
}

void print_semantic_error()
{
	cout << semantic_error << endl;
	exit(0);
}
void print_uninitialized_error()
{
	cout << uninitialized_error << endl;
	exit(0);
}

// Parsing
int main()
{
		
		parse_program();

		if (declaration_error_flag)
		{
			print_declaration_error();
		}
		else if (semantic_error_flag) 
		{
			print_semantic_error();
		}
		else if(uninitialized_error_flag)
		{
			print_uninitialized_error();
		}
		else if (unreferenced_error_flag)
		{
			cout << unreferenced_error;
		}
		else
		{
			print_variable_information();
		}
		
}



void set_semantic_error(string error_name, string id, string rule)
{
	semantic_error = error_name + " " + id + " " + rule + "\n";
	semantic_error_flag = true;
}




void set_declaration_error(string error_name, string id, string rule)
{
	declaration_error = error_name + " " + id + " " + rule + "\n";
	declaration_error_flag = true;
}
void set_uninitialized_error(string error_name, string id, string rule)
{
	uninitialized_error += error_name + " " + id + " " + rule + "\n";
	uninitialized_error_flag = true;
}

void set_unreferenced_error(string id)
{
	unreferenced_error += "ERROR CODE 1.3 " + id + "\n";
	unreferenced_error_flag = true;
}

Token expect(TokenType expected_type)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected_type)
		syntax_error();
	return t;
}

void parse_program()
{
	parse_scope();
}




void print_current_scope() 
{

	cout << "Printing varible made the current scope!\n";
	for (auto it = current_scope->var__type_table.cbegin(); it != current_scope->var__type_table.cend(); ++it)
	{
		std::cout << it->first << " " << it->second << "\n";
	}

}

void parse_scope() 
{
	Token t, t2;
	t = lexer.GetToken();
	if (t.token_type == LBRACE)
	{
		

		scope * new_scope = new scope(); // allocate new scope structure
		new_scope->parent = current_scope;
		current_scope = new_scope;

		// syntax check for empty scope
		t2 = lexer.GetToken();
		if (t2.token_type == RBRACE)
		{
			syntax_error();
		}
		else 
		{
			lexer.UngetToken(t2);
		}
		//line_number++;
		parse_scope_list();
	}
	else
	{
		syntax_error();
	}
	//cout << "stop here" << endl;
	t = lexer.GetToken();
	if (t.token_type == RBRACE)
	{
		//cout << "parsed scope  { scopelist }" << endl;
	}
	else
	{

		syntax_error();
	}

	//print_current_scope();


	// need to check if there were 
	// iterate through var_declared and make sure all of the declared var from this scope are in var_reference
	for (auto it = current_scope->var__declared_at.cbegin(); it != current_scope->var__declared_at.cend(); ++it)
	{
		if (! current_scope->var__reference_table.count(it->first) && !current_scope->var_is_defined.count(it->first))
		{
			set_unreferenced_error(it->first);
			//cout << it->first << " NOT USED, ARRRRMYGARGGGGG!!" << endl;
		}
		//std::cout << it->first << " " << it->second << "\n";
	}

	scope* temp = current_scope;
	current_scope = current_scope->parent;
	delete(temp);


}

void parse_scope_list()
{
	Token t1, t2;
	t1 = lexer.GetToken();
	//line_number++;
	//cout << "line: " << line_number << endl;

	//cout << t1.lexeme << " at line number: " << t1.line_no << endl;

	if (t1.token_type == LBRACE) // scope
	{

		lexer.UngetToken(t1);
		parse_scope();
		parse_scope_list();
	}
	else if (t1.token_type == WHILE) // stmt
	{
		lexer.UngetToken(t1);
		parse_stmt();
		parse_scope_list();		// check for stmt scope list
	}
	else if (t1.token_type == ID)
	{
		t2 = lexer.GetToken();
		if (t2.token_type == EQUAL)
		{
			// t1 is the left side of an assignment
			lexer.UngetToken(t2);
			lexer.UngetToken(t1);
			parse_stmt();

			parse_scope_list();		// check for stmt scope list
		}
		else
		{
			lexer.UngetToken(t2);
			lexer.UngetToken(t1);
			parse_var_decl();
			parse_scope_list();
		}
	}
	else
	{
		lexer.UngetToken(t1);
	}

}


int lookup_type(string id, scope * scope_ptr) 
{
	if (scope_ptr->var__type_table.count(id))
	{
		// key exists
		return scope_ptr->var__type_table.find(id)->second;
	}
	else
	{
		// insert id and type in local list of identifiers, ie cur_scope->var_ref_table
		//cout << "Error, variable " << id << " not declared" << endl;

		// need to check parent scope before determining if its an error on not
		// if no parent scope, error
		if (scope_ptr->parent == NULL)
		{
			//semantic_error = "ERROR CODE 1.2 " + id + "\n";
			set_declaration_error("ERROR CODE", "1.2", id);
			//semantic_error_flag = true;
			//print_semantic_error();
			
		}
		else 
		{
			return lookup_type(id, scope_ptr->parent);
		}
		
	}
}

// confirms that the correct types are used in the expr that was passed in
// returns the type of the resulting expression or detects sematic error
int type_check(int op, int type1, int type2, int line_number) 
{
	int type = -1;
	//cout << op << " " << type1 << " " << type2 << endl;
	
	// arithmetic
	if (op == PLUS || op == MINUS || op == MULT || op == DIV)
	{
		// C3 types of arith operands should be real or int
		if (type1 == REAL || type1 == INT && type2 == REAL || type2 == INT)
		{


			// I1 if either are real, the resulting type is real
			if (type1 == REAL || type2 == REAL)
			{
				type = REAL;			
			}
			// I2 for plus, minus, mult, if both are ints, the resulting is an int
			else if (op == PLUS || op == MINUS || op == MULT)
			{
				if (type1 == INT && type2 == INT)
				{
					type = INT;
				}
				else 
				{
					if (!semantic_error_flag)
						set_semantic_error("TYPE MISMATCH", to_string(line_number), "C3");
				}
			}
			// I3 if both are ints, results in real 
			else if (op == DIV)
			{
				if (type1 == INT && type2 == INT)
				{
					type = REAL;
				}
				else
				{
					if (!semantic_error_flag)
						set_semantic_error("TYPE MISMATCH", to_string(line_number), "C3");
				}
			}
			else
			{
				if (! semantic_error_flag)
				set_semantic_error("TYPE MISMATCH", to_string(line_number), "C3");
			}



		}
		else 
		{
			if (!semantic_error_flag)
			set_semantic_error("TYPE MISMATCH", to_string(line_number), "C3");
		}

	}
	// binary boolean 
	else if (op == AND || op == OR || op == XOR)
	{
		// C4 both types should be boolean
		if (type1 == BOOLEAN || type2 == BOOLEAN)
		{
			type = BOOLEAN;
		}
		else
		{
			if (!semantic_error_flag)
			set_semantic_error("TYPE MISMATCH", to_string(line_number), "C4");
		}


	}
	// relational operator
	else if (op == GREATER || op == GTEQ || op == LESS || op == NOTEQUAL || op == LTEQ)
	{
		// C5 if neither is int or real
		if (type1 != REAL && type1 != INT && type2 != REAL && type2 != INT)
		{
			// C5 if they are the type, the result is the same type, covers string==string and bool==bool
			if (type1 == type2)
			{
				type = BOOLEAN;
			}
			else
			{
				if (!semantic_error_flag)
				set_semantic_error("TYPE MISMATCH", to_string(line_number), "C5");
			}
		}
		// C6 if one type has int or real, the other type should be int or real
		else if ((type1 == REAL || type1 == INT) && (type2 == REAL || type2 == INT))
		{
			// if C5 (handled above) or C6 is valid, result is a bool
			type = BOOLEAN;
		}
		else
		{
			if (!semantic_error_flag)
			set_semantic_error("TYPE MISMATCH", to_string(line_number), "C6");
		}
	}
	// C8 not operator, the type of operand should be boolean
	else if (op == NOT)
	{
		// 
		if (type1 == BOOLEAN) 
		{
			type = BOOLEAN;
		}
		else
		{
			if (!semantic_error_flag)
			set_semantic_error("TYPE MISMATCH", to_string(line_number), "C8");
		}
	}

	
	return type;
}


void insert(string id, int type) 
{

	current_scope->var__type_table.insert({ id, type });
	// need to add line number, need global counter while increments at every line

}


void parse_var_decl()
{
	Token t;
	int type = 0;

	// idlist = parse_id_list()
	// typeName = parse_type_name()
	



	parse_id_list();  // list of var names added to idlist vector

	t = lexer.GetToken();
	if (t.token_type == COLON)
	{
		type = parse_type_name();
	}
	else
	{
		syntax_error();
	}
	t = lexer.GetToken();
	if (t.token_type == SEMICOLON) 
	{
		//cout << "parsed var decl" << endl;
		// for each identifier in idlist do
		for (auto id : idlist)
		{

			// checks if var has already been declared locally
			if (current_scope->var__type_table.count(id)) 
			{
				// key exists
				semantic_error = "ERROR CODE 1.1 " + id + "\n";
				semantic_error_flag = true;
				print_semantic_error();
				
				//cout << "ERROR, variable " << id << " already declared." << endl;

			}
			else 
			{
				// insert id and type in local list of identifiers, ie cur_scope->var_ref_table
				insert(id, type);
				current_scope->var__declared_at.insert({ id, t.line_no }); // save line number where variable(s) were found
			}


			// access by value, the type of i is int
			//std::cout << i.first << ' ';
		}
		idlist.clear();
			// if identifier already declared in current scope then


	}
	else
	{
		syntax_error();
	}


}

void parse_id_list()
{

	Token t1, t2;

	t1 = lexer.GetToken();
	if (t1.token_type == ID)
	{

		idlist.push_back(t1.lexeme);

		//lexer.UngetToken(t1);
		// parse_id(); 
		t2 = lexer.GetToken();
		if (t2.token_type == COMMA)
		{
			parse_id_list();
		}
		else 
		{


			lexer.UngetToken(t2);
		}

	}
	else
	{
		syntax_error();
	}
}


int parse_type_name()
{
	Token t = lexer.GetToken();
	if (t.token_type == REAL)
	{
		return REAL;
		//cout << "parsed REAL" << endl;
	}
	else if (t.token_type == INT) 
	{
		return INT;
		//cout << "parsed INT" << endl;
	}
	else if (t.token_type == BOOLEAN)
	{
		return BOOLEAN;
		//cout << "parsed BOOLEAN" << endl;
	}
	else if (t.token_type == STRING)
	{
		return STRING;
		//cout << "parsed STRING" << endl;
	}
	else
	{
		syntax_error();
		return 0;
	}
}


void parse_stmt_list()
{
	Token t1, t2;
	parse_stmt();
	t1 = lexer.GetToken();
	t2 = lexer.GetToken();
	if (t1.token_type == WHILE || t1.token_type == ID && t2.token_type == EQUAL)
	{
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
		parse_stmt_list();
	}
	else 
	{
		lexer.UngetToken(t2);
		lexer.UngetToken(t1);
	}
	//cout << "parsed stmt list" << endl;
}

void compare(int lhs_type, int rhs_type, int line_number) 
{
	if (semantic_error_flag || declaration_error_flag) return;
	bool type_mismatch = false;
	string error_code = "";

	// C1 if the lhs type is INT, BOOLEAN, or STRING, rhs should be the same
	if (lhs_type == INT || lhs_type == BOOLEAN || lhs_type == STRING) 
	{
			
		if (lhs_type == rhs_type) 
		{
			type_mismatch = false;
		}
		else
		{
			type_mismatch = true;
			error_code = "C1";
			//cout << "C1 error" << endl;
		}


	}
	else if (lhs_type == REAL)
	{
		if (rhs_type == INT || rhs_type == REAL) 
		{
			type_mismatch = false;
		}
		else
		{
			type_mismatch = true;
			error_code = "C2";
			//cout << "C2 error" << endl;
		}
	}
	else { } //cout << "umm" << endl;
	if (type_mismatch && !semantic_error_flag)
	{
		semantic_error =  "TYPE MISMATCH " + to_string(line_number) + " " +  error_code + "\n";
		semantic_error_flag = true;
	}
}

void set_variable_to_defined(string var_name) 
{
	scope* temp = current_scope;
	while (temp != NULL)
	{
		if (temp->var__declared_at.count(var_name))
		{
			temp->var_is_defined.insert({ var_name, true });
			break;
		}
		else
		{
			temp = temp->parent;
		}
	}
}

void parse_stmt()
{
	int lhs_type, rhs_type;
	string var_name;
	int line_number;
	int declared_at;

	Token t1, t2;
	t1 = lexer.GetToken(); 
	if (t1.token_type == ID) 
	{
		add_reference(t1.lexeme, t1.line_no);
		t2 = lexer.GetToken();
		if (t2.token_type == EQUAL)
		{
			//current_scope->var_is_defined.insert({ t1.lexeme, true }); // set variable to be defined
			lhs_type = lookup_type(t1.lexeme, current_scope); // gets the left hand side 's type
			lexer.UngetToken(t2);
			lexer.UngetToken(t1);
			rhs_type = parse_assign_stmt(); // set the right hand side 's 'should be' type
			compare(lhs_type, rhs_type, t1.line_no);
			set_variable_to_defined(t1.lexeme);
		}
		else
		{
			syntax_error();
		}
		
		//var_name = t1.lexeme;// +" " + t1.line_no + " " + current_scope->var__declared_at.find(t1.lexeme)->second << endl;
		//line_number = int(t1.line_no);
		//declared_at = current_scope->var__declared_at.find(t1.lexeme)->second;
		//reference_string = var_name + " " + to_string(line_number) + " " + to_string(declared_at) + "\n";
		
		//cout << reference_string << endl;
	}
	else if (t1.token_type == WHILE)
	{
		lexer.UngetToken(t1);
		parse_while_stmt();
	}
	else
	{
		syntax_error();
	}
}


int parse_assign_stmt() 
{
	int type = 0;
	Token t1, t2, t3;
	t1 = lexer.GetToken();
	if (t1.token_type == ID)
	{
		// parse_id()
		t2 = lexer.GetToken();
		if (t2.token_type == EQUAL)
		{
			type = parse_expr();
			t3 = lexer.GetToken();
			if (t3.token_type == SEMICOLON)
			{
				//cout << "parse assign stmt" << endl;
				return type;
			}
			else
			{
				syntax_error();
			}
		}
		else
		{
			syntax_error();
		}
	}
	else
	{
		syntax_error();
	}
}


void parse_while_stmt()
{
	Token t1, t2, t3;
	t1 = lexer.GetToken();
	if (t1.token_type == WHILE)
	{
		parse_condition();

		t2 = lexer.GetToken();

		if (t2.token_type == LBRACE)
		{
			parse_stmt_list();

			t3 = lexer.GetToken();

			if (t3.token_type == RBRACE)
			{
				//cout << "parsed while stmt" << endl;
			}
			else
			{
				syntax_error();
			}
		}
		else
		{
			parse_stmt();
		}
	}
	else
	{
		syntax_error();
	}
}

// returns true if the variable is defined in current scope, or any parent scopes
bool is_defined(string var_name)
{
	bool flag = false;

	scope* temp = current_scope;

	while (temp != NULL)
	{
		if (temp->var_is_defined.count(var_name))
		{
			flag = true;
			break;
		}
		else
		{
			temp = temp->parent;
		}
	}
	return flag;
}

int parse_primary(Token t1) 
{
	int type = 0;
	bool flag = false;
		
	switch (t1.token_type)
	{

	case ID: 
		flag = is_defined(t1.lexeme);
		if (!flag)
		{
			set_uninitialized_error("UNINITIALIZED", t1.lexeme, to_string(t1.line_no));
		}
		//else 
		//{
		type = lookup_type(t1.lexeme, current_scope);
		current_scope->var__reference_table.insert({ t1.lexeme, true }); // sets ver to be referenced for this scope, handles rhs references
		//}
		break;
	case INT: type = INT;
		break;	
	// I6 the type of NUM constants is int
	case NUM: type = INT;
		break;
	// I7 the type of REALNUM constants is real
	case REALNUM: type = REAL;
		break;
	// I9 the type of string constant is string
	case STRING_CONSTANT: type = STRING;
		break;
	// I8 the type of boolean constant is boolean
	case TRUE: type = BOOLEAN;
		break;
	case FALSE: type = BOOLEAN;
		break;
	}

	return type;

}

void add_reference(string var_name, int line_number) 
{
	int declared_at = -1;

	scope* temp = current_scope;

	while (temp != NULL)
	{
		if (temp->var__declared_at.count(var_name))
		{
			declared_at = temp->var__declared_at.find(var_name)->second;
			break;
		}
		else
		{
			temp = temp->parent;
		}
	}
	current_scope->var__reference_table.insert({var_name, true});
	reference_string += var_name + " " + to_string(line_number) + " " + to_string(declared_at) + "\n";
}


int parse_expr()
{
	Token t1;
	t1 = lexer.GetToken();

	int type = 0, type1, type2;


	// arithmetic operator //OPERATOR EXPR EXPR
	if (t1.token_type == PLUS || t1.token_type == MINUS || t1.token_type == MULT || t1.token_type == DIV)
	{
			
		type1 = parse_expr();
		type2 = parse_expr();
		type = type_check(t1.token_type, type1, type2, t1.line_no);
		

		//cout << "parsed arithmetic operator expr expr" << endl;
	}
	// binary boolean operator
	else if (t1.token_type == AND || t1.token_type == OR || t1.token_type == XOR)
	{
		type1 = parse_expr();
		type2 = parse_expr();
		type = type_check(t1.token_type, type1, type2, t1.line_no);

		//cout << "parsed binary boolean operator expr expr" << endl;
	}
	// relational operator expr expr
	else if (t1.token_type == GREATER || t1.token_type == GTEQ || t1.token_type == LESS || t1.token_type == NOTEQUAL || t1.token_type == LTEQ)
	{
		type1 = parse_expr();
		type2 = parse_expr();
		type = type_check(t1.token_type, type1, type2, t1.line_no);

		//cout << "parsed relational operator expr expr" << endl;
	}
	// NOT operator
	else if (t1.token_type == NOT)
	{
		type1 = parse_expr();
		type2 = -1;
		type = type_check(t1.token_type, type1, type2, t1.line_no);

		//cout << "parsed NOT expr" << endl;
	}
	// primary
	else if (t1.token_type == ID || t1.token_type == NUM || t1.token_type == REALNUM || t1.token_type == STRING_CONSTANT || t1.token_type == TRUE || t1.token_type == FALSE)
	{
		
		type = parse_primary(t1);
		
		if (t1.token_type == ID)
		{
			add_reference(t1.lexeme, t1.line_no);
		}
		//cout << "parsed primary" << endl;
	}
	else
	{
		syntax_error();
	}
	return type;
}

void parse_condition()
{
	int type = -1;
	Token t1, t2;
	t1 = lexer.GetToken();
	if (t1.token_type == LPAREN)
	{
		// the type of a condition should be a boolean
		type = parse_expr();
		t2 = lexer.GetToken();
		if (t2.token_type == RPAREN)
		{
			if (type != BOOLEAN) 
			{
				//if (!semantic_error_flag)
				set_semantic_error("TYPE MISMATCH", to_string(t1.line_no), "C7");
			}
			
			//cout << "parsed condition ( expr )" << endl;
		}
		else
		{
			syntax_error();
		}
	}
	else
	{
		syntax_error();
	}

}
